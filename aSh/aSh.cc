#include "aSh.hh"
#include <cstring>
#include <cerrno>
#include <vector>
#include <sys/wait.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
#include <error.h>

#undef exit
#define exit __DO_NOT_USE // use _exit to avoid cleanup oprations.

using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::cerr;

static int e_ForkFailure = 1;
static int e_ExecvpFailure = 2;
static int e_PipeFailure = 4; 
static int e_AbnormalExit = 8;

/*
 * struct command
 * This struct describes a command-line.
 * list of command-lines forms a pipeline.
*/
struct command {
    std::vector<string> args;
    pid_t pid;
    pid_t pgid;

    bool should_pipe; 
    int read_end;
    int write_end;
    bool pipe_start; // is it the start of a pipe

    string redir_type = "NONE";
    string infile = "";
    string outfile = "";
    string errfile = "";
    
    // commands can only be seperated by a pipe
    command* next;

    command();
    ~command();

    string unparse();

    int run();
    pid_t make_child(pid_t pgid);
};


/*
 * struct pipeline
 * This struct describes a pipeline instance
 * List of pipelines forms a conditional
 * Pipelines seperated by && or ||
 * a | b | c -> One Pipeline.
 * a && b | c -> Two pipelines.
*/
struct pipeline {
    command *cmd;   // ptr to root of child-command list.
    pipeline* next;

    int last_status;
    bool next_is_and;
    bool piped;

    pipeline();
    ~pipeline();

    string unparse();

    void run();
};


/*
 * struct conditional
 * This struct describes a conditional.
 * list of conditionals forms a command-list.
 * Conditionals are seperated by & or ;
 * a; is a conditional
 * a || b && c | d --> {a||b && c} LL of pipes| d : 
*/ 
struct conditional {
    pipeline *pipe;
    conditional *next;
    bool is_background;

    conditional();
    ~conditional();

    string unparse();

    void run();
};

command::command() {
    pid = -1;
    pgid = -1;
    next = nullptr;

    read_end = 0;
    write_end = 1;
    should_pipe = false;

    redir_type = "NONE";
    infile = "";
    outfile = "";
    errfile = "";
}

command::~command(){
}

pipeline::pipeline() {
    next = nullptr;
    cmd = nullptr;
    next_is_and = true;
    last_status = 0;
    piped = false;
}
pipeline::~pipeline() {

}

conditional::conditional() {
    next = nullptr;
    pipe = nullptr;
    is_background = false;

}

conditional::~conditional() {
}
/**
 * The unparsing functions are meant to be called on 
 * conditional only.
*/
string command::unparse() {
    string s = "", separator = "";
    for (auto arg: this->args) {
        s += separator + arg;
        separator = " ";
    }
    if (this->next) {
        s += " | " + this->next->unparse(); // unparse a command
    }
    return s;
}

string pipeline::unparse() {
    string s = this->cmd->unparse(); // missing command-child in 2nd pipe.
    if (this->next) { // multiple pipelines in the present conditional
        s += (this->next_is_and ? " && " : " || ");
        s += (this->next->unparse()); // unparse a pipeline
    }
    return s;

}

string conditional::unparse() {
    string s = this->pipe->unparse();
    if (this->next) { // A single conditional with ; @ end === stripping ;
        s += (this->is_background ? " & " : " ; ");
        s += this->next->unparse(); // unparse a conditional.
    } else if (this->is_background) {
        s += " &";
    }
    return s;
}

/**
 * parse_line(s)
 * Parses a command-string s, and returns a pointer to
 * the newly constructed tree representation of the 
 * command-string. Handles all token-types defined in the ASh 
 * BNF grammar.
 * Have parse_line return a pointer to the first conditional.
*/

conditional *parse_line(const char* s) {    
    int type;
    string token;

    conditional *cond_head = new conditional; // return to main
    conditional *cond = cond_head; // current conditional

    cond->pipe = new pipeline; // always points to first pipe.
    pipeline *pipe_head = cond->pipe; // current pipe

    cond->pipe->cmd = new command;
    command *cmd_head = cond->pipe->cmd; // current cmd

    while ((s = parse_shell_token(s, &type, &token)) != nullptr) {
        if (type == TOKEN_NORMAL) {
            if (!cond->pipe) {
                cond->pipe = new pipeline;
            }
            if (!cond->pipe->cmd) {
                cond->pipe->cmd = new command;
            }
            cond->pipe->cmd->args.push_back(token);
        }
        else if (type == TOKEN_SEQUENCE && token != "") {
            // eg: a; a | b; a && b | c;
            // assume there's always a next token.
            cond->pipe->cmd->next = new command;
            cond->pipe->cmd = cond->pipe->cmd->next;
        }
        else if (type == TOKEN_BACKGROUND) {
            // eg: a & b, a | b &, a &&b | c & 
            // all will be conditionals at this point
            // next token will be part of a new conditional
            cond->is_background = true;
            cond->next = new conditional;
            cond = cond->next;  // update current cond
        }
        else if (type == TOKEN_AND) {
            // eg: a && b
            // pipeline-level only
            cond->pipe->next = new pipeline;
            cond->pipe->next_is_and = true;
            cond->pipe = cond->pipe->next;
        }
        else if (type == TOKEN_OR) {
            cond->pipe->next = new pipeline;
            cond->pipe->next_is_and = false;
            cond->pipe = cond->pipe->next;
        }
        else if (type == TOKEN_PIPE) {
            if (!cond->pipe->piped) {
                // piped: whether the child cmd is part of a pipe.
                // First pipe token encountered, thus
                // the cmd is the first in pipe.
                cond->pipe->cmd->pipe_start = true;
            }
            cond->pipe->piped = true;
            cond->pipe->cmd->should_pipe = true; // flag
            cond->pipe->cmd->next = new command;
            cond->pipe->cmd = cond->pipe->cmd->next;
        }
        else if (type == TOKEN_REDIRECT_OP) {
            cond->pipe->cmd->redir_type = token;
            std::string redir_type = token;
            s = parse_shell_token(s, &type, &token);
            if (redir_type == "<") {
                cond->pipe->cmd->infile = token;
            }
            if (redir_type == ">") {
                cond->pipe->cmd->outfile = token;
            }
            if (redir_type == "2>") {
                cond->pipe->cmd->errfile = token;
            }
        } 
    }

    // string parsed, return cond_head to main
    cond = cond_head;
    cond->pipe = pipe_head;
    cond->pipe->cmd = cmd_head;

    return cond;
}
/**                                  
 * ------------------------ *
 * EXECUTING AN ASh COMMAND *
 * ------------------------ *
 * runner classes are designed parallel to the
 * ASh BNF grammar constructs.
*/

pid_t command::make_child(pid_t pgid) {
    (void) pgid;
    // pipe if reqd.
    int pipefd[2];

    if (should_pipe) {
        int r = pipe(pipefd);
        if (r == -1) {
            error(e_PipeFailure, errno, "pipe() in make_child failed.\n");
            _exit(1);
        }
        write_end = pipefd[1];
        if (next) {
            next->read_end = pipefd[0];
            next->pgid = pgid;
        }
    }

    // cd if reqd
    if (args[0] == "cd") {
        int r = chdir(args[1].c_str());
        if (r != 0) {
            chdir("/");
        }
        return 0;
    }

    // do fork()
    pid = fork();

    if (pid == -1) {
        error(e_ForkFailure, errno, "Fork Failed in make_child.\n");
        _exit(1);
    } 
    else if (pid == 0) {
        if (pipe_start) {
            pgid = getpid();
            next->pgid = pgid;
        }
        setpgid(getpid(), pgid);

        dup2(read_end, STDIN_FILENO);
        dup2(write_end, STDOUT_FILENO);

        if (read_end != 0) {close(read_end);}
        if (write_end != 1) {close(write_end);}
        if (should_pipe) {close(pipefd[0]);}

        if (redir_type == "<") {
            int fid = open(infile.c_str(), O_RDONLY|O_CLOEXEC);
            if (fid == -1) {
                fprintf(stderr,"No such file or directory\n");
                _exit(1);
            }
            dup2(fid, STDIN_FILENO);
        }
        if (redir_type == ">") {
            int fid = open(outfile.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            if (fid == -1) {
                fprintf(stderr,"No such file or directory\n");
                _exit(1);
            }
            dup2(fid, STDOUT_FILENO);
        }
        if (redir_type == "2>") {
            int fid = open(errfile.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            if (fid == -1) {
                fprintf(stderr,"No such file or directory\n");
                _exit(1);
            }
            dup2(fid, STDERR_FILENO);
        }

        // https://stackoverflow.com/questions/35247570/execvp-not-working-when-converting-from-vectorstring-to-vectorchar-to-char/
        std::vector<char*> _argv;
        for (auto const& a : this->args) {
            //cout << a << endl; 
            _argv.emplace_back(const_cast<char*>(a.c_str()));  
        }
        _argv.push_back(nullptr);

        if (execvp(_argv[0], _argv.data()) == -1) {
            error(e_ExecvpFailure, errno, "execvp in make_child() failed.\n");
            _exit(1);
        }
        _exit(0);
    }
    else {
        if (read_end != 0) {close(read_end);}
        if (write_end != 1) {close(write_end);}
        return pid;
    }
}


/**
 * command::run()
 * This function is the closest to actual command-execution. A notion of 
 * Atomicity.
 * Creates a s child process running the command in `this`,
 * sets `this->pid` to pid of child.
 * 
 * If a child process cannot be created, this function calls
 * `_exit(EXIT_FAILURE)` (that is, `_exit(1)`) to exit the containing
 * shell or subshell. If this function returns to its caller,
 * `this->pid > 0` must always hold.
 * 
 * run() returns to caller only in the parent process. 
 * Child process performs the actual calling of the usr/bin executable
 * via execvp()
 * 
 * command::run() handles atomic commands, pipes and complex-redirections. 
*/
int command::run() {
    int wstatus;
    if (!args.empty()) {
        pid = make_child(0); // pgid = 0;
        if (!should_pipe && args[0] != "cd") {
            pid_t exited_pid = waitpid(pid, &wstatus, 0);
            assert(exited_pid == pid);

            if (!WIFEXITED(wstatus)) {
                error(e_AbnormalExit, errno, "Child exited abnormally [%x]\n", wstatus);
            } 
        }
    }
    if (next) {
        return next->run(); // returns exit status
    }

    assert((WIFEXITED(wstatus)));
    return WEXITSTATUS(wstatus);
}

void pipeline::run() {
    if (cmd && (next_is_and)) {
        int status = cmd->run();
        if (next) {
            next->last_status = status;
        }
    }
    else if (next) {
        next->last_status = last_status;
    }
    
    if (next) {
        next->run();
    }
}


/**
 * run_conditional(c)
 * This function drives the execution of parsed commands
 * of varying complexities.
*/
void conditional::run() {
    string testUnparsed = this->unparse();
    
    if (pipe) {
        if (is_background) {
            pid_t p = fork();
            switch (p) {
                case 0:
                    pipe->run();
                    _exit(0);
            }
        }
        else {
            pipe->run();
        }
    }
    if (next) {
        next->run();
    }

}


/**
 * The main() function implements a terminal that is 
 * the entry point the aSh shell process. The new terminal
 * is made the foreground process instead of the terminal 
 * that runs the shell via ./aSh
*/
int main(int argc, const char *argv[]) {
    FILE *command_file = stdin;
    bool quiet = false;

    // Check for `-q` option: be quiet (print no prompts)
    if (argc > 1 && strcmp(argv[1], "-q") == 0) {
        quiet = true;
        --argc, ++argv;
    }

    // Check for filename option: read commands from file
    if (argc > 1) {
        command_file = fopen(argv[1], "rb");
        if (!command_file) {
            perror(argv[1]);
            return 1;
        }
    }

    claim_foreground(0);
    set_signal_handler(SIGTTOU, SIG_IGN);

    char buf[BUFSIZ];
    int bufpos = 0;
    bool needprompt = true;

    /**
     * Run a Read-Evaluate Loop similar in function to 
     * Python REPL. The exit condition is reading a EOF 
     * i.e. ctrl+D from stdin.
    */
    while (!feof(command_file)) {
        if (needprompt && !quiet) {
            printf("aSh[%d]$ ", getpid());
            fflush(stdout);
            needprompt = false;
        }

        // Read the command-line string in stdin and 
        // perform syscall error-handling.
        if (fgets(&buf[bufpos], BUFSIZ - bufpos, command_file) == nullptr) {
            // If fail due to signal interrupt, ignore.
            if (ferror(command_file) && errno == EINTR) {
                clearerr(command_file);
                buf[bufpos] = 0;
            } else {
                if (ferror(command_file)) {
                    perror("aSh");
                }
            }
        }

        // command-line string is stored in buf with null terminator
        bufpos = strlen(buf);
        if (bufpos == BUFSIZ - 1 || (bufpos > 0 && buf[bufpos - 1] == '\n')) {
            // Parse the string to create a grammar-tree
            if (conditional *c = parse_line(buf)) {
                c->run();
                delete c;
            }
            // wait for next command-line
            bufpos = 0;
            needprompt = 1;

            // free tree after exec.
        }
        
        // Handle zombie processes and/or interrupt requests

    }

    return 0;
}
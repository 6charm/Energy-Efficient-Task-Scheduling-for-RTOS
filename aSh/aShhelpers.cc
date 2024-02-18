#include "aSh.hh"
#include <cctype>
#include <sstream>

// isshellspecial(ch)
//    Test if `ch` is a command that's special to the shell (that ends
//    a command word).

inline bool isshellspecial(int ch) {
    return ch == '<' || ch == '>' || ch == '&' || ch == '|' || ch == ';'
        || ch == '(' || ch == ')' || ch == '#';
}


/*
 * Tokeniser. Reads input string char-at-a-time and assigns a token.
*/
const char* parse_shell_token(const char* str, int* type, std::string* token) {
    std::ostringstream buildtoken;

    // skip spaces; return nullptr and token ";" at end of line
    while (str && isspace((unsigned char) *str)) {
        ++str;
    }
    if (!str || !*str || *str == '#') {
        *type = TOKEN_SEQUENCE;
        *token = std::string();
        return nullptr;
    }

    // check for a redirection or special token
    for (; isdigit((unsigned char) *str); ++str) {
        buildtoken << *str;
    }
    if (*str == '<' || *str == '>') {
        *type = TOKEN_REDIRECT_OP;
        buildtoken << *str;
        if (str[1] == '>') {
            buildtoken << str[1];
            ++str;
        } 
        else if (str[1] == '&' && isdigit((unsigned char) str[2])) {
            buildtoken << str[1];
            for (str += 2; isdigit((unsigned char) *str); ++str) {
                buildtoken << *str;
            }
        }
        ++str;
    /* https://stackoverflow.com/questions/3544965/usage-of-tellp-in-c */
    } 
    else if (buildtoken.tellp() == 0
               && (*str == '&' || *str == '|')
               && str[1] == *str) {
        *type = (*str == '&' ? TOKEN_AND : TOKEN_OR);
        buildtoken << *str << str[1];
        str += 2;
    } 
    else if (buildtoken.tellp() == 0
               && isshellspecial((unsigned char) *str)) {
        switch (*str) {
        case ';': *type = TOKEN_SEQUENCE;   break;
        case '&': *type = TOKEN_BACKGROUND; break;
        case '|': *type = TOKEN_PIPE;       break;
        default:  *type = TOKEN_OTHER;      break;
        }
        buildtoken << *str;
        ++str;
    } 
    else {
        // it's a normal token
        *type = TOKEN_NORMAL;
        int quoted = 0;
        // Read characters up to the end of the token.
        while ((*str && quoted)
               || (*str && !isspace((unsigned char) *str)
                   && !isshellspecial((unsigned char) *str))) {
            if ((*str == '\"' || *str == '\'') && !quoted) {
                quoted = *str;
            } 
            else if (*str == quoted) {
                quoted = 0;
            } 
            else if (*str == '\\' && str[1] != '\0' && quoted != '\'') {
                buildtoken << str[1];
                ++str;
            } 
            else {
                buildtoken << *str;
            }
            ++str;
        }
    }

    // store new token and return the location of the next token
    *token = buildtoken.str();
    return str;
}


/* Borrowed from Eddie Kohler's repo */
int claim_foreground(pid_t pgid) {

    // Initialize state first time we're called.
    static int ttyfd = -1;
    static int shell_owns_foreground = 0;
    static pid_t shell_pgid = -1;
    if (ttyfd < 0) {
        // We need a fd for the current terminal, so open /dev/tty.
        int fd = open("/dev/tty", O_RDWR);
        assert(fd >= 0);
        // Re-open to a large file descriptor (>=10) so that pipes and such
        // use the expected small file descriptors.
        ttyfd = fcntl(fd, F_DUPFD, 10);
        assert(ttyfd >= 0);
        close(fd);
        // The /dev/tty file descriptor should be closed in child processes.
        fcntl(ttyfd, F_SETFD, FD_CLOEXEC);
        // Only mess with /dev/tty's controlling process group if the shell
        // is in /dev/tty's controlling process group.
        shell_pgid = getpgrp();
        shell_owns_foreground = (shell_pgid == tcgetpgrp(ttyfd));
    }

    // Set the terminal's controlling process group to `p` (so processes in
    // group `p` can output to the screen, read from the keyboard, etc.).
    if (shell_owns_foreground && pgid) {
        return tcsetpgrp(ttyfd, pgid);
    } else if (shell_owns_foreground) {
        return tcsetpgrp(ttyfd, shell_pgid);
    } else {
        return 0;
    }
}
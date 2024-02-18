/**
 * File: aSh.h
 * -----------
 * Declares functions for tokenisation and parsing.
*/
#ifndef ASH_HH
#define ASH_HH
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <csignal>
#include <string>
#include <fcntl.h>
#include <unistd.h>

/**
 * Tokens according to the aSh grammar.
 */
#define TOKEN_NORMAL         0   // normal command word
#define TOKEN_REDIRECT_OP    1   // Redirection Operator (>, <, 2>)

// Command Terminating tokens
#define TOKEN_SEQUENCE       2   // ';' i.e. sequence operator
#define TOKEN_BACKGROUND     3   // '&' i.e. background operator
#define TOKEN_PIPE           4   // '|' i.e. pipe operator
#define TOKEN_AND            5   // '&&' i.e. conjunction operator
#define TOKEN_OR             6   // '||' i.e. disjunction operator

#define TOKEN_OTHER         -1


// parse_shell_token(str, type, token)
//    Parse the next token from the shell command `str`. Stores the type of
//    the token in `*type`; this is one of the TOKEN_ constants. Stores the
//    token itself in `*token`. Advances `str` to the next token and returns
//    that pointer.
//
//    At the end of the string, returns nullptr, sets `*type` to
//    TOKEN_SEQUENCE, and sets `*token` to en empty string.
const char* parse_shell_token(const char* str, int* type, std::string* token);

// claim_foreground(pgid)
//    Mark `pgid` as the current foreground process group.
int claim_foreground(pid_t pgid);

// set_signal_handler(signo, handler)
//    Install handler `handler` for signal `signo`. `handler` can be SIG_DFL
//    to install the default handler, or SIG_IGN to ignore the signal. Return
//    0 on success, -1 on failure. See `man 2 sigaction` or `man 3 signal`.
inline int set_signal_handler(int signo, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    return sigaction(signo, &sa, NULL);
}

#endif





#include <stdio.h>
#include <signal.h>
int main() {
    sigset_t set, oset, zero;
    sigemptyset(&set);
    sigemptyset(&oset);
    sigemptyset(&zero);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGHUP);
    sigprocmask(SIG_BLOCK, &set, &oset);
     printf("%signal mask set\n");
     kill(getpid(), SIGCHLD);
    sigsuspend(&zero);
     sigprocmask(SIG_SETMASK, NULL, &set);
    if (sigismember(&set, SIGHUP)) printf("SIGHUP \n");
     if (sigismember(&set, SIGCHLD)) printf("SIGCHLD \n");
    if (sigismember(&set, SIGINT)) printf("SIGINT \n");
    }

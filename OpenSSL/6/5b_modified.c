#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

void sigchld_handler(int sig) {
    printf("SIGCHLD signal received! Signal number: %d\n", sig);
}

int main() {
    sigset_t set, oset, zero;
    
    // Install signal handler for SIGCHLD
    signal(SIGCHLD, sigchld_handler);
    
    sigemptyset(&set);
    sigemptyset(&oset);
    sigemptyset(&zero);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGHUP);
    sigprocmask(SIG_BLOCK, &set, &oset);
    printf("Signal mask set\n");
    
    // Send SIGCHLD to self
    kill(getpid(), SIGCHLD);
    
    sigsuspend(&zero);
    
    // Check which signals are in the current mask
    sigprocmask(SIG_SETMASK, NULL, &set);
    if (sigismember(&set, SIGHUP)) printf("SIGHUP is in mask\n");
    if (sigismember(&set, SIGCHLD)) printf("SIGCHLD is in mask\n");
    if (sigismember(&set, SIGINT)) printf("SIGINT is in mask\n");
    
    printf("Program completed successfully\n");
    return 0;
}

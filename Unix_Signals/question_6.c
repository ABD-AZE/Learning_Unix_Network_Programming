#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

// Signal handler: just needed to wake up from pause()
void sig_handler(int signo) {
	if(signo== SIGUSR1) {
        printf("Child process: Received signal from parent\n");
    } else if(signo == SIGUSR2) {
        printf("Parent Process: Child is ready to receive signals\n");
    }
}

void WAIT_PARENT() {
	signal(SIGUSR1, sig_handler);
	// Notify parent that we're ready to receive signals
	kill(getppid(), SIGUSR2);
	// Wait for parent to send SIGUSR1
	pause();
}

void TELL_CHILD(pid_t pid) {
	kill(pid, SIGUSR1);
}

void WAIT_CHILD_READY() {
	signal(SIGUSR2, sig_handler);
	// Wait for child to signal it's ready
	pause();
}

int main() {
    pid_t pid = fork();
    if (pid < 0) { 
        perror("Fork failed");
        exit(1);
    }
	else if (pid == 0) {
        WAIT_PARENT();
        printf("Exiting the child process\n");
		exit(0);
	}
    else{
        WAIT_CHILD_READY();
        TELL_CHILD(pid);
        printf("Parent process: signal sent to child\n");
        wait(NULL); 
        printf("Child process has exited. Parent exiting now.\n");
        exit(0);
    }
}

// Without SIGUSR2 the parent would not know when the child is ready to receive signals i.e. after setting up its signal handler.
// After that the child waits for the parent to signal it and exits after receiving the signal.

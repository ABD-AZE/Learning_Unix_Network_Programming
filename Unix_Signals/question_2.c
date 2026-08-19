#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>

void sigchld_handler(int sig) {
	int status;
	pid_t pid;
	
	// Non-blocking wait to collect all terminated children
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		if (WIFEXITED(status)) {
			printf("Child with PID %d terminated with exit status %d\n", 
				   pid, WEXITSTATUS(status));
		}
	}
}

int main() {
	signal(SIGCHLD, sigchld_handler);
	
	printf("Parent process: PID = %d\n", getpid());
	
	for (int i = 1; i <= 5; i++) {
		pid_t pid = fork();
		
		if (pid < 0) {
			perror("Fork failed");
			exit(1);
		}
		else if (pid == 0) {
			// Child process
			printf("Child %d: PID = %d\n", i, getpid());
			exit(i); 
		}
	}
	
	// Wait for all children to terminate
	int status;
	while (wait(&status) > 0) {
		// Keep waiting until all children are done
	}
	
	printf("All children have terminated. Parent exiting.\n");
	return 0;
}

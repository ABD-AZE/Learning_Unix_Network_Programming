#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>


int child = 0;

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

void alarm_handler(int sig) {
	printf("Child process %d: Alarm expired\n", getpid());
	exit(1); // Using PID modulo 100 as exit status
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
			printf("Child %d: PID = %d, setting alarm for %d seconds\n", i, getpid(), i*10);
			signal(SIGALRM, alarm_handler);
            child = i;
			alarm(i*10); // Set alarm to expire after i*10 seconds
			
			// Wait for alarm to expire
			while(1) {
				sleep(1);
			}
		}
	}
	
	// Parent waits for all children to terminate
	while (wait(NULL) > 0) {
		// Keep waiting until all children are done
	}
	
	printf("All children have terminated. Parent exiting.\n");
	return 0;
}

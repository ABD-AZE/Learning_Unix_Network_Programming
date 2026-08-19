#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>
#include<sys/resource.h>

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
	
	struct rusage usage_start, usage_end, child_usage;
	
	// Get initial resource usage
	getrusage(RUSAGE_SELF, &usage_start);
	
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
	
	getrusage(RUSAGE_SELF, &usage_end);
	getrusage(RUSAGE_CHILDREN, &child_usage);
	
	double parent_user_time = (usage_end.ru_utime.tv_sec - usage_start.ru_utime.tv_sec) + 
							 (usage_end.ru_utime.tv_usec - usage_start.ru_utime.tv_usec) / 1000000.0;
	
	double parent_system_time = (usage_end.ru_stime.tv_sec - usage_start.ru_stime.tv_sec) + 
							   (usage_end.ru_stime.tv_usec - usage_start.ru_stime.tv_usec) / 1000000.0;
	
	double children_user_time = child_usage.ru_utime.tv_sec + 
							   (child_usage.ru_utime.tv_usec / 1000000.0);
	
	double children_system_time = child_usage.ru_stime.tv_sec + 
								 (child_usage.ru_stime.tv_usec / 1000000.0);
	
	printf("All children have terminated. Parent exiting.\n");
	printf("Parent process time usage:\n");
	printf("  User mode: %.6f seconds\n", parent_user_time);
	printf("  System mode: %.6f seconds\n", parent_system_time);
	printf("Children processes time usage:\n");
	printf("  User mode: %.6f seconds\n", children_user_time);
	printf("  System mode: %.6f seconds\n", children_system_time);
	
	return 0;
}

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

int main() {
	sigset_t block_mask, pending_mask;
	
	// Initialize the signal set
	sigemptyset(&block_mask);
	
	// Add SIGALRM and SIGINT to the set of blocked signals
	sigaddset(&block_mask, SIGALRM);
	sigaddset(&block_mask, SIGINT);
	
	// Block the signals
	if (sigprocmask(SIG_BLOCK, &block_mask, NULL) == -1) {
		perror("sigprocmask");
		return 1;
	}
	
	printf("SIGALRM and SIGINT signals are now blocked\n");
	printf("Setting alarm for 10 seconds\n");
	
	alarm(10);
	
	printf("Waiting for 5 minutes. Press CTRL-C twice during this time.\n");

	sleep(300);
	
	printf("\nWait completed. Checking pending signals:\n");
	
	// Get pending signals
	if (sigpending(&pending_mask) == -1) {
		perror("sigpending");
		return 1;
	}
	
	// Check for specific signals
	const struct {
		int signum;
		const char *name;
	} signals[] = {
		{ SIGALRM, "SIGALRM" },
		{ SIGINT, "SIGINT" },
		{ SIGHUP, "SIGHUP" },
		{ SIGTERM, "SIGTERM" },
		{ SIGUSR1, "SIGUSR1" },
		{ SIGUSR2, "SIGUSR2" },
		{ 0, NULL }
	};
	
	int pending_count = 0;
	
	for (int i = 0; signals[i].name != NULL; i++) {
		if (sigismember(&pending_mask, signals[i].signum)) {
			printf("Signal %s (%d) is pending\n", signals[i].name, signals[i].signum);
			pending_count++;
		}
	}
	
	printf("\nTotal pending signals: %d\n", pending_count);
	
	return 0;
}

// This program blocks SIGALRM and SIGINT signals, sets an alarm for 10 seconds,
// and then waits for 5 minutes. During this time, if the user presses
// or when the alarm expires (generating SIGALRM), these signals will be blocked and become pending.
// After the wait period, the program checks and displays all pending signals.
// Expected output: SIGALRM and SIGINT will be shown along with their signum number, if the alarm expired
// and the user pressed CTRL-C during the wait period.
#include<stdio.h>
#include<unistd.h>
#include<signal.h>

void sigint_handler(int sig) {
    printf("SIGINT received.\n");
    fflush(stdout);
}

int main() {
    // Install the SIGINT handler
    signal(SIGINT, sigint_handler);
    
    printf("Entering the infinite loop.\n");
    fflush(stdout);
    while(1){
        sleep(1);
        printf("Looping...\n");
    }
    return 0;
}

// As the program is running as intended on my machine this implies that my OS (ubuntu 22.04) supports reliable Unix signals.
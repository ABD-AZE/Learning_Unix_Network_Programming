#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#define SA struct sockaddr
#define SERV_PORT 9877
#define LISTENQ 1024

void file_transfer(int sockfd);
void sigchld_handler(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}


int main(int argc, char **argv){
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port = htons (SERV_PORT);
    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    signal(SIGCHLD, sigchld_handler); 
     
    printf("Server started with PID: %d\n", getpid());
    for ( ; ; ) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (SA *) &cliaddr, &clilen);
        if ( (childpid = fork()) == 0) {
            printf("Child process created with PID: %d (Parent: %d)\n", getpid(), getppid());
            close(listenfd); 
            file_transfer(connfd);
            exit(0);
        }
        printf("Parent process %d created child with PID: %d\n", getpid(), childpid);
        close(connfd); 
    }
}

void file_transfer(int sockfd){
    char filename[1024];
    char buffer[1024];
    ssize_t n;
    FILE *file;

    n = read(sockfd, filename, sizeof(filename) - 1);
    if (n <= 0) {
        perror("Error reading filename");
        close(sockfd);
        return;
    }
    
    filename[n] = '\0';
    
    char *newline = strchr(filename, '\n');
    if (newline) *newline = '\0';
    
    printf("Server: Requested file: %s\n", filename);
    
    file = fopen(filename, "rb");
    if (file == NULL) {
        const char *error_msg = "ERROR: File not found";
        write(sockfd, error_msg, strlen(error_msg));
        perror("Error opening file");
        close(sockfd);
        return;
    }
    while ((n = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (write(sockfd, buffer, n) != n) {
            perror("Error sending file data");
            break;
        }
    }
    
    printf("Server: File transfer completed\n");
    fclose(file);
    close(sockfd);
}
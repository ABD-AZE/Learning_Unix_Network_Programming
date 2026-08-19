#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<errno.h>
#define SA struct sockaddr
#define SERV_PORT 9877
#define LISTENQ 1024

void file_request(int sockfd, const char *filename) {
    char buffer[1024];
    ssize_t n;
    FILE *file;
    char output_filename[1024];
    
    snprintf(output_filename, sizeof(output_filename), "received_%s", filename);
    
    if (write(sockfd, filename, strlen(filename)) != strlen(filename)) {
        perror("Error sending filename");
        exit(1);
    }
    
    printf("Client: Requesting file: %s\n", filename);
    
    file = fopen(output_filename, "wb");
    if (file == NULL) {
        perror("Error creating output file");
        exit(1);
    }
    
    while ((n = read(sockfd, buffer, sizeof(buffer))) > 0) {
        if (strncmp(buffer, "ERROR:", 6) == 0) {
            buffer[n] = '\0';
            printf("Server error: %s\n", buffer);
            fclose(file);
            unlink(output_filename);
            exit(1);
        }
        
        if (fwrite(buffer, 1, n, file) != n) {
            perror("Error writing to output file");
            fclose(file);
            exit(1);
        }
    }
    
    if (n < 0) {
        perror("Error reading from server");
        fclose(file);
        exit(1);
    }
    
    printf("Client: File saved as: %s\n", output_filename);
    fclose(file);
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in local_address;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    
    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    
    socklen_t addr_size = sizeof(local_address);
    getsockname(sockfd, (SA *)&local_address, &addr_size);
    printf("Client port: %d\n", ntohs(local_address.sin_port));
    
    // Wait for user's go ahead signal
    char input[10];
    printf("Press 'y' and Enter to send the file request: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 'y' && input[0] != 'Y')) {
        printf("Request cancelled by user\n");
        close(sockfd);
        exit(0);
    }
    
    file_request(sockfd, argv[2]);

    close(sockfd);
    exit(0);
}


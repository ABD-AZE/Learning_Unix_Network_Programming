#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>

void dg_file_server(int sockfd, struct sockaddr* pcliaddr, socklen_t clilen) {
    int n;
    socklen_t len;
    char filename[1024];
    char buffer[1024];
    FILE *file;
    
    for(;;){
        len = clilen;
        n = recvfrom(sockfd, filename, sizeof(filename), 0, pcliaddr, &len);
        if (n < 0) {
            perror("recvfrom error");
            continue;   
        }
        
        filename[n] = '\0';

        char *newline = strchr(filename, '\n');
        if (newline) *newline = '\0';
        
        printf("Server: Requested file: %s\n", filename);
        
        file = fopen(filename, "rb");
        if (file == NULL) {
            const char *error_msg = "ERROR: File not found";
            sendto(sockfd, error_msg, strlen(error_msg), 0, pcliaddr, len);
            printf("Server: File not found: %s\n", filename);
            continue;
        }
        
        size_t bytes_read;
        int chunk_num = 0;
        printf("Server: Sending file contents...\n");
        
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            if (sendto(sockfd, buffer, bytes_read, 0, pcliaddr, len) < 0) {
                perror("sendto error");
                break;
            }
            chunk_num++;
            usleep(1000);
        }
        
        const char *eof_marker = "EOF_MARKER";
        sendto(sockfd, eof_marker, strlen(eof_marker), 0, pcliaddr, len);
        
        printf("Server: File transfer completed (%d chunks sent)\n", chunk_num);
        fclose(file);
    }
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr,cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9877);

    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Server is running...\n");
    dg_file_server(sockfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
}
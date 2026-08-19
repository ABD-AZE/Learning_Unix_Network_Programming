#include<stdio.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

void dg_file_client(const char *filename, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen) {
    int n;
    char buffer[1024];
    char output_filename[1024];
    FILE *output_file;
    
    snprintf(output_filename, sizeof(output_filename), "received_%s", filename);
    
    if (sendto(sockfd, filename, strlen(filename), 0, pservaddr, servlen) < 0) {
        perror("sendto error");
        exit(1);
    }
    
    printf("Client: Requesting file: %s\n", filename);
    
    output_file = fopen(output_filename, "wb");
    if (output_file == NULL) {
        perror("Error creating output file");
        exit(1);
    }
    
    while (1) {
        n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (n < 0) {
            perror("recvfrom error");
            fclose(output_file);
            exit(1);
        }
        
        if (strncmp(buffer, "ERROR:", 6) == 0) {
            buffer[n] = '\0';
            printf("Server error: %s\n", buffer);
            fclose(output_file);
            unlink(output_filename);
            exit(1);
        }
        
        if (strncmp(buffer, "EOF_MARKER", 10) == 0) {
            printf("Client: File transfer completed\n");
            break;
        }
        
        if (fwrite(buffer, 1, n, output_file) != n) {
            perror("Error writing to output file");
            fclose(output_file);
            exit(1);
        }
    }
    
    printf("Client: File saved as: %s\n", output_filename);
    fclose(output_file);
}

int main(int argc,char** argv){
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <filename>\n", argv[0]);
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9877);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    dg_file_client(argv[2], sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
    close(sockfd);
    return 0;
}
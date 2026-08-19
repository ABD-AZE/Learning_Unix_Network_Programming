#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024
#define MAX_RETRIES 5
#define TIMEOUT_MS 500  

typedef struct {
    uint32_t seq_num;   
    uint32_t data_size; 
    char data[BUFFER_SIZE];
} Packet;

typedef struct {
    uint32_t seq_num;   
    uint8_t is_ack;     
} Ack;

void dg_file_server(int sockfd, struct sockaddr* pcliaddr, socklen_t clilen) {
    int n;
    socklen_t len;
    char filename[BUFFER_SIZE];
    Packet packet;
    Ack ack;
    FILE *file;
    
    for(;;) {
        len = clilen;
        
        n = recvfrom(sockfd, filename, sizeof(filename), 0, pcliaddr, &len);
        if (n < 0) {
            perror("Server: recvfrom error");
            continue;
        }
        
        filename[n] = '\0';
        
        char *newline = strchr(filename, '\n');
        if (newline) *newline = '\0';
        
        printf("Server: Requested file: %s\n", filename);
        
        file = fopen(filename, "rb");
        if (file == NULL) {
            const char *error_msg = "ERROR: File not found";
            
            packet.seq_num = 0;
            packet.data_size = strlen(error_msg);
            strncpy(packet.data, error_msg, BUFFER_SIZE);
            
            sendto(sockfd, &packet, sizeof(Packet), 0, pcliaddr, len);
            printf("Server: File not found: %s\n", filename);
            continue;
        }
        
        uint32_t seq_num = 0;
        size_t bytes_read;
        int retries;
        
        printf("Server: Starting file transfer...\n");
        
        while ((bytes_read = fread(packet.data, 1, BUFFER_SIZE, file)) > 0) {
            packet.seq_num = seq_num;
            packet.data_size = bytes_read;
            
            retries = 0;
            int ack_received = 0;
            
            while (!ack_received && retries < MAX_RETRIES) {
           
                if (sendto(sockfd, &packet, sizeof(Packet), 0, pcliaddr, len) < 0) {
                    perror("Server: sendto error");
                    break;
                }
                
                printf("Server: Sent packet %d, size %zu bytes\n", seq_num, bytes_read);
                
                fd_set read_fds;
                struct timeval timeout;
                
                FD_ZERO(&read_fds);
                FD_SET(sockfd, &read_fds);
                
                timeout.tv_sec = 0;
                timeout.tv_usec = TIMEOUT_MS * 1000;
                
                if (select(sockfd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
                    // Receive ACK/NACK
                    if (recvfrom(sockfd, &ack, sizeof(Ack), 0, NULL, NULL) > 0) {
                        if (ack.seq_num == seq_num && ack.is_ack) {
                            printf("Server: Received ACK for packet %d\n", seq_num);
                            ack_received = 1;
                        } else if (ack.seq_num == seq_num && !ack.is_ack) {
                            printf("Server: Received NACK for packet %d, retransmitting\n", seq_num);
                        } else {
                            printf("Server: Received unexpected ACK/NACK: seq=%d, ack=%d (expected seq=%d)\n", 
                                  ack.seq_num, ack.is_ack, seq_num);
                        }
                    }
                } else {
                    printf("Server: Timeout waiting for ACK, retry %d\n", retries + 1);
                }
                
                if (!ack_received) {
                    retries++;
                }
            }
            
            if (retries >= MAX_RETRIES) {
                printf("Server: Max retries reached for packet %d, aborting transfer\n", seq_num);
                break;
            }
            
            seq_num++;
        }
        
        packet.seq_num = seq_num;
        packet.data_size = 10;  
        strcpy(packet.data, "EOF_MARKER");
        
        retries = 0;
        int eof_acked = 0;
        
        while (!eof_acked && retries < MAX_RETRIES) {
            if (sendto(sockfd, &packet, sizeof(Packet), 0, pcliaddr, len) < 0) {
                perror("Server: sendto error for EOF");
                break;
            }
            
            printf("Server: Sent EOF packet %d\n", seq_num);
            
            fd_set read_fds;
            struct timeval timeout;
            
            FD_ZERO(&read_fds);
            FD_SET(sockfd, &read_fds);
            
            timeout.tv_sec = 0;
            timeout.tv_usec = TIMEOUT_MS * 1000;
            
            if (select(sockfd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
                if (recvfrom(sockfd, &ack, sizeof(Ack), 0, NULL, NULL) > 0) {
                    if (ack.seq_num == seq_num && ack.is_ack) {
                        printf("Server: Received ACK for EOF\n");
                        eof_acked = 1;
                    }
                }
            } else {
                retries++;
            }
        }
        
        printf("Server: File transfer completed (%d packets sent)\n", seq_num);
        fclose(file);
    }
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Server: socket creation failed");
        exit(1);
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9877);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Server: bind failed");
        exit(1);
    }
    
    printf("Server is running on port %d...\n", ntohs(servaddr.sin_port));
    dg_file_server(sockfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
    
    close(sockfd);
    return 0;
}
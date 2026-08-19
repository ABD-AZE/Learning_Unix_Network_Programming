#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024
#define MAX_RETRIES 5
#define TIMEOUT_MS 2000

typedef struct {
    uint32_t seq_num;    
    uint32_t data_size;  
    char data[BUFFER_SIZE];
} Packet;

typedef struct {
    uint32_t seq_num;    
    uint8_t is_ack;     
} Ack;

void dg_file_client(const char *filename, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen) {
    Packet packet;
    Ack ack;
    char output_filename[BUFFER_SIZE];
    FILE *output_file;
    int n;
    int retries;
    
    snprintf(output_filename, sizeof(output_filename), "received_%s", filename);
    
    // Send filename request with retries
    retries = 0;
    while (retries < MAX_RETRIES) {
        if (sendto(sockfd, filename, strlen(filename), 0, pservaddr, servlen) < 0) {
            perror("Client: sendto error");
            exit(1);
        }
        
        printf("Client: Requesting file: %s (attempt %d)\n", filename, retries + 1);
        
        // Wait for first response to confirm server got the request
        fd_set read_fds;
        struct timeval timeout;
        
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        
        timeout.tv_sec = TIMEOUT_MS / 1000;
        timeout.tv_usec = (TIMEOUT_MS % 1000) * 1000;
        
        if (select(sockfd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
            // Got response, break out of retry loop
            break;
        }
        
        retries++;
        printf("Client: No response, retrying...\n");
    }
    
    if (retries >= MAX_RETRIES) {
        printf("Client: Server not responding, giving up\n");
        exit(1);
    }
    
    output_file = fopen(output_filename, "wb");
    if (output_file == NULL) {
        perror("Client: Error creating output file");
        exit(1);
    }
    
    uint32_t expected_seq = 0;
    int done = 0;
    int consecutive_timeouts = 0;
    
    while (!done) {
        fd_set read_fds;
        struct timeval timeout;
        
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        
        timeout.tv_sec = TIMEOUT_MS / 1000;
        timeout.tv_usec = (TIMEOUT_MS % 1000) * 1000;
        
        if (select(sockfd + 1, &read_fds, NULL, NULL, &timeout) <= 0) {
            consecutive_timeouts++;
            printf("Client: Timeout waiting for packet %d (timeout %d)\n", 
                   expected_seq, consecutive_timeouts);
            
            if (consecutive_timeouts >= MAX_RETRIES) {
                printf("Client: Too many consecutive timeouts, transfer failed\n");
                fclose(output_file);
                unlink(output_filename);
                exit(1);
            }
            
            // Send NACK for missing packet
            ack.seq_num = expected_seq;
            ack.is_ack = 0;  
            
            if (sendto(sockfd, &ack, sizeof(Ack), 0, pservaddr, servlen) < 0) {
                perror("Client: sendto error for NACK");
            } else {
                printf("Client: Sent NACK for packet %d\n", expected_seq);
            }
            continue;
        }
        
        // Reset timeout counter on successful receive
        consecutive_timeouts = 0;
        
        n = recvfrom(sockfd, &packet, sizeof(Packet), 0, NULL, NULL);
        if (n < 0) {
            perror("Client: recvfrom error");
            continue;
        }
        
        printf("Client: Received packet %d, size %d bytes\n", 
              packet.seq_num, packet.data_size);
        
        // Check for error message
        if (packet.seq_num == 0 && strncmp(packet.data, "ERROR:", 6) == 0) {
            packet.data[packet.data_size] = '\0';
            printf("Server error: %s\n", packet.data);
            fclose(output_file);
            unlink(output_filename);
            exit(1);
        }
        
        // Check for EOF marker
        if (strncmp(packet.data, "EOF_MARKER", 10) == 0) {
            printf("Client: Received EOF marker\n");
            done = 1;
            
            // Send ACK for EOF
            ack.seq_num = packet.seq_num;
            ack.is_ack = 1;
            
            if (sendto(sockfd, &ack, sizeof(Ack), 0, pservaddr, servlen) < 0) {
                perror("Client: sendto error for EOF ACK");
            } else {
                printf("Client: Sent ACK for EOF\n");
            }
            
            break;
        }
        
        if (packet.seq_num == expected_seq) {
            // Expected packet - write to file
            if (fwrite(packet.data, 1, packet.data_size, output_file) != packet.data_size) {
                perror("Client: Error writing to output file");
                fclose(output_file);
                exit(1);
            }
            
            // Send ACK
            ack.seq_num = expected_seq;
            ack.is_ack = 1;
            
            if (sendto(sockfd, &ack, sizeof(Ack), 0, pservaddr, servlen) < 0) {
                perror("Client: sendto error for ACK");
            } else {
                printf("Client: Sent ACK for packet %d\n", expected_seq);
            }
            
            expected_seq++;
        } else if (packet.seq_num < expected_seq) {
            // Duplicate packet - send ACK again
            printf("Client: Received duplicate packet %d, expected %d\n", 
                  packet.seq_num, expected_seq);
            
            ack.seq_num = packet.seq_num;
            ack.is_ack = 1;
            
            if (sendto(sockfd, &ack, sizeof(Ack), 0, pservaddr, servlen) < 0) {
                perror("Client: sendto error for duplicate ACK");
            } else {
                printf("Client: Sent duplicate ACK for packet %d\n", packet.seq_num);
            }
        } else {
            // Out-of-order packet - request the expected one
            printf("Client: Received out-of-order packet %d, expected %d\n", 
                  packet.seq_num, expected_seq);
            
            ack.seq_num = expected_seq;
            ack.is_ack = 0;  // NACK
            
            if (sendto(sockfd, &ack, sizeof(Ack), 0, pservaddr, servlen) < 0) {
                perror("Client: sendto error for NACK");
            } else {
                printf("Client: Sent NACK for expected packet %d\n", expected_seq);
            }
        }
    }
    
    printf("Client: File transfer completed\n");
    printf("Client: File saved as: %s\n", output_filename);
    fclose(output_file);
}

int main(int argc, char** argv){
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <filename>\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Client: socket creation failed");
        exit(1);
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9876);
    
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("Client: inet_pton error");
        exit(1);
    }
    
    dg_file_client(argv[2], sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
    close(sockfd);
    return 0;
}
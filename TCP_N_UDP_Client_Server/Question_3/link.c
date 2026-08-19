#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/select.h>

#define BUFFER_SIZE 2048
#define DROP_PROBABILITY 0.2

typedef struct {
    struct sockaddr_in addr;
    socklen_t addr_len;
    int active;
} client_info_t;

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <listen_ip> <listen_port> <forward_ip> <forward_port>\n", argv[0]);
        exit(1);
    }
    
    srand(time(NULL));
    
    struct sockaddr_in link_addr, server_addr;
    int link_sock;
    char buffer[BUFFER_SIZE];
    client_info_t current_client = {0};

    // Create UDP socket
    if ((link_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Link: socket creation failed");
        exit(1);
    }
    
    // Set up link address (where we listen)
    memset(&link_addr, 0, sizeof(link_addr));
    link_addr.sin_family = AF_INET;
    link_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &link_addr.sin_addr);
    
    // Set up server address (where we forward to)
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[4]));
    inet_pton(AF_INET, argv[3], &server_addr.sin_addr);
    
    // Bind the socket
    if (bind(link_sock, (struct sockaddr *)&link_addr, sizeof(link_addr)) < 0) {
        perror("Link: bind failed");
        exit(1);
    }
    
    printf("Link started on %s:%s - Forwarding to %s:%s\n", 
           argv[1], argv[2], argv[3], argv[4]);
    printf("Packet loss probability: %.1f%%\n", DROP_PROBABILITY * 100);
    
    while (1) {
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        int recv_len;
        
        // Receive packet
        recv_len = recvfrom(link_sock, buffer, BUFFER_SIZE, 0, 
                          (struct sockaddr *)&from_addr, &from_len);
        
        if (recv_len < 0) {
            perror("Link: recvfrom failed");
            continue;
        }
        
        char from_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &from_addr.sin_addr, from_ip, INET_ADDRSTRLEN);
        int from_port = ntohs(from_addr.sin_port);
        
        // Determine direction and destination
        int is_from_server = (from_addr.sin_addr.s_addr == server_addr.sin_addr.s_addr && 
                             from_port == ntohs(server_addr.sin_port));
        
        if (is_from_server) {
            // Packet from server → forward to client
            if (!current_client.active) {
                printf("No active client to forward server response\n");
                continue;
            }
            
            printf("Server→Client: %d bytes from %s:%d\n", recv_len, from_ip, from_port);
            
            // Simulate packet loss
            if ((float)rand() / RAND_MAX < DROP_PROBABILITY) {
                printf("Dropping server packet (simulated loss)\n");
                continue;
            }
            
            // Forward to client
            if (sendto(link_sock, buffer, recv_len, 0,
                     (struct sockaddr *)&current_client.addr, current_client.addr_len) < 0) {
                perror("Link: sendto to client failed");
            } else {
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &current_client.addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                printf("Forwarded to client %s:%d\n", 
                       client_ip, ntohs(current_client.addr.sin_port));
            }
        } else {
            // Packet from client → forward to server
            printf("Client→Server: %d bytes from %s:%d\n", recv_len, from_ip, from_port);
            
            // Remember this client
            current_client.addr = from_addr;
            current_client.addr_len = from_len;
            current_client.active = 1;
            
            // Simulate packet loss
            if ((float)rand() / RAND_MAX < DROP_PROBABILITY) {
                printf("Dropping client packet (simulated loss)\n");
                continue;
            }
            
            // Forward to server
            if (sendto(link_sock, buffer, recv_len, 0,
                     (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Link: sendto to server failed");
            } else {
                printf("Forwarded to server %s:%d\n", 
                       argv[3], atoi(argv[4]));
            }
        }
    }
    
    close(link_sock);
    return 0;
}
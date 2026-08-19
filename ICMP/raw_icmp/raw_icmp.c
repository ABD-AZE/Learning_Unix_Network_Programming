#include "raw_icmp.h"

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in source;
    socklen_t source_len = sizeof(source);
    int packet_size;

    // Create raw socket for ICMP
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Socket creation failed. Run as root");
        exit(1);
    }

    printf("Raw ICMP packet capture started. Press Ctrl+C to stop.\n");
    printf("========================================================\n");

    while (1) {
        // Receive packet
        packet_size = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, 
                              (struct sockaddr*)&source, &source_len);
        
        if (packet_size < 0) {
            if (errno == EINTR) {
                continue;  // Interrupted by signal, continue
            }
            perror("recvfrom failed");
            break;
        }

        // Process the captured packet
        process_packet(buffer, packet_size);
    }

    close(sockfd);
    return 0;
}

void process_packet(char *buffer, int size) {
    struct iphdr *ip_header = (struct iphdr*)buffer;
    
    // Check if it's an ICMP packet
    if (ip_header->protocol == IPPROTO_ICMP) {
        printf("\n--- ICMP Packet Captured ---\n");
        print_ip_header(buffer);
        print_icmp_header(buffer, ip_header->ihl * 4);
        printf("----------------------------\n");
    }
}

void print_ip_header(char *buffer) {
    struct iphdr *ip_header = (struct iphdr*)buffer;
    struct sockaddr_in source;
    
    source.sin_addr.s_addr = ip_header->saddr;
    
    printf("Source IP Address: %s\n", inet_ntoa(source.sin_addr));
    printf("Length of IP Header: %d bytes\n", ip_header->ihl * 4);
}

void print_icmp_header(char *buffer, int ip_header_len) {
    struct icmphdr *icmp_header = (struct icmphdr*)(buffer + ip_header_len);
    
    printf("ICMP Type: %d", icmp_header->type);
    switch(icmp_header->type) {
        case ICMP_ECHOREPLY:
            printf(" (Echo Reply)");
            break;
        case ICMP_ECHO:
            printf(" (Echo Request)");
            break;
        case ICMP_DEST_UNREACH:
            printf(" (Destination Unreachable)");
            break;
        case ICMP_TIME_EXCEEDED:
            printf(" (Time Exceeded)");
            break;
        default:
            printf(" (Other)");
            break;
    }
    printf("\n");
    
    printf("ICMP Code: %d\n", icmp_header->code);
}

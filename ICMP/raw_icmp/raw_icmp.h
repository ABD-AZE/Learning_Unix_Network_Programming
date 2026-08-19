#ifndef RAW_ICMP_H
#define RAW_ICMP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFER_SIZE 65536

void process_packet(char *buffer, int size);
void print_ip_header(char *buffer);
void print_icmp_header(char *buffer, int ip_header_len);

#endif

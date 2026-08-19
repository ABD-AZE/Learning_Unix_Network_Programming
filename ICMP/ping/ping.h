#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <bits/getopt_core.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#define BUFSIZE 1500

char sendbuf[BUFSIZE];

int datalen;
char *host;
int nsent;
pid_t pid;
int sockfd;
int verbose;

void proc_v4(char *,ssize_t,struct msghdr *,struct timeval *);
void send_v4(void);
void readloop(void);
void sig_alrm(int);
void tv_sub(struct timeval *,struct timeval *);

struct proto {
    void (*fproc) (char *, ssize_t, struct msghdr *, struct timeval *);
    void (*fsend) (void);
    void (*finit) (void);
    struct sockaddr *sasend;
    struct sockaddr *sarecv;
    socklen_t salen;
    int icmpproto;
} *pr;


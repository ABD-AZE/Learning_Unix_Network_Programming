#include "ping.h"

struct proto proto_v4 = {proc_v4, send_v4, NULL, NULL,NULL,0,IPPROTO_ICMP};

int datalen = 56;

// Add missing function implementations
void err_quit(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
}

struct addrinfo *host_serv(const char *host, const char *serv, int family, int socktype) {
    struct addrinfo hints, *res;
    int n;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = family ? family : AF_UNSPEC;
    hints.ai_socktype = socktype;
    
    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        err_quit("getaddrinfo error for %s: %s", host, gai_strerror(n));
    
    return res;
}

char *sock_ntop_host(struct sockaddr *sa, socklen_t salen) {
    static char str[128];
    
    switch (sa->sa_family) {
    case AF_INET: {
        struct sockaddr_in *sin = (struct sockaddr_in *)sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
            return NULL;
        return str;
    }
    case AF_INET6: {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;
        if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
            return NULL;
        return str;
    }
    default:
        snprintf(str, sizeof(str), "sock_ntop_host: unknown AF_xxx: %d", sa->sa_family);
        return str;
    }
}

void sig_alrm(int signo){
    // Handle the alarm signal, typically used to send a ping
    if(pr->fsend)
        (*pr->fsend) ();
    alarm(1);  // Reschedule the alarm for 1 second later
    return;
}

uint16_t in_cksum(uint16_t * addr, int len){
    int nleft = len;
    uint16_t *w = addr;
    int sum = 0;
    uint16_t answer = 0;

    while(nleft > 1){
        sum += *w++;
        nleft -= 2;
    }
    if(nleft == 1){
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}

void send_v4(void){
    int len;
    struct icmp *icmp;
    icmp = (struct icmp *) sendbuf;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_id = pid;
    icmp->icmp_seq = nsent++;
    memset(icmp->icmp_data, 0xa5, datalen);
    gettimeofday((struct timeval *) icmp->icmp_data, NULL);  // Fixed function name
    len = 8 + datalen;
    icmp->icmp_cksum = 0;
    icmp->icmp_cksum = in_cksum((u_short *) icmp, len);
    sendto(sockfd, sendbuf, len, 0, pr->sasend, pr->salen);
}

void readloop(void){
    int size;
    char recvbuf[BUFSIZE];
    char controlbuf[BUFSIZE];
    struct msghdr msg;
    struct iovec iov;
    ssize_t n;
    struct timeval tval;

    sockfd = socket(pr->sasend->sa_family, SOCK_RAW, pr->icmpproto);
    if (sockfd < 0)
        err_quit("socket error: %s", strerror(errno));
        
    setuid(getuid()); 
    if(pr->finit)
        (*pr->finit) ();
    size = 60*1024;
    setsockopt (sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

    sig_alrm(SIGALRM);

    iov.iov_base = recvbuf;
    iov.iov_len = sizeof(recvbuf);
    msg.msg_name = pr->sarecv;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = controlbuf;
    for(;;){
        msg.msg_namelen = pr->salen;
        msg.msg_controllen = sizeof(controlbuf);
        n = recvmsg(sockfd, &msg, 0);
        if(n<0){
            if(errno == EINTR)
                continue;  // Interrupted by signal, retry
            else
                perror("recvmsg error");
        }
        gettimeofday(&tval, NULL);  // Fixed function name
        (*pr->fproc) (recvbuf, n, &msg, &tval);
    }
}

void proc_v4(char *ptr, ssize_t len, struct msghdr *msg, struct timeval *tvrecv){
    int hlen1, icmplen;
    double rtt;
    struct ip *ip;
    struct icmp *icmp;
    struct timeval *tvsend;
    char *src_str;

    ip = (struct ip *) ptr;
    hlen1 = ip->ip_hl << 2;
    if(ip->ip_p != IPPROTO_ICMP)
        return;
    icmp = (struct icmp *) (ptr + hlen1);
    if((icmplen = len - hlen1) < 8)
        return;
    
    if(icmp->icmp_type == ICMP_ECHOREPLY){
        if(icmp->icmp_id != pid)
            return;  // Not a reply to our ping
        if(icmplen < 16)
            return;  // Not enough data
        tvsend = (struct timeval *) icmp->icmp_data;
        tv_sub(tvrecv, tvsend);
        rtt = tvrecv->tv_sec * 1000.0 + tvrecv->tv_usec / 1000.0;  // Convert to milliseconds
        src_str = sock_ntop_host(msg->msg_name, msg->msg_namelen);  // Get source address string
        printf("%d bytes from %s: seq=%u ttl=%d rtt=%.3f ms\n",
               icmplen, src_str ? src_str : "unknown",
               icmp->icmp_seq, ip->ip_ttl, rtt);
    } else if(verbose){
        src_str = sock_ntop_host(msg->msg_name, msg->msg_namelen);
        printf("%d bytes from %s: type = %d, code = %d\n",
               icmplen, src_str ? src_str : "unknown",
               icmp->icmp_type, icmp->icmp_code);
    }
}

void tv_sub(struct timeval *out, struct timeval *in){
    if( (out->tv_usec -= in->tv_usec) < 0){
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

int main(int argc, char** argv){
    int c;
    struct addrinfo *ai;
    char *h;

    opterr = 0;
    while((c = getopt(argc,argv,"v")) != -1){
        switch(c){
            case 'v':
                verbose++;
                break;
            case '?':
                fprintf(stderr,"unrecognized token: %c", optopt);  // Fixed to use optopt
                exit(1);
            }
    }

    if(optind != argc-1){
        fprintf(stderr,"Usage: %s [-v] <host>\n",argv[0]);
        exit(1);
    }
    h = argv[optind];
    pid = getpid() & 0xFFFF; // Use lower 16 bits of PID
    signal(SIGALRM, sig_alrm);

    ai = host_serv(h, NULL, 0, 0);
    h = sock_ntop_host(ai->ai_addr, ai->ai_addrlen);
    printf("ping %s (%s) : %d data bytes\n",ai->ai_canonname? ai->ai_canonname: h,h,datalen);
    pr = &proto_v4;
    pr->sasend = ai->ai_addr;
    pr->sarecv = calloc(1, ai->ai_addrlen);
    if (!pr->sarecv)
        err_quit("calloc error");
    pr->salen = ai->ai_addrlen;

    readloop();

    exit(0);
}
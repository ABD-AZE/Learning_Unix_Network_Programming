#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#define SA struct sockaddr
#define SERV_PORT 9877
#define LISTENQ 1024
#define SALT_SIZE 16
#define HASH_SIZE 32

void handle_client(int sockfd);
int register_user(const char *username, const char *password);
int authenticate_user(const char *username, const char *password);
void generate_salt(unsigned char *salt);
void compute_hash(const char *password, const unsigned char *salt, unsigned char *hash);
void file_transfer(int sockfd, const char *filename);
ssize_t read_line(int sockfd, char *buffer, size_t max_len);
void send_response(int sockfd, const char *message);
void sigchld_handler(int _signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}


int main(int argc, char **argv){
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port = htons (SERV_PORT);
    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    signal(SIGCHLD, sigchld_handler); 
    printf("Server listening on port %d\n", SERV_PORT);
    for ( ; ; ) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (SA *) &cliaddr, &clilen);
        if ( (childpid = fork()) == 0) {
            close(listenfd); 
            handle_client(connfd);
            exit(0);
        }
        close(connfd); 
    }
}

// Helper function to read a line from socket until newline
ssize_t read_line(int sockfd, char *buffer, size_t max_len) {
    size_t i = 0;
    char c;
    ssize_t rc;
    
    while (i < max_len - 1) {
        rc = read(sockfd, &c, 1);
        if (rc <= 0) return rc;
        
        buffer[i++] = c;
        if (c == '\n') break;
    }
    
    buffer[i-1] = '\0';  // Replace newline with null terminator
    return i-1;
}

// Helper function to send data with delimiter
void send_response(int sockfd, const char *message) {
    char response[256];
    snprintf(response, sizeof(response), "%s\n", message);
    write(sockfd, response, strlen(response));
}

void handle_client(int sockfd) {
    char command[256];
    ssize_t n;
    
    n = read_line(sockfd, command, sizeof(command));
    if (n <= 0) {
        close(sockfd);
        return;
    }
    
    printf("Server: Received command: %s\n", command);
    
    if (strcmp(command, "REGISTER") == 0) {
        char username[256], password[256];
        
        n = read_line(sockfd, username, sizeof(username));
        if (n <= 0) {
            close(sockfd);
            return;
        }
        
        n = read_line(sockfd, password, sizeof(password));
        if (n <= 0) {
            close(sockfd);
            return;
        }
        
        printf("Server: Registering user: %s\n", username);
        
        int result = register_user(username, password);
        if (result) {
            send_response(sockfd, "REG_SUCCESS");
            printf("Server: User %s registered successfully\n", username);
        } else {
            send_response(sockfd, "REG_FAIL");
            printf("Server: Registration failed for %s\n", username);
        }
        
    } else if (strcmp(command, "LOGIN") == 0) {
        char username[256], password[256], filename[256];
        
        n = read_line(sockfd, username, sizeof(username));
        if (n <= 0) {
            close(sockfd);
            return;
        }
        
        n = read_line(sockfd, password, sizeof(password));
        if (n <= 0) {
            close(sockfd);
            return;
        }
        
        printf("Server: Login attempt for user: %s\n", username);

        if (authenticate_user(username, password)) {
            send_response(sockfd, "AUTH_OK");
            printf("Server: User %s authenticated successfully\n", username);
            
            n = read_line(sockfd, filename, sizeof(filename));
            if (n <= 0) {
                close(sockfd);
                return;
            }
            
            printf("Server: File requested: %s\n", filename);
            file_transfer(sockfd, filename);
        } else {
            send_response(sockfd, "AUTH_FAIL");
            printf("Server: Authentication failed for user %s\n", username);
        }
    }
    close(sockfd);
}

void generate_salt(unsigned char *salt) {
    RAND_bytes(salt, SALT_SIZE);
}

void compute_hash(const char *password, const unsigned char *salt, unsigned char *hash) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, password, strlen(password));
    EVP_DigestUpdate(ctx, salt, SALT_SIZE);
    unsigned int hash_len;
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);
}

int register_user(const char *username, const char *password) {
    printf("Server: Registering user %s\n", username);
    FILE *file = fopen("users", "a+");
    if (!file) return 0;
    
    char line[1024];
    rewind(file);
    while (fgets(line, sizeof(line), file)) {
        char *saved_username = strtok(line, ":");
        if (saved_username && strcmp(saved_username, username) == 0) {
            fclose(file);
            return 0;
        }
    }
    
    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];
    generate_salt(salt);
    compute_hash(password, salt, hash);
    
    fprintf(file, "%s:", username);
    for (int i = 0; i < SALT_SIZE; i++) {
        fprintf(file, "%02x", salt[i]);
    }
    fprintf(file, ":");
    for (int i = 0; i < HASH_SIZE; i++) {
        fprintf(file, "%02x", hash[i]);
    }
    fprintf(file, "\n");
    
    fclose(file);
    return 1;
}

int authenticate_user(const char *username, const char *password) {
    FILE *file = fopen("users", "r");
    if (!file) return 0;
    
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        char *saved_username = strtok(line, ":");
        char *salt_hex = strtok(NULL, ":");
        char *hash_hex = strtok(NULL, ":");
        
        if (saved_username && strcmp(saved_username, username) == 0) {
            unsigned char salt[SALT_SIZE];
            unsigned char saved_hash[HASH_SIZE];
            unsigned char computed_hash[HASH_SIZE];
            
            for (int i = 0; i < SALT_SIZE; i++) {
                sscanf(salt_hex + 2*i, "%2hhx", &salt[i]);
            }
            
            for (int i = 0; i < HASH_SIZE; i++) {
                sscanf(hash_hex + 2*i, "%2hhx", &saved_hash[i]);
            }
            
            compute_hash(password, salt, computed_hash);
            
            fclose(file);
            return memcmp(saved_hash, computed_hash, HASH_SIZE) == 0;
        }
    }
    
    fclose(file);
    return 0;
}

void file_transfer(int sockfd, const char *filename) {
    char buffer[1024];
    ssize_t n;
    FILE *file;
    
    printf("Server: Requested file: %s\n", filename);
    
    file = fopen(filename, "rb");
    if (file == NULL) {
        const char *error_msg = "ERROR: File not found";
        write(sockfd, error_msg, strlen(error_msg));
        return;
    }
    
    while ((n = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (write(sockfd, buffer, n) != n) {
            perror("Error sending file data");
            break;
        }
    }
    
    printf("Server: File transfer completed\n");
    fclose(file);
}
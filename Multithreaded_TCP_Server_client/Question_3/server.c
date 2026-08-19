#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#define SA struct sockaddr
#define SERV_PORT 9877
#define LISTENQ 1024
#define SALT_SIZE 16
#define HASH_SIZE 32

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_client(int sockfd);
int register_user(const char *username, const char *password);
int authenticate_user(const char *username, const char *password);
void generate_salt(unsigned char *salt);
void compute_hash(const char *password, const unsigned char *salt, unsigned char *hash);
void file_transfer(int sockfd, const char *filename);
ssize_t read_line(int sockfd, char *buffer, size_t max_len);
void send_response(int sockfd, const char *message);
void* client_thread(void* arg);

typedef struct {
    int sockfd;
    struct sockaddr_in client_addr;
} client_info_t;


int main(int argc, char **argv){
    int listenfd, connfd;
    pthread_t thread_id;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    client_info_t *client_info;
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket creation failed");
        exit(1);
    }
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    
    if (bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(1);
    }
    
    if (listen(listenfd, LISTENQ) < 0) {
        perror("listen failed");
        exit(1);
    }
    
    printf("Threaded server listening on port %d\n", SERV_PORT);
    
    for ( ; ; ) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (SA *) &cliaddr, &clilen);
        if (connfd < 0) {
            perror("accept failed");
            continue;
        }
        
        printf("Client connected from %s:%d\n", 
               inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        
        client_info = malloc(sizeof(client_info_t));
        if (client_info == NULL) {
            perror("malloc failed");
            close(connfd);
            continue;
        }
        
        client_info->sockfd = connfd;
        client_info->client_addr = cliaddr;
        
        if (pthread_create(&thread_id, NULL, client_thread, (void*)client_info) != 0) {
            perror("pthread_create failed");
            free(client_info);
            close(connfd);
            continue;
        }
        
        pthread_detach(thread_id);
    }
    
    close(listenfd);
    return 0;
}

void* client_thread(void* arg) {
    client_info_t *client_info = (client_info_t*)arg;
    int sockfd = client_info->sockfd;
    
    printf("Thread %lu: Handling client from %s:%d\n", 
           pthread_self(), 
           inet_ntoa(client_info->client_addr.sin_addr), 
           ntohs(client_info->client_addr.sin_port));
    
    handle_client(sockfd);
    close(sockfd);
    free(client_info);
    
    printf("Thread %lu: Client disconnected\n", pthread_self());
    pthread_exit(NULL);
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
    printf("Thread %lu: Registering user %s\n", pthread_self(), username);
    
    pthread_mutex_lock(&file_mutex);
    
    FILE *file = fopen("users", "a+");
    if (!file) {
        pthread_mutex_unlock(&file_mutex);
        return 0;
    }
    
    char line[1024];
    rewind(file);
    while (fgets(line, sizeof(line), file)) {
        char *saved_username = strtok(line, ":");
        if (saved_username && strcmp(saved_username, username) == 0) {
            fclose(file);
            pthread_mutex_unlock(&file_mutex);
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
    pthread_mutex_unlock(&file_mutex);
    return 1;
}

int authenticate_user(const char *username, const char *password) {
    pthread_mutex_lock(&file_mutex);
    
    FILE *file = fopen("users", "r");
    if (!file) {
        pthread_mutex_unlock(&file_mutex);
        return 0;
    }
    
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
            pthread_mutex_unlock(&file_mutex);
            return memcmp(saved_hash, computed_hash, HASH_SIZE) == 0;
        }
    }
    
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
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
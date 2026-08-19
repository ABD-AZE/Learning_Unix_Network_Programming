#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<errno.h>

#define SA struct sockaddr
#define SERV_PORT 9877

void register_user(int sockfd, const char *username, const char *password);
void login_and_request_file(int sockfd, const char *username, const char *password, const char *filename);
ssize_t read_line(int sockfd, char *buffer, size_t max_len);
void send_data(int sockfd, const char *data);
int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;
    char username[256], password[256];
    int choice;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <IP address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("1. Register\n2. Login and request file\nChoose option: ");
    scanf("%d", &choice);

    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

    if (choice == 1) {
        register_user(sockfd, username, password);
    } else if (choice == 2) {
        char filename[256];
        printf("Enter filename to request: ");
        scanf("%s", filename);
        login_and_request_file(sockfd, username, password, filename);
    } else {
        printf("Invalid choice\n");
    }

    close(sockfd);
    return 0;
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

// Helper function to send data with newline delimiter
void send_data(int sockfd, const char *data) {
    char message[512];
    snprintf(message, sizeof(message), "%s\n", data);
    write(sockfd, message, strlen(message));
}

void register_user(int sockfd, const char *username, const char *password) {
    char response[256];
    
    send_data(sockfd, "REGISTER");
    send_data(sockfd, username);
    send_data(sockfd, password);
    
    if (read_line(sockfd, response, sizeof(response)) > 0) {
        if (strcmp(response, "REG_SUCCESS") == 0) {
            printf("Registration successful\n");
        } else {
            printf("Registration failed\n");
        }
    } else {
        printf("Error reading server response\n");
    }
}

void login_and_request_file(int sockfd, const char *username, const char *password, const char *filename) {
    char response[256];
    char buffer[1024];
    ssize_t n;
    FILE *file;
    char output_filename[1024];
    
    send_data(sockfd, "LOGIN");
    send_data(sockfd, username);
    send_data(sockfd, password);
    
    if (read_line(sockfd, response, sizeof(response)) <= 0) {
        printf("Error reading authentication response\n");
        return;
    }
    
    if (strcmp(response, "AUTH_OK") == 0) {
        printf("Authentication successful\n");
        
        send_data(sockfd, filename);
        
        snprintf(output_filename, sizeof(output_filename), "received_%s", filename);
        file = fopen(output_filename, "wb");
        if (file == NULL) {
            perror("Error creating output file");
            return;
        }
        
        int first_read = 1;
        while ((n = read(sockfd, buffer, sizeof(buffer))) > 0) {
            if (first_read && strncmp(buffer, "ERROR:", 6) == 0) {
                buffer[n] = '\0';
                printf("Server error: %s\n", buffer);
                fclose(file);
                unlink(output_filename);
                return;
            }
            first_read = 0;
            
            if (fwrite(buffer, 1, n, file) != (size_t)n) {
                perror("Error writing to output file");
                fclose(file);
                return;
            }
        }
        
        printf("File saved as: %s\n", output_filename);
        fclose(file);
    } else {
        printf("Authentication failed: %s\n", response);
    }
}


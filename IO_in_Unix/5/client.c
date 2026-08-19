#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#define SERV_PORT "9877"

void file_request(BIO *bio, const char *filename) {
    char buffer[1024];
    int n;
    FILE *file;
    char output_filename[1024];
    
    snprintf(output_filename, sizeof(output_filename), "received_%s", filename);

    if (BIO_write(bio, filename, strlen(filename)) <= 0) {
        fprintf(stderr, "Error sending filename\n");
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    printf("Client: Requesting file: %s\n", filename);

    file = fopen(output_filename, "wb");
    if (!file) {
        perror("Error creating output file");
        exit(1);
    }

    while ((n = BIO_read(bio, buffer, sizeof(buffer))) > 0) {
        if (strncmp(buffer, "ERROR:", 6) == 0) {
            buffer[n] = '\0';
            printf("Server error: %s\n", buffer);
            fclose(file);
            unlink(output_filename);
            exit(1);
        }
        fwrite(buffer, 1, n, file);
    }

    if (n < 0) {
        fprintf(stderr, "Error reading from server\n");
        ERR_print_errors_fp(stderr);
    }

    printf("Client: File saved as: %s\n", output_filename);
    fclose(file);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    BIO *bio = BIO_new(BIO_s_connect());
    if (!bio) {
        fprintf(stderr, "BIO_new failed\n");
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    BIO_set_conn_hostname(bio, argv[1]);
    BIO_set_conn_port(bio, SERV_PORT);

    if (BIO_do_connect(bio) <= 0) {
        fprintf(stderr, "Connection failed\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        exit(1);
    }

    printf("Connected to server %s:%s\n", argv[1], SERV_PORT);
    
    char input[10];
    printf("Press 'y' and Enter to send the file request: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 'y' && input[0] != 'Y')) {
        printf("Request cancelled by user\n");
        BIO_free_all(bio);
        exit(0);
    }

    file_request(bio, argv[2]);
    BIO_free_all(bio);
    return 0;
}

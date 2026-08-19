#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 4443
#define BUFFER_SIZE 4096

const char* html_response = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n"
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "    <title>HTTPS Server - Self-Signed Certificate</title>\n"
    "    <style>\n"
    "        body { font-family: Arial, sans-serif; margin: 50px; background: #f0f0f0; }\n"
    "        .container { background: white; padding: 30px; border-radius: 10px; max-width: 600px; margin: 0 auto; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n"
    "        h1 { color: #2c3e50; }\n"
    "        .success { color: #27ae60; font-weight: bold; }\n"
    "        .info { background: #ecf0f1; padding: 15px; border-radius: 5px; margin: 15px 0; }\n"
    "    </style>\n"
    "</head>\n"
    "<body>\n"
    "    <div class='container'>\n"
    "        <h1>🔒 HTTPS Connection Successful!</h1>\n"
    "        <p class='success'>Secure connection established using self-signed SSL certificate.</p>\n"
    "        <div class='info'>\n"
    "            <h3>Certificate Information</h3>\n"
    "            <p><strong>Protocol:</strong> HTTPS</p>\n"
    "            <p><strong>Server:</strong> C OpenSSL Server</p>\n"
    "            <p><strong>Port:</strong> 4443</p>\n"
    "            <p><strong>Encryption:</strong> TLS with RSA 2048-bit</p>\n"
    "            <p><strong>Common Name:</strong> localhost</p>\n"
    "        </div>\n"
    "        <div class='info'>\n"
    "            <h3>Note</h3>\n"
    "            <p>This is a self-signed certificate for development/testing purposes.</p>\n"
    "            <p>Browser warnings are expected for self-signed certificates.</p>\n"
    "        </div>\n"
    "    </div>\n"
    "</body>\n"
    "</html>";

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX* create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    if (SSL_CTX_use_certificate_file(ctx, "./ssl_certificates/server-cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "./ssl_certificates/server-key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int create_socket(int port) {
    int sock;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sock, 1) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    return sock;
}

void handle_client(SSL *ssl) {
    char buffer[BUFFER_SIZE];
    int bytes;

    bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        buffer[bytes] = 0;
        printf("Received request:\n%s\n", buffer);
        SSL_write(ssl, html_response, strlen(html_response));
    }
}

int main() {
    int sock;
    SSL_CTX *ctx;

    printf("Initializing HTTPS Server...\n");
    
    init_openssl();
    ctx = create_context();
    configure_context(ctx);

    sock = create_socket(PORT);

    printf("HTTPS Server started on port %d\n", PORT);
    printf("Access at: https://localhost:%d\n", PORT);
    printf("Press Ctrl+C to stop\n\n");

    while(1) {
        struct sockaddr_in addr;
        unsigned int len = sizeof(addr);
        SSL *ssl;

        int client = accept(sock, (struct sockaddr*)&addr, &len);
        if (client < 0) {
            perror("Unable to accept");
            continue;
        }

        printf("Connection from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        } else {
            handle_client(ssl);
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client);
    }

    close(sock);
    SSL_CTX_free(ctx);
    cleanup_openssl();

    return 0;
}

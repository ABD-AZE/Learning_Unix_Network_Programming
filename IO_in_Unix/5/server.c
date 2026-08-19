#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#define SERV_PORT "9877"

void signalhandler(int signo) {
    (void)signo;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void file_request(BIO*);

int main(void) {
    signal(SIGCHLD, signalhandler);
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    BIO *abio = BIO_new_accept(SERV_PORT);
    if (!abio) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    if (BIO_do_accept(abio) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    printf("Listening on port %s\n", SERV_PORT);

    for(;;) {
        if (BIO_do_accept(abio) <= 0) {
            fprintf(stderr, "BIO_do_accept failed\n");
            ERR_print_errors_fp(stderr);
            continue;
        }
        BIO *cbio = BIO_pop(abio);
        if (!cbio) {
            fprintf(stderr, "BIO_pop failed\n");
            continue;
        }
        
        pid_t pid = fork();
        if (pid == 0) {
            file_request(cbio);
            exit(0);
        } else if (pid > 0) {
            BIO_free_all(cbio);
        } else {
            perror("fork");
            BIO_free_all(cbio);
        }
    }
    BIO_free(abio);
    EVP_cleanup();
    ERR_free_strings();
    return 0;
}

void file_request(BIO *cbio) {
    char fname[1024];
    ssize_t rlen = BIO_read(cbio, fname, sizeof(fname) - 1);
    if (rlen <= 0) {
        BIO_free(cbio);
        exit(0);
    }
    fname[rlen] = '\0';
    char *nl = strchr(fname, '\n');
    if (nl) *nl = '\0';
    fprintf(stdout, "Client requested: \"%s\"\n", fname);
    FILE *fp = fopen(fname, "rb");
    if (!fp) {
        const char *err = "ERROR: File not found";
        BIO_write(cbio, err, strlen(err));
        BIO_free(cbio);
        exit(0);
    }
    char buf[2048];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        if (BIO_write(cbio, buf, n) <= 0) break;
    }
    printf("File transfer completed for: %s\n", fname);
    fclose(fp);
    BIO_free_all(cbio);
    exit(0);
}
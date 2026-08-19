#ifndef CRYPTO_COMMON_H
#define CRYPTO_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/time.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define BUFFER_SIZE 4096

// Function prototypes
void handle_errors(const char *msg);
int generate_random_data(unsigned char *buffer, int length);
int encrypt_file(const char *input_file, const char *output_file, 
                 unsigned char *key, unsigned char *iv, const EVP_CIPHER *cipher);
int decrypt_file(const char *input_file, const char *output_file,
                 unsigned char *key, unsigned char *iv, const EVP_CIPHER *cipher);
int compare_files(const char *file1, const char *file2);
double get_time_diff(struct timespec start, struct timespec end);
void print_hex(const char *label, unsigned char *data, int len);

// Error handling function
void handle_errors(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    ERR_print_errors_fp(stderr);
    exit(1);
}

// Generate random data for key and IV
int generate_random_data(unsigned char *buffer, int length) {
    if (RAND_bytes(buffer, length) != 1) {
        handle_errors("Failed to generate random data");
        return 0;
    }
    return 1;
}

// Print data in hexadecimal format
void print_hex(const char *label, unsigned char *data, int len) {
    printf("%s: ", label);
    for (int i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

// Encrypt file using specified cipher
int encrypt_file(const char *input_file, const char *output_file,
                 unsigned char *key, unsigned char *iv, const EVP_CIPHER *cipher) {
    FILE *ifp = NULL, *ofp = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char inbuf[BUFFER_SIZE];
    unsigned char outbuf[BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH];
    int inlen, outlen;
    int ret = 0;

    ifp = fopen(input_file, "rb");
    if (!ifp) {
        fprintf(stderr, "Cannot open input file: %s\n", input_file);
        return 0;
    }

    ofp = fopen(output_file, "wb");
    if (!ofp) {
        fprintf(stderr, "Cannot open output file: %s\n", output_file);
        fclose(ifp);
        return 0;
    }

    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_errors("Failed to create cipher context");

    if (EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv) != 1) {
        handle_errors("Failed to initialize encryption");
    }

    while ((inlen = fread(inbuf, 1, BUFFER_SIZE, ifp)) > 0) {
        if (EVP_EncryptUpdate(ctx, outbuf, &outlen, inbuf, inlen) != 1) {
            handle_errors("Encryption update failed");
        }
        fwrite(outbuf, 1, outlen, ofp);
    }

    if (EVP_EncryptFinal_ex(ctx, outbuf, &outlen) != 1) {
        handle_errors("Encryption finalization failed");
    }
    fwrite(outbuf, 1, outlen, ofp);

    ret = 1;
    EVP_CIPHER_CTX_free(ctx);
    fclose(ifp);
    fclose(ofp);

    return ret;
}

// Decrypt file using specified cipher
int decrypt_file(const char *input_file, const char *output_file,
                 unsigned char *key, unsigned char *iv, const EVP_CIPHER *cipher) {
    FILE *ifp = NULL, *ofp = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char inbuf[BUFFER_SIZE];
    unsigned char outbuf[BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH];
    int inlen, outlen;
    int ret = 0;

    ifp = fopen(input_file, "rb");
    if (!ifp) {
        fprintf(stderr, "Cannot open input file: %s\n", input_file);
        return 0;
    }

    ofp = fopen(output_file, "wb");
    if (!ofp) {
        fprintf(stderr, "Cannot open output file: %s\n", output_file);
        fclose(ifp);
        return 0;
    }

    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_errors("Failed to create cipher context");

    if (EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv) != 1) {
        handle_errors("Failed to initialize decryption");
    }

    while ((inlen = fread(inbuf, 1, BUFFER_SIZE, ifp)) > 0) {
        if (EVP_DecryptUpdate(ctx, outbuf, &outlen, inbuf, inlen) != 1) {
            handle_errors("Decryption update failed");
        }
        fwrite(outbuf, 1, outlen, ofp);
    }

    if (EVP_DecryptFinal_ex(ctx, outbuf, &outlen) != 1) {
        handle_errors("Decryption finalization failed");
    }
    fwrite(outbuf, 1, outlen, ofp);

    ret = 1;
    EVP_CIPHER_CTX_free(ctx);
    fclose(ifp);
    fclose(ofp);

    return ret;
}

// Compare two files byte by byte
int compare_files(const char *file1, const char *file2) {
    FILE *fp1 = fopen(file1, "rb");
    FILE *fp2 = fopen(file2, "rb");
    
    if (!fp1 || !fp2) {
        if (fp1) fclose(fp1);
        if (fp2) fclose(fp2);
        return 0;
    }

    int ch1, ch2;
    int match = 1;
    
    while (((ch1 = fgetc(fp1)) != EOF) && ((ch2 = fgetc(fp2)) != EOF)) {
        if (ch1 != ch2) {
            match = 0;
            break;
        }
    }

    if (fgetc(fp1) != EOF || fgetc(fp2) != EOF) {
        match = 0;
    }

    fclose(fp1);
    fclose(fp2);

    return match;
}

// Calculate time difference in seconds
double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

#endif // CRYPTO_COMMON_H

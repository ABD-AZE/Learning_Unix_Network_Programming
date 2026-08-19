#include<openssl/sha.h>
#include<openssl/rand.h>
#include<stdio.h>
#include<string.h>

#define MAX_INPUT_SIZE 1000

int main(){
    unsigned char input[MAX_INPUT_SIZE];
    printf("Enter a string: ");
    fgets(input, sizeof(input), stdin);
    // Remove newline character if present
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }
    unsigned char random_bytes[16];
    if(RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {
        fprintf(stderr, "Error generating random bytes\n");
        return 1;
    }
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned char input_with_random[strlen(input) + 16+1];
    snprintf((char*)input_with_random, sizeof(input_with_random), "%s%.*s", input, 16, random_bytes);
    SHA256((unsigned char*)input_with_random, strlen(input_with_random), hash);
    
    printf("Input bytes: ");
    for(int i = 0; i < strlen(input); i++) {
        printf("%02x", input[i]);
    }

    printf("\nRandom bytes: ");
    for(int i = 0; i < sizeof(random_bytes); i++) {
        printf("%02x", random_bytes[i]);
    }

    printf("\nconcatenated input and random bytes: ");
    for(int i = 0; i < strlen(input) + 16; i++)
    {
        printf("%02x", input_with_random[i]);
    }
    printf("\n");
    printf("\nSHA-256 hash of input+random_bytes: ");
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
    return 0;
}
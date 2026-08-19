#include "crypto_common.h"

#define KEY_LEN 16  // 128 bits
#define IV_LEN 16   // 128 bits

int main(int argc, char *argv[]) {
    const char *plaintext_file = "plaintext.txt";
    const char *encrypted_file = "aes128_encrypted.bin";
    const char *decrypted_file = "aes128_decrypted.txt";
    
    unsigned char key[KEY_LEN];
    unsigned char iv[IV_LEN];
    struct timespec start, end;
    double encrypt_time, decrypt_time;
    
    // Check if custom plaintext file is provided
    if (argc > 1) {
        plaintext_file = argv[1];
    }
    
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    // Generate random key and IV
    generate_random_data(key, KEY_LEN);
    generate_random_data(iv, IV_LEN);
    
    print_hex("Key", key, KEY_LEN);
    print_hex("IV", iv, IV_LEN);
    
    // Encryption
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (!encrypt_file(plaintext_file, encrypted_file, key, iv, EVP_aes_128_cbc())) {
        fprintf(stderr, "Encryption failed!\n");
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    encrypt_time = get_time_diff(start, end);
    printf("Encryption completed in %.6f seconds\n", encrypt_time);
    printf("Encrypted file: %s\n", encrypted_file);
    
    // Decryption
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (!decrypt_file(encrypted_file, decrypted_file, key, iv, EVP_aes_128_cbc())) {
        fprintf(stderr, "Decryption failed!\n");
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    decrypt_time = get_time_diff(start, end);
    printf("Decryption completed in %.6f seconds\n", decrypt_time);
    printf("Decrypted file: %s\n", decrypted_file);
    
    // Verification
    if (compare_files(plaintext_file, decrypted_file)) {
        printf("Decrypted file matches original!\n");
    } else {
        printf("Decrypted file does NOT match original!\n");
        return 1;
    }
    
    // Performance Summary
    printf("Encryption time: %.6f seconds\n", encrypt_time);
    printf("Decryption time: %.6f seconds\n", decrypt_time);
    printf("Total time:      %.6f seconds\n", encrypt_time + decrypt_time);
    
    // Output timing for script parsing
    printf("\nTIMING_RESULT:AES-128-CBC:%.6f:%.6f:%.6f\n", 
           encrypt_time, decrypt_time, encrypt_time + decrypt_time);
    
    // Cleanup
    EVP_cleanup();
    ERR_free_strings();
    
    return 0;
}

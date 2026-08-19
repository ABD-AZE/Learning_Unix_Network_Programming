#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

int main() {
	BIO *file_bio = NULL;
	BIO *buffer_bio = NULL;
	BIO *cipher_bio = NULL;
	BIO *bio_chain = NULL;
	
	unsigned char key[32]; // 256 bits key
	unsigned char iv[16];  // 128 bits IV for AES
	const char *plaintext = "Hello World!";
	int len;
	
	// Initialize OpenSSL
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
	
	// Generate random key and IV
	if (RAND_bytes(key, sizeof(key)) != 1) {
		fprintf(stderr, "Error generating random key\n");
		return 1;
	}
	
	if (RAND_bytes(iv, sizeof(iv)) != 1) {
		fprintf(stderr, "Error generating random IV\n");
		return 1;
	}

    // writing key and iv to a file for decryption purpose
    FILE *keyfile = fopen("key.bin", "wb");
    FILE *ivfile = fopen("iv.bin", "wb");
    if (!keyfile || !ivfile) {
        fprintf(stderr, "Error opening key.bin or iv.bin for writing\n");
        return 1;
    }
    fwrite(key, 1, sizeof(key), keyfile);
    fwrite(iv, 1, sizeof(iv), ivfile);
    fclose(keyfile);
    fclose(ivfile);
	
	// (i) Create file BIO
	file_bio = BIO_new_file("Sample.bin", "wb");
	if (!file_bio) {
		fprintf(stderr, "Error creating file BIO\n");
		ERR_print_errors_fp(stderr);
		return 1;
	}
	
	// (ii) Create buffering filter BIO
	buffer_bio = BIO_new(BIO_f_buffer());
	if (!buffer_bio) {
		fprintf(stderr, "Error creating buffer BIO\n");
		ERR_print_errors_fp(stderr);
		BIO_free(file_bio);
		return 1;
	}
	
	// (ii) Create cipher filter BIO with AES-256-CBC
	cipher_bio = BIO_new(BIO_f_cipher());
	if (!cipher_bio) {
		fprintf(stderr, "Error creating cipher BIO\n");
		ERR_print_errors_fp(stderr);
		BIO_free(buffer_bio);
		BIO_free(file_bio);
		return 1;
	}
	
	// (iii) Set up AES-256-CBC encryption
	BIO_set_cipher(cipher_bio, EVP_aes_256_cbc(), key, iv, 1); // 1 for encryption
	
	// (iv) Assemble BIO chain: cipher-buffer-file
	bio_chain = BIO_push(cipher_bio, buffer_bio);
	bio_chain = BIO_push(bio_chain, file_bio);
	
	// (v) Encrypt and write the string
	len = BIO_write(bio_chain, plaintext, strlen(plaintext));
	if (len <= 0) {
		fprintf(stderr, "Error writing to BIO chain\n");
		ERR_print_errors_fp(stderr);
	} else {
		printf("Successfully encrypted and wrote %d bytes to Sample.bin\n", len);
	}
	
	// Flush the BIO chain
	BIO_flush(bio_chain);
	
	// Clean up
	BIO_free_all(bio_chain);
	
	// Clean up OpenSSL
	EVP_cleanup();
	ERR_free_strings();
	
	printf("Encryption completed. Check Sample.bin file.\n");
	return 0;
}

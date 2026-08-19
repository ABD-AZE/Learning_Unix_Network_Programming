#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

int main() {
	BIO *bio_file, *bio_cipher, *bio_buffer;
	EVP_CIPHER_CTX *ctx;
	unsigned char key[32];
	unsigned char iv[16];
	char buffer[1024];
	int bytes_read;
	FILE *key_file, *iv_file;
	// Initialize OpenSSL
	OpenSSL_add_all_algorithms();
	
	// Load key from key.bin
	key_file = fopen("key.bin", "rb");
	if (!key_file) {
		fprintf(stderr, "Error: Could not open key.bin\n");
		return 1;
	}
	if (fread(key, 1, 32, key_file) != 32) {
		fprintf(stderr, "Error: Could not read 32 bytes from key.bin\n");
		fclose(key_file);
		return 1;
	}
	fclose(key_file);
	
	// Load IV from iv.bin
	iv_file = fopen("iv.bin", "rb");
	if (!iv_file) {
		fprintf(stderr, "Error: Could not open iv.bin\n");
		return 1;
	}
	if (fread(iv, 1, 16, iv_file) != 16) {
		fprintf(stderr, "Error: Could not read 16 bytes from iv.bin\n");
		fclose(iv_file);
		return 1;
	}
	fclose(iv_file);
	
	// Create file BIO for reading Sample.bin
	bio_file = BIO_new_file("Sample.bin", "rb");
	if (!bio_file) {
		fprintf(stderr, "Error: Could not open Sample.bin\n");
		return 1;
	}
	
	// Create cipher BIO for AES-256-CBC decryption
	bio_cipher = BIO_new(BIO_f_cipher());
	if (!bio_cipher) {
		fprintf(stderr, "Error: Could not create cipher BIO\n");
		BIO_free(bio_file);
		return 1;
	}
	
	// Set up cipher for decryption
	BIO_set_cipher(bio_cipher, EVP_aes_256_cbc(), key, iv, 0); // 0 for decrypt
	
	// Create buffer BIO
	bio_buffer = BIO_new(BIO_f_buffer());
	if (!bio_buffer) {
		fprintf(stderr, "Error: Could not create buffer BIO\n");
		BIO_free(bio_file);
		BIO_free(bio_cipher);
		return 1;
	}
	
	// Chain the BIOs: buffer -> cipher -> file
	BIO_push(bio_buffer, bio_cipher);
	BIO_push(bio_cipher, bio_file);
    printf("Decrypted content of Sample.bin:\n");
	// Read and display decrypted data
	while ((bytes_read = BIO_read(bio_buffer, buffer, sizeof(buffer) - 1)) > 0) {
		buffer[bytes_read] = '\0'; // Null terminate
		printf("%s", buffer);
	}
		
	// Cleanup
	BIO_free_all(bio_buffer); // This frees the entire chain
	
	// Cleanup OpenSSL
	EVP_cleanup();
	
	return 0;
}

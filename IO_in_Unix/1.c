#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <unistd.h>

int main(void) {
    FILE *fp = fopen("Sample.txt", "w+");
    if (!fp) return 1;

    BIO *b = BIO_new_fp(fp, BIO_NOCLOSE); /* buffered by default */
    const char *s = "Hello World";
    char buf[64];

    BIO_write(b, s, strlen(s));
    BIO_seek(b, 0);
    int n = BIO_read(b, buf, sizeof(buf)-1);
    if (n > 0) {
        buf[n] = '\0';
        puts(buf);
    }

    /* also read via fgets from FILE* */
    fseek(fp, 0, SEEK_SET);
    if (fgets(buf, sizeof(buf), fp)) puts(buf);

    BIO_free(b);

    fclose(fp);
    return 0;
}

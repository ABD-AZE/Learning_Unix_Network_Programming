#include <stdio.h>
#include <arpa/inet.h>
int main(){
    short port = 5120, port2;
    char *p = (char *) &port;
   char *p2 = (char *) &port2;
    for (int i = 0; i < sizeof(port); i++)
    printf("%x ", (unsigned char) *p++);
    printf("\n");
    port2 = htons(port);
   for (int i = 0; i < sizeof(port2); i++)
   printf("%x ", (unsigned char) *p2++);
   }
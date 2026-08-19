#!/bin/bash
openssl dsaparam -genkey 1024 > dsaparam.pem
openssl gendsa dsaparam.pem > dsaprivatekey.pem
openssl dgst -sha1 -hex Alice.txt | cut -d' ' -f2 > digest.txt
openssl dgst -sha1 -sign dsaprivatekey.pem -out dsasign.bin Alice.txt
openssl dsa -in dsaprivatekey.pem -pubout > dsapubkey.pem
openssl dgst -sha1 -verify dsapubkey.pem -signature dsasign.bin Alice.txt

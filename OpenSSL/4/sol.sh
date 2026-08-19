#!/bin/bash
./base64 encode Alice.txt out.pem
openssl base64 -d -A -in out.pem -out out2.txt
diff Alice.txt out2.txt
openssl base64 -A -in Alice.txt -out out3.txt
./base64 decode out3.txt out4.txt
diff Alice.txt out4.txt

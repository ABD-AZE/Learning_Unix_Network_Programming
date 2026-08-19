openssl rand 8 > des_key
openssl des3 -in Alice.txt -out Alice.enc -K $(xxd -p des_key) -iv $(xxd -p des_key)
openssl des3 -d -in Alice.enc -out Alice.dec -K $(xxd -p des_key) -iv $(xxd -p des_key)
diff Alice.txt Alice.dec
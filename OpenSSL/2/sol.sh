openssl rand 16 > aes_key
openssl rand 16 > aes_iv

openssl aes-128-ecb -in Sample.bmp -out Sample_ecb.enc -K $(xxd -p aes_key)

openssl aes-128-cbc -in Sample.bmp -out Sample_cbc.enc -K $(xxd -p aes_key) -iv $(xxd -p aes_iv)

head -c 54 Sample.bmp > header

tail -c +55 Sample_ecb.enc > body
cat header body > new_ecb.bmp
tail -c +55 Sample_cbc.enc > body
cat header body > new_cbc.bmp
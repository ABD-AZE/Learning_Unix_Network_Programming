## Compile and Run

- To compile, run and compare various encryption and decryption algorithms:
```
  chmod +x ./run.sh
  ./run.sh
```

- Put the plaintext to be encrypted in **plaintext.txt**

## Performance Comparison

- The encryption/decryption timings for 3des_cbc is the maximum (approx 10 times that of others).
- The timings for aes algorithms (256,128,192) is almost the same for small files.
- For larger files the timings are: aes256 > aes192 > aes128, which is expected because of number of rounds of encryption they perform.

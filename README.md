# Learning Unix Network Programming

> **Description:** A comprehensive collection of hands-on assignments, lab exercises, and implementation projects for learning Unix Network Programming, Socket API engineering, System Signal Handling, Network Security, Firewall Management, and Cryptography using OpenSSL.

---

## Repository Overview

| Directory | Topic / Focus Area | Key Concepts & File Summary |
| :--- | :--- | :--- |
| [**`Unix_Signals/`**](./Unix_Signals) | Unix Signals & Process Control | Custom signal handlers (`SIGINT`, `SIGCHLD`), zombie process prevention with non-blocking `waitpid()`, timers (`SIGALRM`), signal masking (`sigprocmask`), resource limits, and inter-process synchronization using `SIGUSR1`/`SIGUSR2`. |
| [**`TCP_N_UDP_Client_Server/`**](./TCP_N_UDP_Client_Server) | Basic Socket Programming | Multi-process concurrent TCP file servers (`fork`), connectionless UDP file transfers (`recvfrom`/`sendto`), and reliable Stop-and-Wait ARQ protocol implementation over UDP. |
| [**`Multithreaded_TCP_Server_client/`**](./Multithreaded_TCP_Server_client) | Multi-threaded & Multi-process Architecture | Comparative design of process-per-client vs. thread-per-client (`pthread`) TCP servers, mutex synchronization, salted SHA-256 password authentication, and concurrency load benchmarking. |
| [**`ICMP/`**](./ICMP) | Raw Sockets & ICMP Protocols | Packet sniffing with raw sockets (`SOCK_RAW`, `IPPROTO_ICMP`), IP/ICMP header parsing, and a custom Unix `ping` utility implementing ICMP Echo Request/Reply with RTT measurement. |
| [**`IO_in_Unix/`**](./IO_in_Unix) | Unix I/O Abstractions & OpenSSL BIO | High-level I/O abstractions using OpenSSL BIOs (`BIO_new_fp`, `BIO_new_fd`), cryptographic BIO pipelines (`cipher_bio` -> `file_bio`), and socket BIO abstractions (`BIO_new_accept`, `BIO_new_connect`). |
| [**`OpenSSL_Crypto_API/`**](./OpenSSL_Crypto_API) | OpenSSL EVP C Cryptographic API | Performance comparison and file encryption/decryption using OpenSSL EVP C APIs (`EVP_CIPHER_CTX`), benchmarking AES-128-CBC, AES-192-CBC, AES-256-CBC, and 3DES-CBC. |
| [**`OpenSSL/`**](./OpenSSL) | OpenSSL CLI & Cryptographic Labs | Symmetric encryption (3DES, AES ECB vs. CBC mode image leakage), DSA digital signature generation/verification, custom C++ Base64 codec vs. OpenSSL, network byte ordering (`htons`), and signal mask interactions. |
| [**`SSL_Certificates/`**](./SSL_Certificates) | PKI & HTTPS Server in C | Shell scripts for generating self-signed SSL/TLS certificates and CSRs, and an HTTPS web server implemented in C using OpenSSL TLS socket APIs (`SSL_CTX`, `SSL_accept`). |
| [**`IP_Tables/`**](./IP_Tables) | Packet Filtering & Firewalling | Linux `iptables` administration scripts covering packet counting, ICMP/Ping blocking, interface traffic monitoring (`eth0`/`wlan0`), NAT port forwarding, host blocking, kernel packet logging, and SSH brute-force rate-limiting. |
| [**`Network_security_using_snort/`**](./Network_security_using_snort) | Network Intrusion Detection (NIDS) | Custom signature-based Snort rules (`local.rules`) for detecting TCP SYN port scans, ICMP flood attacks, and SSH brute-force login attempts with thresholding. |

---

## Detailed Folder Analysis

### 1. [`Unix_Signals`](./Unix_Signals)
Focuses on asynchronous event handling, POSIX signal mechanics, and IPC signal synchronization.
- **`question_1.c`**: Trapping `SIGINT` to test signal handler reliability.
- **`question_2.c`**: `SIGCHLD` handler using `waitpid(-1, NULL, WNOHANG)` to reap terminated child processes without blocking.
- **`question_3.c`**: Process execution timeout using `alarm()` and `SIGALRM`.
- **`question_4.c`**: Combining signal-driven timeouts with system resource inspection (`sys/resource.h`).
- **`question_5.c`**: Blocking and unblocking signals via `sigprocmask()`, checking pending signals with `sigpending()`.
- **`question_6.c`**: Parent-child synchronization primitives (`TELL_PARENT`, `WAIT_CHILD`, `TELL_CHILD`, `WAIT_PARENT`) using `SIGUSR1` and `SIGUSR2`.

### 2. [`TCP_N_UDP_Client_Server`](./TCP_N_UDP_Client_Server)
Covers core socket programming concepts across TCP and UDP protocols.
- **`Question_1/`**: Concurrent TCP file server using `fork()` for handling multiple clients simultaneously.
- **`Question_2/`**: Iterative UDP file server using connectionless datagram sockets (`sendto`, `recvfrom`).
- **`Question_3/`**: Custom Stop-and-Wait Automatic Repeat-reQuest (ARQ) protocol built on top of UDP to guarantee reliable deliverability with packet sequence numbers, timeouts (`struct timeval`), and ACKs.

### 3. [`Multithreaded_TCP_Server_client`](./Multithreaded_TCP_Server_client)
Explores concurrency models, multithreading synchronization, and secure network services.
- **`Question_1/`**: Secure hashing using OpenSSL SHA-256 with cryptographically generated random salt (`RAND_bytes`).
- **`Question_2/`**: Multi-process TCP server featuring secure user registration, salted password verification, and file downloads.
- **`Question_3/`**: Multi-threaded TCP server using `pthread_create()`, `pthread_detach()`, and `pthread_mutex_t` for synchronized user database access.
- **`Question_4/`**: Performance benchmarks and metrics comparing thread-per-client vs process-per-client concurrency models under client load.

### 4. [`ICMP`](./ICMP)
Deep dive into network layer protocols and raw socket engineering.
- **`raw_icmp/`**: Raw socket program (`socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)`) for capturing and analyzing raw IP and ICMP headers in real-time.
- **`ping/`**: Full implementation of the Unix `ping` command, handling ICMP Echo Request construction, internet checksum calculations, address resolution (`getaddrinfo`), and RTT timing.

### 5. [`IO_in_Unix`](./IO_in_Unix)
Demonstrates stream abstractions and OpenSSL's Basic Input/Output (BIO) framework.
- **`1.c`**: OpenSSL BIO wrappers over C standard file streams (`BIO_new_fp`).
- **`2.c`**: BIO wrappers over low-level Unix file descriptors (`BIO_new_fd`).
- **`3.c` & `4.c`**: Crypto BIO chaining (`cipher_bio` -> `buffer_bio` -> `file_bio`) for transparent file encryption/decryption pipelines.
- **`5/`**: High-level Socket BIO connections (`BIO_new_accept`, `BIO_new_connect`) abstracting raw socket boilerplate.

### 6. [`OpenSSL_Crypto_API`](./OpenSSL_Crypto_API)
C-level cryptographic performance profiling using OpenSSL High-Level Cipher APIs (EVP).
- **`crypto_common.h`**: Common helper routines for OpenSSL error handling, random key/IV generation, timing measurements (`timespec`), and file encryption/decryption routines (`EVP_EncryptInit_ex`, `EVP_EncryptUpdate`, `EVP_EncryptFinal_ex`).
- **Cipher Implementations**: Implementations for `aes128_cbc.c`, `aes192_cbc.c`, `aes256_cbc.c`, and `3des_cbc.c`.
- **`run.sh`**: Automated compilation and comparative execution speed benchmarking script.

### 7. [`OpenSSL`](./OpenSSL)
Applied cryptographic lab experiments utilizing OpenSSL CLI tools and custom utility scripts.
- **`1/`**: Symmetric encryption and decryption using 3DES (`openssl des3`).
- **`2/`**: Visual comparison of AES-128 ECB vs CBC modes on bitmap images (`Sample.bmp`), highlighting ECB structural pattern leakage.
- **`3/`**: Digital signature creation and verification workflow using DSA keys (`openssl gendsa`, `openssl dgst -sign`).
- **`4/`**: Custom C++ implementation of Base64 encoding/decoding verified against OpenSSL's base64 command line.
- **`5/` & `6/`**: Endianness testing (`htons`) and interaction analysis between blocking signals (`sigprocmask`) and signal suspension (`sigsuspend`).

### 8. [`SSL_Certificates`](./SSL_Certificates)
Public Key Infrastructure (PKI) management and secure socket communication.
- **`generate_ssl_certificate.sh`**: Shell script automating RSA key pair generation, CSR creation, X.509 v3 extension configuration, and self-signed certificate generation.
- **`https_server.c`**: Lightweight embedded HTTPS web server in C built using OpenSSL TLS socket contexts (`SSL_CTX`, `TLS_server_method`, `SSL_accept`).
- **`setup_https_server.sh`**: Helper script to compile dependencies and run the HTTPS server on port `4443`.

### 9. [`IP_Tables`](./IP_Tables)
Shell scripts demonstrating host firewall management using Linux `iptables`.
- **`1.sh`**: Resetting packet counters (`iptables -Z`) and displaying detailed verbose rule statistics (`iptables -L -n -v`).
- **`2.sh`**: Blocking incoming and outgoing ICMP echo requests and replies.
- **`3.sh`**: Network interface traffic accounting for Ethernet (`eth0`) and Wi-Fi (`wlan0`/`wl01`).
- **`4.sh`**: NAT PREROUTING port redirection (forwarding incoming traffic from port 108 to SSH port 22).
- **`5.sh`**: Filtering incoming packets from specific external IP addresses.
- **`6.sh`**: Configuring kernel logging for outgoing ICMP traffic using custom log prefixes.
- **`7.sh`**: Mitigating SSH brute-force attacks using `state` and `recent` match modules for connection rate-limiting.

### 10. [`Network_security_using_snort`](./Network_security_using_snort)
Network Intrusion Detection System (NIDS) rule configuration and threat identification.
- **`rules`**: Custom Snort rules (`local.rules`) written to generate alerts for:
  - TCP SYN Port Scanning (`threshold: count 20, seconds 10`)
  - ICMP Flood Attacks (`threshold: count 10, seconds 1`)
  - SSH Brute-Force Attempts (`threshold: count 3, seconds 30`)
- **`explanation`**: Technical breakdown of signature-based detection, protocol analysis, threshold tracking, and alert mechanics.

---

## Resource Reference
- **TCP State Transition Diagram**: See [`TCP_state_transition_diagram.png`](./TCP_state_transition_diagram.png) at the repository root for visual reference on TCP connection lifecycles (SYN, ESTABLISHED, FIN-WAIT, TIME-WAIT, etc.).
- **Unix Network Programming, Volume 1: The Sockets Networking API** [Book](https://github.com/ben-elbert/books/blob/master/UNIX%20Network%20Programming,%20Volume%201,%20Third%20Edition,%20The%20Sockets%20Networking%20API.pdf)

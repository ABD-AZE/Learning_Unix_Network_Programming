## Compilation

```bash
make
```

This compiles both server and client programs.

## Usage

### Server
Start the server:
```bash
./server
```
The server listens on port 9877.

### Client
Run the client:
```bash
./client <server_ip>
```

The client will prompt you to:
1. Choose between registration (1) or login with file request (2)
2. Enter username and password
3. If option 2 is selected, enter the filename to request

### Example Session

1. Register a new user:
   - Run: `./client 127.0.0.1`
   - Choose option: `1`
   - Enter username and password

2. Login and request a file:
   - Run: `./client 127.0.0.1`
   - Choose option: `2`
   - Enter username and password
   - Enter filename (e.g., `test.txt`)

## Database Format

The `users` file stores user data in the format:
```
username:salt_hex:password_hash_hex
```

Where:
- `salt_hex`: 16-byte salt in hexadecimal
- `password_hash_hex`: SHA-256 hash of (password + salt) in hexadecimal

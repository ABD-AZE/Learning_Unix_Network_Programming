#!/bin/bash

# HTTPS Web Server Setup and Run Script

set -e

CERT_DIR="./ssl_certificates"

echo "Step 1: Checking for SSL certificates..."
if [ ! -f "$CERT_DIR/server-cert.pem" ] || [ ! -f "$CERT_DIR/server-key.pem" ]; then
    echo "Error: SSL certificates not found"
    echo "Please run ./generate_ssl_certificate.sh first"
    exit 1
fi

echo ""
echo "Step 2: Checking for OpenSSL development libraries..."
if ! pkg-config --exists openssl; then
    echo "Warning: OpenSSL development libraries may not be installed"
    echo "Install with: sudo apt-get install libssl-dev"
fi

echo ""
echo "Step 3: Compiling C HTTPS server..."
gcc -o https_server https_server.c -lssl -lcrypto

echo ""
echo "Step 4: Starting HTTPS server..."
echo "Access at: https://localhost:4443"
echo "Press Ctrl+C to stop"
echo ""
./https_server

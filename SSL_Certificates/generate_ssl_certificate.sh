#!/bin/bash

# Self-Signed SSL Certificate Generation Script

set -e

CERT_DIR="./ssl_certificates"
CONFIG_FILE="./openssl.cnf"
DAYS_VALID=365

echo "Step 1: Checking OpenSSL installation..."
if ! command -v openssl &> /dev/null; then
    echo "Error: OpenSSL is not installed"
    exit 1
fi
openssl version

echo ""
echo "Step 2: Creating certificate directory..."
mkdir -p "$CERT_DIR"

echo ""
echo "Step 3: Checking configuration file..."
if [ ! -f "$CONFIG_FILE" ]; then
    echo "Error: Configuration file not found: $CONFIG_FILE"
    exit 1
fi

echo ""
echo "Step 4: Generating RSA private key (2048 bits)..."
openssl genrsa -out "$CERT_DIR/server-key.pem" 2048

echo ""
echo "Step 5: Generating Certificate Signing Request (CSR)..."
openssl req -new \
    -key "$CERT_DIR/server-key.pem" \
    -out "$CERT_DIR/server-csr.pem" \
    -config "$CONFIG_FILE"

echo ""
echo "Step 6: Generating self-signed certificate (valid for $DAYS_VALID days)..."
openssl x509 -req \
    -days "$DAYS_VALID" \
    -in "$CERT_DIR/server-csr.pem" \
    -signkey "$CERT_DIR/server-key.pem" \
    -out "$CERT_DIR/server-cert.pem" \
    -extensions v3_ca \
    -extfile "$CONFIG_FILE"

echo ""
echo "Step 7: Setting permissions..."
chmod 600 "$CERT_DIR/server-key.pem"
chmod 644 "$CERT_DIR/server-cert.pem"

echo ""
echo "Step 8: Certificate Information:"
openssl x509 -in "$CERT_DIR/server-cert.pem" -text -noout | grep -A 2 "Subject:"
openssl x509 -in "$CERT_DIR/server-cert.pem" -text -noout | grep -A 1 "Validity"

echo ""
echo "Certificate generation complete!"
echo "Files created in $CERT_DIR/"

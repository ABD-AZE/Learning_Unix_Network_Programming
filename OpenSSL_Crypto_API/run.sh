#!/bin/bash

# Compilation flags
CFLAGS="-Wall -Wextra -O2"
LDFLAGS="-lssl -lcrypto"

# Function to compile a program
compile_program() {
    local SOURCE=$1
    local OUTPUT=$2
    
    gcc $CFLAGS -o "$OUTPUT" "$SOURCE" $LDFLAGS 2>/dev/null
}

# Compile all programs
FAILED=0

compile_program "aes256_cbc.c" "aes256_cbc" || FAILED=1
compile_program "aes192_cbc.c" "aes192_cbc" || FAILED=1
compile_program "aes128_cbc.c" "aes128_cbc" || FAILED=1
compile_program "3des_cbc.c" "3des_cbc" || FAILED=1

if [ $FAILED -eq 0 ]; then
compile_program "aes256_cbc.c" "aes256_cbc" || FAILED=1
compile_program "aes192_cbc.c" "aes192_cbc" || FAILED=1
compile_program "aes128_cbc.c" "aes128_cbc" || FAILED=1
compile_program "3des_cbc.c" "3des_cbc" || FAILED=1
fi

# Run timing comparison if all compiled successfully
if [ $FAILED -eq 0 ]; then
    echo "Compiled successfully. Running timing comparison..."
    PLAINTEXT_FILE=${1:-plaintext.txt}

    if [ ! -f "$PLAINTEXT_FILE" ]; then
        for i in {1..16384}; do
            echo "This is line $i of the sample plaintext file for encryption testing. The quick brown fox jumps over the lazy dog. 0123456789"
        done > "$PLAINTEXT_FILE"
    fi

    PROGRAMS=("aes256_cbc" "aes192_cbc" "aes128_cbc" "3des_cbc")

    for PROGRAM in "${PROGRAMS[@]}"; do
        if [ -f "./$PROGRAM" ]; then
            OUTPUT=$(./$PROGRAM "$PLAINTEXT_FILE" 2>&1)
            if [ $? -eq 0 ]; then
                echo "$OUTPUT" | grep "TIMING_RESULT:"
            fi
        fi
    done
fi

exit $FAILED
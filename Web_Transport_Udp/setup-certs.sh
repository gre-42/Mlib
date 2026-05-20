#!/bin/bash -eu

cd "$(dirname "${BASH_SOURCE[0]}")"

cd certs

# Configuration
CERT_FILE="cert.pem"
KEY_FILE="key.pem"
HASH_FILE="wt-hash.json"
DAYS_VALID=10
REGEN_THRESHOLD_DAYS=7

generate_certs() {
    echo "🔄 Generating new ${DAYS_VALID}-day ECDSA WebTransport certificates..."
    
    # 1. Generate compliant ECDSA key and cert
    openssl req -newkey ec -pkeyopt ec_paramgen_curve:prime256v1 \
        -keyout "$KEY_FILE" -nodes \
        -x509 -days "$DAYS_VALID" -out "$CERT_FILE" \
        -subj "/CN=localhost" \
        -addext "subjectAltName = DNS:localhost, IP:127.0.0.1" 2>/dev/null

    # 2. Compute the exact SHA-256 fingerprint Chrome requires
    FINGERPRINT=$(openssl x509 -in "$CERT_FILE" -outform der | \
                  openssl dgst -sha256 -binary | base64)

    echo "✅ Certificates generated successfully!"
    echo -e "\n📋 Your Chrome JS Fingerprint value:\n👉 $FINGERPRINT\n"

    # 3. Save to JSON for your frontend environment to consume
    echo "{\"hash\": \"$FINGERPRINT\"}" > "$HASH_FILE"
}

# Check if certificates exist
if [ ! -f "$CERT_FILE" ] || [ ! -f "$KEY_FILE" ]; then
    generate_certs
else
    # Calculate age of the existing certificate in days (works on Linux and macOS)
    if [[ "$OSTYPE" == "darwin"* ]]; then
        FILE_MOD_TIME=$(stat -f "%m" "$CERT_FILE")
    else
        FILE_MOD_TIME=$(stat -c "%Y" "$CERT_FILE")
    fi
    
    CURRENT_TIME=$(date +%s)
    AGE_DAYS=$(( (CURRENT_TIME - FILE_MOD_TIME) / 86400 ))

    if [ "$AGE_DAYS" -ge "$REGEN_THRESHOLD_DAYS" ]; then
        echo "⏳ Current certificates are $AGE_DAYS days old (Threshold: $REGEN_THRESHOLD_DAYS)."
        generate_certs
    else
        echo "💪 Certificates are still fresh ($AGE_DAYS days old). Skipping generation."
        
        # Read the current fingerprint from the existing cert just in case
        FINGERPRINT=$(openssl x509 -in "$CERT_FILE" -outform der | \
                      openssl dgst -sha256 -binary | base64)

        echo -e "\n📋 Current Chrome JS Fingerprint value:\n👉 $FINGERPRINT\n"
    fi
fi

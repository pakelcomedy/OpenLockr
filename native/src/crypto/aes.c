// native/src/crypto/aes.c
// AES-256-CBC encryption/decryption using OpenSSL EVP interface.

#include "aes.h"
#include <openssl/evp.h>
#include <openssl/err.h>
#include <stdlib.h>
#include <string.h>

/**
 * Encrypt plaintext using AES-256-CBC.
 *
 * @param key           Pointer to 32-byte AES key.
 * @param key_len       Length of key (must be 32).
 * @param iv            Pointer to 16-byte IV.
 * @param plaintext     Pointer to input data.
 * @param plaintext_len Length of input data.
 * @param ciphertext    Pointer to output buffer (must be at least plaintext_len + 16).
 * @return Number of bytes written to ciphertext, or -1 on error.
 */
int aes_256_cbc_encrypt(const uint8_t *key, size_t key_len,
                        const uint8_t *iv,
                        const uint8_t *plaintext, size_t plaintext_len,
                        uint8_t *ciphertext)
{
    if (!key || key_len != 32 || !iv || !plaintext || !ciphertext) {
        return -1;
    }

    int len = 0, ciphertext_len = 0;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return -1;
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // EVP_EncryptUpdate can be called multiple times if needed
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, (int)plaintext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len = len;

    // Finalize encryption (writes any remaining bytes)
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

/**
 * Decrypt ciphertext using AES-256-CBC.
 *
 * @param key           Pointer to 32-byte AES key.
 * @param key_len       Length of key (must be 32).
 * @param iv            Pointer to 16-byte IV.
 * @param ciphertext    Pointer to input data.
 * @param ciphertext_len Length of input data.
 * @param plaintext     Pointer to output buffer (must be at least ciphertext_len).
 * @return Number of bytes written to plaintext, or -1 on error.
 */
int aes_256_cbc_decrypt(const uint8_t *key, size_t key_len,
                        const uint8_t *iv,
                        const uint8_t *ciphertext, size_t ciphertext_len,
                        uint8_t *plaintext)
{
    if (!key || key_len != 32 || !iv || !ciphertext || !plaintext) {
        return -1;
    }

    int len = 0, plaintext_len = 0;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return -1;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, (int)ciphertext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}
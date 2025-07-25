// native/src/crypto/aes.h
// AES-256-CBC encryption/decryption interface for OpenLockr.
// Uses OpenSSL EVP under the hood.
//
// Functions return the number of bytes written on success, or -1 on error.

#ifndef OPENLOCKR_AES_H
#define OPENLOCKR_AES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encrypt plaintext using AES-256-CBC.
 *
 * @param key             Pointer to a 32-byte (256‑bit) AES key.
 * @param key_len         Length of the key; must be 32.
 * @param iv              Pointer to a 16-byte (128‑bit) initialization vector.
 * @param plaintext       Pointer to the input data to encrypt.
 * @param plaintext_len   Length in bytes of the input data.
 * @param ciphertext      Pointer to an output buffer to receive ciphertext.
 *                        Must be at least plaintext_len + AES_BLOCK_SIZE bytes.
 * @return The number of bytes written to ciphertext (≥ 1), or -1 on error.
 */
int aes_256_cbc_encrypt(const uint8_t *key,
                        size_t key_len,
                        const uint8_t *iv,
                        const uint8_t *plaintext,
                        size_t plaintext_len,
                        uint8_t *ciphertext);

/**
 * Decrypt ciphertext using AES-256-CBC.
 *
 * @param key              Pointer to a 32-byte (256‑bit) AES key.
 * @param key_len          Length of the key; must be 32.
 * @param iv               Pointer to a 16-byte (128‑bit) initialization vector.
 * @param ciphertext       Pointer to the input data to decrypt.
 * @param ciphertext_len   Length in bytes of the input data.
 * @param plaintext        Pointer to an output buffer to receive plaintext.
 *                         Must be at least ciphertext_len bytes.
 * @return The number of bytes written to plaintext (≥ 0), or -1 on error.
 */
int aes_256_cbc_decrypt(const uint8_t *key,
                        size_t key_len,
                        const uint8_t *iv,
                        const uint8_t *ciphertext,
                        size_t ciphertext_len,
                        uint8_t *plaintext);

#ifdef __cplusplus
}
#endif

#endif // OPENLOCKR_AES_H

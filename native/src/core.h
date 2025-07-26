// native/src/core.h
// Core API for OpenLockr: initialization, encrypt/decrypt, storage & sync.
// All functions return OLKR_OK (0) on success or a non-zero OLKR_ERR_* code on failure.

#ifndef OPENLOCKR_CORE_H
#define OPENLOCKR_CORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=============================================================================
  Return codes
=============================================================================*/
#define OLKR_OK               0   ///< Success
#define OLKR_ERR_INVALID_ARG  1   ///< One or more arguments were NULL or invalid
#define OLKR_ERR_OOM          2   ///< Memory allocation failed
#define OLKR_ERR_CRYPTO       3   ///< Encryption/decryption or key derivation error
#define OLKR_ERR_STORAGE      4   ///< Local storage (file or DB) error
#define OLKR_ERR_SYNC         5   ///< Cloud sync (Firestore) error
#define OLKR_ERR_NOT_FOUND    6   ///< Requested entry not found locally or remotely

/*=============================================================================
  Internal helpers (used by core.c; not part of the public API)
=============================================================================*/

/**
 * Derive key via PBKDF2‑HMAC‑SHA256
 * (wrapper around OpenSSL PKCS5_PBKDF2_HMAC or your own implementation).
 */
int pbkdf2_hmac_sha256(
    const char *password, size_t password_len,
    const uint8_t *salt,   size_t salt_len,
    uint32_t iterations,
    uint8_t *out_key,      size_t key_len
);

/**
 * Upload Base64 ciphertext to Firestore under `id`.
 */
int firestore_sync_upload(const char *id, const char *b64_cipher);

/**
 * Download Base64 ciphertext from Firestore for `id`.
 * Allocates *out_b64_cipher via malloc(); caller must free().
 */
int firestore_sync_download(const char *id, char **out_b64_cipher);


/*=============================================================================
  Public API
=============================================================================*/

/**
 * Initialize OpenLockr core with the given master password.
 * Must be called once before any other operations.
 *
 * Internally performs:
 *  - PBKDF2-HMAC-SHA256 key derivation (AES-256 key)
 *  - Zero-initialization of IV (or other IV init)
 *  - Opening/creating local database
 *
 * @param master_password  Null-terminated master password string.
 * @return OLKR_OK on success, or OLKR_ERR_* on failure.
 */
int openlockr_init(const char *master_password);

/**
 * Clean up OpenLockr core.
 * Closes local database, wipes key material from memory.
 * After cleanup, openlockr_init() must be called again before reuse.
 */
void openlockr_cleanup(void);

/**
 * Encrypt a UTF-8 plaintext string into a Base64-encoded ciphertext.
 *
 * Allocates a null-terminated output string via malloc(). Caller must free().
 *
 * @param plain      Null-terminated input plaintext.
 * @param out_b64    Pointer to char*; on success *out_b64 = malloc’d Base64 string.
 * @return OLKR_OK on success, or OLKR_ERR_* on failure.
 */
int openlockr_lock(const char *plain, char **out_b64);

/**
 * Decrypt a Base64-encoded ciphertext back into a UTF-8 plaintext.
 *
 * Allocates a null-terminated output string via malloc(). Caller must free().
 *
 * @param b64_cipher Null-terminated Base64 ciphertext.
 * @param out_plain  Pointer to char*; on success *out_plain = malloc’d plaintext.
 * @return OLKR_OK on success, or OLKR_ERR_* on failure.
 */
int openlockr_unlock(const char *b64_cipher, char **out_plain);

/**
 * Save an encrypted entry identified by `id` to both local storage and Firestore.
 *
 * @param id         Null-terminated unique entry identifier (e.g., UUID).
 * @param b64_cipher Null-terminated Base64 ciphertext for this entry.
 * @return OLKR_OK on success, or OLKR_ERR_STORAGE / OLKR_ERR_SYNC on failure.
 */
int openlockr_save_entry(const char *id, const char *b64_cipher);

/**
 * Load an entry by `id`, decrypting and returning plaintext.
 *
 * Behavior:
 *  1) Attempt to load Base64 ciphertext from local storage.
 *  2) If not found locally, download from Firestore and cache locally.
 *  3) Base64-decode & decrypt.
 *
 * Allocates a null-terminated output string via malloc(). Caller must free().
 *
 * @param id         Null-terminated unique entry identifier.
 * @param out_plain  Pointer to char*; on success *out_plain = malloc’d plaintext.
 * @return OLKR_OK on success,
 *         OLKR_ERR_NOT_FOUND if entry missing,
 *         or OLKR_ERR_* on other failures.
 */
int openlockr_load_entry(const char *id, char **out_plain);

#ifdef __cplusplus
}
#endif

#endif // OPENLOCKR_CORE_H

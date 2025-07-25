// native/src/core.c

#include "core.h"
#include "crypto/aes.h"
#include "storage/localdb.h"
#include "sync/firestore_sync.h"
#include "utils/base64.h"

#include <stdlib.h>
#include <string.h>

#define MASTER_SALT       "OpenLockrSaltValue"  // you should choose a secure, unique salt
#define MASTER_SALT_LEN   (sizeof(MASTER_SALT) - 1)
#define KEY_LEN_BYTES     32                     // AES-256
#define IV_LEN_BYTES      16                     // AES block size

// Global context holding derived key & default IV
static struct {
    uint8_t key[KEY_LEN_BYTES];
    uint8_t iv[IV_LEN_BYTES];
    int     initialized;
} g_ctx = { {0}, {0}, 0 };

/**
 * Initialize the OpenLockr core with a master password.
 * Derives AES key via PBKDF2-HMAC-SHA256, stores in g_ctx.
 */
int openlockr_init(const char *master_password) {
    if (!master_password) return OLKR_ERR_INVALID_ARG;

    // Derive KEY_LEN_BYTES key using PBKDF2(master_password, MASTER_SALT)
    int rc = pbkdf2_hmac_sha256(
        (const uint8_t *)master_password,
        strlen(master_password),
        (const uint8_t *)MASTER_SALT,
        MASTER_SALT_LEN,
        /*iterations=*/100000,
        g_ctx.key,
        KEY_LEN_BYTES
    );
    if (rc != 0) return OLKR_ERR_CRYPTO;

    // Initialize IV to zeros or derive per-entry if you prefer
    memset(g_ctx.iv, 0, IV_LEN_BYTES);

    // Initialize local DB
    rc = localdb_init();
    if (rc != 0) return OLKR_ERR_STORAGE;

    g_ctx.initialized = 1;
    return OLKR_OK;
}

/**
 * Encrypt plaintext into a Base64-encoded ciphertext.
 * Caller must free(*out_b64).
 */
int openlockr_lock(const char *plain, char **out_b64) {
    if (!g_ctx.initialized || !plain || !out_b64) return OLKR_ERR_INVALID_ARG;

    size_t plain_len = strlen(plain);
    // Allocate buffer for cipher: ciphertext length = plain_len + AES block size
    size_t cipher_buf_len = plain_len + IV_LEN_BYTES;
    uint8_t *cipher_buf = malloc(cipher_buf_len);
    if (!cipher_buf) return OLKR_ERR_OOM;

    // Perform AES-256-CBC encryption
    int cipher_len = aes_256_cbc_encrypt(
        g_ctx.key, IV_LEN_BYTES, g_ctx.iv,
        (const uint8_t *)plain, plain_len,
        cipher_buf
    );
    if (cipher_len < 0) {
        free(cipher_buf);
        return OLKR_ERR_CRYPTO;
    }

    // Base64-encode the cipher blob
    size_t b64_len = 0;
    *out_b64 = base64_encode(cipher_buf, cipher_len, &b64_len);
    free(cipher_buf);
    if (!*out_b64) return OLKR_ERR_CRYPTO;

    return OLKR_OK;
}

/**
 * Decrypt a Base64-encoded ciphertext back into plaintext.
 * Caller must free(*out_plain).
 */
int openlockr_unlock(const char *b64_cipher, char **out_plain) {
    if (!g_ctx.initialized || !b64_cipher || !out_plain) return OLKR_ERR_INVALID_ARG;

    // Base64-decode
    size_t cipher_len = 0;
    uint8_t *cipher_buf = base64_decode(b64_cipher, strlen(b64_cipher), &cipher_len);
    if (!cipher_buf) return OLKR_ERR_CRYPTO;

    // Allocate plaintext buffer
    size_t plain_buf_len = cipher_len;
    uint8_t *plain_buf = malloc(plain_buf_len + 1);
    if (!plain_buf) {
        free(cipher_buf);
        return OLKR_ERR_OOM;
    }

    // AES-256-CBC decrypt
    int dec_len = aes_256_cbc_decrypt(
        g_ctx.key, IV_LEN_BYTES, g_ctx.iv,
        cipher_buf, cipher_len,
        plain_buf
    );
    free(cipher_buf);
    if (dec_len < 0) {
        free(plain_buf);
        return OLKR_ERR_CRYPTO;
    }

    // Null-terminate and return
    plain_buf[dec_len] = '\0';
    *out_plain = (char *)plain_buf;
    return OLKR_OK;
}

/**
 * Save a locked entry to local database, and push to Firestore.
 */
int openlockr_save_entry(const char *id, const char *b64_cipher) {
    if (!g_ctx.initialized || !id || !b64_cipher) return OLKR_ERR_INVALID_ARG;

    int rc = localdb_put_entry(id, b64_cipher);
    if (rc != 0) return OLKR_ERR_STORAGE;

    rc = firestore_sync_upload(id, b64_cipher);
    if (rc != 0) return OLKR_ERR_SYNC;

    return OLKR_OK;
}

/**
 * Load an entry: first try local DB, if missing, fetch from Firestore.
 * Output plaintext via out_plain (caller must free).
 */
int openlockr_load_entry(const char *id, char **out_plain) {
    if (!g_ctx.initialized || !id || !out_plain) return OLKR_ERR_INVALID_ARG;

    char *b64_cipher = NULL;
    int rc = localdb_get_entry(id, &b64_cipher);
    if (rc == OLKR_ERR_NOT_FOUND) {
        // Try Firestore
        rc = firestore_sync_download(id, &b64_cipher);
        if (rc != OLKR_OK) return rc;
        // Save to local DB for caching
        localdb_put_entry(id, b64_cipher);
    } else if (rc != OLKR_OK) {
        return OLKR_ERR_STORAGE;
    }

    // Decrypt
    rc = openlockr_unlock(b64_cipher, out_plain);
    free(b64_cipher);
    return rc;
}

/**
 * Clean up resources.
 */
void openlockr_cleanup() {
    if (!g_ctx.initialized) return;
    localdb_close();
    // Zero out key material
    memset(&g_ctx, 0, sizeof(g_ctx));
}

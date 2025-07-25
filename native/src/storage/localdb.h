// native/src/storage/localdb.h
// Local storage backend API for OpenLockr using SQLite3 key–value store.

#ifndef OPENLOCKR_LOCALDB_H
#define OPENLOCKR_LOCALDB_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the local database.
 * Opens (or creates) the SQLite file and ensures the `entries` table exists.
 *
 * @return 0 on success, non-zero on error.
 */
int localdb_init(void);

/**
 * Close the local database, freeing resources.
 */
void localdb_close(void);

/**
 * Store or update an entry in the local database.
 *
 * @param id         Null-terminated unique entry identifier.
 * @param b64_cipher Null-terminated Base64-encoded ciphertext.
 * @return 0 on success, non-zero on error.
 */
int localdb_put_entry(const char *id, const char *b64_cipher);

/**
 * Retrieve an entry's Base64-encoded ciphertext from the local database.
 *
 * @param id       Null-terminated unique entry identifier.
 * @param out_b64  Pointer-to-pointer; on success *out_b64 will be set to a
 *                 malloc()’d null-terminated string containing the ciphertext.
 *                 Caller must free(*out_b64).
 * @return  0 on success,
 *         -2 if the entry is not found,
 *         -1 on other errors.
 */
int localdb_get_entry(const char *id, char **out_b64);

#ifdef __cplusplus
}
#endif

#endif // OPENLOCKR_LOCALDB_H

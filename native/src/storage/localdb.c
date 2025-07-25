// native/src/storage/localdb.c
// Local storage backend for OpenLockr using SQLite3.
// Implements simple key–value store: table `entries(id TEXT PRIMARY KEY, cipher TEXT)`.

#include "localdb.h"
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

#define DB_FILENAME    "openlockr.db"
#define SQL_CREATE     "CREATE TABLE IF NOT EXISTS entries (id TEXT PRIMARY KEY, cipher TEXT);"
#define SQL_INSERT     "INSERT OR REPLACE INTO entries (id, cipher) VALUES (?, ?);"
#define SQL_SELECT     "SELECT cipher FROM entries WHERE id = ?;"

static sqlite3 *g_db = NULL;

/**
 * Initialize the local SQLite database.
 * Opens (or creates) DB_FILENAME in working directory and ensures table exists.
 */
int localdb_init(void) {
    int rc = sqlite3_open(DB_FILENAME, &g_db);
    if (rc != SQLITE_OK) {
        sqlite3_close(g_db);
        g_db = NULL;
        return -1;
    }
    rc = sqlite3_exec(g_db, SQL_CREATE, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(g_db);
        g_db = NULL;
        return -1;
    }
    return 0;
}

/**
 * Close the local database.
 */
void localdb_close(void) {
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}

/**
 * Store or update an entry in local DB.
 *
 * @param id         Null-terminated entry identifier.
 * @param b64_cipher Null-terminated Base64 ciphertext.
 * @return 0 on success, non-zero on error.
 */
int localdb_put_entry(const char *id, const char *b64_cipher) {
    if (!g_db || !id || !b64_cipher) return -1;

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(g_db, SQL_INSERT, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }
    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, b64_cipher, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        return -1;
    }
    return 0;
}

/**
 * Retrieve an entry's Base64 ciphertext by id.
 *
 * @param id        Null-terminated entry identifier.
 * @param out_b64   Pointer-to-pointer; on success *out_b64 = malloc'd cipher string.
 *                  Caller must free().
 * @return  0 on success,
 *         -2 if not found,
 *         -1 on other errors.
 */
int localdb_get_entry(const char *id, char **out_b64) {
    if (!g_db || !id || !out_b64) return -1;

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(g_db, SQL_SELECT, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }
    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        if (text) {
            size_t len = strlen((const char *)text);
            *out_b64 = (char *)malloc(len + 1);
            if (!*out_b64) {
                sqlite3_finalize(stmt);
                return -1;
            }
            memcpy(*out_b64, text, len + 1);
            sqlite3_finalize(stmt);
            return 0;
        }
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    return -2;  // not found
}

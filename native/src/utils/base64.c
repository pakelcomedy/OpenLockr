// native/src/utils/base64.c
// Minimal Base64 encode/decode implementation for OpenLockr.
// Provides malloc()-allocated output; caller must free().

#include "base64.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Base64 character set
static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Reverse lookup table (built at runtime)
static uint8_t b64_rev[256];
static bool    b64_rev_inited = false;

// Build reverse lookup table once
static void init_b64_rev(void) {
    if (b64_rev_inited) return;
    memset(b64_rev, 0xFF, sizeof(b64_rev));
    for (int i = 0; i < 64; i++) {
        b64_rev[(unsigned char)b64_table[i]] = (uint8_t)i;
    }
    b64_rev_inited = true;
}

char *base64_encode(const uint8_t *data, size_t len, size_t *out_len) {
    if (!data || !out_len) return NULL;

    // Calculate output length: 4 * ceil(len/3)
    size_t enc_len = ((len + 2) / 3) * 4;
    char *enc = malloc(enc_len + 1);
    if (!enc) return NULL;

    size_t di = 0, ei = 0;
    while (di < len) {
        uint32_t a = di < len ? data[di++] : 0;
        uint32_t b = di < len ? data[di++] : 0;
        uint32_t c = di < len ? data[di++] : 0;
        uint32_t triple = (a << 16) | (b << 8) | c;

        enc[ei++] = b64_table[(triple >> 18) & 0x3F];
        enc[ei++] = b64_table[(triple >> 12) & 0x3F];
        enc[ei++] = b64_table[(triple >> 6 ) & 0x3F];
        enc[ei++] = b64_table[ triple         & 0x3F];
    }

    // Add padding if needed
    int mod = len % 3;
    if (mod) {
        enc[enc_len - 1] = '=';
        if (mod == 1) enc[enc_len - 2] = '=';
    }

    enc[enc_len] = '\0';
    *out_len = enc_len;
    return enc;
}

uint8_t *base64_decode(const char *b64, size_t len, size_t *out_len) {
    if (!b64 || !out_len || (len % 4) != 0) return NULL;
    init_b64_rev();

    // Count padding
    size_t pad = 0;
    if (len > 0 && b64[len - 1] == '=') pad++;
    if (len > 1 && b64[len - 2] == '=') pad++;

    size_t dec_len = (len / 4) * 3 - pad;
    uint8_t *dec = malloc(dec_len);
    if (!dec) return NULL;

    size_t di = 0, bi = 0;
    while (bi < len) {
        uint32_t sa = b64_rev[(unsigned char)b64[bi++]];
        uint32_t sb = b64_rev[(unsigned char)b64[bi++]];
        uint32_t sc = b64_rev[(unsigned char)b64[bi++]];
        uint32_t sd = b64_rev[(unsigned char)b64[bi++]];

        // Validate
        if (sa == 0xFF || sb == 0xFF ||
            (b64[bi-2] != '=' && sc == 0xFF) ||
            (b64[bi-1] != '=' && sd == 0xFF)) {
            free(dec);
            return NULL;
        }

        uint32_t triple = (sa << 18) | (sb << 12) | ((sc & 0x3F) << 6) | (sd & 0x3F);

        if (di < dec_len) dec[di++] = (triple >> 16) & 0xFF;
        if (di < dec_len) dec[di++] = (triple >>  8) & 0xFF;
        if (di < dec_len) dec[di++] =  triple        & 0xFF;
    }

    *out_len = dec_len;
    return dec;
}

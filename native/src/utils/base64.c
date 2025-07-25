// native/src/utils/base64.c
// Minimal Base64 encode/decode implementation for OpenLockr.
// Provides malloc()-allocated output; caller must free().

#include "base64.h"
#include <stdlib.h>
#include <string.h>

// Base64 character set
static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Reverse lookup table: maps ASCII to 6-bit value, 0xFF = invalid
static const uint8_t b64_rev[256] = {
    [0 ... 255] = 0xFF,
    ['A'] = 0,  ['B'] = 1,  ['C'] = 2,  ['D'] = 3,
    ['E'] = 4,  ['F'] = 5,  ['G'] = 6,  ['H'] = 7,
    ['I'] = 8,  ['J'] = 9,  ['K'] = 10, ['L'] = 11,
    ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15,
    ['Q'] = 16, ['R'] = 17, ['S'] = 18, ['T'] = 19,
    ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
    ['Y'] = 24, ['Z'] = 25,
    ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29,
    ['e'] = 30, ['f'] = 31, ['g'] = 32, ['h'] = 33,
    ['i'] = 34, ['j'] = 35, ['k'] = 36, ['l'] = 37,
    ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41,
    ['q'] = 42, ['r'] = 43, ['s'] = 44, ['t'] = 45,
    ['u'] = 46, ['v'] = 47, ['w'] = 48, ['x'] = 49,
    ['y'] = 50, ['z'] = 51,
    ['0'] = 52, ['1'] = 53, ['2'] = 54, ['3'] = 55,
    ['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59,
    ['8'] = 60, ['9'] = 61,
    ['+'] = 62, ['/'] = 63
};

char *base64_encode(const uint8_t *data, size_t len, size_t *out_len) {
    if (!data || !out_len) return NULL;

    // Calculate output length: 4 * ceil(len/3)
    size_t enc_len = ((len + 2) / 3) * 4;
    char *enc = (char *)malloc(enc_len + 1);
    if (!enc) return NULL;

    size_t di = 0, ei = 0;
    while (di < len) {
        uint32_t octet_a = di < len ? data[di++] : 0;
        uint32_t octet_b = di < len ? data[di++] : 0;
        uint32_t octet_c = di < len ? data[di++] : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        enc[ei++] = b64_table[(triple >> 18) & 0x3F];
        enc[ei++] = b64_table[(triple >> 12) & 0x3F];
        enc[ei++] = b64_table[(triple >> 6) & 0x3F];
        enc[ei++] = b64_table[triple & 0x3F];
    }

    // Add padding '=' if needed
    int mod = len % 3;
    if (mod) {
        enc[enc_len - 1] = '=';
        if (mod == 1) {
            enc[enc_len - 2] = '=';
        }
    }

    enc[enc_len] = '\0';
    *out_len = enc_len;
    return enc;
}

uint8_t *base64_decode(const char *b64, size_t len, size_t *out_len) {
    if (!b64 || !out_len) return NULL;
    if (len % 4 != 0) return NULL;  // invalid base64 length

    // Count padding
    size_t pad = 0;
    if (len >= 1 && b64[len - 1] == '=') pad++;
    if (len >= 2 && b64[len - 2] == '=') pad++;

    // Calculate decoded length
    size_t dec_len = (len / 4) * 3 - pad;
    uint8_t *dec = (uint8_t *)malloc(dec_len);
    if (!dec) return NULL;

    size_t di = 0, bi = 0;
    while (bi < len) {
        uint32_t sextet_a = b64_rev[(unsigned char)b64[bi++]];
        uint32_t sextet_b = b64_rev[(unsigned char)b64[bi++]];
        uint32_t sextet_c = b64_rev[(unsigned char)b64[bi++]];
        uint32_t sextet_d = b64_rev[(unsigned char)b64[bi++]];

        if (sextet_a == 0xFF || sextet_b == 0xFF ||
            (b64[bi-2] != '=' && sextet_c == 0xFF) ||
            (b64[bi-1] != '=' && sextet_d == 0xFF)) {
            free(dec);
            return NULL;
        }

        uint32_t triple = (sextet_a << 18)
                        | (sextet_b << 12)
                        | ((sextet_c & 0x3F) << 6)
                        | (sextet_d & 0x3F);

        if (di < dec_len) dec[di++] = (triple >> 16) & 0xFF;
        if (di < dec_len) dec[di++] = (triple >> 8) & 0xFF;
        if (di < dec_len) dec[di++] = triple & 0xFF;
    }

    *out_len = dec_len;
    return dec;
}

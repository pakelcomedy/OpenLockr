// native/src/utils/base64.h
// Base64 encoding and decoding interface for OpenLockr.
// Provides malloc()-allocated output; caller is responsible for free().

#ifndef OPENLOCKR_BASE64_H
#define OPENLOCKR_BASE64_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encode binary data to a Base64 null-terminated string.
 *
 * Allocates via malloc() an output buffer of length 4 * ceil(len/3) + 1.
 * Caller must free() the returned pointer.
 *
 * @param data      Pointer to input binary data.
 * @param len       Length in bytes of input data.
 * @param out_len   Pointer to size_t to receive length of output (excluding NUL).
 * @return          Pointer to malloc()-allocated Base64 string (NUL terminated),
 *                  or NULL on error (invalid args or OOM).
 */
char *base64_encode(const uint8_t *data, size_t len, size_t *out_len);

/**
 * Decode a Base64 string to binary data.
 *
 * Allocates via malloc() an output buffer of exact decoded size.
 * Caller must free() the returned pointer.
 *
 * @param b64       Pointer to Base64 string (may include padding '=').
 * @param len       Length in bytes of the Base64 string.
 * @param out_len   Pointer to size_t to receive length of decoded data.
 * @return          Pointer to malloc()-allocated binary data,
 *                  or NULL on error (invalid args, bad input, or OOM).
 */
uint8_t *base64_decode(const char *b64, size_t len, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif // OPENLOCKR_BASE64_H

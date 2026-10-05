#ifndef CMAIL_BASE64_H
#define CMAIL_BASE64_H

#include <stddef.h>

/*
 * Encode arbitrary binary data into Base64.
 *
 * The returned buffer is NUL-terminated, but `output_length`
 * is the authoritative length of the encoded data.
 *
 * Returns:
 *   - allocated Base64 buffer on success
 *   - NULL on allocation failure
 */
char *cmail_base64_encode(
    const unsigned char *data,
    size_t length,
    size_t *output_length
);

#endif // CMAIL_BASE64_H

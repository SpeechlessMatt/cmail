#include "base64.h"

#include <stdint.h>
#include <stdlib.h>

static const char cmail_base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

char *cmail_base64_encode(
    const unsigned char *data,
    size_t length,
    size_t *output_length
)
{
    size_t encoded_length;
    char *output;
    size_t input_pos = 0;
    size_t output_pos = 0;

    /*
     * Base64 expands every 3 bytes into 4 bytes.
     *
     * encoded_length = 4 * ceil(length / 3)
     *
     * Check for overflow before doing the arithmetic.
     */
    if (length > (SIZE_MAX - 2) / 3) {
        return NULL;
    }

    encoded_length = ((length + 2) / 3) * 4;

    /*
     * +1 for the terminating '\0'.
     */
    if (encoded_length == SIZE_MAX) {
        return NULL;
    }

    output = malloc(encoded_length + 1);
    if (!output) {
        return NULL;
    }

    while (input_pos < length) {
        uint32_t a = data[input_pos++];
        uint32_t b = 0;
        uint32_t c = 0;

        size_t remaining = length - (input_pos - 1);

        if (remaining > 1) {
            b = data[input_pos++];
        }

        if (remaining > 2) {
            c = data[input_pos++];
        }

        uint32_t triple =
            (a << 16) |
            (b << 8) |
            c;

        output[output_pos++] =
            cmail_base64_table[(triple >> 18) & 0x3F];

        output[output_pos++] =
            cmail_base64_table[(triple >> 12) & 0x3F];

        if (remaining > 1) {
            output[output_pos++] =
                cmail_base64_table[(triple >> 6) & 0x3F];
        } else {
            output[output_pos++] = '=';
        }

        if (remaining > 2) {
            output[output_pos++] =
                cmail_base64_table[triple & 0x3F];
        } else {
            output[output_pos++] = '=';
        }
    }

    output[output_pos] = '\0';

    if (output_length) {
        *output_length = output_pos;
    }

    return output;
}

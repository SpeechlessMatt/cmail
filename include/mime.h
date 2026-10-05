#ifndef CMAIL_MIME_H
#define CMAIL_MIME_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define CMAIL_MIME_LINE_LENGTH 76

static inline char* cmail_mime_patch(const char* in_base64, size_t in_len, size_t line_len, size_t* out_len) {
    if (!in_base64 || line_len == 0) return NULL;

    if (in_len == 0) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        if (out_len) *out_len = 0;
        return empty;
    }

    size_t lines = (in_len + line_len - 1) / line_len;
    size_t new_len = in_len + (lines * 2);

    if (new_len < in_len) return NULL;

    char* wrapped = malloc(new_len + 1);
    if (!wrapped) return NULL;

    size_t src_idx = 0;
    size_t dst_idx = 0;

    while (src_idx < in_len) {
        size_t chunk = in_len - src_idx;
        if (chunk > line_len) {
            chunk = line_len;
        }

        memcpy(wrapped + dst_idx, in_base64 + src_idx, chunk);
        dst_idx += chunk;
        src_idx += chunk;

        wrapped[dst_idx++] = '\r';
        wrapped[dst_idx++] = '\n';
    }

    wrapped[dst_idx] = '\0';

    if (out_len) {
        *out_len = dst_idx;
    }

    return wrapped;
}

#endif // CMAIL_MIME_H

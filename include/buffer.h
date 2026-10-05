#ifndef CMAIL_BUFFER_H
#define CMAIL_BUFFER_H

#include <stddef.h>

#define CMAIL_BUFFER_OK 1
#define CMAIL_BUFFER_ERROR 0

struct cmail_buffer {
    char *data;
    size_t length;
    size_t capacity;
};

void cmail_buffer_init(struct cmail_buffer *buffer);

void cmail_buffer_free(struct cmail_buffer *buffer);

int cmail_buffer_append(
    struct cmail_buffer *buffer,
    const void *data,
    size_t length
);

int cmail_buffer_append_string(
    struct cmail_buffer *buffer,
    const char *string
);

#endif // CMAIL_BUFFER_H

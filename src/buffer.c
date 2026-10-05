#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "buffer.h"

#define CMAIL_BUFFER_INITIAL_CAPACITY 256

void cmail_buffer_init(struct cmail_buffer *buffer)
{
    if (!buffer) {
        return;
    }

    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}

void cmail_buffer_free(struct cmail_buffer *buffer)
{
    if (!buffer) {
        return;
    }

    free(buffer->data);

    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}

int cmail_buffer_append(
    struct cmail_buffer *buffer,
    const void *data,
    size_t length
) {
    size_t required;
    size_t new_capacity;
    char *new_data;

    if (!buffer || (!data && length != 0)) {
        return CMAIL_BUFFER_ERROR;
    }

    if (length == 0) {
        return CMAIL_BUFFER_OK;
    }

    if (buffer->length > SIZE_MAX - length - 1) {
        return CMAIL_BUFFER_ERROR;
    }

    required = buffer->length + length + 1;

    if (required <= buffer->capacity) {
        memcpy(buffer->data + buffer->length, data, length);

        buffer->length += length;
        buffer->data[buffer->length] = '\0';

        return CMAIL_BUFFER_OK;
    }

    if (buffer->capacity == 0) {
        new_capacity = CMAIL_BUFFER_INITIAL_CAPACITY;
    } else {
        new_capacity = buffer->capacity;
    }

    while (new_capacity < required) {
        if (new_capacity > SIZE_MAX / 2) {
            new_capacity = required;
            break;
        }

        new_capacity *= 2;
    }

    new_data = realloc(buffer->data, new_capacity);

    if (!new_data) {
        return CMAIL_BUFFER_ERROR;
    }

    buffer->data = new_data;
    buffer->capacity = new_capacity;

    memcpy(buffer->data + buffer->length, data, length);

    buffer->length += length;
    buffer->data[buffer->length] = '\0';

    return CMAIL_BUFFER_OK;
}

int cmail_buffer_append_string(
    struct cmail_buffer *buffer,
    const char *string
) {
    if (!string) {
        return CMAIL_BUFFER_ERROR;
    }

    return cmail_buffer_append(buffer, string, strlen(string));
}

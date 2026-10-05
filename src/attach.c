#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attach.h"
#include "base64.h"

struct cmail_attachment* cmail_attachment_new_buffer(
    const char* filename,
    const char* mime_type,
    const unsigned char* data,
    size_t length
) {
    if (!filename || !mime_type || !data || length == 0) {
        return NULL;
    }

    struct cmail_attachment* attachment = calloc(1, sizeof(struct cmail_attachment));
    if (!attachment) {
        return NULL;
    }

    attachment->filename = strdup(filename);
    attachment->mime_type = strdup(mime_type);
    attachment->type = CMAIL_ATTACH_TYPE_DATA;
    attachment->buffer.data = malloc(length);
    if (!attachment->buffer.data) {
        free(attachment->filename);
        free(attachment->mime_type);
        free(attachment);
        return NULL;
    }

    memcpy((void*)attachment->buffer.data, data, length);
    attachment->buffer.length = length;

    return attachment;
}

struct cmail_attachment* cmail_attachment_new_file(
    const char *filename,
    const char *mime_type,
    const char *filepath
) {
    if (!filename || !mime_type || !filepath) {
        return NULL;
    }

    struct cmail_attachment* attachment = calloc(1, sizeof(struct cmail_attachment));
    if (!attachment) {
        return NULL;
    }

    attachment->filename = strdup(filename);
    attachment->mime_type = strdup(mime_type);
    attachment->type = CMAIL_ATTACH_TYPE_FILE;
    attachment->file.path = strdup(filepath);

    if (!attachment->filename || !attachment->mime_type || !attachment->file.path) {
        free(attachment->filename);
        free(attachment->mime_type);
        free(attachment->file.path);
        free(attachment);
        return NULL;
    }

    return attachment;
}

int cmail_attachment_is_valid(const struct cmail_attachment* attachment) {
    if (!attachment) {
        return 0;
    }

    if (!attachment->filename || !attachment->mime_type) {
        return 0;
    }

    if (attachment->type == CMAIL_ATTACH_TYPE_DATA) {
        if (!attachment->buffer.data || attachment->buffer.length == 0) {
            return 0;
        }
    }
    else if (attachment->type == CMAIL_ATTACH_TYPE_FILE) {
        if (!attachment->file.path) {
            return 0;
        }
    }
    else {
        return 0;
    }

    return 1;
}

cmail_error_t cmail_attachment_to_base64(
    const struct cmail_attachment* attachment, 
    char** out_base64, 
    size_t* out_length
) {
    if (!out_base64 || !out_length) {
        return CMAIL_ERROR_INVALID_ARGUMENT;
    }

    *out_base64 = NULL;
    *out_length = 0;

    if (!attachment) {
        return CMAIL_ERROR_INVALID_ARGUMENT;
    }

    const unsigned char *file_data = NULL;

    unsigned char *file_buffer = NULL;
    size_t file_length = 0;

    switch (attachment->type) {
        case CMAIL_ATTACH_TYPE_DATA:
            file_data = attachment->buffer.data;
            file_length = attachment->buffer.length;
            break;

        case CMAIL_ATTACH_TYPE_FILE: {
            FILE *file;
            long file_size;

            file = fopen(attachment->file.path, "rb");
            if (!file) {
                return CMAIL_ERROR_FILE;
            }

            if (fseek(file, 0, SEEK_END) != 0) {
                fclose(file);
                return CMAIL_ERROR_IO;
            }

            file_size = ftell(file);
            if (file_size < 0) {
                fclose(file);
                return CMAIL_ERROR_IO;
            }

            if (fseek(file, 0, SEEK_SET) != 0) {
                fclose(file);
                return CMAIL_ERROR_IO;
            }

            file_length = (size_t)file_size;

            if (file_length > 0) {
                file_buffer = malloc(file_length);
                if (!file_buffer) {
                    fclose(file);
                    return CMAIL_ERROR_MEMORY;
                }

                if (fread(file_buffer, 1, file_length, file) != file_length) {
                    free(file_buffer);
                    fclose(file);
                    return CMAIL_ERROR_IO;
                }

                file_data = file_buffer;
            }

            fclose(file);
            break;
        }

        default:
            return CMAIL_ERROR_INVALID_ARGUMENT;
    }

    *out_base64 = cmail_base64_encode(
        file_data,
        file_length,
        out_length
    );

    if (attachment->type == CMAIL_ATTACH_TYPE_FILE) {
        free(file_buffer);
    }

    if (!*out_base64) {
        *out_length = 0;
        return CMAIL_ERROR_MEMORY;
    }

    return CMAIL_OK;
}

void cmail_attachment_free(struct cmail_attachment* attachment) {
    if (!attachment) {
        return;
    }

    free(attachment->filename);
    free(attachment->mime_type);

    if (attachment->type == CMAIL_ATTACH_TYPE_DATA) {
        free((void*)attachment->buffer.data);
    }
    else if (attachment->type == CMAIL_ATTACH_TYPE_FILE) {
        free(attachment->file.path);
    }

    free(attachment);
}

struct cmail_attachment* cmail_attachment_clone(const struct cmail_attachment* attachment) {
    if (!attachment) {
        return NULL;
    }

    struct cmail_attachment* clone = calloc(1, sizeof(struct cmail_attachment));
    if (!clone) {
        return NULL;
    }

    clone->type = attachment->type;

    if (attachment->filename && !(clone->filename = strdup(attachment->filename))) {
        goto clean;
    }

    if (attachment->mime_type && !(clone->mime_type = strdup(attachment->mime_type))) {
        goto clean;
    }

    if (attachment->type == CMAIL_ATTACH_TYPE_DATA) {
        clone->buffer.length = attachment->buffer.length;

        if (attachment->buffer.data && attachment->buffer.length > 0) {
            clone->buffer.data = malloc(attachment->buffer.length);
            if (!clone->buffer.data) {
                goto clean;
            }

            memcpy((void*)clone->buffer.data, attachment->buffer.data, attachment->buffer.length);
        }
    }
    else if (attachment->type == CMAIL_ATTACH_TYPE_FILE) {
        if (attachment->file.path && !(clone->file.path = strdup(attachment->file.path))) {
            goto clean;
        }
    }
    else {
        goto clean;
    }

    return clone;

clean:
    cmail_attachment_free(clone);
    return NULL;
}

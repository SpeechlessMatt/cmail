#ifndef CMAIL_ATTACH_H
#define CMAIL_ATTACH_H

#include <stdint.h>
#include <stdlib.h>

#include "error.h"

#define CMAIL_MIME_TYPE_DEFAULT "application/octet-stream"

typedef enum {
    CMAIL_ATTACH_TYPE_FILE,
    CMAIL_ATTACH_TYPE_DATA
} cmail_attachment_type_t ;

struct cmail_attachment {
    char* filename;
    char* mime_type;

    cmail_attachment_type_t type;

    union {
        struct {
            char* path;
        } file;

        struct {
            const unsigned char* data;
            size_t length;
        } buffer;
    };
};

struct cmail_attachment* cmail_attachment_new_buffer(
    const char* filename,
    const char* mime_type,
    const unsigned char* data,
    size_t length
);

struct cmail_attachment* cmail_attachment_new_file(
    const char *filename,
    const char *content_type,
    const char *filepath
);

struct cmail_attachment* cmail_attachment_clone(const struct cmail_attachment* attachment);

cmail_error_t cmail_attachment_to_base64(
    const struct cmail_attachment* attachment, 
    char** out_base64, 
    size_t* out_length
);

void cmail_attachment_free(struct cmail_attachment* attachment);

#endif // CMAIL_ATTACH_H

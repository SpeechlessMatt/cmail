#ifndef CMAIL_PAYLOAD_H
#define CMAIL_PAYLOAD_H

#include <stdint.h>
#include <string.h>

#include "error.h"

struct cmail_attachment;

struct cmail_header {
    char* from;
    char* to;
    char* subject;
};

typedef enum {
    CMAIL_BODY_TEXT,
    CMAIL_BODY_HTML
} cmail_body_type_t;

struct cmail_payload {
    union {
        struct cmail_header header;
        struct {
            char* from;
            char* to;
            char* subject;
        };
    };

    char* body;
    size_t body_length;
    cmail_body_type_t body_type;

    struct cmail_attachment** attachments;
    size_t attachment_count;
};

struct cmail_payload* cmail_payload_create(struct cmail_header* header, const char* body, size_t body_length);
struct cmail_payload* cmail_payload_create_html(struct cmail_header* header, const char* body, size_t body_length);

#define cmail_payload_create_with_attachment(header, body, body_length, attachment) \
    cmail_payload_create_with_attachments(header, body, body_length, &attachment, 1)

#define cmail_payload_create_html_with_attachment(header, body, body_length, attachment) \
    cmail_payload_create_html_with_attachments(header, body, body_length, &attachment, 1)

struct cmail_payload* cmail_payload_create_with_attachments(struct cmail_header* header, const char* body, size_t body_length, struct cmail_attachment** attachments, size_t attachment_count);

struct cmail_payload* cmail_payload_create_html_with_attachments(struct cmail_header* header, const char* body, size_t body_length, struct cmail_attachment** attachments, size_t attachment_count);

struct cmail_payload* cmail_payload_clone(const struct cmail_payload* payload);

cmail_error_t cmail_payload_build(const struct cmail_payload* payload, char** out_buffer, size_t* out_length);

void cmail_payload_clean(struct cmail_payload* payload);

#endif // CMAIL_PAYLOAD_H

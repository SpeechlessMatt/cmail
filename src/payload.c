#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "payload.h"
#include "buffer.h"
#include "attach.h"
#include "mime.h"

#define CMAIL_BOUNDARY "----cmail-boundary"

/*
 * Initializes a cmail_payload structure with the given header and body.
 * Returns a pointer to the initialized cmail_payload structure, or NULL on failure.
 */
struct cmail_payload* cmail_payload_create(
    struct cmail_header* header,
    const char* body,
    size_t body_length
) {
    if (body == NULL && header == NULL) {
        return NULL;
    }

    struct cmail_payload* payload = calloc(1, sizeof(struct cmail_payload));
    if (!payload) {
        return NULL;
    }

    if (header != NULL) {
        if (header->from && !(payload->header.from = strdup(header->from))) goto clean;
        if (header->to && !(payload->header.to = strdup(header->to))) goto clean;
        if (header->subject && !(payload->header.subject = strdup(header->subject))) goto clean;
    }

    if (body != NULL && body_length > 0) {
        payload->body = malloc(body_length + 1);
        if (!payload->body) {
            goto clean;
        }

        memcpy(payload->body, body, body_length);
        payload->body[body_length] = '\0';
        payload->body_length = body_length;
        payload->body_type = CMAIL_BODY_TEXT;
    }

    return payload;

clean:
    free(payload->header.from);
    free(payload->header.to);
    free(payload->header.subject);
    free(payload->body);
    free(payload);
    return NULL;
}

struct cmail_payload* cmail_payload_create_html(struct cmail_header* header, const char* body, size_t body_length) {
    struct cmail_payload* payload = cmail_payload_create(header, body, body_length);
    if (!payload) {
        return NULL;
    }

    payload->body_type = CMAIL_BODY_HTML;
    return payload;
}

/*
 * Initializes a cmail_payload structure with the given header, body, and attachments.
 * Returns a pointer to the initialized cmail_payload structure, or NULL on failure.
 */
struct cmail_payload* cmail_payload_create_with_attachments(
    struct cmail_header* header, 
    const char* body, 
    size_t body_length, 
    struct cmail_attachment** attachments, 
    size_t attachment_count
) {
    struct cmail_payload* payload = cmail_payload_create(header, body, body_length);
    if (!payload) {
        return NULL;
    }

    if (attachments != NULL && attachment_count > 0) {
        payload->attachments = malloc(sizeof(struct cmail_attachment*) * attachment_count);
        if (!payload->attachments) {
            cmail_payload_clean(payload);
            return NULL;
        }

        for (size_t i = 0; i < attachment_count; ++i) {
            payload->attachments[i] = attachments[i];
        }
        payload->attachment_count = attachment_count;
    }

    return payload;
}

struct cmail_payload* cmail_payload_create_html_with_attachments(struct cmail_header* header, const char* body, size_t body_length, struct cmail_attachment** attachments, size_t attachment_count) {
    struct cmail_payload* payload = cmail_payload_create_with_attachments(header, body, body_length, attachments, attachment_count);
    if (!payload) {
        return NULL;
    }

    payload->body_type = CMAIL_BODY_HTML;
    return payload;
}

cmail_error_t cmail_payload_build(
    const struct cmail_payload* payload, 
    char** out_buffer, 
    size_t* out_length
) {
    struct cmail_buffer buffer;

    if (!payload || !out_buffer || !out_length) {
        return CMAIL_ERROR_INVALID_ARGUMENT;
    }

    *out_buffer = NULL;
    *out_length = 0;

    cmail_buffer_init(&buffer);

    if (!cmail_buffer_append_string(&buffer, "From: ") ||
        !cmail_buffer_append_string(&buffer, payload->from) ||
        !cmail_buffer_append_string(&buffer, "\r\n")) {
        goto memory_error;
    }

    if (!cmail_buffer_append_string(&buffer, "To: ") ||
        !cmail_buffer_append_string(&buffer, payload->to) ||
        !cmail_buffer_append_string(&buffer, "\r\n")) {
        goto memory_error;
    }

    if (!cmail_buffer_append_string(&buffer, "Subject: ") ||
        !cmail_buffer_append_string(&buffer, payload->subject) ||
        !cmail_buffer_append_string(&buffer, "\r\n")) {
        goto memory_error;
    }

    if (payload->attachment_count == 0) {
        const char *content_type = payload->body_type == CMAIL_BODY_HTML
            ? "text/html; charset=utf-8\r\n"
            : "text/plain; charset=utf-8\r\n";

        if (!cmail_buffer_append_string(&buffer, "Content-Type: ") ||
            !cmail_buffer_append_string(&buffer, content_type) ||
            !cmail_buffer_append_string(&buffer, "MIME-Version: 1.0\r\n") ||
            !cmail_buffer_append_string(&buffer, "\r\n") ||
            !cmail_buffer_append(&buffer, payload->body, payload->body_length) ||
            !cmail_buffer_append_string(&buffer, "\r\n")) {
            goto memory_error;
        }

        goto success;
    }

    if (!cmail_buffer_append_string(&buffer, "MIME-Version: 1.0\r\n") ||
        !cmail_buffer_append_string(&buffer, "Content-Type: multipart/mixed; boundary=\"" CMAIL_BOUNDARY "\"\r\n") ||
        !cmail_buffer_append_string(&buffer, "\r\n")) {
        goto memory_error;
    }

    if (!cmail_buffer_append_string(&buffer, "--" CMAIL_BOUNDARY "\r\n") ||
        !cmail_buffer_append_string(&buffer, "Content-Type: text/plain; charset=utf-8\r\n") ||
        !cmail_buffer_append_string(&buffer, "\r\n") ||
        !cmail_buffer_append(&buffer, payload->body, payload->body_length) ||
        !cmail_buffer_append_string(&buffer, "\r\n")) {
        goto memory_error;
    }

    for (size_t i = 0; i < payload->attachment_count; i++) {
        const struct cmail_attachment *attachment = payload->attachments[i];
        if (!attachment) {
            cmail_buffer_free(&buffer);
            return CMAIL_ERROR_INVALID_ARGUMENT;
        }

        char *base64 = NULL;
        size_t base64_length = 0;

        cmail_error_t error = cmail_attachment_to_base64(attachment, &base64, &base64_length);

        if (error != CMAIL_OK) {
            cmail_buffer_free(&buffer);
            return error;
        }

        // 76 is the standard line length for base64 encoding in MIME
        // We will wrap the base64 string to ensure it adheres to this standard.
        size_t mime_base64_length = 0;
        char* mime_base64 = cmail_mime_patch(
            base64,
            base64_length,
            CMAIL_MIME_LINE_LENGTH,
            &mime_base64_length
        );

        free(base64);

        if (!mime_base64) {
            goto memory_error;
        }

        if (!cmail_buffer_append_string(&buffer, "--" CMAIL_BOUNDARY "\r\n") ||
            !cmail_buffer_append_string(&buffer, "Content-Type: ") ||
            !cmail_buffer_append_string(&buffer, attachment->mime_type) ||
            !cmail_buffer_append_string(&buffer, "\r\n") ||
            !cmail_buffer_append_string(&buffer, "Content-Disposition: attachment; filename=\"") ||
            !cmail_buffer_append_string(&buffer, attachment->filename) ||
            !cmail_buffer_append_string(&buffer, "\"\r\n") ||
            !cmail_buffer_append_string(&buffer, "Content-Transfer-Encoding: base64\r\n") ||
            !cmail_buffer_append_string(&buffer, "\r\n") ||
            !cmail_buffer_append(&buffer, mime_base64, mime_base64_length) ||
            !cmail_buffer_append_string(&buffer, "\r\n")) {
            free(mime_base64);
            goto memory_error;
        }

        free(mime_base64);
    }

    // End multipart
    if (!cmail_buffer_append_string(&buffer, "--" CMAIL_BOUNDARY "--\r\n")) {
        goto memory_error;
    }

success:
    *out_buffer = buffer.data;
    *out_length = buffer.length;

    return CMAIL_OK;

memory_error:
    cmail_buffer_free(&buffer);
    return CMAIL_ERROR_MEMORY;
}

void cmail_payload_clean(struct cmail_payload* payload) {
    if (!payload) {
        return;
    }

    free(payload->header.from);
    free(payload->header.to);
    free(payload->header.subject);
    free(payload->body);
    free(payload);
}

struct cmail_payload* cmail_payload_clone(const struct cmail_payload* payload) {
    if (!payload) {
        return NULL;
    }

    struct cmail_payload* clone = calloc(1, sizeof(struct cmail_payload));
    if (!clone) {
        return NULL;
    }

    if (payload->header.from && !(clone->header.from = strdup(payload->header.from))) {
        goto clean;
    }

    if (payload->header.to && !(clone->header.to = strdup(payload->header.to))) {
        goto clean;
    }

    if (payload->header.subject && !(clone->header.subject = strdup(payload->header.subject))) {
        goto clean;
    }

    if (payload->body && payload->body_length > 0) {
        clone->body = malloc(payload->body_length + 1);
        if (!clone->body) {
            goto clean;
        }

        memcpy(clone->body, payload->body, payload->body_length);
        clone->body[payload->body_length] = '\0';
        clone->body_length = payload->body_length;
    }

    clone->body_type = payload->body_type;

    if (payload->attachments && payload->attachment_count > 0) {
        clone->attachments = calloc(payload->attachment_count, sizeof(struct cmail_attachment*));
        if (!clone->attachments) {
            goto clean;
        }

        clone->attachment_count = payload->attachment_count;

        for (size_t i = 0; i < payload->attachment_count; i++) {
            clone->attachments[i] = cmail_attachment_clone(payload->attachments[i]);
            if (!clone->attachments[i]) {
                goto clean;
            }
        }
    }

    return clone;

clean:
    if (clone->attachments) {
        for (size_t i = 0; i < clone->attachment_count; i++) {
            cmail_attachment_free(clone->attachments[i]);
        }
    }

    free(clone->attachments);
    free(clone->header.from);
    free(clone->header.to);
    free(clone->header.subject);
    free(clone->body);
    free(clone);
    return NULL;
}

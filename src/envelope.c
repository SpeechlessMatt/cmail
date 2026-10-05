#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#include "envelope.h"

struct cmail_envelope* cmail_envelope_create(const char* from, const char* recipient) {
    if (!from || !recipient) {
        return NULL;
    }

    struct cmail_envelope* envelope = malloc(sizeof(struct cmail_envelope));
    if (!envelope) {
        return NULL;
    }

    envelope->from = strdup(from);
    if (!envelope->from) {
        free(envelope);
        return NULL;
    }

    envelope->recipients = curl_slist_append(NULL, recipient);
    if (!envelope->recipients) {
        free(envelope->from);
        free(envelope);
        return NULL;
    }

    return envelope;
}

void cmail_envelope_add_recipient(struct cmail_envelope* envelope, const char* recipient) {
    if (!envelope || !recipient) {
        return;
    }

    struct curl_slist *new_recipients = curl_slist_append(envelope->recipients, recipient);
    if (!new_recipients) {
        return;
    }

    envelope->recipients = new_recipients;
}

void cmail_envelope_clean(struct cmail_envelope* envelope) {
    if (!envelope) {
        return;
    }

    free(envelope->from);
    curl_slist_free_all(envelope->recipients);
    free(envelope);
}

struct cmail_envelope* cmail_envelope_clone(const struct cmail_envelope* envelope) {
    if (!envelope) {
        return NULL;
    }

    struct cmail_envelope* clone = malloc(sizeof(struct cmail_envelope));
    if (!clone) {
        return NULL;
    }

    clone->from = NULL;
    clone->recipients = NULL;

    if (envelope->from && !(clone->from = strdup(envelope->from))) {
        free(clone);
        return NULL;
    }

    for (const struct curl_slist* node = envelope->recipients; node; node = node->next) {
        struct curl_slist* new_recipients = curl_slist_append(clone->recipients, node->data);
        if (!new_recipients) {
            curl_slist_free_all(clone->recipients);
            free(clone->from);
            free(clone);
            return NULL;
        }

        clone->recipients = new_recipients;
    }

    return clone;
}

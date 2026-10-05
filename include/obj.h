#ifndef CMAIL_OBJ_H
#define CMAIL_OBJ_H

#define CMAIL_DEFAULT_MAX_RETRIES 3
#define CMAIL_DEFAULT_RETRY_INTERVAL 5
#define CMAIL_DEFAULT_TIMEOUT_MS 10000

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct cmail_cred {
    char* smtp_url;
    char* smtp_username;
    char* smtp_password;
};

struct cmail_options {
    time_t   send_at;
    uint8_t  max_retries;
    uint8_t  retry_count;
    uint16_t retry_interval_s;
    uint32_t timeout_ms;
};

static inline void cmail_options_init_defaults(struct cmail_options *info) {
    if (!info) return;
    info->send_at          = 0;
    info->max_retries      = CMAIL_DEFAULT_MAX_RETRIES;
    info->retry_count      = 0;
    info->retry_interval_s = CMAIL_DEFAULT_RETRY_INTERVAL;
    info->timeout_ms       = CMAIL_DEFAULT_TIMEOUT_MS;
}

static inline struct cmail_cred* cmail_cred_clone(const struct cmail_cred* cred) {
    if (!cred) {
        return NULL;
    }

    struct cmail_cred* clone = calloc(1, sizeof(struct cmail_cred));
    if (!clone) {
        return NULL;
    }

    if (cred->smtp_url && !(clone->smtp_url = strdup(cred->smtp_url))) {
        goto clean;
    }

    if (cred->smtp_username && !(clone->smtp_username = strdup(cred->smtp_username))) {
        goto clean;
    }

    if (cred->smtp_password && !(clone->smtp_password = strdup(cred->smtp_password))) {
        goto clean;
    }

    return clone;

clean:
    free(clone->smtp_url);
    free(clone->smtp_username);
    free(clone->smtp_password);
    free(clone);
    return NULL;
}

static inline struct cmail_options* cmail_options_clone(const struct cmail_options* option) {
    if (!option) {
        return NULL;
    }

    struct cmail_options* clone = calloc(1, sizeof(struct cmail_options));
    if (!clone) {
        return NULL;
    }

    memcpy(clone, option, sizeof(struct cmail_options));
    return clone;
}

#endif // CMAIL_OBJ_H

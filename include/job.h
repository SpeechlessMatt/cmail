#ifndef CMAIL_JOB_H
#define CMAIL_JOB_H

#include <stddef.h>
#include <time.h>

#include "error.h"

enum cmail_job_status {
    CMAIL_JOB_PENDING = 0,
    CMAIL_JOB_RUNNING,
    CMAIL_JOB_COMPLETED,
    CMAIL_JOB_FAILED
};

struct cmail_job {
    unsigned long long id;

    struct cmail_cred *cred;
    struct cmail_envelope *envelope;
    struct cmail_payload *payload;
    struct cmail_options *option;

    enum cmail_job_status status;

    unsigned int retry_count;
    unsigned int max_retries;

    time_t next_retry_at;

    cmail_error_t last_error;
};

struct cmail_job* cmail_job_create(
    struct cmail_cred* cred,
    struct cmail_envelope* envelope,
    struct cmail_payload* payload,
    struct cmail_options* option
);

void cmail_job_free(struct cmail_job* job);

#endif // CMAIL_JOB_H

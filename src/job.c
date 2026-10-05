#include "job.h"
#include "obj.h"
#include "payload.h"
#include "envelope.h"

#include <stdlib.h>

struct cmail_job* cmail_job_create(
    struct cmail_cred* cred,
    struct cmail_envelope* envelope,
    struct cmail_payload* payload,
    struct cmail_options* option
) {
    static unsigned long long next_id = 1;

    struct cmail_job* job;

    if (!payload) {
        return NULL;
    }

    job = calloc(1, sizeof(*job));

    if (!job) {
        return NULL;
    }

    job->id = next_id++;

    if (job->cred = cmail_cred_clone(cred), !job->cred) goto error;
    if (job->envelope = cmail_envelope_clone(envelope), !job->envelope) goto error;
    if (job->payload = cmail_payload_clone(payload), !job->payload) goto error;
    if (job->option = cmail_options_clone(option), !job->option) goto error;

    job->status = CMAIL_JOB_PENDING;

    job->retry_count = 0;
    job->max_retries = option && option->retry_count > 0 ? option->max_retries : CMAIL_DEFAULT_MAX_RETRIES;

    job->next_retry_at = 0;
    job->last_error = CMAIL_OK;

    return job;

error:
    cmail_job_free(job);
    return NULL;
}

void cmail_job_free(struct cmail_job* job) {
    if (!job) {
        return;
    }

    cmail_payload_clean(job->payload);
    cmail_envelope_clean(job->envelope);

    free(job);
}

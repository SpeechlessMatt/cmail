#include "worker.h"
#include "dispatch.h"

#include <string.h>
#include <unistd.h>

#define CMAIL_WORKER_DEFAULT_POLL_INTERVAL 1

static cmail_error_t cmail_worker_process_job(struct cmail_worker* worker, struct cmail_job* job) {
    cmail_error_t error;

    (void)worker;

    job->status = CMAIL_JOB_RUNNING;

    error = cmail_send_with_option(job->cred, job->envelope, job->payload, job->option);

    if (error == CMAIL_OK) {
        job->status = CMAIL_JOB_COMPLETED;
        job->last_error = CMAIL_OK;

        return CMAIL_OK;
    }

    job->last_error = error;
    job->retry_count++;

    if (job->retry_count >= job->max_retries) {
        job->status = CMAIL_JOB_FAILED;
        return error;
    }

    job->status = CMAIL_JOB_PENDING;

    return error;
}

cmail_error_t cmail_worker_init(struct cmail_worker *worker) {
    if (!worker) {
        return CMAIL_ERROR_INVALID_ARGUMENT;
    }

    memset(worker, 0, sizeof(*worker));

    worker->queue = cmail_job_queue_create();

    if (!worker->queue) {
        return CMAIL_ERROR_MEMORY;
    }

    worker->running = 0;
    worker->poll_interval = CMAIL_WORKER_DEFAULT_POLL_INTERVAL;

    return CMAIL_OK;
}

void cmail_worker_cleanup(struct cmail_worker *worker) {
    if (!worker) {
        return;
    }

    cmail_job_queue_free(worker->queue);

    worker->queue = NULL;
    worker->running = 0;
}

void cmail_worker_stop(struct cmail_worker *worker) {
    if (!worker) {
        return;
    }

    worker->running = 0;
}

cmail_error_t cmail_worker_run(struct cmail_worker *worker) {
    if (!worker || !worker->queue) {
        return CMAIL_ERROR_INVALID_ARGUMENT;
    }

    worker->running = 1;

    while (worker->running) {
        struct cmail_job *job;

        job = cmail_job_queue_pop(worker->queue);

        if (!job) {
            sleep(worker->poll_interval);
            continue;
        }

        cmail_worker_process_job(worker, job);

        if (job->status == CMAIL_JOB_COMPLETED) {
            cmail_job_free(job);
        } else if (job->status == CMAIL_JOB_FAILED) {
            cmail_job_free(job);
        } else {
            /* retry */
            cmail_job_queue_push(worker->queue, job);
        }
    }

    return CMAIL_OK;
}

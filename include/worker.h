#ifndef CMAIL_WORKER_H
#define CMAIL_WORKER_H

#include "error.h"
#include "job_queue.h"

struct cmail_worker {
    int running;
    unsigned int poll_interval;

    struct cmail_job_queue* queue;
};

cmail_error_t cmail_worker_init(struct cmail_worker* worker);
void cmail_worker_cleanup(struct cmail_worker* worker);
cmail_error_t cmail_worker_run(struct cmail_worker* worker);
void cmail_worker_stop(struct cmail_worker* worker);

#endif

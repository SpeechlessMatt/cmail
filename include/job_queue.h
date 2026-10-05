#ifndef CMAIL_JOB_QUEUE_H
#define CMAIL_JOB_QUEUE_H

#include <stddef.h>

#include "job.h"

struct cmail_job_queue;

struct cmail_job_queue *cmail_job_queue_create(void);

void cmail_job_queue_free(struct cmail_job_queue *queue);
cmail_error_t cmail_job_queue_push(struct cmail_job_queue *queue, struct cmail_job *job);
struct cmail_job *cmail_job_queue_pop(struct cmail_job_queue *queue);
size_t cmail_job_queue_size(const struct cmail_job_queue *queue);

#endif // CMAIL_JOB_QUEUE_H


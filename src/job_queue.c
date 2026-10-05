#include <stdlib.h>

#include "job_queue.h"

struct cmail_job_queue_node {
    struct cmail_job *job;
    struct cmail_job_queue_node *next;
};

struct cmail_job_queue {
    struct cmail_job_queue_node *head;
    struct cmail_job_queue_node *tail;
    size_t size;
};

struct cmail_job_queue* cmail_job_queue_create(void)
{
    struct cmail_job_queue *queue;

    queue = calloc(1, sizeof(*queue));

    return queue;
}

void cmail_job_queue_free(struct cmail_job_queue *queue)
{
    struct cmail_job_queue_node *node;
    struct cmail_job_queue_node *next;

    if (!queue) {
        return;
    }

    node = queue->head;

    while (node) {
        next = node->next;

        cmail_job_free(node->job);
        free(node);

        node = next;
    }

    free(queue);
}

cmail_error_t cmail_job_queue_push(struct cmail_job_queue* queue, struct cmail_job* job) {
    struct cmail_job_queue_node* node;

    if (!queue || !job) {
        return CMAIL_ERROR_INVALID_ARGUMENT;
    }

    node = malloc(sizeof(*node));

    if (!node) {
        return CMAIL_ERROR_MEMORY;
    }

    node->job = job;
    node->next = NULL;

    if (queue->tail) {
        queue->tail->next = node;
    } else {
        queue->head = node;
    }

    queue->tail = node;
    queue->size++;

    return CMAIL_OK;
}

struct cmail_job* cmail_job_queue_pop(struct cmail_job_queue* queue) {
    struct cmail_job_queue_node* node;
    struct cmail_job* job;

    if (!queue || !queue->head) {
        return NULL;
    }

    node = queue->head;

    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }

    queue->size--;

    job = node->job;
    free(node);

    return job;
}

size_t cmail_job_queue_size(const struct cmail_job_queue* queue) {
    if (!queue) {
        return 0;
    }

    return queue->size;
}

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "coro_link.h"

struct cmail_coro_link {
    struct cmail_coro_link* next;
    void* user_data;
};

struct cmail_coro_link_table {
    uint16_t link_max_count;

    struct cmail_coro_link* effective_links;
    struct cmail_coro_link* effective_links_head;
    struct cmail_coro_link* effective_links_tail;

    uint16_t* usable_links;
    uint16_t usable_links_head;
    uint16_t usable_links_tail;
};

struct cmail_coro_link_table* cmail_coro_link_table_create(uint16_t link_max_count) {
    if (link_max_count == 0) {
        return NULL;
    }

    struct cmail_coro_link_table* table = malloc(sizeof(struct cmail_coro_link_table));
    if (!table) {
        return NULL;
    }

    table->link_max_count = link_max_count;
    table->effective_links_head = NULL;
    table->effective_links_tail = NULL;

    table->effective_links = malloc(link_max_count * sizeof(struct cmail_coro_link));
    if (!table->effective_links) {
        goto error;
    }

    table->usable_links = malloc(sizeof(uint16_t) * (link_max_count + 1));
    if (!table->usable_links) {
        goto error;
    }

    // initialize
    for (uint16_t i = 0; i < link_max_count; i++) {
        table->effective_links[i].next = NULL;
        table->effective_links[i].user_data = NULL;
        table->usable_links[i] = i;
    }

    table->usable_links_head = 0;
    table->usable_links_tail = link_max_count;

    return table;

error:
    free(table->effective_links);
    free(table->usable_links);
    free(table);
    return NULL;
}

void cmail_coro_link_table_destroy(
    struct cmail_coro_link_table* table, 
    cmail_coro_link_user_data_destroy_t destroy_cb
) {
    if (!table) {
        return;
    }

    if (destroy_cb) {
        struct cmail_coro_link* link = table->effective_links_head;
        while (link) {
            if (link->user_data) {
                destroy_cb(link->user_data);
            }
            link = link->next;
        }
    }

    free(table->effective_links);
    free(table->usable_links);
    free(table);
}

// error code is emmm...
// I will do it.
int cmail_coro_link_append(struct cmail_coro_link_table* table, void* user_data) {
    if (!table) {
        return -1;
    }

    // no more usable
    if (table->usable_links_head == table->usable_links_tail) {
        perror("");
        return -1;
    }

    uint16_t queue_capacity = table->link_max_count + 1;

    uint16_t index = table->usable_links[table->usable_links_head];
    table->usable_links_head = (table->usable_links_head + 1) % queue_capacity;

    struct cmail_coro_link* new_link = &table->effective_links[index];
    new_link->user_data = user_data;
    new_link->next = NULL;

    if (table->effective_links_tail == NULL) {
        table->effective_links_head = new_link;
        table->effective_links_tail = new_link;
    } else {
        table->effective_links_tail->next = new_link;
        table->effective_links_tail = new_link;
    }

    return 0;
}

int cmail_coro_link_remove(
    struct cmail_coro_link_table* table, 
    struct cmail_coro_link* last_effective_link, 
    struct cmail_coro_link* link
) {
    if (!table || !link) {
        return -1;
    }

    // remember to review
    if (last_effective_link == NULL) {
        if (table->effective_links_head != link) {
            return -1;
        }
        table->effective_links_head = link->next;
    }
    else {
        if (last_effective_link->next != link) {
            return -1;
        }
        last_effective_link->next = link->next;
    }

    if (table->effective_links_tail == link) {
        table->effective_links_tail = last_effective_link;
    }

    uint16_t queue_capacity = table->link_max_count + 1;
    uint16_t next_tail = (table->usable_links_tail + 1) % queue_capacity;

    // Recheck for queue
    if (next_tail == table->usable_links_head) {
        return -1; 
    }

    uint16_t index = (uint16_t)(link - table->effective_links);
    table->usable_links[table->usable_links_tail] = index;
    table->usable_links_tail = next_tail;

    link->next = NULL;
    link->user_data = NULL;

    return 0;
}


struct cmail_coro_link* cmail_coro_link_head(struct cmail_coro_link_table* table) {
    return table ? table->effective_links_head : NULL;
}

struct cmail_coro_link* cmail_coro_link_next(struct cmail_coro_link* current_link) {
    return current_link ? current_link->next : NULL;
}

void* cmail_coro_link_get_user_data(struct cmail_coro_link* link) {
    return link ? link->user_data : NULL;
}

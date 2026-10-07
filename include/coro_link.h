#ifndef CMAIL_CORO_LINK_H
#define CMAIL_CORO_LINK_H

#include <stdint.h>

struct cmail_coro_link;
struct cmail_coro_link_table;

typedef void (*cmail_coro_link_user_data_destroy_t)(void* user_data);

struct cmail_coro_link_table* cmail_coro_link_table_create(uint16_t link_max_count);

void cmail_coro_link_table_destroy(
    struct cmail_coro_link_table* table, 
    cmail_coro_link_user_data_destroy_t destroy_cb
);

int cmail_coro_link_append(struct cmail_coro_link_table* table, void* user_data);
int cmail_coro_link_remove(
    struct cmail_coro_link_table* table, 
    struct cmail_coro_link* last_effective_link, 
    struct cmail_coro_link* link
);

struct cmail_coro_link* cmail_coro_link_head(struct cmail_coro_link_table* table);
struct cmail_coro_link* cmail_coro_link_next(struct cmail_coro_link* current_link);
void* cmail_coro_link_get_user_data(struct cmail_coro_link* link);

#endif

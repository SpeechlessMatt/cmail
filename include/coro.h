#ifndef CMAIL_CORO_H
#define CMAIL_CORO_H

#define CMAIL_MAX_CLIENTS 1024

struct cmail_coro_handle;
typedef void (*cmail_coro_handler_t)(struct cmail_coro_handle* handle, void* user_data);

void* cmail_coro_get_user_data(struct cmail_coro_handle* handle);
void cmail_coro_spawn(struct cmail_coro_handle* handle, cmail_coro_handler_t client_handler, void* user_data);
void cmail_coro_yield(void);

struct cmail_coro_handle* cmail_coro_create(cmail_coro_handler_t listen_handler, void* user_data);
void cmail_coro_destroy(struct cmail_coro_handle* handle);
void cmail_coro_loop(struct cmail_coro_handle* handle);

#endif // CMAIL_CORO_H

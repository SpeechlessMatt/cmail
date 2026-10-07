#include <stdlib.h>

#include "coro.h"
#include "coro_link.h"

#define MINICORO_IMPL
#include "minicoro.h"

struct cmail_coro_handle {
    mco_coro* listen_co;
    struct cmail_coro_link_table* client_co;
};

struct cmail_coro_ctx {
    struct cmail_coro_handle* handle;
    cmail_coro_handler_t handler;
    void* user_data;
};

static void cmail_coro_client_destroy(void* user_data) {
    if (!user_data) {
        return;
    }

    mco_coro* co = (mco_coro*)user_data;
    mco_destroy(co);
}

static void cmail_coro_listener_wrapper(mco_coro* co) {
    struct cmail_coro_ctx* ctx = (struct cmail_coro_ctx*)mco_get_user_data(co);
    if (!ctx) {
        return;
    }

    struct cmail_coro_handle* handle = ctx->handle;
    if (!handle) {
        goto clean;
    }
    
    cmail_coro_handler_t handler = ctx->handler;
    if (handler) {
        handler(handle, ctx->user_data);
    }

clean:
    free(ctx);
}

void cmail_coro_spawn(struct cmail_coro_handle* handle, cmail_coro_handler_t client_handler, void* user_data) {
    if (!handle || !client_handler || !handle->client_co) {
        return;
    }

    struct cmail_coro_ctx* ctx = (struct cmail_coro_ctx*)malloc(sizeof(struct cmail_coro_ctx));
    if (!ctx) {
        return;
    }

    *ctx = (struct cmail_coro_ctx){
        .handle = handle,
        .user_data = user_data,
        .handler = client_handler
    };

    mco_desc desc = mco_desc_init(cmail_coro_listener_wrapper, 0);
    desc.user_data = ctx;

    mco_coro* coro = NULL;
    if (mco_create(&coro, &desc) != MCO_SUCCESS) {
        free(ctx);
        return;
    }

    // MUST DO THIS CHECK, if append failed, we need to destroy the coro and free the ctx
    if (cmail_coro_link_append(handle->client_co, coro) != 0) {
        mco_destroy(coro);
    }
}

void cmail_coro_yield() {
    mco_coro* current = mco_running();
    if (current) {
        mco_yield(current);
    }
}

/*
 * Create a new coroutine handle with the specified listen handler and user data.
 * Returns a pointer to the newly created coroutine handle, or NULL on failure.
 */
struct cmail_coro_handle* cmail_coro_create(cmail_coro_handler_t listen_handler, void* user_data) {
    if (!listen_handler) {
        return NULL;
    }

    struct cmail_coro_handle* handle = (struct cmail_coro_handle*)malloc(sizeof(struct cmail_coro_handle));
    if (!handle) {
        return NULL;
    }

    struct cmail_coro_ctx* ctx = (struct cmail_coro_ctx*)malloc(sizeof(struct cmail_coro_ctx));
    if (!ctx) {
        free(handle);
        return NULL;
    }

    *ctx = (struct cmail_coro_ctx){
        .handle = handle,
        .user_data = user_data,
        .handler = listen_handler
    };

    mco_desc desc = mco_desc_init(cmail_coro_listener_wrapper, 0);
    desc.user_data = ctx;

    mco_coro* coro = NULL;
    if (mco_create(&coro, &desc) != MCO_SUCCESS) {
        free(handle);
        free(ctx);
        return NULL;
    }

    struct cmail_coro_link_table* client_co = cmail_coro_link_table_create(CMAIL_MAX_CLIENTS);
    if (!client_co) {
        mco_destroy(coro);
        free(handle);
        free(ctx);
        return NULL;
    }

    *handle = (struct cmail_coro_handle){
        .listen_co = coro,
        .client_co = client_co
    };

    return handle;
}

void cmail_coro_destroy(struct cmail_coro_handle* handle) {
    if (!handle) {
        return;
    }

    if (handle->listen_co) {
        mco_destroy(handle->listen_co);
    }

    if (handle->client_co) {
        cmail_coro_link_table_destroy(handle->client_co, cmail_coro_client_destroy);
    }

    free(handle);
}

void cmail_coro_loop(struct cmail_coro_handle* handle) {
    if (!handle || !handle->listen_co) {
        return;
    }

    mco_resume(handle->listen_co);

    if (mco_status(handle->listen_co) != MCO_DEAD) {
        mco_resume(handle->listen_co);
    }
    
    if (!handle->client_co) {
        return;
    }

    struct cmail_coro_link* last_link = NULL;
    struct cmail_coro_link* current_link = cmail_coro_link_head(handle->client_co);

    while (current_link) {
        struct cmail_coro_link* next_link = cmail_coro_link_next(current_link);

        mco_coro* client_co = (mco_coro*)cmail_coro_link_get_user_data(current_link);
        if (client_co) {
            if (mco_status(client_co) != MCO_DEAD) {
                mco_resume(client_co);
            }

            mco_state state = mco_status(client_co);
            if (state == MCO_DEAD) {
                cmail_coro_link_remove(handle->client_co, last_link, current_link);
                mco_destroy(client_co);
            } else {
                last_link = current_link;
            }

        } else {
            last_link = current_link;
        }

        current_link = next_link;
    }
}

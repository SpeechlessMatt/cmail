#ifndef CMAIL_SOCKET_H
#define CMAIL_SOCKET_H

#include <stddef.h>

#include "error.h"

#define CMAIL_SOCKET_PATH "/tmp/cmail.sock"

int cmail_socket_server_create(void);

int cmail_socket_accept(int server_fd);

int cmail_socket_connect(void);

cmail_error_t cmail_socket_send(int fd, const void *data, size_t length);
cmail_error_t cmail_socket_recv(int fd, void *buffer, size_t capacity, size_t *received);
void cmail_socket_close(int fd);

#endif

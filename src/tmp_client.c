#include "socket.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    int fd;
    const char *message = "hello from cmail client";

    fd = cmail_socket_connect();

    if (fd < 0) {
        fprintf(stderr, "failed to connect worker\n");
        return 1;
    }

    if (cmail_socket_send(fd, message, strlen(message)) != CMAIL_OK) {
        fprintf(stderr, "failed to send\n");
        cmail_socket_close(fd);
        return 1;
    }

    cmail_socket_close(fd);

    return 0;
}

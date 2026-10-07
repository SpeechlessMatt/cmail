#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <stdio.h>

#include "socket.h"

int cmail_socket_server_create(void)
{
    int fd;
    struct sockaddr_un address;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (fd < 0) {
        return -1;
    }

    memset(&address, 0, sizeof(address));

    address.sun_family = AF_UNIX;

    // FIXME: 这里使用 strncpy 是为了避免 sun_path 溢出，但实际上应该使用 snprintf 或者其他更安全的方式来处理路径
    strncpy(address.sun_path, CMAIL_SOCKET_PATH, sizeof(address.sun_path) - 1);

    /*
     * 如果上一次 worker 异常退出，
     * socket 文件可能还存在
     */
    unlink(CMAIL_SOCKET_PATH);

    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(fd);
        return -1;
    }

    if (listen(fd, 8) < 0) {
        close(fd);
        unlink(CMAIL_SOCKET_PATH);

        return -1;
    }

    return fd;
}

// Accept a new connection on the server socket
int cmail_socket_accept(int server_fd) {
    return accept(server_fd, NULL, NULL);
}

int cmail_socket_connect(void) {
    int fd;
    struct sockaddr_un address;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (fd < 0) {
        return -1;
    }

    memset(&address, 0, sizeof(address));

    address.sun_family = AF_UNIX;

    strncpy(
        address.sun_path,
        CMAIL_SOCKET_PATH,
        sizeof(address.sun_path) - 1
    );

    if (connect(fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

cmail_error_t cmail_socket_send(
    int fd,
    const void *data,
    size_t length
) {
    const unsigned char *buffer = data;
    size_t position = 0;

    if (fd < 0 || (!data && length > 0)) {
        return CMAIL_ERROR_INVALID_ARGUMENT;
    }

    while (position < length) {
        ssize_t result;

        result = send(fd, buffer + position, length - position, 0);

        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }

            return CMAIL_ERROR_IO;
        }

        if (result == 0) {
            return CMAIL_ERROR_IO;
        }

        position += (size_t)result;
    }

    return CMAIL_OK;
}

cmail_error_t cmail_socket_recv(
    int fd,
    void *buffer,
    size_t capacity,
    size_t *received
) {
    ssize_t result;

    if (fd < 0 || !buffer || !received || capacity == 0) {
        return CMAIL_ERROR_INVALID_ARGUMENT;
    }

    do {
        result = recv(fd, buffer, capacity, 0);
    } while (result < 0 && errno == EINTR);

    if (result < 0) {
        return CMAIL_ERROR_IO;
    }

    if (result == 0) {
        *received = 0;
        return CMAIL_OK;
    }

    *received = (size_t)result;

    return CMAIL_OK;
}

void cmail_socket_close(int fd)
{
    if (fd >= 0) {
        close(fd);
    }
}

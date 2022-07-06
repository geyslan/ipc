#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/adder-socket"
#define MAX_CLIENT 20
#define BUFFER_SIZE 128

int monitored_fd_set[MAX_CLIENT];
int client_sum[MAX_CLIENT] = { 0 };

static void
initialize_monitored_fd_set()
{
    for (int i = 0; i < MAX_CLIENT; i++)
        monitored_fd_set[i] = -1;
}

static void
add_to_monitored_fd_set(int skt_fd)
{
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (monitored_fd_set[i] == -1) {
            monitored_fd_set[i] = skt_fd;
            break;
        }
    }
}

static void
del_from_monitored_fd_set(int skt_fd)
{
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (monitored_fd_set[i] != skt_fd)
            continue;

        monitored_fd_set[i] = -1;
        break;
    }
}

static void
refresh_fd_set(fd_set *fd_set_ptr)
{
    FD_ZERO(fd_set_ptr);
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (monitored_fd_set[i] != -1)
            FD_SET(monitored_fd_set[i], fd_set_ptr);
    }
}

static int
get_max_fd()
{
    int max = -1;

    for (int i = 0; i < MAX_CLIENT; i++) {
        if (monitored_fd_set[i] > max)
            max = monitored_fd_set[i];
    }

    return max;
}

int
main(int argc, char *argv[])
{
    int ret;
    int connection_socket;
    struct sockaddr_un name;

    initialize_monitored_fd_set();

    ret = unlink(SOCKET_NAME);
    if (ret == -1 && errno != ENOENT) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connection_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    printf("Master socket created\n");

    add_to_monitored_fd_set(connection_socket);

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    ret = bind(connection_socket, (const struct sockaddr *) &name,
               sizeof(name));
    if (ret == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("Master socket binded\n");

    ret = listen(connection_socket, MAX_CLIENT);
    if (ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        fd_set readfds;
        int data_socket = -1;
        int data;
        char buffer[BUFFER_SIZE];

        refresh_fd_set(&readfds);

        printf("Waiting incoming connection\n");

        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(connection_socket, &readfds)) {
            data_socket = accept(connection_socket, NULL, NULL);
            if (data_socket == -1) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("Incoming connection accepted\n");
            add_to_monitored_fd_set(data_socket);
            continue;
        }

        for (int i = 0; i < MAX_CLIENT; i++) {
            if (FD_ISSET(monitored_fd_set[i], &readfds)) {
                data_socket = monitored_fd_set[i];
                memset(buffer, 0, BUFFER_SIZE);

                printf("Waiting for client data\n");
                ret = read(data_socket, buffer, BUFFER_SIZE);
                if (ret == -1) {
                    perror("read");
                    exit(EXIT_FAILURE);
                }

                memcpy(&data, buffer, sizeof(data));

                if (data == 0) {
                    memset(buffer, 0, BUFFER_SIZE);
                    sprintf(buffer, "Sum = %d", client_sum[i]);
                    printf("Sending sum back to client\n");
                    ret = write(data_socket, buffer, BUFFER_SIZE);
                    if (ret == -1) {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }

                    ret = close(data_socket);
                    if (ret == -1) {
                        perror("close");
                        exit(EXIT_FAILURE);
                    }
                    client_sum[i] = 0;
                    del_from_monitored_fd_set(data_socket);
                    continue;
                }

                client_sum[i] += data;
            }
        }
    }

    close(connection_socket);
    del_from_monitored_fd_set(connection_socket);
    printf("Connection closed\n");

    exit(EXIT_SUCCESS);
}

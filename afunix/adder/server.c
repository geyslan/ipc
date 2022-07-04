#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/adder-socket"
#define CLIENT_QUEUE 20
#define BUFFER_SIZE 128

int
main(int argc, char *argv[])
{
    int ret;
    int connection_socket;
    struct sockaddr_un name;

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

    ret = listen(connection_socket, CLIENT_QUEUE);
    if (ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        int data_socket;
        int sum;
        int data;
        char buffer[BUFFER_SIZE];

        printf("Waiting incoming connection\n");

        data_socket = accept(connection_socket, NULL, NULL);
        if (data_socket == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Incoming connection accepted\n");

        sum = 0;
        for (;;) {
            memset(buffer, 0, BUFFER_SIZE);
            printf("Waiting for client data\n");
            ret = read(data_socket, buffer, BUFFER_SIZE);
            if (ret == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            memcpy(&data, buffer, sizeof(data));
            if (data == 0)
                break;

            sum += data;
        }

        memset(buffer, 0, BUFFER_SIZE);
        sprintf(buffer, "Sum = %d", sum);
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
    }

    close(connection_socket);
    printf("Connection closed\n");

    exit(EXIT_SUCCESS);
}

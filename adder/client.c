#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/adder-socket"
#define BUFFER_SIZE 128

int
main(int argc, char *argv[])
{
    int ret;
    int data_socket;
    struct sockaddr_un addr;
    int n;
    char buffer[BUFFER_SIZE];

    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (data_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

    ret = connect(data_socket, (const struct sockaddr *) &addr,
                  sizeof(addr));
    if (ret == -1) {
        fprintf(stderr, "Not possible to connect to the server\n");
        exit(EXIT_FAILURE);
    }

    do {
        printf("Enter integer to send to server:\n");
        errno = 0;
        ret = scanf("%d", &n);
        if (ret == EOF && errno) {
            perror("scanf");
            exit(EXIT_FAILURE);
        }

        ret = write(data_socket, &n, sizeof(n));
        if (ret == -1) {
            perror("write");
            break;
        }

        printf("Bytes sent = %d, data sent = %d\n", ret, n);
    } while(n);

    memset(buffer, 0, BUFFER_SIZE);
    ret = read(data_socket, buffer, BUFFER_SIZE);
    if (ret == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    printf("Received from server: %s\n", buffer);

    close(data_socket);

    exit(EXIT_SUCCESS);
}
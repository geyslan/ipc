// #include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "log-pub-test.h"
#include "log.h"
#include "log-hub.h"

int main(int argc, char *argv[])
{
    int ret;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <origin>\n", argv[0]);
        return EXIT_FAILURE;
    }

    enum log_origin origin = atoi(argv[1]);

    // connect to the hub
    struct sockaddr_un addr;
    int data_socket;

    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (data_socket == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SUB_SOCK_NAME, sizeof(addr.sun_path) - 1);

    ret = connect(data_socket,
                  (const struct sockaddr *) &addr,
                  sizeof(addr));
    if (ret == -1) {
        perror("connect");
        return EXIT_FAILURE;
    }

    // send the requested origin to the hub so it broadcasts the logs
    ret = send(data_socket, &origin, sizeof(origin), 0);
    if (ret == -1) {
        perror("send");
        return EXIT_FAILURE;
    }

    char buf_meta[LOG_METADATA_SIZE];
    char buf_raw[LOG_RAW_MAX_SIZE];

    for (;;) {
        // receive logs from the hub
        int ret = recv(data_socket, buf_meta, LOG_METADATA_SIZE, 0);
        if (ret == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        if (ret == 0) {
            printf("Read nothing from %d\n", data_socket);
            close(data_socket);
            continue;
        }

        struct log_metadata metadata;
        log_metadata_decode(buf_meta, &metadata);

        ret = recv(data_socket, buf_raw, metadata.raw_size, 0);
        if (ret == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        if (ret == 0) {
            printf("Read nothing from %d\n", data_socket);
            close(data_socket);
            continue;
        }

        struct log *log = log_decode(buf_raw);
        if (!log) {
            perror("log_decode");
            exit(EXIT_FAILURE);
        }

        printf("%s: %s (level %d)\n", LOG_ORIGIN_STR(metadata.origin), log->msg, log->level);

        log_free(log);
    }

    return EXIT_SUCCESS;
}

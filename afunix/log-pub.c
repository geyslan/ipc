// #include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "log-pub-test.h"
#include "log.h"
// #include "log-hub.h"

int send_logs(int data_socket, struct tt *tt, int n)
{
    int ret;
    struct log *log;

    for (int i = 0; i < n; i++) {
        log = log_new(tt[i].level, tt[i].msg);
        if (!log) {
            perror("log_new");
            ret = -1;
            goto err;
        }

        ret = send_log(data_socket, log, tt[i].origin);
        if (ret < 0) {
            goto err1;
        }

err1:
        log_free(log);
err:
        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

int conn_send(char *path, struct tt *tt, int n)
{
    struct sockaddr_un addr;
    int data_socket;
    int ret;

    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (data_socket == -1) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    ret = connect(data_socket,
                  (const struct sockaddr *) &addr,
                  sizeof(addr));
    if (ret == -1) {
        perror("connect");
        goto err;
    }

    ret = send_logs(data_socket, tt, n);
    if (ret < 0) {
        goto err;
    }

    return data_socket;

err:
    close(data_socket);
    return ret;
}

#define DECLARE_THREAD_FUNC(array)                             \
void *thread_##array(void *arg) {                              \
    int data_socket;                                           \
    arg = arg;                                                 \
                                                               \
    printf("- Started thread for " #array "\n");               \
                                                               \
    data_socket = conn_send(PUB_SOCK_NAME, array,              \
                            sizeof(array) / sizeof(array[0])); \
    if (data_socket < 0) {                                     \
        return NULL;                                           \
    }                                                          \
                                                               \
    printf("Sent logs of " #array "\n");                       \
    close(data_socket);                                        \
                                                               \
    return NULL;                                               \
}

DECLARE_THREAD_FUNC(tt_app)
DECLARE_THREAD_FUNC(tt_db)
DECLARE_THREAD_FUNC(tt_iot)
DECLARE_THREAD_FUNC(tt_sys)

int main(int argc, char *argv[])
{
    int ret;
    pthread_t threads[4];
    argc = argc;
    argv = argv;

    PTHREAD_CREATE(threads[0], tt_app);
    PTHREAD_CREATE(threads[1], tt_db);
    PTHREAD_CREATE(threads[2], tt_iot);
    PTHREAD_CREATE(threads[3], tt_sys);

    for (int i = 0; i < 4; i++) {
        ret = pthread_join(threads[i], NULL);
        if (ret != 0) {
            perror("pthread_join");
            continue;
        }
    }

    return EXIT_SUCCESS;
}

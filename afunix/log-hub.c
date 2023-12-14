#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "log.h"
#include "log-hub.h"

#define MAX_PULISHER_SUPPORTED   100
#define MAX_SUBSCRIBER_SUPPORTED 100
#define FREE_SLOT -1

int pub_fds[MAX_PULISHER_SUPPORTED];

struct subscriber {
    int fd;
    enum log_origin origin;
};

struct subscriber subs[MAX_SUBSCRIBER_SUPPORTED];

int sub_fds_origin[MAX_LOG_ORIGIN][MAX_SUBSCRIBER_SUPPORTED];

void pub_fd_set_init()
{
    for (int i = 0; i < MAX_PULISHER_SUPPORTED; i++) {
        pub_fds[i] = FREE_SLOT;
    }
}

void sub_fd_set_init()
{
    for (int i = 0; i < MAX_SUBSCRIBER_SUPPORTED; i++) {
        subs[i].fd = FREE_SLOT;
        subs[i].origin = FREE_SLOT;
    }
}

void sub_fds_origin_init()
{
    for (int i = 0; i < MAX_LOG_ORIGIN; i++) {
        for (int j = 0; j < MAX_SUBSCRIBER_SUPPORTED; j++) {
            sub_fds_origin[i][j] = FREE_SLOT;
        }
    }
}

int pub_fd_set_add(int sockfd)
{
    for (int i = 0; i < MAX_PULISHER_SUPPORTED; i++) {
        if (pub_fds[i] != FREE_SLOT)
            continue;

        pub_fds[i] = sockfd;
        return 0;
    }

    return -1;
}

int sub_fd_set_add(int sockfd)
{
    for (int i = 0; i < MAX_SUBSCRIBER_SUPPORTED; i++) {
        if (subs[i].fd != FREE_SLOT)
            continue;

        subs[i].fd = sockfd;
        subs[i].origin = LOG_ORIGIN_UNKNOWN;

        return 0;
    }

    return -1;
}

void pub_fd_set_rem(int sockfd)
{
    for (int i = 0; i < MAX_PULISHER_SUPPORTED; i++) {
        if (pub_fds[i] != sockfd)
            continue;

        pub_fds[i] = FREE_SLOT;

        return;
    }
}

void sub_fd_set_rem(int sockfd)
{
    for (int i = 0; i < MAX_SUBSCRIBER_SUPPORTED; i++) {
        if (subs[i].fd != sockfd)
            continue;

        subs[i].fd = FREE_SLOT;
        subs[i].origin = FREE_SLOT;

        return;
    }
}

void pub_fd_set_refresh(fd_set *fdset)
{
    FD_ZERO(fdset);
    for (int i = 0; i < MAX_PULISHER_SUPPORTED; i++) {
        if (pub_fds[i] == FREE_SLOT)
            continue;
        
        FD_SET(pub_fds[i], fdset);
    }
}

void sub_fd_set_refresh(fd_set *fdset)
{
    FD_ZERO(fdset);
    for (int i = 0; i < MAX_SUBSCRIBER_SUPPORTED; i++) {
        if (subs[i].fd == FREE_SLOT)
            continue;
        
        FD_SET(subs[i].fd, fdset);
    }
}

int pub_get_max_fd(void)
{
    int max_fd = FREE_SLOT;

    for (int i = 0; i < MAX_PULISHER_SUPPORTED; i++) {
        if (pub_fds[i] > max_fd)
            max_fd = pub_fds[i];
    }

    return max_fd;
}

int sub_get_max_fd(void)
{
    int max_fd = FREE_SLOT;

    for (int i = 0; i < MAX_SUBSCRIBER_SUPPORTED; i++) {
        if (subs[i].fd > max_fd)
            max_fd = subs[i].fd;
    }

    return max_fd;
}

void sub_fds_origin_add(int fd, enum log_origin orig)
{
    for (int i = 0; i < MAX_SUBSCRIBER_SUPPORTED; i++) {
        if (sub_fds_origin[orig][i] != FREE_SLOT)
            continue;

        sub_fds_origin[orig][i] = fd;

        return;
    }
}

void sub_fds_origin_rem(int fd)
{
    for (int i = 0; i < MAX_LOG_ORIGIN; i++) {
        for (int j = 0; j < MAX_SUBSCRIBER_SUPPORTED; j++) {
            if (sub_fds_origin[i][j] != fd)
                continue;

            sub_fds_origin[i][j] = FREE_SLOT;

            return;
        }
    }
}

void send_to_subscribers(struct log *log, enum log_origin origin)
{
    for (int i = 0; i < MAX_SUBSCRIBER_SUPPORTED; i++) {
        int sub_fd = sub_fds_origin[origin][i];
        if (sub_fd == FREE_SLOT)
            continue;

        printf("Sending log to subscriber %d, from origin %s\n", sub_fd, LOG_ORIGIN_STR(origin));
        int ret = send_log(sub_fd, log, origin);
        if (ret < 0) {
            perror("send_log");
        }
    }
}

int listen_afunix(const char *path)
{
    struct sockaddr_un addr;
    int conn_sockfd;
    int ret;

    unlink(path);

    conn_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (conn_sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    ret = bind(conn_sockfd,
               (const struct sockaddr *) &addr,
               sizeof(addr));
    if (ret == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    printf("Listening on %s\n", path);

    ret = listen(conn_sockfd, 20);
    if (ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return conn_sockfd;
}

int pub_listen(void)
{
    return listen_afunix(PUB_SOCK_NAME);
}

int sub_listen(void)
{
    return listen_afunix(SUB_SOCK_NAME);
}

void *thread_publisher(void *arg) {
    int pub_conn_sockfd;
    pub_conn_sockfd = pub_listen();

    arg = arg;

    pub_fd_set_init();
    pub_fd_set_add(pub_conn_sockfd);
    sub_fds_origin_init();

    char buf_raw[LOG_RAW_MAX_SIZE];
    fd_set pub_fd_set;
    int ret;

    for (;;) {
        pub_fd_set_refresh(&pub_fd_set);

        ret = select(pub_get_max_fd() + 1, &pub_fd_set, NULL, NULL, NULL);
        if (ret == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(pub_conn_sockfd, &pub_fd_set)) {
            int data_sockfd;

            data_sockfd = accept(pub_conn_sockfd, NULL, NULL);
            if (data_sockfd == -1) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            pub_fd_set_add(data_sockfd);

            printf("Publisher %d connected\n", data_sockfd);
        } else if (FD_ISSET(STDIN_FILENO, &pub_fd_set)) {
            // TODO: discard it            
        } else {
            for (int i = 0; i < MAX_PULISHER_SUPPORTED; i++) {
                int pub_fd = pub_fds[i];
                if (!FD_ISSET(pub_fd, &pub_fd_set))
                    continue;
                
                char buf_meta[LOG_METADATA_SIZE];
                int ret = recv(pub_fd, buf_meta, LOG_METADATA_SIZE, 0);
                if (ret == -1) {
                    perror("recv");
                    exit(EXIT_FAILURE);
                }
                if (ret == 0) {
                    printf("Publisher %d disconnected\n", pub_fd);
                    pub_fd_set_rem(pub_fd);
                    close(pub_fd);
                    continue;
                }

                struct log_metadata metadata;
                log_metadata_decode(buf_meta, &metadata);
                
                ret = recv(pub_fd, buf_raw, metadata.raw_size, 0);
                if (ret == -1) {
                    perror("recv");
                    exit(EXIT_FAILURE);
                }
                if (ret == 0) {
                    printf("Publisher %d disconnected\n", pub_fd);
                    pub_fd_set_rem(pub_fd);
                    close(pub_fd);
                    continue;
                }

                struct log *log = log_decode(buf_raw);
                if (!log) {
                    perror("log_decode");
                    exit(EXIT_FAILURE);
                }

                send_to_subscribers(log, metadata.origin);

                log_free(log);
            }
        }
    }

    return NULL;
}

void *thread_subscriber(void *arg) {
    int sub_conn_sockfd;
    sub_conn_sockfd = sub_listen();

    arg = arg;

    sub_fd_set_init();
    sub_fd_set_add(sub_conn_sockfd);

    fd_set sub_fd_set;
    int ret;

    for (;;) {
        sub_fd_set_refresh(&sub_fd_set);

        ret = select(sub_get_max_fd() + 1, &sub_fd_set, NULL, NULL, NULL);
        if (ret == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(sub_conn_sockfd, &sub_fd_set)) {
            int data_sockfd;

            data_sockfd = accept(sub_conn_sockfd, NULL, NULL);
            if (data_sockfd == -1) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            sub_fd_set_add(data_sockfd);

            printf("Subscriber %d connected\n", data_sockfd);
        } else if (FD_ISSET(STDIN_FILENO, &sub_fd_set)) {
            // TODO: discard it
        } else {
            for (int i = 0; i < MAX_SUBSCRIBER_SUPPORTED; i++) {
                struct subscriber *sub = &subs[i];
                if (!FD_ISSET(sub->fd, &sub_fd_set))
                    continue;

                enum log_origin orig = LOG_ORIGIN_UNKNOWN;
                int ret = recv(sub->fd, &orig, sizeof(orig), 0);
                if (ret == -1) {
                    perror("recv");
                    exit(EXIT_FAILURE);
                }
                if (ret == 0) {
                    printf("Subscriber %d disconnected\n", sub->fd);
                    int fd = sub->fd;
                    sub_fd_set_rem(fd);
                    sub_fds_origin_rem(fd);
                    close(fd);
                    continue;
                }

                sub->origin = orig;
                printf("Subscriber %d subscribed to %s\n", sub->fd, LOG_ORIGIN_STR(orig));

                sub_fds_origin_add(sub->fd, orig);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int ret;
    argc = argc;
    argv = argv;

    pthread_t thread_pub, thread_sub;
    PTHREAD_CREATE(thread_pub, publisher);
    PTHREAD_CREATE(thread_sub, subscriber);

    ret = pthread_join(thread_pub, NULL);
    if (ret != 0) {
        perror("pthread_join");
        return EXIT_FAILURE;
    }

    ret = pthread_join(thread_sub, NULL);
    if (ret != 0) {
        perror("pthread_join");
        return EXIT_FAILURE;
    }

    return 0;
}

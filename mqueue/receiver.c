#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#define MAX_MESSAGES      10
#define MAX_MSG_SIZE      256
#define QUEUE_PERMISSIONS 0660

int
main(int argc, char *argv[])
{
    int mq_fd;

    if (argc <= 1) {
        printf("usage: receiver [/mq-name]\n");
        exit(EXIT_SUCCESS);
    }

    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MESSAGES,
        .mq_msgsize = MAX_MSG_SIZE,
        .mq_curmsgs = 0
    };

    mq_fd = mq_open(argv[1], O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr);
    if (mq_fd == -1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(mq_fd, &readfds);

    for (;;) {
        char buffer[MAX_MSG_SIZE];

        printf("Waiting message from sender\n");
        select(mq_fd + 1, &readfds, NULL, NULL, NULL);
        if (FD_ISSET(mq_fd, &readfds)) {
            printf("Receiving message from %s queue\n", argv[1]);
            memset(buffer, 0, MAX_MSG_SIZE);
            if (mq_receive(mq_fd, buffer, MAX_MSG_SIZE, NULL) == -1) {
                perror("mq_receive");
                exit(EXIT_FAILURE);
            }
            printf("Message received from queue: %s\n", buffer);
        }
    }

    exit(EXIT_SUCCESS);
}
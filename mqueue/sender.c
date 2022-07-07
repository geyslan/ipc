#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#define MAX_MSG_SIZE 255

#define xstr1(s) #s
#define str(s) xstr1(s)

int
main(int argc, char *argv[])
{
    char buffer[MAX_MSG_SIZE + 1];
    int mq_fd;

    if (argc <= 1) {
        printf("usage: sender [/mq-name]\n");
        exit(EXIT_SUCCESS);
    }

    memset(buffer, 0, MAX_MSG_SIZE + 1);
    printf("Enter message to be enqueued in %s: ", argv[1]);
    scanf("%" str(MAX_MSG_SIZE) "s", buffer);

    mq_fd = mq_open(argv[1], O_WRONLY | O_CREAT);
    if (mq_fd == -1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    if (mq_send(mq_fd, buffer, strlen(buffer) + 1, 0) == -1) {
        perror("mq_send");
        exit(EXIT_FAILURE);
    }

    mq_close(mq_fd);

    exit(EXIT_SUCCESS);
}
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "log.h"

struct log *log_new(enum log_level level, char *msg)
{
    if (!msg) {
        return NULL;
    }

    struct log *log;

    log = malloc(sizeof(struct log));
    if (!log) {
        return NULL;
    }

    log->level = level;
    log->msg = strndup(msg, LOG_MAX_MSG_SIZE-1);
    if (!log->msg) {
        goto err;
    }

    log->msg_size = (uint16_t) strlen(log->msg) + 1;

    // log must be freed by caller via 'log_free()'
    return log;

err:
    free(log);
    return NULL;
}

void log_free(struct log *log)
{
    if (!log) {
        return;
    }

    free(log->msg);
    free(log);
}

char *log_encode(struct log *log, uint8_t origin, int16_t *size)
{
    if (!log || !size) {
        return NULL;
    }

    char *buf, *pos;
    int16_t raw_size = sizeof(log->level) +
                              log->msg_size;

    int16_t buf_size = LOG_METADATA_SIZE +
                       raw_size;
    buf = malloc(buf_size);
    if (!buf) {
        return NULL;
    }

    pos = buf;
    // log_metadata_decode()
    memcpy(pos, &origin, sizeof(origin));
    pos += sizeof(origin);
    memcpy(pos, &raw_size, sizeof(raw_size));
    pos += sizeof(raw_size);

    // log_decode()
    memcpy(pos, &log->level, sizeof(log->level));
    pos += sizeof(log->level);
    memcpy(pos, log->msg, log->msg_size);

    // return the size of the buffer
    *size = buf_size;

    // buf must be freed by caller
    return buf;
}

void log_metadata_decode(char *buf, struct log_metadata *metadata)
{
    if (!buf || !metadata) {
        return;
    }

    char *pos = buf;

    memcpy(&metadata->origin, pos, sizeof(metadata->origin));
    pos += sizeof(metadata->origin);
    memcpy(&metadata->raw_size, pos, sizeof(metadata->raw_size));
}

struct log *log_decode(char *buf)
{
    if (!buf) {
        return NULL;
    }

    struct log tmp_log; // used to get field sizes
    char *pos;
    char *msg;

    pos = buf;
    memcpy(&tmp_log.level, pos, sizeof(tmp_log.level));
    pos += sizeof(tmp_log.level);
    msg = pos;

    // log must be freed by caller via 'log_free()'
    return log_new(tmp_log.level, msg);
}

void read_into(char *buf, int pos, void *dst, int size)
{
    if (!buf || !dst) {
        return;
    }

    memcpy(dst, buf + pos, size);
}

int send_log(int data_socket, struct log *log, enum log_origin origin)
{
    int ret;
    int16_t bsize;
    char *buf = log_encode(log, origin, &bsize);
    if (!buf) {
        perror("log_encode");
        ret = -1;
        goto err;
    }

    ret = send(data_socket, buf, bsize, 0);
    if (ret != bsize) {
        perror("send");
        ret = -1;
        goto err1;
    }

err1:
    free(buf);
err:
    return ret;
}
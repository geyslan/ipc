#ifndef __LOG_H__
#define __LOG_H__

#include <stdint.h>
#include <stdio.h>

#include "log-hub.h"

#define LOG_MAX_MSG_SIZE 1024

enum log_level: uint8_t {
    LOG_LEVEL_DEBUG = 1,
    LOG_LEVEL_ERROR = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_WARN = 4
};

struct log *log_new(enum log_level level, char *msg);
void log_free(struct log *log);
char *log_encode(struct log *log, uint8_t origin, int16_t *size);
struct log *log_decode(char *buf);

struct log {
    enum log_level level;
    char *msg;
    uint16_t msg_size;
};

struct log_metadata {
    enum log_origin origin;
    uint16_t raw_size;
};

void log_metadata_decode(char *buf, struct log_metadata *metadata);
int send_log(int data_socket, struct log *log, enum log_origin origin);

#define LOG_METADATA_SIZE (sizeof(((struct log_metadata *) 0)->origin) + \
                           sizeof(((struct log_metadata *) 0)->raw_size))

#define LOG_RAW_MAX_SIZE (sizeof(((struct log *) 0)->level) + \
                          LOG_MAX_MSG_SIZE)

#endif // __LOG_H__
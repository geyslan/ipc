#ifndef __LOG_HUB_H__
#define __LOG_HUB_H__

#include <pthread.h>
#include <stdint.h>

#include "log.h"

#define PUB_SOCK_NAME "/tmp/log-hub-pub.sock"
#define SUB_SOCK_NAME "/tmp/log-hub-sub.sock"

#define STR(v) #v

enum log_origin: uint8_t {
    LOG_ORIGIN_UNKNOWN = 0,
    LOG_ORIGIN_DB      = 1,
    LOG_ORIGIN_APP     = 2,
    LOG_ORIGIN_IOT     = 3,
    LOG_ORIGIN_SYS     = 4
};

#define MAX_LOG_ORIGIN 5

#define LOG_ORIGIN_UNKNOWN_STR STR(LOG_ORIGIN_UNKNOWN)
#define LOG_ORIGIN_DB_STR      STR(LOG_ORIGIN_DB)
#define LOG_ORIGIN_APP_STR     STR(LOG_ORIGIN_APP)
#define LOG_ORIGIN_IOT_STR     STR(LOG_ORIGIN_IOT)
#define LOG_ORIGIN_SYS_STR     STR(LOG_ORIGIN_SYS)

#define LOG_ORIGIN_STR(origin) \
    ({ \
        char *str; \
        switch (origin) { \
            case LOG_ORIGIN_UNKNOWN: \
                str = LOG_ORIGIN_UNKNOWN_STR; \
                break; \
            case LOG_ORIGIN_DB: \
                str = LOG_ORIGIN_DB_STR; \
                break; \
            case LOG_ORIGIN_APP: \
                str = LOG_ORIGIN_APP_STR; \
                break; \
            case LOG_ORIGIN_IOT: \
                str = LOG_ORIGIN_IOT_STR; \
                break; \
            case LOG_ORIGIN_SYS: \
                str = LOG_ORIGIN_SYS_STR; \
                break; \
            default: \
                str = LOG_ORIGIN_UNKNOWN_STR; \
                break; \
        } \
        str; \
    })

// make it enclosed in a own scope
#define PTHREAD_CREATE(thread, v)                          \
    ret = pthread_create(&thread, NULL, thread_##v, NULL); \
    if (ret != 0) {                                        \
        perror("pthread_create");                          \
        return EXIT_FAILURE;                               \
    }                                                      \
                                                           \
    printf("* Created thread for " #v "\n");

#endif // __LOG_HUB_H__
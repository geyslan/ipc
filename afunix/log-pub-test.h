#ifndef __LOG_PUB_TEST_H__
#define __LOG_PUB_TEST_H__

#include "log.h"
#include "log-hub.h"

struct tt {
    uint8_t origin;
    enum log_level level;
    char *msg;
};

struct tt tt_app[] = {
    { LOG_ORIGIN_APP, LOG_LEVEL_INFO, "INFO from APP" },
    { LOG_ORIGIN_APP, LOG_LEVEL_WARN, "WARN from APP" },
    { LOG_ORIGIN_APP, LOG_LEVEL_ERROR, "ERROR from APP" },
    { LOG_ORIGIN_APP, LOG_LEVEL_DEBUG, "DEBUG from APP" }
};

struct tt tt_db[] = {
    { LOG_ORIGIN_DB, LOG_LEVEL_INFO, "INFO from DB" },
    { LOG_ORIGIN_DB, LOG_LEVEL_WARN, "WARN from DB" },
    { LOG_ORIGIN_DB, LOG_LEVEL_ERROR, "ERROR from DB" },
    { LOG_ORIGIN_DB, LOG_LEVEL_DEBUG, "DEBUG from DB" }
};

struct tt tt_iot[] = {
    { LOG_ORIGIN_IOT, LOG_LEVEL_INFO, "INFO from IOT" },
    { LOG_ORIGIN_IOT, LOG_LEVEL_WARN, "WARN from IOT" },
    { LOG_ORIGIN_IOT, LOG_LEVEL_ERROR, "ERROR from IOT" },
    { LOG_ORIGIN_IOT, LOG_LEVEL_DEBUG, "DEBUG from IOT" }
};

struct tt tt_sys[] = {
    { LOG_ORIGIN_SYS, LOG_LEVEL_INFO, "INFO from SYS" },
    { LOG_ORIGIN_SYS, LOG_LEVEL_WARN, "WARN from SYS" },
    { LOG_ORIGIN_SYS, LOG_LEVEL_ERROR, "ERROR from SYS" },
    { LOG_ORIGIN_SYS, LOG_LEVEL_DEBUG, "DEBUG from SYS" }
};

#endif // __LOG_PUB_TEST_H__
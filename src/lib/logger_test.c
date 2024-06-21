#include "logger.h"
#include <assert.h>
#include <stdio.h>

#define LOGFILE "./logfile.log"

int main (void){
    LoggerConfig cfg = {
        .log_prefix = "This_is a Test 0123456789 &",
        .with_datetime = true,
        .with_level = true,
        .flush_immediately = true,
        .min_log_level = LOGGER_DEFAULT_MIN_LOG_LEVEL
    };
    Logger logger = Logger_create(cfg, LOGFILE);
    assert(logger != NULL);

    assert(! LOG_DEBUG("msg"));
    assert(! LOG_VERBOSE("msg"));
    assert(LOG_MSG("hello"));
    assert(LOG_ERR("this is a test, %s", "a nice test"));
    assert(LOG_MSG("%d I am printing two variables! %d", -15, 2378));

    Logger_cleanup(logger);

    assert(system("python3 check_logs.py") == 0);

    puts("Logger: All tests passed!");
}

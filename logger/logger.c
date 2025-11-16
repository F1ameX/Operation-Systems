#include "logger/logger.h"

#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

static log_level_t g_level = LOG_INFO;
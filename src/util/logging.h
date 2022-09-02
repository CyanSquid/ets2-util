#ifndef __UTIL__LOGGING__
#define __UTIL__LOGGING__

#include <stdbool.h>

#define LOG_MSG(fmt, ...) logger__log_msg(fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) logger__log_err(fmt, ##__VA_ARGS__)
#define LOG_FTL(fmt, ...) logger__log_ftl(fmt, ##__VA_ARGS__)

#define LOG_DBG(fmt, ...) logger__log_dbg(fmt, ##__VA_ARGS__)

void logger__log_msg(const char* fmt, ...);
void logger__log_err(const char* fmt, ...);
void logger__log_ftl(const char* fmt, ...);
void logger__log_dbg(const char* fmt, ...);

bool logger__initialize(void);
void logger__uninitialize(void);

#endif // __UTIL__LOGGING__
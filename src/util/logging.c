#include "logging.h"

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <share.h>
#include <stdarg.h>
#include <time.h>

static FILE* file_log = NULL;

#define TIME_FMT_DATE "%02d-%02d-%04d"
#define TIME_FMT_TIME "%02d:%02d:%02d"

static void get_time(struct tm* out)
{
    time_t current_time;

    time(&current_time);
    localtime_s
    (
        out,
        &current_time
    );
    return;
}

bool logger__initialize(void)
{
    if (file_log != NULL)
    {
        return true;
    }

    file_log = _fsopen("ets2util.log", "a+", _SH_DENYNO);

    return (file_log != NULL);
}

void logger__uninitialize(void)
{
    if (file_log != NULL)
    {
        fclose(file_log);
    }
    return;
}

static void write_type(const char* prefix, const char* format, va_list args)
{
    char message[0xFF];
    char file_output[0x130];
    char time_str[0x20];

    struct tm time = { 0 };

    get_time(&time);

    vsprintf_s(message, sizeof(message), format, args);
    sprintf_s(time_str, sizeof(time_str), TIME_FMT_DATE " " TIME_FMT_TIME, time.tm_mday, (time.tm_mon + 1), (time.tm_year + 1900), time.tm_hour, time.tm_min, time.tm_sec);
    sprintf_s(file_output, sizeof(file_output), "[%s] [%s] %s\n", time_str, prefix, message);

    if (file_log)
    {
        fputs(file_output, file_log);
        fflush(file_log);
    }
    return;
}

void logger__log_msg(const char* fmt, ...)
{
    va_list params;
    va_start(params, fmt);
    write_type("msg", fmt, params);
    va_end(params);
    return;
}

void logger__log_err(const char* fmt, ...)
{
    va_list params;
    va_start(params, fmt);
    write_type("err", fmt, params);
    va_end(params);
    return;
}

void logger__log_ftl(const char* fmt, ...)
{
    va_list params;
    va_start(params, fmt);
    write_type("ftl", fmt, params);
    va_end(params);
    return;
}

void logger__log_dbg(const char* fmt, ...)
{
    va_list params;
    va_start(params, fmt);
    write_type("dbg", fmt, params);
    va_end(params);
    return;
}
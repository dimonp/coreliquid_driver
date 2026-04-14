#include "logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

static int dosyslog = 0;

void open_log(int syslog, const char* ident)
{
    dosyslog = syslog;

    if (dosyslog)
        openlog(ident, LOG_PID, LOG_DAEMON);
}

void close_log(void)
{
    if (dosyslog)
        closelog();
}


void loginfo(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

#ifndef _DEBUG
    if (dosyslog)
        vsyslog(LOG_INFO, format, ap);
    else
#endif
        vfprintf(stderr, format, ap);

    va_end(ap);
}

void logerror(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

#ifndef _DEBUG
    if (dosyslog)
        vsyslog(LOG_ERR, format, ap);
    else
#endif
        vfprintf(stderr, format, ap);

    va_end(ap);
}

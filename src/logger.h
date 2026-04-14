#ifndef _LOGGER__H
#define _LOGGER__H

void open_log(int syslog, const char* ident);
void close_log(void);
void loginfo(const char *format, ...);
void logerror(const char *format, ...);

#endif // _LOGGER__H

#ifndef _CORELIQUID_HID__H
#define _CORELIQUID_HID__H

#include <stddef.h>
#include <inttypes.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

void loginfo(const char *format, ...);
void logerror(const char *format, ...);


struct coreliquid_device_;
typedef struct coreliquid_device_ coreliquid_device;

void init_coreliquid(int syslog);
void shutdown_coreliquid(void);

/**
* Searches for and opens a CoreLiquid HID device matching any of the specified vendor and product IDs.
*
* @param vids Array of vendor IDs to search for.
* @param vids_len Length of the vendor IDs array.
* @param pids Array of product IDs to search for.
* @param pids_len Length of the product IDs array.
* @return Handler of the opened CoreLiquid device if found; NULL otherwise.
*/
coreliquid_device* search_and_open_device(const uint16_t *vids, size_t vids_len, const uint16_t *pids, size_t pids_len);
void close_coreliquid_device(coreliquid_device *cl_handle);
int set_report(coreliquid_device* cl_handle, uint8_t* output_report, size_t length);
int write_output(coreliquid_device* cl_handle, uint8_t* output_report, size_t length);
int read_input(coreliquid_device* cl_handle, uint8_t* input_report, size_t length);

#endif // _CORELIQUID_HID__H

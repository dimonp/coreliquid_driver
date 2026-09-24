#ifndef _CORELIQUID_HID__H
#define _CORELIQUID_HID__H

#include <stddef.h>
#include <stdint.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

typedef struct {
    uint16_t vid;
    uint16_t pid;
} usb_device_id_t;

struct coreliquid_device_;
typedef struct coreliquid_device_ coreliquid_device;

void init_coreliquid(void);
void shutdown_coreliquid(void);

/**
* Searches for and opens a CoreLiquid HID device matching any of the specified vendor and product IDs.
*
* @param supported_devices Array of vendor and product IDs to search for.
* @param supported_devices_len Length of the vendor and product IDs array.
* @return Handler of the opened CoreLiquid device if found; NULL otherwise.
*/
coreliquid_device* search_and_open_device(
    const usb_device_id_t *supported_devices,
    size_t supported_devices_len
);

void close_coreliquid_device(coreliquid_device *cl_handle);
int set_report(coreliquid_device* cl_handle, uint8_t* output_report, size_t length);
int get_report(coreliquid_device* cl_handle, uint8_t* input_report, size_t length);
int write_output(coreliquid_device* cl_handle, uint8_t* output_report, size_t length);
int read_input(coreliquid_device* cl_handle, uint8_t* input_report, size_t length);

#endif // _CORELIQUID_HID__H

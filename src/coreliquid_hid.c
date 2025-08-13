#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include <hidapi/hidapi.h>

#include "coreliquid_hid.h"

static int dosyslog = 0;

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

struct coreliquid_device_ {
    hid_device* hid_device_handle;
};
typedef struct coreliquid_device_ coreliquid_device;

void init_coreliquid(int syslog)
{
    dosyslog = syslog;

    if (dosyslog)
        openlog("MSI_Coreliquid", LOG_PID, LOG_DAEMON);        

    hid_init();
}

void shutdown_coreliquid(void)
{
    hid_exit();

    if (dosyslog)
        closelog();
}

/**
* Opens a connection to a CoreLiquid device with specified vendor and product IDs.
*
* @param vid The vendor ID of the CoreLiquid device to open.
* @param pid The product ID of the CoreLiquid device to open.
* @return Pointer to the initialized coreliquid_device structure if successful; 0 on failure.
*/
coreliquid_device* open_coreliquid_device(uint16_t vid, uint16_t pid) 
{
    coreliquid_device *cl_handle = NULL;

    cl_handle = (coreliquid_device*) calloc(1, sizeof(coreliquid_device));
	if (!cl_handle)
		return NULL;

    hid_device* handle = hid_open(vid, pid, NULL);
    if (handle == NULL) {
        logerror("Failed to open device (%hx:%hx): %ls \n", vid, pid, hid_error(NULL));
        free(cl_handle);
        return NULL;
    }

    cl_handle->hid_device_handle = handle;
    return cl_handle;
}

/**
* Closes and frees the CoreLiquid device handle.
*
* @param cl_handle Pointer to the CoreLiquid device handle to be closed and freed.
*/
void close_coreliquid_device(coreliquid_device *cl_handle)
{
	if (!cl_handle)
		return;

    hid_close(cl_handle->hid_device_handle);
	free(cl_handle);
}

/**
* Searches for and opens a CoreLiquid device matching specified vendor and product IDs.
*
* @param vids Array of vendor IDs to search for.
* @param vids_len Length of the vendor IDs array.
* @param pids Array of product IDs to search for.
* @param pids_len Length of the product IDs array.
* @return Pointer to the opened CoreLiquid device if found; 0 otherwise.
*/
coreliquid_device* search_and_open_device(const uint16_t *vids, size_t vids_len, const uint16_t *pids, size_t pids_len) 
{
    struct hid_device_info *devices = hid_enumerate(0x0, 0x0);
    struct hid_device_info *current_dev = devices;

    coreliquid_device* result = {0};
    while (current_dev) {

        for (size_t i = 0; i < vids_len; ++i) {
            if (current_dev->vendor_id != vids[i]) {
                continue;
            }

            for (size_t j = 0; j < pids_len; ++j) {
                if (current_dev->product_id != pids[j])
                    continue;

                loginfo("Found device: %s %hx:%hx interface: %i bus_type: %i manufacturer: %ls product: %ls\n",                     
                    current_dev->path, current_dev->vendor_id, current_dev->product_id, 
                    current_dev->interface_number, current_dev->bus_type,
                    current_dev->manufacturer_string, current_dev->product_string);

                result = open_coreliquid_device(current_dev->vendor_id, current_dev->product_id);
                goto found;
            }
        }

        current_dev = current_dev->next;
    }

found:
    hid_free_enumeration(devices);

    return result;
}

/**
* Sends a feature report to the HID device.
* 
* @param cl_handle Pointer to the coreliquid device handle.
* @param output_report Pointer to the report data to be sent.
* @param length Length of the report data in bytes.
* @return 1 if the report was successfully sent, 0 otherwise.
*/
int set_report(coreliquid_device* cl_handle, uint8_t* output_report, size_t length) 
{

    int ret = hid_send_feature_report(cl_handle->hid_device_handle, output_report, length);
    for (int i = 0; (i < 10) && ret < 0; ++i) {
        ret = hid_send_feature_report(cl_handle->hid_device_handle, output_report, length);
        usleep(1000);
    }

    if (ret < 0) {
#ifdef _DEBUG
        logerror("Unable to set report: %ls\n", hid_error(cl_handle->hid_device_handle));
#endif        
        return 0;
    }

    return 1;
}

/**
* Writes an output report to the CoreLiquid HID device.
*
* @param cl_handle Pointer to the CoreLiquid device handle.
* @param output_report Pointer to the output report data to be written.
* @param length Size of the output report data.
* @return 1 if the write operation was successful, 0 otherwise.
*/
int write_output(coreliquid_device* cl_handle, uint8_t* output_report, size_t length) 
{
    int res = hid_write(cl_handle->hid_device_handle, output_report, length);
    if (res < 0) {
#ifdef _DEBUG
        logerror("Unable to write output: %ls\n", hid_error(cl_handle->hid_device_handle));
#endif
        return 0;
    }
        
    return 1;
}

/**
* Reads an input report from the CoreLiquid HID device.
*
* @param cl_handle Pointer to the CoreLiquid device handle.
* @param input_report Buffer to store the read input report.
* @param length Size of the input report buffer.
* @return 1 if the read operation was successful, 0 otherwise.
*/
int read_input(coreliquid_device* cl_handle, uint8_t* input_report, size_t length) 
{
    int res = hid_read(cl_handle->hid_device_handle, input_report, length);
    if (res < 0) {
#ifdef _DEBUG
        logerror("Unable to read input: %ls\n", hid_error(cl_handle->hid_device_handle));
#endif
        return 0;
    }

    return 1;
}


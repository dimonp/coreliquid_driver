#ifndef _CORELIQUID__H
#define _CORELIQUID__H

#include "coreliquid_hid.h"

enum fan_mode {
    FAN_MODE_SILENT = 0,
    FAN_MODE_BALANCE = 1,
    FAN_MODE_GAME = 2,
    FAN_MODE_CUSTOM = 3,
    FAN_MODE_DEFAULT = 4,
    FAN_MODE_SMART = 5
};
typedef enum fan_mode fan_mode_t;

int get_cooler_status(coreliquid_device* handle, int* temperature_in, int* temperature_out, int* fan_speed);
void set_fan_mode(coreliquid_device* handle, fan_mode_t fan_mode);
void set_oled_cpu_status(coreliquid_device* handle, int temperature, int frequency);
void set_oled_show_clock(coreliquid_device* handle, uint8_t style);
int get_model_index(coreliquid_device* handle, int* model_idx);
int get_fw_version_ldprom(coreliquid_device* handle, int* version_major, int* version_minor);

coreliquid_device* open_device_aio(void);

#endif // _CORELIQUID__H

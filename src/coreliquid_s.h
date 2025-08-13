#ifndef _CORELIQUID_S__H
#define _CORELIQUID_S__H

#include "coreliquid_hid.h"

enum lcm_dir {
    LCM_DIR_90      = 90,
    LCM_DIR_180     = 180,
    LCM_DIR_270     = 270,
    LCM_DIR_DEFAULT = 0
};
typedef enum lcm_dir lcm_dir_t;

#define LCM_DEFAULT_BRIGHTNESS 100

enum monitor_style {
    STYLE_1 = 1,
    STYLE_2 = 2,
    STYLE_3 = 3,
    STYLE_4 = 4,
};
typedef enum monitor_style monitor_style_t;

#define DISPLAY_FEATURES_COUNT 15
enum display_features {
    SHOW_CPU_FREQ           = 0b0000000000000001,
    SHOW_CPU_TEMP           = 0b0000000000000010,
    SHOW_GPU_FREQ           = 0b0000000000000100,
    SHOW_GPU_USAGE          = 0b0000000000001000,
    SHOW_PUMP_FAN           = 0b0000000000010000,
    SHOW_RADIATOR_FAN       = 0b0000000000100000,
    SHOW_WATER_BLOCK_FAN    = 0b0000000001000000,
    SHOW_PSU_FAN            = 0b0000000010000000,
    SHOW_LIQUID_TEMP        = 0b0000000100000000,
    SHOW_FPS                = 0b0000001000000000,
    SHOW_PSU_TEMP           = 0b0000010000000000,
    SHOW_PSU_OUTPUT_WATTAGE = 0b0000100000000000,
    SHOW_PSU_EFFICIENCY     = 0b0001000000000000,
    SHOW_CPU_USAGE          = 0b0010000000000000,
    SHOW_GPU_TEMP           = 0b0100000000000000
};
typedef enum display_features display_features_t;

#define DEFAULT_DISPLAY_FEATURES (SHOW_CPU_TEMP | SHOW_PUMP_FAN | SHOW_RADIATOR_FAN)


void send_cpu_temperature(coreliquid_device *cl_handle, int cpu_temperature, int cpu_frequqency);
void set_lcm_back_light(coreliquid_device *handle, int brightness);
void set_lcm_direction(coreliquid_device *handle, lcm_dir_t direction);
void send_host_text(coreliquid_device *handle, const char *text);
void set_display_mode(coreliquid_device *cl_handle, display_features_t features, monitor_style_t style);
coreliquid_device* open_s_device();

#endif // _CORELIQUID_S__H

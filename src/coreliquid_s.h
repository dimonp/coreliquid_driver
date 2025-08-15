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
    SHOW_CPU_FREQ           = 0x0001,
    SHOW_CPU_TEMP           = 0x0002,
    SHOW_GPU_FREQ           = 0x0004,
    SHOW_GPU_USAGE          = 0x0008,
    SHOW_PUMP_FAN           = 0x0010,
    SHOW_RADIATOR_FAN       = 0x0020,
    SHOW_WATER_BLOCK_FAN    = 0x0040,
    SHOW_PSU_FAN            = 0x0080,
    SHOW_LIQUID_TEMP        = 0x0100,
    SHOW_FPS                = 0x0200,
    SHOW_PSU_TEMP           = 0x0400,
    SHOW_PSU_OUTPUT_WATTAGE = 0x0800,
    SHOW_PSU_EFFICIENCY     = 0x1000,
    SHOW_CPU_USAGE          = 0x2000,
    SHOW_GPU_TEMP           = 0x4000
};
typedef enum display_features display_features_t;

#define DEFAULT_DISPLAY_FEATURES (SHOW_CPU_TEMP | SHOW_PUMP_FAN | SHOW_RADIATOR_FAN)


void send_cpu_info(coreliquid_device *cl_handle, int temperature, int frequency);
void set_lcm_back_light(coreliquid_device *handle, int brightness);
void set_lcm_direction(coreliquid_device *handle, lcm_dir_t direction);
int send_host_msg(coreliquid_device *handle, const char *text);
void set_display_mode(coreliquid_device *cl_handle, display_features_t features, monitor_style_t style);
void set_sync_mode(coreliquid_device *handle, int mode);
void set_temperature_unit(coreliquid_device *handle, int unit);

coreliquid_device* open_s_device(void);

#endif // _CORELIQUID_S__H

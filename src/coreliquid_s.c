#include <string.h>

#include "coreliquid_hid.h"
#include "coreliquid_s.h"

#define HID_REPORT_SIZE 64
#define REPORT_ID_S360 1

#define MAGIC_CODE_MCU 0x5a6b

#define REFLASH_TIME 5

static const uint16_t supported_vids[] = { 0x0db0, 0x1462 };
static const uint16_t supported_pids[] = { 0x5259, 0x75B6, 0x8DBF, 0x9BA6, 0xC7B2, 0xD085 };

enum display_mode {
    DISPLAY_MODE_HW_MONITOR = 0,
    DISPLAY_MODE_IMAGE      = 1,
    DISPLAY_MODE_BANNER     = 2,
    DISPLAY_MODE_CLOCK      = 5,
    DISPLAY_MODE_WEATHER    = 6,
    DISPLAY_MODE_DYNAMIC    = 7,
};

enum command_code_s360 {
    GET_DEV_ID               = 0x10,
    GET_DEV_ID_R             = 0x11,
    GET_DEV_INFO             = 0x14,
    GET_DEV_INFO_R           = 0x15,
    SET_DEV_TIME             = 0x16,
    SET_LCM_BACKLIGHT        = 0x18,
    SET_LCM_DIR              = 0x1A,
    SET_LCM_RESET            = 0x1C,
    SEND_FILE_NODE           = 0x20,
    SEND_FILE_NODE_R         = 0x21,
    HMI_SEND_FILE_DATA_START = 0x22,
    HMI_SEND_FILE_DATA_END   = 0x26,
    SAVE_FILE_NODE           = 0x28,
    DEL_FILE_NODE            = 0x2A,
    SEND_HOST_CPU_INFO       = 0x30,
    SEND_HOST_FAN_SPEED      = 0x32,
    SEND_HOST_FPS            = 0x34,
    SEND_HOST_TIME           = 0x36,
    SEND_HOST_WEATHER        = 0x38,
    SEND_HOST_MSG            = 0x3A,
    SEND_HOST_GPU_INFO       = 0x3C,
    PLAY_MEDIA_NODE          = 0x40,
    STOP_MEDIA_NODE          = 0x42,
    PAUSE_MEDIA_NODE         = 0x44,
    SET_DISPLAY_MODE         = 0x50,
    GET_DISPLAY_MODE         = 0x52,
    GET_DISPLAY_MODE_R       = 0x53,
};
typedef enum command_code_mcu command_code_mcu_t;

#pragma pack(1)

struct message_header_s {
    uint8_t  report_id;
    uint16_t magic_two; // always = MAGIC_CODE_S (0x5a6b)
    uint16_t command_code;
    uint32_t length;
};

// SEND_HOST_CPU_INFO
struct hw_info {
    struct message_header_s header;
    struct {
        uint16_t cpu_freq;
        uint16_t cpu_temp;
        uint16_t gpu_freq;
        uint16_t gpu_usage;
        uint16_t pump_fan;
        uint16_t radiator_fan;
        uint16_t water_block_fan;
        uint16_t psu_fan;
        uint16_t liquid_temp;
        uint16_t fps;
        uint16_t psu_temp;
        uint16_t psu_output_wattage;
        uint16_t psu_efficiency;
        uint16_t cpu_usage;
        uint16_t gpu_temp;
        uint16_t reserved;
    } payload;
};
struct hw_info_message {
    union {
        struct hw_info data;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

// SET_DISPLAY_MODE
struct is_show_info {
    uint8_t cpu_freq;
    uint8_t cpu_temp;
    uint8_t gpu_freq;
    uint8_t gpu_usage;
    uint8_t pump_fan;
    uint8_t radiator_fan;
    uint8_t water_block_fan;
    uint8_t psu_fan;
    uint8_t liquid_temp;
    uint8_t fps;
    uint8_t psu_temp;
    uint8_t psu_output_wattage;
    uint8_t psu_efficiency;
    uint8_t cpu_usage;
    uint8_t gpu_temp;
    uint8_t reserved;
};
struct hw_monitor {
    struct message_header_s header;
    struct {
        uint8_t mode;
        uint8_t slide_show;
        uint8_t count;
        struct is_show_info show_info;
    } payload;
};
struct hw_monitor_message {
    union {
        struct hw_monitor data;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

// SET_LCM_BACKLIGHT
struct back_light {
    struct message_header_s header;
    struct {
        uint32_t brightness;
    } payload;
};
struct back_light_message {
    union {
        struct back_light data;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

// SET_LCM_DIR
struct lcm_direction {
    struct message_header_s header;
    struct {
        uint32_t direction;
    } payload;
};
struct lcm_direction_message {
    union {
        struct lcm_direction data;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

// SEND_HOST_MSG
struct host_text {
    struct message_header_s header;
    struct {
        char text[55];
    } payload;
};
struct host_text_message {
    union {
        struct host_text data;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

#pragma pack()

/**
* Sends CPU temperature and frequency information to the device.
*
* @param cl_handle Pointer to the coreliquid device handle.
* @param temperature The CPU temperature (Celsius).
* @param frequency The CPU frequency (MHz).
* @param usage The CPU usage (%).
*/
void send_cpu_info(coreliquid_device *cl_handle, int temperature, int frequency)
{
    struct hw_info_message message;
    memset(&message.raw_buffer, 0, sizeof(message.raw_buffer));

    message = (struct hw_info_message) {
        .data = {
            .header = {
                .report_id = REPORT_ID_S360,
                .magic_two = MAGIC_CODE_MCU,
                .command_code = SEND_HOST_CPU_INFO,
                .length = sizeof(message.data.payload),
            },
            .payload = {
                .cpu_temp = temperature,
                .cpu_freq = frequency,
            }
        }
    };

    set_report(cl_handle, message.raw_buffer, sizeof(message.raw_buffer));
}

/**
* Sets the backlight brightness of the LCM (LCD Module).
*
* @param handle Pointer to the coreliquid_device instance.
* @param brightness The brightness level to set for the backlight.
*/
void set_lcm_back_light(coreliquid_device *handle, int brightness)
{
    struct back_light_message message;
    memset(&message.raw_buffer, 0, sizeof(message.raw_buffer));

    message = (struct back_light_message) {
        .data = {
            .header = {
                .report_id = REPORT_ID_S360,
                .magic_two = MAGIC_CODE_MCU,
                .command_code = SET_LCM_BACKLIGHT,
                .length = sizeof(message.data.payload),
            },
            .payload = {
                .brightness = brightness,
            }
        }
    };

    set_report(handle, message.raw_buffer, sizeof(message.raw_buffer));
}

/**
* Sets the direction of the LCM (LCD Module) for a coreliquid device.
*
* @param handle Pointer to the coreliquid device handle.
* @param direction The direction to set for the LCM (lcm_dir_t value).
*/
void set_lcm_direction(coreliquid_device *handle, lcm_dir_t direction)
{
    struct lcm_direction_message message;
    memset(&message.raw_buffer, 0, sizeof(message.raw_buffer));

    message = (struct lcm_direction_message) {
        .data = {
            .header = {
                .report_id = REPORT_ID_S360,
                .magic_two = MAGIC_CODE_MCU,
                .command_code = SET_LCM_DIR,
                .length = sizeof(message.data.payload),
            },
            .payload = {
                .direction = direction,
            }
        }
    };

    set_report(handle, message.raw_buffer, sizeof(message.raw_buffer));
}

/**
 * Sends text to display on screen ().
 *
 * @param handle Pointer to the coreliquid device handle.
 * @param text The text to be sent to the device.
 */
void send_host_text(coreliquid_device *handle, const char *text)
{
    struct host_text_message message;
    memset(&message.raw_buffer, 0, sizeof(message.raw_buffer));

    message = (struct host_text_message) {
        .data = {
            .header = {
                .report_id = REPORT_ID_S360,
                .magic_two = MAGIC_CODE_MCU,
                .command_code = SEND_HOST_MSG,
                .length = sizeof(message.data.payload),
            },
        }
    };
    strncpy(message.data.payload.text, text, sizeof(message.data.payload.text));

    set_report(handle, message.raw_buffer, sizeof(message.raw_buffer));
}

/**
* Sets the hardware display mode for a Coreliquid device.
*
* @param cl_handle Pointer to the CoreLiquid device handle.
* @param features Display features to be set (bitmask).
* @param style Monitor style to be applied to the features.
*/
void set_display_mode(coreliquid_device *cl_handle, display_features_t features, monitor_style_t style)
{
    struct hw_monitor_message message;
    memset(&message.raw_buffer, 0, sizeof(message.raw_buffer));

    uint8_t features_buf[DISPLAY_FEATURES_COUNT] = { 0 };
    size_t i = 0;
    for (size_t i = 0 ; (i < sizeof(features_buf)) && features; ++i, features >>= 1) {
        if (features & 1) {
            features_buf[i] = style;
        }
    }

    message = (struct hw_monitor_message) {
        .data = {
            .header = {
                .report_id = REPORT_ID_S360,
                .magic_two = MAGIC_CODE_MCU,
                .command_code = SET_DISPLAY_MODE,
                .length = sizeof(message.data.payload),
            },
            .payload = {
                .mode = DISPLAY_MODE_HW_MONITOR,
                .slide_show = REFLASH_TIME,
                .count = sizeof(features_buf),
            }
        }
    };
    memcpy(&(message.data.payload.show_info), features_buf, sizeof(features_buf));

    set_report(cl_handle, message.raw_buffer, sizeof(message.raw_buffer));
}


/**
* Opens a Coreliquid S* device by searching for supported VID/PID combinations.
*
* @return Pointer to the opened device handle if successful; NULL otherwise.
*/
coreliquid_device* open_s_device()
{
    coreliquid_device* cl_handle = NULL;
    cl_handle = search_and_open_device(supported_vids, ARRAY_SIZE(supported_vids), supported_pids, ARRAY_SIZE(supported_pids));
    if (!cl_handle) {
        logerror("Failed to open Coreliquid S device.\n");
    }

    return cl_handle;
}
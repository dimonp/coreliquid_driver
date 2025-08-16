#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include "coreliquid.h"

#define HID_REPORT_SIZE 64
#define REPORT_ID_LED 0x01
#define REPORT_ID_COMMON 0xD0
#define REPORT_ID_LEDPE0 0xFA
#define CHECK_FILL_VALUE 0xCC

static const uint16_t supported_vids[] = { 0x0db0 };
static const uint16_t supported_pids[] = { 0x6a04, 0x6a05 };

// command codes for reports with id = 0x01
enum command_code_mcu {
    GET_RESPONSE_COMMAND    = 0x5A,

    GET_FW_VERSION_APROM    = 0xB0,
    GET_CURRENT_MODEL_INDEX = 0xB1,
    GET_FWCHECKSUM_APROM    = 0xB4,
    GET_FW_VERSION_LDROM    = 0xB6,
    GET_FW_CHECKSUM_LDROM   = 0xB8,
    GET_LED_GLOBAL          = 0xBA,
    SET_LED_GLOBAL          = 0xBB,
    SET_BUZZER              = 0xC2,
    SET_VOLUME              = 0xC0,
    SET_RESET_MCU           = 0xD0,
};

// command codes for reports with id = 0xFA
enum command_code_ledpe0 {
    GET_LED_PE0 = 0xA0,
};

// command codes for reports with id = 0xD0
enum command_code_ext {
    GET_COOLER_STATUS               = 0x31,

    SET_FAN_DUTY_MODE               = 0x40,
    SET_FAN_TEMPERATURE_MODE        = 0x41,

    SET_OLED_SHOW_PROFILE           = 0x70,
    SET_OLED_SHOW_HWMONITOR         = 0x71,
    SET_OLED_SHOW_SYSTEM_MESSAGE    = 0x72,
    SET_OLED_SHOW_GAMEBOOT          = 0x73,
    SET_OLED_SHOW_USER_MESSAGE      = 0x74,
    SET_OLED_SHOW_DEMOMODE          = 0x77,
    SET_OLED_SHOW_BANNER            = 0x79,
    SET_OLED_SHOW_CLOCK             = 0x7A,
    SET_OLED_BRIGHTNESS_AND_DIRECTION = 0x7E,
    SET_OLED_SHOW_DISABLE           = 0x7F,
    SET_OLED_CLOCK                  = 0x83,
    SET_OLED_CPU_STATUS             = 0x85,
    SET_OLED_GPU_STATUS             = 0x86,
    SET_OLED_CPU_MESSAGE            = 0x90,
    SET_OLED_MEMORY_MESSAGE         = 0x91,
    SET_OLED_VGA_MESSAGE            = 0x92,
    SET_OLED_USER_MESSAGE           = 0x93,
    SET_OLED_UPLOAD_GIF             = 0xC0,
    GET_OLED_GIF_CHECKSUM           = 0xC2,
    SET_OLED_UPLOAD_BANNER          = 0xD0,
    GET_OLED_FW_VERSION             = 0xF0,
    SET_OLED_START_IS_PPROCESS      = 0xFA,
};

#pragma pack(1)

struct message_header {
    uint8_t  report_id;
    uint8_t  command;
};

struct message_request {
    union {
        struct message_header header;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

struct aprom_model_response {
    struct message_header header;
    uint8_t value;
};
struct message_aprom_model_response {
    union {
        struct aprom_model_response report;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

// GET_COOLER_STATUS
struct fan_status_response {
    struct message_header header;
    uint16_t fan_speed_1; // Raditor fan
    uint16_t fan_speed_2;
    uint16_t fan_speed_3;
    uint16_t fan_speed_4; // Water block fan
    uint16_t fan_speed_5; // Pump fan
    uint16_t temperature_in;
    uint16_t temperature_out;  // Liquid temp
    uint16_t temperature_sensor_1;
    uint16_t temperature_sensor_2;
    uint16_t fan_duty_1;
    uint16_t fan_duty_2;
    uint16_t fan_duty_3;
    uint16_t fan_duty_4;
    uint16_t fan_duty_5;
};
struct message_fan_status_response {
    union {
        struct fan_status_response report;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

#define CONFIG_COUNT_FAN 7

// SET_FAN_DUTY_MODE
struct config_fan_duty {
    struct message_header header;
    uint8_t fan_mode_1;
    uint8_t duty_cycle_1[CONFIG_COUNT_FAN];
    uint8_t fan_mode_2;
    uint8_t duty_cycle_2[CONFIG_COUNT_FAN];
    uint8_t fan_mode_3;
    uint8_t duty_cycle_3[CONFIG_COUNT_FAN];
    uint8_t fan_mode_4;
    uint8_t duty_cycle_4[CONFIG_COUNT_FAN];
    uint8_t fan_mode_5;
    uint8_t duty_cycle_5[CONFIG_COUNT_FAN];
};
struct message_fan_duty {
    union {
        struct config_fan_duty config;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

// SET_FAN_TEMPERATURE_MODE
struct config_fan_temperature {
    struct message_header header;
    uint8_t fan_mode_1;
    uint8_t fan_temp_1[CONFIG_COUNT_FAN];
    uint8_t fan_mode_2;
    uint8_t fan_temp_2[CONFIG_COUNT_FAN];
    uint8_t fan_mode_3;
    uint8_t fan_temp_3[CONFIG_COUNT_FAN];
    uint8_t fan_mode_4;
    uint8_t fan_temp_4[CONFIG_COUNT_FAN];
    uint8_t fan_mode_5;
    uint8_t fan_temp_5[CONFIG_COUNT_FAN];
};
struct message_fan_temperature {
    union {
        struct config_fan_temperature config;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

// SET_OLED_CPU_STATUS
struct config_oled_cpu {
    struct message_header header;
    uint16_t cpu_freq;
    uint16_t cpu_temp;
};
struct message_oled_cpu {
    union {
        struct config_oled_cpu config;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

// SET_OLED_SHOW_CLOCK
struct config_show_clock {
    struct message_header header;
    uint8_t style;
};
struct message_oled_show_clock {
    union {
        struct config_show_clock config;
        unsigned char raw_buffer[HID_REPORT_SIZE];
    };
};

#pragma pack()

const uint8_t fan_temp_preset_1[CONFIG_COUNT_FAN] = {35, 40, 70, 81, 81, 81, 81};
const uint8_t fan_temp_preset_4[CONFIG_COUNT_FAN] = {35, 40, 70, 81, 81, 81, 81};
const uint8_t fan_temp_preset_5[CONFIG_COUNT_FAN] = {35, 60, 70, 81, 81, 81, 81};

const uint8_t fan_duty_preset_1[CONFIG_COUNT_FAN] = {28, 40, 70, 100, 100, 100, 100};
const uint8_t fan_duty_preset_4[CONFIG_COUNT_FAN] = {60, 70, 100, 100, 100, 100, 100};
const uint8_t fan_duty_preset_5[CONFIG_COUNT_FAN] = {20, 40, 50, 100, 100, 100, 100};

/**
* Retrieves the status of the Coreliquid cooler device.
*
* @param handle Pointer to the CoreLiquid device handle.
* @param temperature_in Pointer to store the input temperature value (in degrees Celsius).
* @param temperature_out Pointer to store the output temperature value (in degrees Celsius).
* @param fan_speed Pointer to store the fan speed value (in RPM).
* @return 1 if the cooler status was successfully retrieved, 0 otherwise.
*/
int get_cooler_status(coreliquid_device* handle, int* temperature_in, int* temperature_out, int* fan_speed)
{
    struct message_fan_status_response message_input;
    struct message_request message;
    memset(&message.raw_buffer, 0, sizeof(message.raw_buffer));

    message.header = (struct message_header) {
        .report_id = REPORT_ID_COMMON,
        .command = GET_COOLER_STATUS,
    };

    if (!write_output(handle, message.raw_buffer, sizeof(message.raw_buffer))) {
        return 0;
    }

    usleep(10000);

    if (read_input(handle, message_input.raw_buffer, sizeof(message_input.raw_buffer))
            && message_input.report.header.command == GET_COOLER_STATUS) {

        *temperature_in = message_input.report.temperature_in;
        *temperature_out = message_input.report.temperature_out;
        *fan_speed = message_input.report.fan_speed_1;

        return 1;
    }

    return 0;
}

/**
 * Sets the CPU status (temperature and frequency) to be displayed on the OLED screen.
 *
 * @param handle Pointer to the CoreLiquid device handle.
 * @param temperature The CPU temperature to display.
 * @param frequency The CPU frequency to display.
 */
void set_oled_cpu_status(coreliquid_device* handle, int temperature, int frequency)
{
    struct message_oled_cpu message;
    memset(&message.raw_buffer, 0, sizeof(message.raw_buffer));

    message = (struct message_oled_cpu) {
        .config = {
            .header = {
                .report_id = REPORT_ID_COMMON,
                .command = SET_OLED_CPU_STATUS,
            },
            .cpu_freq = frequency,
            .cpu_temp = temperature,
        }
    };

    write_output(handle, message.raw_buffer, sizeof(message.raw_buffer));
}



/**
 * Sets the fan duty mode for a Coreliquid device.
 *
 * @param handle Pointer to the CoreLiquid device handle.
 * @param fan_mode The fan mode to set for all fans. If set to FAN_MODE_CUSTOM,
 *                 predefined duty cycles will be applied to the fans.
 */
void set_fan_duty_mode(coreliquid_device* handle, uint8_t fan_mode)
{
    struct message_fan_duty message;
    memset(&message.raw_buffer, 0, sizeof(message.raw_buffer));

    message = (struct message_fan_duty) {
        .config = {
            .header = {
                .report_id = REPORT_ID_COMMON,
                .command = SET_FAN_DUTY_MODE,
            },
            .fan_mode_1 = fan_mode,
            .fan_mode_2 = fan_mode,
            .fan_mode_3 = fan_mode,
            .fan_mode_4 = fan_mode,
            .fan_mode_5 = fan_mode,
        },
    };

    if (fan_mode == FAN_MODE_CUSTOM) {
        memcpy(message.config.duty_cycle_1, fan_duty_preset_1, sizeof(fan_duty_preset_1));
        memcpy(message.config.duty_cycle_2, fan_duty_preset_1, sizeof(fan_duty_preset_1));
        memcpy(message.config.duty_cycle_3, fan_duty_preset_1, sizeof(fan_duty_preset_1));
        memcpy(message.config.duty_cycle_4, fan_duty_preset_4, sizeof(fan_duty_preset_4));
        memcpy(message.config.duty_cycle_4, fan_duty_preset_5, sizeof(fan_duty_preset_5));
    }

    write_output(handle, message.raw_buffer, sizeof(message.raw_buffer));
}

/**
* Sets the temperature mode for all fans on a CoreLiquid device.
*
* @param handle Pointer to the CoreLiquid device handle.
* @param fan_mode The temperature mode to set for all fans. If set to FAN_MODE_CUSTOM,
*                 custom temperature presets will be applied to each fan.
*/
void set_fan_temperature_mode(coreliquid_device* handle, uint8_t fan_mode)
{
    struct message_fan_temperature message;
    memset(&message.raw_buffer, 0, sizeof(message.raw_buffer));

    message = (struct message_fan_temperature ) {
        .config = {
            .header = {
                .report_id = REPORT_ID_COMMON,
                .command = SET_FAN_TEMPERATURE_MODE,
            },
            .fan_mode_1 = fan_mode,
            .fan_mode_2 = fan_mode,
            .fan_mode_3 = fan_mode,
            .fan_mode_4 = fan_mode,
            .fan_mode_5 = fan_mode
        }
    };

    if (fan_mode == FAN_MODE_CUSTOM) {
        memcpy(message.config.fan_temp_1, fan_temp_preset_1, sizeof(fan_temp_preset_1));
        memcpy(message.config.fan_temp_2, fan_temp_preset_1, sizeof(fan_temp_preset_1));
        memcpy(message.config.fan_temp_3, fan_temp_preset_1, sizeof(fan_temp_preset_1));
        memcpy(message.config.fan_temp_4, fan_temp_preset_4, sizeof(fan_temp_preset_4));
        memcpy(message.config.fan_temp_5, fan_temp_preset_5, sizeof(fan_temp_preset_5));
    }

    write_output(handle, message.raw_buffer, sizeof(message.raw_buffer));
}

/**
 * Sets the fan mode for a CoreLiquid device.
 *
 * @param handle Pointer to the CoreLiquid device handle.
 * @param fan_mode The desired fan mode to set.
 */
void set_fan_mode(coreliquid_device* handle, fan_mode_t fan_mode)
{
    set_fan_duty_mode(handle, fan_mode);
    usleep(10000);
    set_fan_temperature_mode(handle, fan_mode);
    usleep(10000);
}

/**
 * Sets the clock display style on the OLED screen.
 *
 * @param handle Pointer to the coreliquid device handle.
 * @param style The style of the clock to display.
 */
void set_oled_show_clock(coreliquid_device* handle, uint8_t style)
{
    struct message_oled_show_clock message;
    memset(&message.raw_buffer, 0, sizeof(message.raw_buffer));

    message = (struct message_oled_show_clock) {
        .config = {
            .header = {
                .report_id = REPORT_ID_COMMON,
                .command = SET_OLED_SHOW_CLOCK,
            },
            .style = style,
        }
    };

    write_output(handle, message.raw_buffer, sizeof(message.raw_buffer));
}

/**
* Retrieves the model index from a coreliquid device.
*
* @param handle Pointer to the coreliquid device handle.
* @param model_idx Pointer to store the retrieved model index.
* @return 1 if the model index was successfully retrieved, 0 otherwise.
*/
int get_model_index(coreliquid_device* handle, int* model_idx)
{
    struct message_aprom_model_response message_input;
    struct message_request message;
    memset(&message.raw_buffer, CHECK_FILL_VALUE, sizeof(message.raw_buffer));

    message.header = (struct message_header) {
        .report_id = REPORT_ID_LED,
        .command = GET_CURRENT_MODEL_INDEX,
    };

    if (!write_output(handle, message.raw_buffer, sizeof(message.raw_buffer))) {
        return 0;
    }

    usleep(10000);

    if (read_input(handle, message_input.raw_buffer, sizeof(message_input.raw_buffer))
            && message_input.report.header.command == GET_RESPONSE_COMMAND
            && message_input.report.value != CHECK_FILL_VALUE) {

        for (size_t i = 3; i < sizeof(message_input.raw_buffer); ++i) {
            if (message_input.raw_buffer[i] != CHECK_FILL_VALUE) {
                return 0;
            }
        }

        *model_idx = message_input.report.value;

        return 1;
    }

    return 0;
}

/**
* Retrieves the firmware version from of a CoreLiquid LED device.
*
* @param handle Pointer to the CoreLiquid device handle.
* @param version_major Pointer to store the major version number.
* @param version_minor Pointer to store the minor version number.
* @return 1 if the version was successfully retrieved, 0 otherwise.
*/
int get_fw_version_ldprom(coreliquid_device* handle, int* version_major, int* version_minor)
{
    struct message_aprom_model_response message_input;
    struct message_request message;
    memset(&message.raw_buffer, 0xcc, sizeof(message.raw_buffer));

    message.header = (struct message_header) {
        .report_id = REPORT_ID_LED,
        .command = GET_FW_VERSION_APROM,
    };

    if (!write_output(handle, message.raw_buffer, sizeof(message.raw_buffer))) {
        return 0;
    }

    usleep(10000);

    if (read_input(handle, message_input.raw_buffer, sizeof(message_input.raw_buffer))
            && message_input.report.header.command == GET_RESPONSE_COMMAND) {

        *version_major = message_input.report.value >> 4;
        *version_minor = message_input.report.value & 0xf;

        return 1;
    }

    return 0;
}

/**
* Opens a Coreliquid fan device by searching for supported VID/PID combinations.
*
* @return Pointer to the opened coreliquid_device handle if successful, 0 otherwise.
*/
coreliquid_device* open_device_aio(void)
{
    coreliquid_device* handle = NULL;
    handle = search_and_open_device(supported_vids, ARRAY_SIZE(supported_vids), supported_pids, ARRAY_SIZE(supported_pids));
    if (!handle) {
        loginfo("Failed to open Coreliquid AIO device.\n");
    }

    return handle;
}

#include "coreliquid_s.h"
#include "coreliquid.h"
#include "sensors_wrap.h"
#include "sensors_dbus.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


// Flags to stop and suspend the daemon
static volatile sig_atomic_t is_stop = 0;
static volatile sig_atomic_t is_suspend = 0;

/**
 * Monitor the CPU temperature and send it to the AIO.
 *
 * \param handle handle on the AIO device
 */
void monitor_cpu_temperature(
    coreliquid_device* handle_s,
    coreliquid_device* handle_cl,
    dbus_device* handle_dbus)
{
    sensors_values_t data = {0};
    dbus_cooler_stats_t dbus_stats = {0};
    cooler_status_t cooler_status = {0};

    // Listen to temperature in an infinite loop
    while (!is_stop) {
        if (is_suspend) {
            loginfo("Suspended, waiting ...\n");
            pause();
            loginfo("Waked up ...\n");
        }

        fetch_sensor_values(&data);

        if (data.cpu_temp > 0 && data.cpu_freq > 0) {
            set_oled_cpu_status(handle_cl, data.cpu_temp, data.cpu_freq);
            usleep(10000);
            send_cpu_info(handle_s, data.cpu_temp, data.cpu_freq);
            usleep(10000);

            if (get_cooler_status(handle_cl, &cooler_status) > 0) {
                dbus_stats.fan_radiator_speed = cooler_status.fan_radiator_speed;
                dbus_stats.fan_water_block_speed = cooler_status.fan_water_block_speed;
                dbus_stats.pump_speed = cooler_status.pump_speed;
                dbus_stats.liquid_temperature = cooler_status.liquid_temperature;

                update_aio_status(handle_dbus, &dbus_stats);
            }
        }

        // Wait 1s
        usleep(1000*1000);
    }
}

/**
 * Signal handler to stop the daemon.
 * Can take up to 2s to stop (sleeping time between temperature reads).
 */
void stopit(__attribute__((unused)) int sig)
{
    is_stop = 1;
}

/**
 * Signal handler to suspend the daemon.
 * Can take up to 2s to suspend (sleeping time between temperature reads).
 */
void suspendit(__attribute__((unused)) int sig)
{
    is_suspend = 1;
}

/**
 * Signal handler to resume the daemon.
 */
void resumeit(__attribute__((unused)) int sig)
{
    is_suspend = 0;
}

/**
 * Main program
 */
int main(int argc, char *argv[])
{
    int exit_status = EXIT_SUCCESS;
    int fan_mode = FAN_MODE_SMART;
    int start_daemon = 0;
    int opt;

     while ((opt = getopt(argc, argv, "M:")) != -1) {
        switch (opt) {
            case 'M':
                fan_mode = atoi(optarg);
                if ((fan_mode < 0) || (fan_mode > 5) || (fan_mode == 3)) {
                    printf("Allowed modes:\n");
                    printf("0 : silent\n");
                    printf("1 : balance\n"),
                    printf("2 : game\n");
                    printf("4 : default (constant)\n");
                    printf("5 : smart\n");

                    exit(0);
                }
                break;

            case '?': // Unrecognized option
                fprintf(stderr, "Unknown option: %c\n", optopt);
                break;

            case ':': // Missing argument for an option
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                break;
        }
    }

    if (!strcmp(argv[optind], "startd"))
            start_daemon = 1;

    // Initialize the subsystems
    open_log(start_daemon, "MSI_Coreliquid_S360");
    init_coreliquid();
    init_sensors();

    coreliquid_device* handle_cl = open_device_aio();
    if (!handle_cl) {
        logerror("Failed to open Coreliquid AIO device.\n");
        exit_status = EXIT_FAILURE;
        goto exit_shutdown;
    }

    coreliquid_device* handle_s = open_s_device();
    if (!handle_s) {
        logerror("Failed to open Coreliquid S device.\n");
        exit_status = EXIT_FAILURE;
        goto exit_free_cl;
    }

    dbus_device* handle_dbus = open_dbus();
    if (!handle_dbus) {
        logerror("Failed to open system bus.\n");
        exit_status = EXIT_FAILURE;
        goto exit_free;
    }

    detect_lm_sensors();

    int model_idx;
    if (!get_model_index(handle_cl, &model_idx)) {
        exit_status = EXIT_FAILURE;
        goto exit_free;
    }

    loginfo("LED device model index: %d\n", model_idx);

    int version_high, version_low;
    if (!get_fw_version_ldprom(handle_cl, &version_high, &version_low)) {
        exit_status = EXIT_FAILURE;
        goto exit_free;
    }

    loginfo("LED device firmware version: %d.%d\n", version_high, version_low);

    set_fan_mode(handle_cl, fan_mode);

    int fw_version;
    if (!get_device_info(handle_s, &fw_version)) {
        exit_status = EXIT_FAILURE;
        goto exit_free;
    }

    loginfo("Found S device. FW version: %d\n", fw_version);

    set_lcm_back_light(handle_s, LCM_DEFAULT_BRIGHTNESS);
    set_lcm_direction(handle_s, LCM_DIR_DEFAULT);
    set_temperature_unit(handle_s, 0);
    set_display_mode(handle_s, SHOW_CPU_FREQ | SHOW_CPU_TEMP, STYLE_3);

    // Start daemon if requested
    if (start_daemon) {
        signal(SIGTERM, stopit);
        signal(SIGINT, stopit);
        signal(SIGTSTP, suspendit);
        signal(SIGCONT, resumeit);

        monitor_cpu_temperature(handle_s, handle_cl, handle_dbus);
    }

    close_dbus(handle_dbus);
exit_free:
    close_coreliquid_device(handle_s);
exit_free_cl:
    close_coreliquid_device(handle_cl);

exit_shutdown:
    shutdown_sensors();
    shutdown_coreliquid();
    close_log();

    exit(exit_status);
}

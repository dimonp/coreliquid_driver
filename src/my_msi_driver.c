// Compilation:
// gcc my_msi_driver.c -lhidapi-hidraw -lsensors -o /where/you/want/my_msi_driver

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "coreliquid_s.h"
#include "coreliquid.h"
#include "sensors_wrap.h"

// Flag to stop the daemon
int stop = 0;

/**
 * Monitor the CPU temperature and send it to the AIO.
 *
 * \param handle handle on the AIO device
 */
void monitor_cpu_temperature(coreliquid_device* handle_s, coreliquid_device* handle_cl)
{
    sensors_data_t data;

    // Listen to temperature in an infinite loop
    while (!stop) {
        fetch_sensors_data(&data);

        if (data.cpu_temp > 0 && data.cpu_freq > 0) {
            set_oled_cpu_status(handle_cl, data.cpu_temp, data.cpu_freq);
            usleep(10000);
            send_cpu_info(handle_s, data.cpu_temp, data.cpu_freq);
        }

        // Wait 2s
        usleep(2000*1000);
    }

}

/**
 * Signal handler to stop the daemon.
 * Can take up to 2s to stop (sleeping time between temperature reads).
 */
void stopit(__attribute__((unused)) int sig)
{
    stop = 1;
}

/**
 * Main program
 */
int main(int argc, char *argv[])
{
    int fan_mode = FAN_MODE_SMART;
    int start_daemon = 0;

    // Check options
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-M")) {
            fan_mode = atoi(argv[++i]);
            if ((fan_mode < 0) || (fan_mode > 5) || (fan_mode == 3)) {
                printf("Allowed modes:\n");
                printf("0 : silent\n");
                printf("1 : balance\n"),
                printf("2 : game\n");
                printf("4 : default (constant)\n");
                printf("5 : smart\n");
                exit(0);
            }
        }
        else if (!strcmp(argv[i], "startd"))
            start_daemon = 1;
    }

    // Initialize the subsystems
    init_coreliquid(start_daemon);
    init_sensors();

    coreliquid_device* handle_cl = open_fan_device();
    if (!handle_cl) {
        shutdown_coreliquid();
        exit(1);
    }

    coreliquid_device* handle_s = open_s_device();
    if (!handle_s) {
        close_coreliquid_device(handle_cl);
        shutdown_coreliquid();
        exit(1);
    }

    detect_sensors();

    set_fan_mode(handle_cl, fan_mode);
    set_lcm_back_light(handle_s, LCM_DEFAULT_BRIGHTNESS);
    set_lcm_direction(handle_s, LCM_DIR_DEFAULT);
    set_temperature_unit(handle_s, 1);
    set_display_mode(handle_s, SHOW_CPU_FREQ | SHOW_CPU_TEMP, STYLE_3);

    // Start daemon if requested
    if (start_daemon) {
        signal(SIGTERM, stopit);
        signal(SIGINT, stopit);

        monitor_cpu_temperature(handle_s, handle_cl);
    }

    close_coreliquid_device(handle_s);
    close_coreliquid_device(handle_cl);

    shutdown_sensors();
    shutdown_coreliquid();

    exit(0);
}

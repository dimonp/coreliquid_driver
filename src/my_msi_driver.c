// Compilation: 
// gcc my_msi_driver.c -lhidapi-hidraw -lsensors -o /where/you/want/my_msi_driver

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <hidapi/hidapi.h>
#include <sensors/sensors.h>

// Fan modes, unused
typedef enum FanMode {
    SILENT = 0,
    BALANCE = 1,
    GAME = 2,
    CUSTOMIZE = 3,
    DEFAULT = 4,
    SMART = 5
} FanMode;

// Flag to stop the daemon
int stop = 0;

/**
 * Monitor the CPU temperature and send it to the AIO.
 * 
 * \param handle handle on the AIO device
 */
void monitor_cpu_temperature(hid_device *handle) 
{
    int nr, ret, res;
    unsigned char buf[65];
    const sensors_chip_name *chip;
    const sensors_feature *feature;
    const sensors_subfeature *subfeature;
    int ifreq = 3000, itemp;
    double temp;
    
    // Initialize the libsensor library
    ret = sensors_init(NULL);
    if (ret != 0) {
        fprintf(stderr, "Error while initializing libsensor: %d\n", ret);
        return;
    }
    
    memset(buf,0,sizeof(buf));
    buf[0] = 0xD0;
    buf[1] = 0x85;
    // The AIO doesn't care about CPU frequency to adapt fan speed. Set it to a dummy value.
    buf[2] = ifreq & 0xFF;
    buf[3] = (ifreq >> 8) & 0xFF;
    // Loop on chips
    nr = 0;
    while (!stop && ((chip = sensors_get_detected_chips(NULL, &nr)) != NULL)) {
        if (!strcmp(chip->prefix, "coretemp") || !strcmp(chip->prefix, "k10temp")) { // This chip gives CPU temperatures
            // Loop on features for this chip
            int nf = 0;
            while (!stop && ((feature = sensors_get_features(chip, &nf)) != NULL)) {
                if (feature->type == SENSORS_FEATURE_TEMP) {
                    if (!strcmp(feature->name, "temp1") || !strcmp(feature->name, "Tctl")) { // This feature is the global core CPU temperature
                        // Loop on subfeatures for this chip feature
                        int ns = 0;
                        while (!stop && ((subfeature = sensors_get_all_subfeatures(chip, feature, &ns)) != NULL)) {
                            if (subfeature->type == SENSORS_SUBFEATURE_TEMP_INPUT) {
                                // Temperature subfeature found, initialize the hidapi library
                                res = hid_init();
                                // Listen to temperature in an infinite loop
                                while (!stop) {
                                    ret = sensors_get_value(chip, subfeature->number, &temp);
                                    if (ret == 0) {
                                        itemp = (int)temp;
                                        printf("CPU Temperature: %d°C\n", itemp);
                                        // Set CPU status (cmd 0x85)
                                        buf[4] = itemp & 0xFF;
                                        buf[5] = (itemp >> 8) & 0xFF;
                                        res = hid_write(handle, buf, 65);
                                    }
                                    // Wait 2s
                                    usleep(2000*1000);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Free resources
    sensors_cleanup();
}


/**
 * Set the fan mode.
 * 
 * \param handle handle on the AIO device
 * \param fan_mode the choosen fan mode
 */
void set_fan_mode(hid_device *handle, int fan_mode) 
{
    unsigned char buf[65];    

    memset(buf,0,sizeof(buf));
    buf[0] = 0xD0;
    buf[1] = 0x40;
    buf[2] = fan_mode;
    buf[10] = fan_mode;
    buf[18] = fan_mode;
    buf[26] = fan_mode;
    buf[34] = fan_mode;
    hid_write(handle, buf, 65);
    buf[1] = 0x41;
    hid_write(handle, buf, 65);  
}

/**
 * Signal handler to stop the daemon.
 * Can take up to 2s to stop (sleeping time between temperature reads).
 */
void stopit(int sig)
{
    stop = 1;
}

/**
 * Main program
 */
int main(int argc, char *argv[]) 
{
    int i, fan_mode = SMART;
    int start_daemon = 0;
    hid_device *handle = NULL;
    
    // Check options
    for (i = 1; i < argc; i++) {
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
    
    // Initialize the hidapi library
    hid_init();
    // Open the device using the VID, PID
    handle = hid_open(0x0db0, 0xb130, NULL);
    if (!handle) {
        fprintf(stderr, "Unable to open device\n");
        hid_exit();
        exit(1);
    }
    set_fan_mode(handle, fan_mode);
    // Start daemon if requested
    if (start_daemon) {
        signal(SIGTERM, stopit);
        monitor_cpu_temperature(handle);
    }
    // Close the device
    hid_close(handle);
    // Finalize the hidapi library
    hid_exit();
    
    exit(0);
}

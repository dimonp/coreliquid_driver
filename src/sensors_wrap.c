#include <stdlib.h>
#include "string.h"
#include <sensors/sensors.h>

#include "sensors_wrap.h"
#include "coreliquid_hid.h"

static struct {
    const sensors_chip_name *name_cpu_temp;
    int idx_cpu_temp;

    const sensors_chip_name *name_gpu_temp;
    int idx_gpu_temp;

    const sensors_chip_name *name_gpu_freq;
    int idx_gpu_freq;

} sensors_bank;


int detect_cpu_temp_sensor(
    const sensors_chip_name *chip,
    const sensors_feature *feature,
    const sensors_subfeature *subfeature)
{
    if (strcmp(chip->prefix, "coretemp") && strcmp(chip->prefix, "k10temp"))
        return 0;

    if (feature->type != SENSORS_FEATURE_TEMP)
        return 0;

    if (strcmp(feature->name, "temp1") && strcmp(feature->name, "Tctl"))
        return 0;

    if (subfeature->type != SENSORS_SUBFEATURE_TEMP_INPUT)
        return 0;

    sensors_bank.name_cpu_temp = chip;
    sensors_bank.idx_cpu_temp = subfeature->number;

    return 1;
}

int detect_gpu_temp_sensor(
    const sensors_chip_name *chip,
    const sensors_feature *feature,
    const sensors_subfeature *subfeature)
{
    if (strcmp(chip->prefix, "amdgpu"))
        return 0;

    if (feature->type != SENSORS_FEATURE_TEMP)
        return 0;

    if (strcmp(feature->name, "temp2"))
        return 0;

    if (subfeature->type != SENSORS_SUBFEATURE_TEMP_INPUT)
        return 0;

    sensors_bank.name_gpu_temp = chip;
    sensors_bank.idx_gpu_temp = subfeature->number;

    return 1;
}

int detect_gpu_freq_sensor(
    const sensors_chip_name *chip,
    const sensors_feature *feature,
    const sensors_subfeature *subfeature)
{
    if (strcmp(chip->prefix, "amdgpu"))
        return 0;

    if (feature->type != SENSORS_FEATURE_FREQ)
        return 0;

    if (strcmp(feature->name, "freq1"))
        return 0;

    if (subfeature->type != SENSORS_SUBFEATURE_FREQ_INPUT)
        return 0;

    sensors_bank.name_gpu_freq = chip;
    sensors_bank.idx_gpu_freq = subfeature->number;

    return 1;
}

void init_sensors(void)
{
    memset(&sensors_bank, 0, sizeof(sensors_bank));

    int ret = sensors_init(NULL);
    if (ret != 0) {
        loginfo("Error while initializing libsensor: %d\n", ret);
        return;
    }

}

void shutdown_sensors(void)
{
    sensors_cleanup();
    memset(&sensors_bank, 0, sizeof(sensors_bank));
}

void detect_sensors(void)
{
    const sensors_chip_name *chip;
    int chip_nr = 0;

    // Iterate through detected sensor chips
    while ((chip = sensors_get_detected_chips(NULL, &chip_nr)) != NULL) {
        const sensors_feature *feature;
        int feature_nr = 0;

#ifdef _DEBUG
        		const char *adap = sensors_get_adapter_name(&chip->bus);
                loginfo("Chip: %s: %s\n", chip->prefix, adap);
#endif

        // Iterate through features of the current chip
        while ((feature = sensors_get_features(chip, &feature_nr)) != NULL) {
            const sensors_subfeature *subfeature;
            int subfeature_nr = 0;

#ifdef _DEBUG
                char *label = sensors_get_label(chip, feature);
                loginfo("\tFeature: %s (%d)\n", label, feature->type);
                free(label);
#endif

            // Iterate through subfeatures of the current feature
            while ((subfeature = sensors_get_all_subfeatures(chip, feature, &subfeature_nr)) != NULL) {
#ifdef _DEBUG
                loginfo("\t\tSubfeature: %s (%d)\n", subfeature->name, subfeature->type);
#endif

                detect_cpu_temp_sensor(chip, feature, subfeature);
                detect_gpu_temp_sensor(chip, feature, subfeature);
                detect_gpu_freq_sensor(chip, feature, subfeature);
            }
        }
    }
}

int get_cpu_frequency(void)
{
    long long current_freq;
    int result = 0;
    FILE *fp;

    fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
    if (fp == NULL) {
        return 0;
    }

    // Read the frequency value
    if (fscanf(fp, "%lld", &current_freq) == 1) {
        result = (int)(current_freq / 1000);
    }

    fclose(fp);

    return result;
}

int get_cpu_usage(void)
{
    static long long last_total_idle, last_total;
    long long total_user, total_nice, total_system, total_idle, total;
    int result = 0;

    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        return 0;
    }

    if (fscanf(fp, "cpu %lld %lld %lld %lld", &total_user, &total_nice, &total_system, &total_idle) == 4) {

        total = total_user + total_nice + total_system + total_idle;

        if (last_total != 0) {
            result = 100 - ((total_idle - last_total_idle) * 100 / (total - last_total));
        }

        last_total = total;
        last_total_idle = total_idle;
    }

    fclose(fp);

    return result;
}


/**
 * Fetches the current sensor values and stores them in the provided data structure.
 *
 * @param data Pointer to a sensors_values_t structure where the sensor data will be stored.
 *
 * Each sensor value is only retrieved and stored if the corresponding sensor name in sensors_bank is not NULL.
 */
void fetch_sensor_values(sensors_values_t *data)
{
    int ret;
    double value;

    if (data == NULL) {
        return;
    }

    data->cpu_freq = get_cpu_frequency();

    if (sensors_bank.name_cpu_temp != NULL) {
        ret = sensors_get_value(sensors_bank.name_cpu_temp, sensors_bank.idx_cpu_temp, &value);
        if (ret == 0) {
            data->cpu_temp = (int)value;
        }
    }

    if (sensors_bank.name_gpu_temp != NULL) {
        ret = sensors_get_value(sensors_bank.name_gpu_temp, sensors_bank.idx_gpu_temp, &value);
        if (ret == 0) {
            data->gpu_temp = (int)value;
        }
    }

    if (sensors_bank.name_gpu_freq != NULL) {
        ret = sensors_get_value(sensors_bank.name_gpu_freq, sensors_bank.idx_gpu_freq, &value);
        if (ret == 0) {
            data->gpu_freq = (int)(value / 1000000); // to MHz
        }
    }

#ifdef _DEBUG
    loginfo("Sensors data: cpu_freq=%d, cpu_temp=%d\n", data->cpu_freq, data->cpu_temp);
    loginfo("Sensors data: gpu_freq=%d, gpu_temp=%d\n", data->gpu_freq, data->gpu_temp);
#endif
}

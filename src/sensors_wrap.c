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

typedef struct {
    const char *chip_prefix;
    sensors_feature_type feature_type;
    const char *feature_name;
    sensors_subfeature_type subfeature_type;
    const sensors_chip_name **chip_name;
    int *subfeature_index;
} sensor_config_t;

static const sensor_config_t sensor_configs[] = {
    {
        .chip_prefix = "coretemp|k10temp",
        .feature_type = SENSORS_FEATURE_TEMP,
        .feature_name = "temp1|Tctl",
        .subfeature_type = SENSORS_SUBFEATURE_TEMP_INPUT,
        .chip_name = &sensors_bank.name_cpu_temp,
        .subfeature_index = &sensors_bank.idx_cpu_temp
    },
    {
        .chip_prefix = "amdgpu",
        .feature_type = SENSORS_FEATURE_TEMP,
        .feature_name = "temp2",
        .subfeature_type = SENSORS_SUBFEATURE_TEMP_INPUT,
        .chip_name = &sensors_bank.name_gpu_temp,
        .subfeature_index = &sensors_bank.idx_gpu_temp
    },
    {
        .chip_prefix = "amdgpu",
        .feature_type = SENSORS_FEATURE_FREQ,
        .feature_name = "freq1",
        .subfeature_type = SENSORS_SUBFEATURE_FREQ_INPUT,
        .chip_name = &sensors_bank.name_gpu_freq,
        .subfeature_index = &sensors_bank.idx_gpu_freq
    }
};

/**
* Checks if a string matches any of the patterns separated by '|'.
*
* @param str The string to be matched against the patterns.
* @param pattern A string containing patterns separated by '|'.
* @return 1 if the string matches any of the patterns, 0 otherwise.
*/
int pattern_match(const char *str, const char *pattern) {
    char *pattern_copy = strdup(pattern);
    char *token = strtok(pattern_copy, "|");

    while (token != NULL) {
        if (strcmp(str, token) == 0) {
            free(pattern_copy);
            return 1;
        }
        token = strtok(NULL, "|");
    }

    free(pattern_copy);
    return 0;
}

/**
 * Detects if a sensor matches the specified configuration.
 *
 * @param chip The sensor chip to check.
 * @param feature The sensor feature to check.
 * @param subfeature The sensor subfeature to check.
 * @param config The sensor configuration to match against.
 * @return 1 if the sensor matches the configuration, 0 otherwise.
 */
int detect_sensor(
    const sensors_chip_name *chip,
    const sensors_feature *feature,
    const sensors_subfeature *subfeature,
    const sensor_config_t *config)
{
    if (!pattern_match(chip->prefix, config->chip_prefix))
        return 0;

    if (feature->type != config->feature_type)
        return 0;

    if (!pattern_match(feature->name, config->feature_name))
        return 0;

    if (subfeature->type != config->subfeature_type)
        return 0;

    *(config->chip_name) = chip;
    *(config->subfeature_index) = subfeature->number;

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

/**
 * Detects and initializes LM sensors by iterating through all detected sensor chips,
 * their features, and subfeatures. For each subfeature, it checks against all configured
 * sensor configurations to identify and initialize matching sensors.
 *
 * This function uses the libsensors library to enumerate hardware monitoring chips
 * and their associated features (e.g., temperature, voltage, fan sensors).
 */
void detect_lm_sensors(void)
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

                // Check against all configured sensors
                for (size_t i = 0; i < ARRAY_SIZE(sensor_configs); i++) {
                    detect_sensor(chip, feature, subfeature, &sensor_configs[i]);
                }
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
    data->cpu_usage = get_cpu_usage();

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

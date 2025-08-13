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
    sensors_bank.idx_cpu_temp = feature->number;

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
        
    if (strcmp(feature->name, "temp3"))
        return 0;

    if (subfeature->type != SENSORS_SUBFEATURE_TEMP_INPUT)
        return 0;

    sensors_bank.name_gpu_temp = chip;
    sensors_bank.idx_cpu_temp = feature->number;

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
    sensors_bank.idx_gpu_freq = feature->number;

    return 1;
}

void init_sensors()
{
    memset(&sensors_bank, 0, sizeof(sensors_bank));

    int ret = sensors_init(NULL);
    if (ret != 0) {
        loginfo("Error while initializing libsensor: %d\n", ret);
        return;
    }

};

void shutdown_sensors()
{
    sensors_cleanup();
    memset(&sensors_bank, 0, sizeof(sensors_bank));
}

void detect_sensors() 
{
    const sensors_chip_name *chip;
    int chip_nr = 0;

    // Iterate through detected sensor chips
    while ((chip = sensors_get_detected_chips(NULL, &chip_nr)) != NULL) {
        const sensors_feature *feature;
        int feature_nr = 0;

#ifdef _DEBUG
        		const char *adap = sensors_get_adapter_name(&chip->bus);
                printf("Chip: %s: %s\n", chip->prefix, adap);
#endif                

        // Iterate through features of the current chip
        while ((feature = sensors_get_features(chip, &feature_nr)) != NULL) {
            const sensors_subfeature *subfeature;
            int subfeature_nr = 0;

#ifdef _DEBUG
                const char *label = sensors_get_label(chip, feature);
                printf("\tFeature: %s\n", label);
#endif                

            // Iterate through subfeatures of the current feature
            while ((subfeature = sensors_get_all_subfeatures(chip, feature, &subfeature_nr)) != NULL) {
#ifdef _DEBUG
                printf("\t\tSubfeature: %s\n", subfeature->name);
#endif                

                detect_cpu_temp_sensor(chip, feature, subfeature);
            }
        }
    }    
}

int get_cpu_frequency() 
{
    static char *path = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq";
    long long current_freq;
    int result = 0;
    FILE *fp;

    fp = fopen(path, "r");
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

/**
 * Fetches various sensor data and populates the provided data structure.
 * 
 * @param data Pointer to the sensors_data_t structure.
 */
void fetch_sensors_data(sensors_data_t *data) 
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
            data->gpu_freq = (int)value;
        }
    }

#ifdef _DEBUG
    loginfo("Sensors data: cpu_freq=%d, cpu_temp=%d \n", data->cpu_freq, data->cpu_temp);
    loginfo("Sensors data: gpu_freq=%d, gpu_temp=%d\n", data->gpu_freq, data->gpu_temp);
#endif                
}

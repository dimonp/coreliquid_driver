#ifndef _SENSORS_WRAP__H
#define _SENSORS_WRAP__H

struct sensors_data {
    int cpu_temp;
    int cpu_freq;
    int cpu_usage;
    int gpu_temp;
    int gpu_freq;
};
typedef struct sensors_data sensors_data_t;

void init_sensors();
void shutdown_sensors();
void detect_sensors();
void fetch_sensors_data(sensors_data_t *data);



#endif // _SENSORS_WRAP__H

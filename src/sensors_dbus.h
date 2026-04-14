#ifndef _SENSORS_DBUS__H
#define _SENSORS_DBUS__H

#include <stdint.h>

struct dbus_cooler_stats {
    uint16_t fan_radiator_speed;
    uint16_t fan_water_block_speed;
    uint16_t pump_speed;
    uint8_t liquid_temperature;
};
typedef struct dbus_cooler_stats dbus_cooler_stats_t;

struct dbus_device_;
typedef struct dbus_device_ dbus_device;

dbus_device* open_dbus(void);
void close_dbus(dbus_device* dbus_handle);
int update_aio_status(dbus_device* dbus_handle, dbus_cooler_stats_t* aio_stats);

#endif

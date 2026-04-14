#include "sensors_dbus.h"
#include "logger.h"

#include <stdlib.h>
#include <systemd/sd-bus.h>

static struct dbus_cooler_stats g_aio_stats;

static const sd_bus_vtable cooler_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("FanRadiatorSpeed", "q", NULL, offsetof(dbus_cooler_stats_t, fan_radiator_speed), SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("FanWaterBlockSpeed", "q", NULL, offsetof(dbus_cooler_stats_t, fan_water_block_speed), SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("PumpSpeed", "q", NULL, offsetof(dbus_cooler_stats_t, pump_speed), SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("LiquidTemp", "q", NULL, offsetof(dbus_cooler_stats_t, liquid_temperature), SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_VTABLE_END
};

struct dbus_device_ {
    sd_bus *bus;
};
typedef struct dbus_device_ dbus_device;

static const char* DBUS_PATH = "/io/github/MSICoreliquid";
static const char* DBUS_INTERFACE = "io.github.MSICoreliquid";

dbus_device* open_dbus(void)
{
    dbus_device *dbus_handle = NULL;
    int result;

    dbus_handle = (dbus_device*) calloc(1, sizeof(dbus_device));
    if (!dbus_handle)
        return NULL;

    result = sd_bus_open_system(&dbus_handle->bus);
    if (result < 0) {
        logerror("Failed to connect to system bus: %s\n", strerror(-result));
        return NULL;
    }

    result = sd_bus_add_object_vtable(dbus_handle->bus, NULL, DBUS_PATH, DBUS_INTERFACE, cooler_vtable, &g_aio_stats);
    if (result < 0) {
        logerror("Failed to add object to system bus: %s\n", strerror(-result));
        return NULL;
    }

    result = sd_bus_request_name(dbus_handle->bus, DBUS_INTERFACE, SD_BUS_NAME_REPLACE_EXISTING);
    if (result < 0) {
        logerror("Failed to request system bus name: %s\n", strerror(-result));
        return NULL;
    }
    return dbus_handle;
}

void close_dbus(dbus_device* dbus_handle)
{
    if (!dbus_handle)
        return;

    sd_bus_unref(dbus_handle->bus);
    free(dbus_handle);
}

int update_aio_status(dbus_device* dbus_handle, dbus_cooler_stats_t* aio_stats)
{
    int result;

    if (!dbus_handle)
        return -1;

    while ((result = sd_bus_process(dbus_handle->bus, NULL)) > 0) {}
    if (result < 0) {
        logerror("DBus processing error: %s\n", strerror(-result));
        return -1;
    }

    if (!aio_stats)
        return -1;

    memcpy(&g_aio_stats, aio_stats, sizeof(dbus_cooler_stats_t));

    result = sd_bus_emit_properties_changed(
        dbus_handle->bus,
        DBUS_PATH,
        DBUS_INTERFACE,
        "FanRadiatorSpeed", "FanWaterBlockSpeed", "PumpSpeed", "LiquidTemp", NULL
    );
    if (result < 0) {
        logerror("Failed to emit notification: %s\n", strerror(-result));
    }
    return result;
}
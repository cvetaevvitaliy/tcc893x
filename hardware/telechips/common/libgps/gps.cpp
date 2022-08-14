#include <hardware/gps.h>
//#include <hardware_legacy/gps.h>
#include <cutils/properties.h>

#define LOG_TAG "libhardware_legacy"
#include <utils/Log.h>

static const GpsInterface*  sGpsInterface = NULL;

static void
gps_find_hardware( void )
{
//    LOGD("gps_find_hardware\n");
    sGpsInterface = gps_get_hardware_interface();

    if (!sGpsInterface)
        LOGD("no GPS hardware on this device\n");
}

const GpsInterface*
gps_get_interface()
{
//    LOGD("gps_get_interface\n");
    if (sGpsInterface == NULL)
         gps_find_hardware();

    return sGpsInterface;
}

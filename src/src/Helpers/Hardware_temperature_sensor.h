#ifndef HELPERS_HARDWARE_TEMPERATURE_SENSOR_H
#define HELPERS_HARDWARE_TEMPERATURE_SENSOR_H

#include "../../ESPEasy_common.h"

#if FEATURE_INTERNAL_TEMPERATURE

// Get internal chip temperature
// @retval Whether the temperature could be read
bool getInternalTemperature(float& temperatureCelsius);

// Get internal chip temperature
// When temperature could not be read, last successful value will be returned.
float getInternalTemperature();

#endif // if FEATURE_INTERNAL_TEMPERATURE


#endif // ifndef HELPERS_HARDWARE_TEMPERATURE_SENSOR_H

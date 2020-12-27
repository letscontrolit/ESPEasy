#ifndef PLUGINSTRUCTS_P106_DATA_STRUCT_H
#define PLUGINSTRUCTS_P106_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P106


# include <Adafruit_Sensor.h>
# include <Adafruit_BME680.h>

struct P106_data_struct : public PluginTaskData_base {
  bool begin(uint8_t addr,
             bool    initSettings = true);

  Adafruit_BME680 bme; // I2C
  // Adafruit_BME680 bme(BME_CS); // hardware SPI
  // Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);
  bool initialized = false;
};

#endif // ifdef USES_P106
#endif // ifndef PLUGINSTRUCTS_P106_DATA_STRUCT_H

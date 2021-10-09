#ifndef PLUGINSTRUCTS_P053_DATA_STRUCT_H
#define PLUGINSTRUCTS_P053_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P053


# include <ESPeasySerial.h>

// Can be unset for memory-tight  builds to remove support for the PMSx003ST and PMS2003/PMS3003 sensor models
# define PLUGIN_053_ENABLE_EXTRA_SENSORS 
// #define PLUGIN_053_ENABLE_S_AND_T // Enable setting to support S and T types, in addition to bas PMSx003 and PMSx003ST

# if defined(SIZE_1M) && defined(PLUGIN_BUILD_MINIMAL_OTA) && defined(PLUGIN_053_ENABLE_EXTRA_SENSORS) // Turn off for 1M OTA builds
#  undef PLUGIN_053_ENABLE_EXTRA_SENSORS
# endif // if defined(SIZE_1M) && defined(PLUGIN_BUILD_MINIMAL_OTA) && defined(PLUGIN_053_ENABLE_EXTRA_SENSORS)


# define PMSx003_SIG1 0x42
# define PMSx003_SIG2 0x4d
# define PMSx003_SIZE   32
# define PMSx003S_SIZE  32
# define PMSx003T_SIZE  32
# define PMSx003ST_SIZE 40
# define PMS3003_SIZE   24

# define PMSx003_TYPE    0            // PMSx003 = PMS1003 / PMS5003 / PMS7003
# define PMS3003_TYPE    1            // PMS2003/PMS3003
# define PMSx003_TYPE_S  2            // PMS5003S // Not supported yet
# define PMSx003_TYPE_T  3            // PMS5003T // Not supported yet
# define PMSx003_TYPE_ST 4            // PMS5003ST

# define PLUGIN_053_OUTPUT_PART 0     // Particles pm1.0/pm2.5/pm10
# define PLUGIN_053_OUTPUT_THC  1     // pm2.5/Temp/Hum/HCHO
# define PLUGIN_053_OUTPUT_CNT  2     // cnt1.0/cnt2.5/cnt10

# define PLUGIN_053_EVENT_NONE      0 // Events: None
# define PLUGIN_053_EVENT_PARTICLES 1 // Particles/temp/humi/hcho
# define PLUGIN_053_EVENT_PARTCOUNT 2 // also Particle count

# define PLUGIN_053_SENSOR_MODEL_SELECTOR PCONFIG(0)
# define PLUGIN_053_OUTPUT_SELECTOR       PCONFIG(1)
# define PLUGIN_053_EVENT_OUT_SELECTOR    PCONFIG(2)


struct P053_data_struct : public PluginTaskData_base {
public:

  P053_data_struct(int8_t                  rxPin,
                   int8_t                  txPin,
                   const ESPEasySerialPort port,
                   int8_t                  resetPin,
                   uint8_t                 sensortype);

  P053_data_struct() = delete;

  ~P053_data_struct();

  bool initialized() const;

private:

  void    SerialRead16(uint16_t *value,
                       uint16_t *checksum);

  void    SerialFlush();

  uint8_t packetSize(uint8_t sensorType);

public:

  bool packetAvailable();

private:

  void sendEvent(const String             & baseEvent,
                 const __FlashStringHelper *name,
                 float                      value);

public:

  bool processData(struct EventStruct *event);

  bool checkAndClearValuesReceived();

private:

  ESPeasySerial *easySerial            = nullptr;
  bool           values_received       = false;
  uint8_t        Plugin_053_sensortype = PMSx003_TYPE;
};


#endif // ifdef USES_P053
#endif // ifndef PLUGINSTRUCTS_P053_DATA_STRUCT_H

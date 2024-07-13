#ifndef PLUGINSTRUCTS_P154_DATA_STRUCT_H
#define PLUGINSTRUCTS_P154_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#if defined(USES_P154) || defined(USES_P172)

# include <Adafruit_Sensor.h>
# include <Adafruit_BMP3XX.h>


# define P154_I2C_ADDR        PCONFIG(0)
# define P154_ALTITUDE        PCONFIG(1)


struct P154_data_struct : public PluginTaskData_base {
public:

  P154_data_struct(struct EventStruct *event);
  P154_data_struct()          = delete;
  virtual ~P154_data_struct() = default;

  bool        begin(bool _i2cMode = true);

  bool        read(float& temp,
                   float& pressure);
  uint32_t    chipID();

  static bool webformLoad(struct EventStruct *event,
                          bool                _i2cMode = true);
  static bool webformSave(struct EventStruct *event);

private:

  Adafruit_BMP3XX bmp;

  uint8_t i2cAddress;
  int16_t elevation{};
  int16_t csPin{};

  bool initialized = false;
  bool i2cMode     = true;
};
#endif // if defined(USES_P154) || defined(USES_P172)
#endif // ifndef PLUGINSTRUCTS_P154_DATA_STRUCT_H

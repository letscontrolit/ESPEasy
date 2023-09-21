#ifndef PLUGINSTRUCTS_P154_DATA_STRUCT_H
#define PLUGINSTRUCTS_P154_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P154

# include <Adafruit_Sensor.h>
# include <Adafruit_BMP3XX.h>


# define P154_I2C_ADDR        PCONFIG(0)
# define P154_ALTITUDE        PCONFIG(1)


struct P154_data_struct : public PluginTaskData_base {
public:

  P154_data_struct(struct EventStruct *event);
  P154_data_struct()          = delete;
  virtual ~P154_data_struct() = default;

  bool        begin();

  bool        read(float& temp,
                   float& pressure);

  static bool webformLoad(struct EventStruct *event);
  static bool webformSave(struct EventStruct *event);

private:

  Adafruit_BMP3XX bmp;

  uint8_t i2cAddress;
  int16_t elevation{};

  bool initialized = false;
};
#endif // ifdef USES_P154
#endif // ifndef PLUGINSTRUCTS_P154_DATA_STRUCT_H

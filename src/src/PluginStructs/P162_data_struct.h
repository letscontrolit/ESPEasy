#ifndef PLUGINSTRUCTS_P162_DATA_STRUCT_H
#define PLUGINSTRUCTS_P162_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P162

# define P162_CS_PIN            PIN(0)
# define P162_RST_PIN           PIN(1)
# define P162_SHD_PIN           PIN(2)

# define P162_INIT_W0           PCONFIG(0)
# define P162_INIT_W1           PCONFIG(1)
# define P162_SHUTDOWN_W0       PCONFIG(2)
# define P162_SHUTDOWN_W1       PCONFIG(3)
# define P162_SHUTDOWN_VALUE    PCONFIG(4)
# define P162_CHANGED_EVENTS    PCONFIG(5)

// # define P162_REMOVALVALUE      PCONFIG_LONG(0)
// # define P162_REMOVALTIMEOUT    PCONFIG_LONG(1)

// potentiometer select byte
const uint8_t P162_POT0_SEL     = 0x11;
const uint8_t P162_POT1_SEL     = 0x12;
const uint8_t P162_BOTH_POT_SEL = 0x13;

// shutdown the device to put it into power-saving mode.
// In this mode, terminal A is open-circuited and the B and W terminals are shorted together.
// send new command and value to exit shutdowm mode.
const uint8_t P162_POT0_SHUTDOWN     = 0x21;
const uint8_t P162_POT1_SHUTDOWN     = 0x22;
const uint8_t P162_BOTH_POT_SHUTDOWN = 0x23;

const uint8_t P162_RESET_VALUE = 0x80; // Pot setting on power-up/reset

struct P162_data_struct : public PluginTaskData_base {
  P162_data_struct(int8_t csPin,
                   int8_t rstPin,
                   int8_t shdPin);
  P162_data_struct() = delete;
  virtual ~P162_data_struct();

  bool plugin_init(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);

private:

  bool hw_reset();
  void write_pot(uint8_t cmd,
                 uint8_t val);
  void updateUserVars(struct EventStruct *event);

  int16_t _pot0_value = P162_RESET_VALUE;
  int16_t _pot1_value = P162_RESET_VALUE;
  int8_t  _csPin;
  int8_t  _rstPin;
  int8_t  _shdPin;
  uint8_t _shdState    = HIGH;
  bool    _initialized = false;
};

#endif // ifdef USES_P162
#endif // ifndef PLUGINSTRUCTS_P162_DATA_STRUCT_H

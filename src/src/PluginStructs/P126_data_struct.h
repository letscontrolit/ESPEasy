#ifndef PLUGINSTRUCTS_P126_DATA_STRUCT_H
#define PLUGINSTRUCTS_P126_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P126

# include <ShiftRegister74HC595_NonTemplate.h>

# define P126_DEBUG_LOG // Enable for some (extra) logging

# define P126_CONFIG_CHIP_COUNT       PCONFIG(0)
# define P126_CONFIG_DATA_PIN         PIN(0)
# define P126_CONFIG_CLOCK_PIN        PIN(1)
# define P126_CONFIG_LATCH_PIN        PIN(2)
# define P126_CONFIG_FLAGS            PCONFIG_LONG(0)

# define P126_FLAGS_VALUES_DISPLAY    0 // 0/off = HEX, 1/on = BIN

# define P126_CONFIG_FLAGS_GET_VALUES_DISPLAY (bitRead(P126_CONFIG_FLAGS, P126_FLAGS_VALUES_DISPLAY))

# define P126_MAX_CHIP_COUNT          16 // Number of chips to support undefined = 1, range 1..16

# if !defined(P126_MAX_CHIP_COUNT)
#  define P126_MAX_CHIP_COUNT         1  // Fallback value
# endif // if !defined(P126_MAX_CHIP_COUNT)

struct P126_data_struct : public PluginTaskData_base {
public:

  P126_data_struct(int8_t  dataPin,
                   int8_t  clockPin,
                   int8_t  latchPin,
                   uint8_t chipCount);

  P126_data_struct() = delete;
  ~P126_data_struct();

  bool isInitialized() {
    return nullptr != shift;
  }

  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  bool plugin_read(struct EventStruct *event);

private:

  uint32_t getChannelState(uint8_t offset,
                           uint8_t size);

  bool     validChannel(uint channel) {
    return channel > 0 && channel <= (_chipCount * 8);
  }

  ShiftRegister74HC595_NonTemplate *shift = nullptr;

  int8_t  _dataPin;
  int8_t  _clockPin;
  int8_t  _latchPin;
  uint8_t _chipCount;
};
#endif // ifdef USES_P126
#endif // ifndef PLUGINSTRUCTS_P126_DATA_STRUCT_H

#ifndef PLUGINSTRUCTS_P126_DATA_STRUCT_H
#define PLUGINSTRUCTS_P126_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P126

# include <ShiftRegister74HC595_NonTemplate.h>

# ifndef BUILD_NO_DEBUG
#  define P126_DEBUG_LOG // Enable for some (extra) logging
# endif

# define P126_CONFIG_CHIP_COUNT       PCONFIG(0)
# define P126_CONFIG_SHOW_OFFSET      PCONFIG(1)
# define P126_CONFIG_DATA_PIN         PIN(0)
# define P126_CONFIG_CLOCK_PIN        PIN(1)
# define P126_CONFIG_LATCH_PIN        PIN(2)
# define P126_CONFIG_FLAGS            PCONFIG_ULONG(0)

# define P126_FLAGS_VALUES_DISPLAY    0 // 0/off = HEX, 1/on = BIN
// Restore values from RTC after warm boot (default enabled, inverted logic)
# define P126_FLAGS_VALUES_RESTORE    1
// 0 = decimal & hex/bin, 1 = Decimal, 2 = hex/bin
# define P126_FLAGS_OUTPUT_SELECTION  4

# define P126_CONFIG_FLAGS_GET_VALUES_DISPLAY (bitRead(P126_CONFIG_FLAGS, P126_FLAGS_VALUES_DISPLAY))
# define P126_CONFIG_FLAGS_GET_VALUES_RESTORE (bitRead(P126_CONFIG_FLAGS, P126_FLAGS_VALUES_RESTORE) == 0) // Inverted logic
# define P126_CONFIG_FLAGS_GET_OUTPUT_SELECTION (get4BitFromUL(P126_CONFIG_FLAGS, P126_FLAGS_OUTPUT_SELECTION))

# define P126_OUTPUT_BOTH             0 // Decimal + hex/bin
# define P126_OUTPUT_DEC_ONLY         1 // Decimal
# define P126_OUTPUT_HEXBIN           2 // Hex/bin

// Number of chips to support undefined = 1, range 1..255 = 8..2048 pins
# define P126_MAX_CHIP_COUNT          255

// Multiple of 4, and less than P126_MAX_CHIP_COUNT
# define P126_MAX_SHOW_OFFSET         252

# if !defined(P126_MAX_CHIP_COUNT)
#  define P126_MAX_CHIP_COUNT         1 // Fallback value
# endif // if !defined(P126_MAX_CHIP_COUNT)

# define P126_SHOW_VALUES               // When defined, will show the selected output values in either Hex or Bin format on the Devices
                                        // page. Disabled when LIMIT_BUILD_SIZE is set
# if defined(LIMIT_BUILD_SIZE) && defined(P126_SHOW_VALUES)
#  undef P126_SHOW_VALUES
# endif // if defined(LIMIT_BUILD_SIZE) && defined(P126_SHOW_VALUES)

struct P126_data_struct : public PluginTaskData_base {
public:

  P126_data_struct(int8_t  dataPin,
                   int8_t  clockPin,
                   int8_t  latchPin,
                   uint8_t chipCount);

  P126_data_struct() = delete;
  virtual ~P126_data_struct();

  const bool isInitialized() const {
    return nullptr != shift;
  }

  bool plugin_init(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);

private:

  const uint32_t getChannelState(uint8_t offset,
                                 uint8_t size) const;

  const bool     validChannel(uint channel) const {
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

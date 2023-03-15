#ifndef PLUGINSTRUCTS_P129_DATA_STRUCT_H
#define PLUGINSTRUCTS_P129_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P129

# ifndef BUILD_NO_DEBUG
#  define P129_DEBUG_LOG // Enable for some (extra) logging
# endif // ifndef BUILD_NO_DEBUG

# define P129_CONFIG_CHIP_COUNT       PCONFIG(0)
# define P129_CONFIG_DATA_PIN         PIN(0)
# define P129_CONFIG_CLOCK_PIN        PIN(1)
# define P129_CONFIG_ENABLE_PIN       PIN(2)
# define P129_CONFIG_LOAD_PIN         PCONFIG(1)
# define P129_CONFIG_FLAGS            PCONFIG(7) // MAX 16 bits!!!

# define P129_FLAGS_VALUES_DISPLAY    0          // 0/off = HEX, 1/on = BIN
# define P129_FLAGS_READ_FREQUENCY    1          // 0 = 10x/sec, 1 = 50x/sec
# define P129_FLAGS_SEPARATE_EVENTS   2          // Enable/disable separate events per pin <taskname>[#<pin>]=state,chip,port,pin
# define P129_FLAGS_OUTPUT_SELECTION  4          // 0 = decimal & hex/bin, 1 = Decimal, 2= hex/bin

# define P129_CONFIG_SHOW_OFFSET      0          // Fixed setting

# define P129_CONFIG_FLAGS_GET_VALUES_DISPLAY   (bitRead(P129_CONFIG_FLAGS, P129_FLAGS_VALUES_DISPLAY))
# define P129_CONFIG_FLAGS_GET_READ_FREQUENCY   (bitRead(P129_CONFIG_FLAGS, P129_FLAGS_READ_FREQUENCY))
# define P129_CONFIG_FLAGS_GET_SEPARATE_EVENTS  (bitRead(P129_CONFIG_FLAGS, P129_FLAGS_SEPARATE_EVENTS))
# define P129_CONFIG_FLAGS_GET_OUTPUT_SELECTION (get4BitFromUL(P129_CONFIG_FLAGS, P129_FLAGS_OUTPUT_SELECTION))

# define P129_FREQUENCY_10            0 // 10x per second
# define P129_FREQUENCY_50            1 // 50x per second

# define P129_OUTPUT_BOTH             0 // Decimal + hex/bin
# define P129_OUTPUT_DEC_ONLY         1 // Decimal
# define P129_OUTPUT_HEXBIN           2 // Hex/bin

// Number of chips to support undefined = 1, range 1..16 = 8..128 pins
# define P129_MAX_CHIP_COUNT          16

# if !defined(P129_MAX_CHIP_COUNT)
#  define P129_MAX_CHIP_COUNT         1 // Fallback value
# endif // if !defined(P129_MAX_CHIP_COUNT)
# if P129_MAX_CHIP_COUNT > 16           // Check for max. supported no. of chips
#  undef P129_MAX_CHIP_COUNT
#  define P129_MAX_CHIP_COUNT 16
# endif // if P129_MAX_CHIP_COUNT > 16

# define P129_SHOW_VALUES // When defined, will show the selected inputs in either Hex or Bin format on the Devices
                          // page. Disabled when LIMIT_BUILD_SIZE is set
# if defined(LIMIT_BUILD_SIZE) && defined(P129_SHOW_VALUES)
#  undef P129_SHOW_VALUES
# endif // if defined(LIMIT_BUILD_SIZE) && defined(P129_SHOW_VALUES)

struct P129_data_struct : public PluginTaskData_base {
public:

  P129_data_struct(int8_t  dataPin,
                   int8_t  clockPin,
                   int8_t  enablePin,
                   int8_t  loadPin,
                   uint8_t chipCount);

  P129_data_struct() = delete;
  virtual ~P129_data_struct() = default;

  const bool isInitialized() const { // All GPIO's defined
    return _dataPin != -1 &&
           _clockPin != -1 &&
           _loadPin != -1;
  }

  bool plugin_init(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    const String      & string);
  bool plugin_readData(struct EventStruct *event);

private:

  uint32_t getChannelState(uint8_t offset,
                           uint8_t size) const;

  bool     validChannel(uint channel) const {
    return channel > 0 && channel <= (_chipCount * 8);
  }

  void checkDiff(struct EventStruct *event);
  void sendInputEvent(struct EventStruct *event,
                      uint8_t             group,
                      uint8_t             bit,
                      uint8_t             state);


  const int8_t  _dataPin;
  const int8_t  _clockPin;
  const int8_t  _enablePin;
  const int8_t  _loadPin;
  uint8_t _chipCount;

  uint8_t readBuffer[P129_MAX_CHIP_COUNT] = { 0 };
  uint8_t prevBuffer[P129_MAX_CHIP_COUNT] = { 0 }; // To compare to
};

#endif // ifdef USES_P129

#endif // ifndef PLUGINSTRUCTS_P129_DATA_STRUCT_H

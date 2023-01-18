#ifndef PLUGINSTRUCTS_P124_DATA_STRUCT_H
#define PLUGINSTRUCTS_P124_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P124

# include <multi_channel_relay.h>

// # define P124_DEBUG_LOG // Enable for some (extra) logging

# define P124_CONFIG_RELAY_COUNT  PCONFIG(0)
# define P124_CONFIG_I2C_ADDRESS  PCONFIG(7) // As P0 is already in use, pick the last of the range
# define P124_CONFIG_FLAGS        PCONFIG_LONG(0)

# define P124_FLAGS_INIT_OFFSET   0          // 0..7 hold the on/off state of each relay
# define P124_FLAGS_INIT_RELAYS   8          // Initialize relays at startup
# define P124_FLAGS_INIT_ALWAYS   9          // Apply relay initialization on every plugin restart
# define P124_FLAGS_EXIT_RELAYS   10         // Initialize relays when the plugin is disabled
# define P124_FLAGS_LOOP_GET      11         // Loop the state of the channels into Channel/Get values
# define P124_FLAGS_EXIT_OFFSET   16         // 16..23 hold the on/off exit-state of each relay

struct P124_data_struct : public PluginTaskData_base {
public:

  P124_data_struct(int8_t  i2c_address,
                   uint8_t relayCount,
                   bool    changeAddress = false);

  P124_data_struct() = delete;
  virtual ~P124_data_struct();

  bool init();

  bool isInitialized() {
    return relay != nullptr;
  }

  uint8_t getChannelState();
  uint8_t getFirmwareVersion();
  bool    channelCtrl(uint8_t state);
  bool    turn_on_channel(uint8_t channel);
  bool    turn_off_channel(uint8_t channel);
  uint8_t getNextLoop();
  bool    isLoopEnabled() {
    return _loopEnabled;
  }

  void setLoopState(bool state) {
    _loopEnabled = state;
  }

private:

  bool validChannel(uint channel) {
    return channel > 0 && channel <= _relayCount;
  }

  Multi_Channel_Relay *relay = nullptr;

  const int8_t  _i2c_address;
  const uint8_t _relayCount;
  const bool _changeAddress;
  uint8_t _getLoop     = 0;
  bool    _loopEnabled = false;
};
#endif // ifdef USES_P124
#endif // ifndef PLUGINSTRUCTS_P124_DATA_STRUCT_H

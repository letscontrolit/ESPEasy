#include "../PluginStructs/P124_data_struct.h"

#ifdef USES_P124

// **************************************************************************/
// Constructor
// **************************************************************************/
P124_data_struct::P124_data_struct(uint8_t relayCount)
  : _relayCount(relayCount) {
  relay = new (std::nothrow) Multi_Channel_Relay(); // Use default address
}

// **************************************************************************/
// Destructor
// **************************************************************************/
P124_data_struct::~P124_data_struct() {
  if (isInitialized()) {
    delete relay;
    relay = nullptr;
  }
}

uint8_t P124_data_struct::getChannelState() {
  if (isInitialized()) {
    return relay->getChannelState();
  }
  return 0u;
}

uint8_t P124_data_struct::getFirmwareVersion() {
  if (isInitialized()) {
    return relay->getFirmwareVersion();
  }
  return 0u;
}

bool P124_data_struct::channelCtrl(uint8_t state) {
  if (isInitialized()) {
    relay->channelCtrl(state);
    return true;
  }
  return false;
}

bool P124_data_struct::turn_on_channel(uint8_t channel) {
  if (isInitialized() && (validChannel(channel))) {
    relay->turn_on_channel(channel);
    return true;
  }
  return false;
}

bool P124_data_struct::turn_off_channel(uint8_t channel) {
  if (isInitialized() && (validChannel(channel))) {
    relay->turn_off_channel(channel);
    return true;
  }
  return false;
}

#endif // ifdef USES_P124

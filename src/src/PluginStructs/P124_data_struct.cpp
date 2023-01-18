#include "../PluginStructs/P124_data_struct.h"

#ifdef USES_P124

// **************************************************************************/
// Constructor
// **************************************************************************/
P124_data_struct::P124_data_struct(int8_t  i2c_address,
                                   uint8_t relayCount,
                                   bool    changeAddress)
  : _i2c_address(i2c_address), _relayCount(relayCount), _changeAddress(changeAddress) 
{}

// **************************************************************************/
// Destructor
// **************************************************************************/
P124_data_struct::~P124_data_struct() {
  if (isInitialized()) {
    delete relay;
    relay = nullptr;
  }
}

bool P124_data_struct::init() {
  relay = new (std::nothrow) Multi_Channel_Relay();

  if (isInitialized()) {
    relay->begin(_i2c_address);

    if (_changeAddress) {
      // This increment shpould match with the range of addresses in _P124_MultiRelay.ino PLUGIN_I2C_HAS_ADDRESS
      uint8_t _new_address = _i2c_address == 0x18 ? 0x11 : _i2c_address + 1; // Set to next address
      relay->changeI2CAddress(_new_address, _i2c_address);
      # ifndef BUILD_NO_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("MultiRelay: Change I2C address 0x");
        log += String(_i2c_address, HEX);
        log += F(" to 0x");
        log += String(_new_address, HEX);
        addLogMove(LOG_LEVEL_INFO, log);
      }
      # endif // ifndef BUILD_NO_DEBUG
    }
  }
  return isInitialized();
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

uint8_t P124_data_struct::getNextLoop() {
  if (_loopEnabled) {
    _getLoop++;

    if (_getLoop > _relayCount) {
      _getLoop = 1;
    }
    return _getLoop;
  }
  return 0;
}

#endif // ifdef USES_P124

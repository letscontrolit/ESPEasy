#include "DeviceStruct.h"

  DeviceStruct::DeviceStruct() :
    Number(0), Type(0), VType(SENSOR_TYPE_NONE), Ports(0), ValueCount(0),
    PullUpOption(false), InverseLogicOption(false), FormulaOption(false),
    Custom(false), SendDataOption(false), GlobalSyncOption(false),
    TimerOption(false), TimerOptional(false), DecimalsOnly(false) {}

  bool DeviceStruct::connectedToGPIOpins() const {
    switch(Type) {
      case DEVICE_TYPE_SINGLE:
      case DEVICE_TYPE_DUAL:
      case DEVICE_TYPE_TRIPLE:
      case DEVICE_TYPE_SERIAL:
      case DEVICE_TYPE_SERIAL_PLUS1:
        return true;
      default:
        return false;
    }
  }

  bool DeviceStruct::usesTaskDevicePin(int pin) const {
    switch (pin) {
    case 1:
      return connectedToGPIOpins();
    case 2:
      return connectedToGPIOpins() && Type != DEVICE_TYPE_SINGLE;
    case 3:
      return Type == DEVICE_TYPE_TRIPLE || Type == DEVICE_TYPE_SERIAL_PLUS1;
    }
    return false;
  }

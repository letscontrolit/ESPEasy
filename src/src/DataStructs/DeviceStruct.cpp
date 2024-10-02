#include "../DataStructs/DeviceStruct.h"



DeviceStruct::DeviceStruct() :
  Number(0), Type(0), VType(Sensor_VType::SENSOR_TYPE_NONE), Ports(0), ValueCount(0),
  OutputDataType(Output_Data_type_t::Default),
  Pin1Direction(static_cast<uint8_t>(gpio_direction::gpio_direction_MAX)),
  Pin2Direction(static_cast<uint8_t>(gpio_direction::gpio_direction_MAX)), 
  Pin3Direction(static_cast<uint8_t>(gpio_direction::gpio_direction_MAX)),
  PullUpOption(false), InverseLogicOption(false), FormulaOption(false),
  Custom(false), SendDataOption(false), GlobalSyncOption(false),
  TimerOption(false), TimerOptional(false), DecimalsOnly(false),
  DuplicateDetection(false), ExitTaskBeforeSave(true), ErrorStateValues(false), 
  PluginStats(false), PluginLogsPeaks(false), PowerManager(false),
  TaskLogsOwnPeaks(false), I2CNoDeviceCheck(false),
  I2CMax100kHz(false), HasFormatUserVar(false) 
{}

bool DeviceStruct::connectedToGPIOpins() const {
  switch(Type) {
    case DEVICE_TYPE_SINGLE:  // Single GPIO
    case DEVICE_TYPE_SPI:
    case DEVICE_TYPE_CUSTOM1:

    case DEVICE_TYPE_DUAL:    // Dual GPIOs
    case DEVICE_TYPE_SERIAL:
    case DEVICE_TYPE_SPI2:
    case DEVICE_TYPE_CUSTOM2:

    case DEVICE_TYPE_TRIPLE:  // Triple GPIOs
    case DEVICE_TYPE_SERIAL_PLUS1:
    case DEVICE_TYPE_SPI3:
    case DEVICE_TYPE_CUSTOM3:    
      return true;
    default:
      return false;
  }
}

bool DeviceStruct::usesTaskDevicePin(int pin) const {
  if (pin == 1)
      return connectedToGPIOpins();
  if (pin == 2)
      return connectedToGPIOpins() && 
            !(Type == DEVICE_TYPE_SINGLE  ||
              Type == DEVICE_TYPE_SPI ||
              Type == DEVICE_TYPE_CUSTOM1);
  if (pin == 3)
      return Type == DEVICE_TYPE_TRIPLE || 
             Type == DEVICE_TYPE_SERIAL_PLUS1 || 
             Type == DEVICE_TYPE_SPI3 ||
             Type == DEVICE_TYPE_CUSTOM3;
  return false;
}

bool DeviceStruct::isSerial() const {
  return (Type == DEVICE_TYPE_SERIAL) || 
         (Type == DEVICE_TYPE_SERIAL_PLUS1);
}

bool DeviceStruct::isSPI() const {
  return (Type == DEVICE_TYPE_SPI) || 
         (Type == DEVICE_TYPE_SPI2) || 
         (Type == DEVICE_TYPE_SPI3);
}

bool DeviceStruct::isCustom() const {
  return (Type == DEVICE_TYPE_CUSTOM0) || 
         (Type == DEVICE_TYPE_CUSTOM1) || 
         (Type == DEVICE_TYPE_CUSTOM2) || 
         (Type == DEVICE_TYPE_CUSTOM3);
}

void DeviceStruct::setPinDirection(int pin, gpio_direction direction)
{
  const uint8_t val = static_cast<uint8_t>(direction) & ((1 << GPIO_DIRECTION_NR_BITS) - 1);
  switch (pin) {
    case 1: Pin1Direction = val; break;
    case 2: Pin2Direction = val; break;
    case 3: Pin3Direction = val; break;
  }
}

gpio_direction DeviceStruct::getPinDirection(int pin) const {
  switch (pin) {
    case 1:
      return static_cast<gpio_direction>(Pin1Direction);
    case 2:
      return static_cast<gpio_direction>(Pin2Direction);
    case 3:
      return static_cast<gpio_direction>(Pin3Direction);
  }
  return gpio_direction::gpio_direction_MAX;
}

PinSelectPurpose DeviceStruct::pinDirectionToPurpose(gpio_direction direction) const {
  switch (direction) {
  case gpio_direction::gpio_input:
    return PinSelectPurpose::Generic_input;
  case gpio_direction::gpio_output:
    return PinSelectPurpose::Generic_output;
  case gpio_direction::gpio_bidirectional:
    return PinSelectPurpose::Generic_bidir;
  case gpio_direction::gpio_direction_MAX:
    break;
  }
  return PinSelectPurpose::Generic;
}


PinSelectPurpose DeviceStruct::getPinSelectPurpose(int pin) const {
  return pinDirectionToPurpose(getPinDirection(pin));
}

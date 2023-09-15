#ifndef DATASTRUCTS_DEVICESTRUCTS_H
#define DATASTRUCTS_DEVICESTRUCTS_H


#include "../../ESPEasy_common.h"

#include <vector>

#include "../DataTypes/DeviceIndex.h"
#include "../DataTypes/PluginID.h"
#include "../DataTypes/SensorVType.h"


#define DEVICE_TYPE_SINGLE                  1 // connected through 1 datapin
#define DEVICE_TYPE_DUAL                    2 // connected through 2 datapins
#define DEVICE_TYPE_TRIPLE                  3 // connected through 3 datapins
#define DEVICE_TYPE_ANALOG                 10 // AIN/tout pin
#define DEVICE_TYPE_I2C                    20 // connected through I2C
#define DEVICE_TYPE_SERIAL                 21 // connected through UART/Serial
#define DEVICE_TYPE_SERIAL_PLUS1           22 // connected through UART/Serial + 1 extra signal pin
#define DEVICE_TYPE_SPI                    23 // connected through SPI
#define DEVICE_TYPE_SPI2                   24 // connected through SPI, 2 GPIOs
#define DEVICE_TYPE_SPI3                   25 // connected through SPI, 3 GPIOs
#define DEVICE_TYPE_CUSTOM0                30 // Custom labels, Not using TaskDevicePin1 ... TaskDevicePin3
#define DEVICE_TYPE_CUSTOM1                31 // Custom labels, 1 GPIO
#define DEVICE_TYPE_CUSTOM2                32 // Custom labels, 2 GPIOs
#define DEVICE_TYPE_CUSTOM3                33 // Custom labels, 3 GPIOs
#define DEVICE_TYPE_DUMMY                  99 // Dummy device, has no physical connection

#define I2C_MULTIPLEXER_NONE               -1 // None selected
#define I2C_MULTIPLEXER_TCA9548A            0 // TCA9548a 8 channel I2C switch, with reset, addresses 0x70-0x77
#define I2C_MULTIPLEXER_TCA9546A            1 // TCA9546a or TCA9545a 4 channel I2C switch, with reset, addresses 0x70-0x77 (no interrupt
                                              // support on TCA9545a)
#define I2C_MULTIPLEXER_TCA9543A            2 // TCA9543a 2 channel I2C switch, with reset, addresses 0x70-0x73
#define I2C_MULTIPLEXER_PCA9540             3 // PCA9540 2 channel I2C switch, no reset, address 0x70, different channel addressing

#define I2C_FLAGS_SLOW_SPEED                0 // Force slow speed when this flag is set
#define I2C_FLAGS_MUX_MULTICHANNEL          1 // Allow multiple multiplexer channels when set



/*********************************************************************************************\
* DeviceStruct
* Description of a plugin
\*********************************************************************************************/
struct __attribute__((__packed__)) DeviceStruct
{
  DeviceStruct();

  bool connectedToGPIOpins() const;

  bool usesTaskDevicePin(int pin) const;

  bool configurableDecimals() const
  {
    return FormulaOption || DecimalsOnly;
  }

  bool isSerial() const;

  bool isSPI() const;

  bool isCustom() const;

  pluginID_t getPluginID() const
  {
    return pluginID_t::toPluginID(Number);
  }


  uint8_t            Number;         // Plugin ID number.   (PLUGIN_ID_xxx)
  uint8_t            Type;           // How the device is connected. e.g. DEVICE_TYPE_SINGLE => connected through 1 datapin
  Sensor_VType       VType;          // Type of value the plugin will return. e.g. SENSOR_TYPE_STRING
  uint8_t            Ports;          // Port to use when device has multiple I/O pins  (N.B. not used much)
  uint8_t            ValueCount;     // The number of output values of a plugin. The value should match the number of keys PLUGIN_VALUENAME1_xxx
  Output_Data_type_t OutputDataType; // Subset of selectable output data types (Default = no selection)
                                     
  bool PullUpOption       : 1;       // Allow to set internal pull-up resistors.
  bool InverseLogicOption : 1;       // Allow to invert the boolean state (e.g. a switch)
  bool FormulaOption      : 1;       // Allow to enter a formula to convert values during read. (not possible with Custom enabled)
  bool Custom             : 1;
  bool SendDataOption     : 1;       // Allow to send data to a controller.
  bool GlobalSyncOption   : 1;       // No longer used. Was used for ESPeasy values sync between nodes
  bool TimerOption        : 1;       // Allow to set the "Interval" timer for the plugin.
  bool TimerOptional      : 1;       // When taskdevice timer is not set and not optional, use default "Interval" delay (Settings.Delay)
  bool DecimalsOnly       : 1;       // Allow to set the number of decimals (otherwise treated a 0 decimals)
  bool DuplicateDetection : 1;       // Some (typically receiving) plugins may receive the same data on multiple nodes. Such a plugin must help detect message duplicates.
  bool ExitTaskBeforeSave : 1;       // Optimization in memory usage, Do not exit when task data is needed during save.
  bool ErrorStateValues   : 1;       // Support Error State Values, can be called to retrieve surrogate values when PLUGIN_READ returns false
  bool PluginStats        : 1;       // Support for PluginStats to record last N task values, show charts etc.
  bool PluginLogsPeaks    : 1;       // When PluginStats is enabled, a call to PLUGIN_READ will also check for peaks. With this enabled, the plugin must call to check for peaks itself.
  bool PowerManager       : 1;       // Is a Power management controller (Power manager), that can be selected to be intialized *before* the SPI interface is started.
                                     // (F.e.: M5Stack Core/Core2 needs to power the TFT before SPI can be started)
  bool TaskLogsOwnPeaks   : 1;       // When PluginStats is enabled, a call to PLUGIN_READ will also check for peaks. With this enabled, the plugin must call to check for peaks itself.
  bool I2CNoDeviceCheck   : 1;       // When enabled, NO I2C check will be done on the I2C address returned from PLUGIN_I2C_GET_ADDRESS function call
};


// Since Device[] is used in all plugins, creating a strict struct for it will increase build size by about 5k.
// So for ESP8266, which is severely build size constraint, we use a simple vector typedef.
// For ESP32, we use the more strictly typed struct to let the compiler find undesired use of this.
#ifdef ESP8266
//typedef std::vector<DeviceStruct> DeviceVector;
typedef DeviceStruct* DeviceVector;
#else

// Specific struct used to only allow changing Device vector in the PLUGIN_ADD call
struct DeviceCount_t {
  DeviceCount_t() = default;

  DeviceCount_t& operator++() {
    // pre-increment, ++a
    ++value;
    return *this;
  }
 
  // operator int() const { return value; }

  int value = -1;

};

struct DeviceVector {

  // Regular access to DeviceStruct elements is 'const'
  const DeviceStruct& operator[](deviceIndex_t index) const
  {
    return _vector[index.value];
  }


  // Only 'write' access to DeviceStruct elements via DeviceCount_t type
  // This should only be done during call to PLUGIN_ADD
  DeviceStruct& operator[](DeviceCount_t index)
  {
    return _vector[index.value];
  }


  // Should not change anything in the device vector except for the PLUGIN_ADD call
  // Whichever calls this function should reconsider doing this
  // FIXME TD-er: Fix whereever this is called.
  DeviceStruct& getDeviceStructForEdit(deviceIndex_t index)
  {
    return _vector[index.value];
  }


  size_t size() const
  {
    return _vector.size();
  }


  void resize(size_t newSize)
  {
    _vector.resize(newSize);
  }

private:
  std::vector<DeviceStruct> _vector;
};
#endif


#endif // DATASTRUCTS_DEVICESTRUCTS_H

#ifndef DATASTRUCTS_DEVICESTRUCTS_H
#define DATASTRUCTS_DEVICESTRUCTS_H


#include "../../ESPEasy_common.h"

#include <vector>

#include "../DataTypes/DeviceIndex.h"
#include "../DataTypes/PluginID.h"
#include "../DataTypes/SensorVType.h"

#include "../Helpers/StringGenerator_GPIO.h"


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
#define I2C_FLAGS_BUS_NUMBER                2 // 3 bits. The I2C bus number to use (ESP32 only), 3 bits allow for future expansion

#if FEATURE_I2C_MULTIPLE
#define I2C_PERIPHERAL_BUS_CLOCK  0 // bit-offset for I2C bus used for the RTC clock device
#define I2C_PERIPHERAL_BUS_WDT    3 // bit-offset for I2C bus used for the watchdog timer
#define I2C_PERIPHERAL_BUS_PCFMCP 6 // bit-offset for I2C bus used for PCF & MCP direct access
// #define I2C_PERIPHERAL_BUS_???    9 // bit-offset for I2C bus used for the ???
#endif // if FEATURE_I2C_MULTIPLE


/*********************************************************************************************\
* DeviceStruct
* Description of a plugin
\*********************************************************************************************/
struct DeviceStruct
{
  DeviceStruct();

  void clear();

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

  void setPinDirection(int pin, gpio_direction direction);
  void setPin1Direction(gpio_direction direction) { setPinDirection(1, direction); }
  void setPin2Direction(gpio_direction direction) { setPinDirection(2, direction); }
  void setPin3Direction(gpio_direction direction) { setPinDirection(3, direction); }

  gpio_direction   getPinDirection(int pin) const;
  PinSelectPurpose pinDirectionToPurpose(gpio_direction direction) const;
  PinSelectPurpose getPinSelectPurpose(int pin) const;

  union {
    uint32_t _all;

    struct {
      uint32_t PullUpOption       : 1;       // Allow to set internal pull-up resistors.
      uint32_t InverseLogicOption : 1;       // Allow to invert the boolean state (e.g. a switch)
      uint32_t FormulaOption      : 1;       // Allow to enter a formula to convert values during read. (not possible with Custom enabled)
      uint32_t Custom             : 1;
      uint32_t SendDataOption     : 1;       // Allow to send data to a controller.
      uint32_t GlobalSyncOption   : 1;       // No longer used. Was used for ESPeasy values sync between nodes
      uint32_t TimerOption        : 1;       // Allow to set the "Interval" timer for the plugin.
      uint32_t TimerOptional      : 1;       // When taskdevice timer is not set and not optional, use default "Interval" delay (Settings.Delay)
      uint32_t DecimalsOnly       : 1;       // Allow to set the number of decimals (otherwise treated a 0 decimals)
      uint32_t DuplicateDetection : 1;       // Some (typically receiving) plugins may receive the same data on multiple nodes. Such a plugin must help detect message duplicates.
      uint32_t ExitTaskBeforeSave : 1;       // Optimization in memory usage, Do not exit when task data is needed during save.
      uint32_t ErrorStateValues   : 1;       // Support Error State Values, can be called to retrieve surrogate values when PLUGIN_READ returns false
      uint32_t PluginStats        : 1;       // Support for PluginStats to record last N task values, show charts etc.
      uint32_t PluginLogsPeaks    : 1;       // When PluginStats is enabled, a call to PLUGIN_READ will also check for peaks. With this enabled, the plugin must call to check for peaks itself.
      uint32_t PowerManager       : 1;       // Is a Power management controller (Power manager), that can be selected to be intialized *before* the SPI interface is started.
                                             // (F.e.: M5Stack Core/Core2 needs to power the TFT before SPI can be started)
      uint32_t TaskLogsOwnPeaks   : 1;       // When PluginStats is enabled, a call to PLUGIN_READ will also check for peaks. With this enabled, the plugin must call to check for peaks itself.
      uint32_t I2CNoDeviceCheck   : 1;       // When enabled, NO I2C check will be done on the I2C address returned from PLUGIN_I2C_GET_ADDRESS function call
      uint32_t I2CMax100kHz       : 1;       // When enabled, the device is only able to handle 100 kHz bus-clock speed, shows warning and enables "Force Slow I2C speed" by default

      uint32_t HasFormatUserVar   : 1;       // Optimization to only call this when PLUGIN_FORMAT_USERVAR is implemented
      uint32_t I2CNoBusSelection  : 1;       // Dis-allow I2C Bus selection in device configuration

      uint32_t CustomVTypeVar     : 1;       // User-selectable VType per value
      uint32_t MqttStateClass     : 1;       // MQTT StateClass setting in DevicesPage
      uint32_t HideDerivedValues  : 1;       // Hide the options for derived values
      uint32_t Dummy24            : 1;       // Dummy added to force alignment, can be re-used
      uint32_t Dummy25            : 1;       // Dummy added to force alignment, can be re-used
      uint32_t Dummy26            : 1;       // Dummy added to force alignment, can be re-used
      uint32_t Dummy27            : 1;       // Dummy added to force alignment, can be re-used
      uint32_t Dummy28            : 1;       // Dummy added to force alignment, can be re-used
      uint32_t Dummy29            : 1;       // Dummy added to force alignment, can be re-used
      uint32_t Dummy30            : 1;       // Dummy added to force alignment, can be re-used
      uint32_t Dummy31            : 1;       // Dummy added to force alignment, can be re-used
      uint32_t Dummy32            : 1;       // Dummy added to force alignment, can be re-used
    };
  };

  uint8_t            Number;         // Plugin ID number.   (PLUGIN_ID_xxx)
  uint8_t            Type;           // How the device is connected. e.g. DEVICE_TYPE_SINGLE => connected through 1 datapin
  Sensor_VType       VType;          // Type of value the plugin will return. e.g. SENSOR_TYPE_STRING
  uint8_t            Ports;          // Port to use when device has multiple I/O pins  (N.B. not used much)
  uint8_t            ValueCount;     // The number of output values of a plugin. The value should match the number of keys PLUGIN_VALUENAME1_xxx
  Output_Data_type_t OutputDataType; // Subset of selectable output data types (Default = no selection)
  union {
    uint8_t PinDirection_allBits;
    struct {
      uint8_t Pin1Direction : GPIO_DIRECTION_NR_BITS;
      uint8_t Pin2Direction : GPIO_DIRECTION_NR_BITS;
      uint8_t Pin3Direction : GPIO_DIRECTION_NR_BITS;
      uint8_t PinDirection_unused : GPIO_DIRECTION_NR_BITS;
    };
  };
  uint8_t            Unused{}; // Padding to 12 bytes struct size
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

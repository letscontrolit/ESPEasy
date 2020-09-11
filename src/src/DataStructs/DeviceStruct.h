#ifndef DATASTRUCTS_DEVICESTRUCTS_H
#define DATASTRUCTS_DEVICESTRUCTS_H


#include <Arduino.h>
#include <vector> 

#define DEVICE_TYPE_SINGLE                  1  // connected through 1 datapin
#define DEVICE_TYPE_DUAL                    2  // connected through 2 datapins
#define DEVICE_TYPE_TRIPLE                  3  // connected through 3 datapins
#define DEVICE_TYPE_ANALOG                 10  // AIN/tout pin
#define DEVICE_TYPE_I2C                    20  // connected through I2C
#define DEVICE_TYPE_SERIAL                 21  // connected through UART/Serial
#define DEVICE_TYPE_SERIAL_PLUS1           22  // connected through UART/Serial + 1 extra signal pin
#define DEVICE_TYPE_SPI                    23  // connected through SPI
#define DEVICE_TYPE_DUMMY                  99  // Dummy device, has no physical connection

#define I2C_MULTIPLEXER_NONE               -1  // None selected
#define I2C_MULTIPLEXER_TCA9548A            0  // TCA9548a 8 channel I2C switch, with reset, addresses 0x70-0x77
#define I2C_MULTIPLEXER_TCA9546A            1  // TCA9546a or TCA9545a 4 channel I2C switch, with reset, addresses 0x70-0x77 (no interrupt support on TCA9545a)
#define I2C_MULTIPLEXER_TCA9543A            2  // TCA9543a 2 channel I2C switch, with reset, addresses 0x70-0x73
#define I2C_MULTIPLEXER_PCA9540             3  // PCA9540 2 channel I2C switch, no reset, address 0x70, different channel addressing

#define I2C_FLAGS_SLOW_SPEED                0  // Force slow speed when this flag is set
#define I2C_FLAGS_MUX_MULTICHANNEL          1  // Allow multiple multiplexer channels when set

// Used for VType
#define SENSOR_TYPE_NONE                    0
#define SENSOR_TYPE_SINGLE                  1
#define SENSOR_TYPE_TEMP_HUM                2
#define SENSOR_TYPE_TEMP_BARO               3
#define SENSOR_TYPE_TEMP_HUM_BARO           4
#define SENSOR_TYPE_DUAL                    5
#define SENSOR_TYPE_TRIPLE                  6
#define SENSOR_TYPE_QUAD                    7
#define SENSOR_TYPE_TEMP_EMPTY_BARO         8
#define SENSOR_TYPE_SWITCH                 10
#define SENSOR_TYPE_DIMMER                 11
#define SENSOR_TYPE_LONG                   20
#define SENSOR_TYPE_WIND                   21
#define SENSOR_TYPE_STRING                 22

/*********************************************************************************************\
 * DeviceStruct
 * Description of a plugin
\*********************************************************************************************/
struct DeviceStruct
{
  DeviceStruct();

  bool connectedToGPIOpins() const;

  bool usesTaskDevicePin(int pin) const;

  byte Number;  // Plugin ID number.   (PLUGIN_ID_xxx)
  byte Type;    // How the device is connected. e.g. DEVICE_TYPE_SINGLE => connected through 1 datapin
  byte VType;   // Type of value the plugin will return. e.g. SENSOR_TYPE_STRING
  byte Ports;   // Port to use when device has multiple I/O pins  (N.B. not used much)
  byte ValueCount;             // The number of output values of a plugin. The value should match the number of keys PLUGIN_VALUENAME1_xxx
  bool PullUpOption : 1;       // Allow to set internal pull-up resistors.
  bool InverseLogicOption : 1; // Allow to invert the boolean state (e.g. a switch)
  bool FormulaOption : 1;      // Allow to enter a formula to convert values during read. (not possible with Custom enabled)
  bool Custom : 1;
  bool SendDataOption : 1;     // Allow to send data to a controller.
  bool GlobalSyncOption : 1;   // No longer used. Was used for ESPeasy values sync between nodes
  bool TimerOption : 1;        // Allow to set the "Interval" timer for the plugin.
  bool TimerOptional : 1;      // When taskdevice timer is not set and not optional, use default "Interval" delay (Settings.Delay)
  bool DecimalsOnly : 1;       // Allow to set the number of decimals (otherwise treated a 0 decimals)
};
typedef std::vector<DeviceStruct> DeviceVector;



#endif // DATASTRUCTS_DEVICESTRUCTS_H
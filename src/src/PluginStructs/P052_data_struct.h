#ifndef PLUGINSTRUCTS_P052_DATA_STRUCT_H
#define PLUGINSTRUCTS_P052_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P052


# define P052_MEASUREMENT_INTERVAL 60000L


# define P052_QUERY1_CONFIG_POS  0
# define P052_SENSOR_TYPE_INDEX  (P052_QUERY1_CONFIG_POS + (VARS_PER_TASK + 1))
# define P052_ABC_PERIOD         (P052_SENSOR_TYPE_INDEX + 1)
# define P052_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P052_SENSOR_TYPE_INDEX)))
# define P052_NR_OUTPUT_OPTIONS  8


// For layout and status flags in RAM/EEPROM, see document
// "CO2-Engine-BLG_ELG configuration guide Rev 1_02.docx"

// RAM layout
# define P052_RAM_ADDR_ERROR_STATUS      0x1E // U8 (error flags)
# define P052_RAM_ADDR_METER_STATUS      0x1D // U8 (status flags)
# define P052_RAM_ADDR_ALARM_STATUS      0x1C // U8 (alarm flags)
# define P052_RAM_ADDR_CO2               0x08 // S16 BLG: x.xxx%  ELG: x ppm
# define P052_RAM_ADDR_DET_TEMPERATURE   0x0A // S16 x.xx °C    (S8 sensor)
# define P052_RAM_ADDR_SPACE_TEMPERATURE 0x12 // S16 x.xx °C
# define P052_RAM_ADDR_RELATIVE_HUMIDITY 0x14 // S16 x.xx %
# define P052_RAM_ADDR_MIXING_RATIO      0x16 // S16 x.xx g/kg
# define P052_RAM_ADDR_HR1               0x40 // U16
# define P052_RAM_ADDR_HR2               0x42 // U16
# define P052_RAM_ADDR_ANIN4             0x69 // U16 x.xxx volt
# define P052_RAM_ADDR_RTC               0x65 // U32 x seconds   (virtual real time clock)
# define P052_RAM_ADDR_SCR               0x60 // U8 (special control register)

# define P052_CMD_READ_RAM  0x44

// EEPROM layout
# define P052_EEPROM_ADDR_METERCONTROL 0x03               // U8
# define P052_EEPROM_ADDR_METERCONFIG 0x06                // U16
# define P052_EEPROM_ADDR_ABC_PERIOD 0x40                 // U16 ABC period in hours
# define P052_EEPROM_ADDR_HEARTBEATPERIOD 0xA2            // U8 Period in seconds
# define P052_EEPROM_ADDR_PUMPPERIOD 0xA3                 // U8 Period in seconds
# define P052_EEPROM_ADDR_MEASUREMENT_SLEEP_PERIOD  0xB0  // U24 Measurement period (unit = seconds)
# define P052_EEPROM_ADDR_LOGGER_STRUCTURE_ADDRESS  0x200 // 16b Described in "BLG_ELG Logger Structure"

// SCR (Special Control Register) commands
# define P052_SCR_FORCE_START_MEASUREMENT 0x30
# define P052_SCR_FORCE_STOP_MEASUREMENT 0x31
# define P052_SCR_RESTART_LOGGER 0x32      // (logger data erased)
# define P052_SCR_REINITIALIZE_LOGGER 0x33 // (logger data unaffected)
# define P052_SCR_WRITE_TIMESTAMP_TO_LOGGER 0x34
# define P052_SCR_SINGLE_MEASUREMENT 0x35

// IR (Input Register)
# define P052_IR_ERRORSTATUS  0
# define P052_IR_ALARMSTATUS  1
# define P052_IR_OUTPUTSTATUS 2
# define P052_IR4_MEASURED_FILTERED_CO2    0x03
# define P052_IR5_TEMPERATURE              0x04 // Chip temperature in 1/100th degree C
# define P052_IR6_SPACE_HUMIDITY           0x05
# define P052_IR7_MEASUREMENT_COUNT        0x06 // Range 0 .. 255, to see if a measurement has been done.
# define P052_IR8_MEASUREMENT_CYCLE_TIME   0x07 // Time in current cycle (in 2 seconds steps)
# define P052_IR9_MEASURED_UNFILTERED_CO2  0x08
# define P052_IR11_MEASURED_CONCENTRATION_UNFILTERED  0x0A

# define P052_IR24_FW_TYPE                  0x17
# define P052_IR29_FW_REV                   0x1C
# define P052_IR30_SENSOR_ID_HIGH           0x1D
# define P052_IR31_SENSOR_ID_LOW            0x1E

// HR (Holding Register)
# define P052_HR1_CALIBRATION_STATUS    0
# define P052_HR2_CALIBRATION_COMMAND   0x01
# define P052_HR5_ABC_TIME              0x04
# define P052_HR11_MEASUREMENT_MODE     0x0A
# define P052_HR12_MEASUREMENT_PERIOD   0x0B // Measurement period in seconds
# define P052_HR13_NR_OF_SAMPLES        0x0C
# define P052_HR14_ABC_PERIOD           0x0D
# define P052_HR19_METER_CONTROL        0x12

// #define P052_MODBUS_SLAVE_ADDRESS 0x68
# define P052_MODBUS_SLAVE_ADDRESS 0xFE // Modbus "any address"

# define P052_MODBUS_TIMEOUT  180       // 180 msec communication timeout.

# include <ESPeasySerial.h>

# include "../Helpers/Modbus_RTU.h"
# include "../Helpers/_Plugin_Helper_serial.h"


struct P052_data_struct : public PluginTaskData_base {
  P052_data_struct() = default;

  virtual ~P052_data_struct();

  void reset();

  bool init(const ESPEasySerialPort port,
            const int16_t           serial_rx,
            const int16_t           serial_tx);

  bool isInitialized() const {
    return modbus.isInitialized();
  }

  static const __FlashStringHelper* Plugin_052_valuename(uint8_t value_nr,
                                                         bool    displayString);

  void                              setABCperiod(int hours);

  uint32_t                          getSensorID();

  // Return true, when read was successful
  bool                              readInputRegister(short addr,
                                                      int & value);

  // Return true, when read was successful
  bool readHoldingRegister(short addr,
                           int & value);

  bool plugin_write(struct EventStruct *event, const String& string);
  
  ModbusRTU_struct modbus;
};

// unsigned int _plugin_052_last_measurement = 0;


#endif // ifdef USES_P052
#endif // ifndef PLUGINSTRUCTS_P052_DATA_STRUCT_H

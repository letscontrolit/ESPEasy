#ifdef USES_P027
//#######################################################################################################
//######################### Plugin 027: INA219 DC Voltage/Current sensor ################################
//#######################################################################################################

#define PLUGIN_027
#define PLUGIN_ID_027         27
#define PLUGIN_NAME_027       "Energy (DC) - INA219"
#define PLUGIN_VALUENAME1_027 "Voltage"
#define PLUGIN_VALUENAME2_027 "Current"
#define PLUGIN_VALUENAME3_027 "Power"

// Many boards, like Adafruit INA219: https://learn.adafruit.com/adafruit-ina219-current-sensor-breakout/assembly
// A0 and A1 are default connected to GND with 10k pull-down resistor.
// To select another address, bridge either A0 and/or A1 to set to VS+ level.
#define INA219_ADDRESS                         (0x40)    // 1000000 (A0+A1=GND)
#define INA219_ADDRESS2                        (0x41)    // 1000001 (A0=VS+, A1=GND)
#define INA219_ADDRESS3                        (0x44)    // 1000100 (A0=GND, A1=VS+)
#define INA219_ADDRESS4                        (0x45)    // 1000101 (A0=VS+, A1=VS+)
#define INA219_READ                            (0x01)
#define INA219_REG_CONFIG                      (0x00)
#define INA219_CONFIG_RESET                    (0x8000)  // Reset Bit

#define INA219_CONFIG_BVOLTAGERANGE_MASK       (0x2000)  // Bus Voltage Range Mask
#define INA219_CONFIG_BVOLTAGERANGE_16V        (0x0000)  // 0-16V Range
#define INA219_CONFIG_BVOLTAGERANGE_32V        (0x2000)  // 0-32V Range

#define INA219_CONFIG_GAIN_MASK                (0x1800)  // Gain Mask
#define INA219_CONFIG_GAIN_1_40MV              (0x0000)  // Gain 1, 40mV Range
#define INA219_CONFIG_GAIN_2_80MV              (0x0800)  // Gain 2, 80mV Range
#define INA219_CONFIG_GAIN_4_160MV             (0x1000)  // Gain 4, 160mV Range
#define INA219_CONFIG_GAIN_8_320MV             (0x1800)  // Gain 8, 320mV Range

#define INA219_CONFIG_BADCRES_MASK             (0x0780)  // Bus ADC Resolution Mask
#define INA219_CONFIG_BADCRES_9BIT             (0x0080)  // 9-bit bus res = 0..511
#define INA219_CONFIG_BADCRES_10BIT            (0x0100)  // 10-bit bus res = 0..1023
#define INA219_CONFIG_BADCRES_11BIT            (0x0200)  // 11-bit bus res = 0..2047
#define INA219_CONFIG_BADCRES_12BIT            (0x0400)  // 12-bit bus res = 0..4097

#define INA219_CONFIG_SADCRES_MASK             (0x0078)  // Shunt ADC Resolution and Averaging Mask
#define INA219_CONFIG_SADCRES_9BIT_1S_84US     (0x0000)  // 1 x 9-bit shunt sample
#define INA219_CONFIG_SADCRES_10BIT_1S_148US   (0x0008)  // 1 x 10-bit shunt sample
#define INA219_CONFIG_SADCRES_11BIT_1S_276US   (0x0010)  // 1 x 11-bit shunt sample
#define INA219_CONFIG_SADCRES_12BIT_1S_532US   (0x0018)  // 1 x 12-bit shunt sample
#define INA219_CONFIG_SADCRES_12BIT_2S_1060US  (0x0048)	 // 2 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_4S_2130US  (0x0050)  // 4 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_8S_4260US  (0x0058)  // 8 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_16S_8510US (0x0060)  // 16 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_32S_17MS   (0x0068)  // 32 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_64S_34MS   (0x0070)  // 64 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_128S_69MS  (0x0078)  // 128 x 12-bit shunt samples averaged together

#define INA219_CONFIG_MODE_MASK                (0x0007)  // Operating Mode Mask
#define INA219_CONFIG_MODE_POWERDOWN           (0x0000)
#define INA219_CONFIG_MODE_SVOLT_TRIGGERED     (0x0001)
#define INA219_CONFIG_MODE_BVOLT_TRIGGERED     (0x0002)
#define INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED (0x0003)
#define INA219_CONFIG_MODE_ADCOFF              (0x0004)
#define INA219_CONFIG_MODE_SVOLT_CONTINUOUS    (0x0005)
#define INA219_CONFIG_MODE_BVOLT_CONTINUOUS    (0x0006)
#define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS (0x0007)

#define INA219_REG_SHUNTVOLTAGE                (0x01)
#define INA219_REG_BUSVOLTAGE                  (0x02)
#define INA219_REG_POWER                       (0x03)
#define INA219_REG_CURRENT                     (0x04)
#define INA219_REG_CALIBRATION                 (0x05)

typedef struct {
  uint32_t calValue;
  // The following multipliers are used to convert raw current and power
  // values to mA and mW, taking into account the current config settings
  uint32_t currentDivider_mA;
} ina219_data;

ina219_data _ina219_data[4];
int Plugin_27_i2c_addresses[4] = { INA219_ADDRESS, INA219_ADDRESS2, INA219_ADDRESS3, INA219_ADDRESS4 };

uint8_t Plugin_027_i2c_addr(struct EventStruct *event) {
   return (uint8_t)Settings.TaskDevicePluginConfig[event->TaskIndex][1];
}

uint8_t Plugin_027_device_index(const uint8_t i2caddr) {
  switch(i2caddr) {
    case INA219_ADDRESS:   return 0u;
    case INA219_ADDRESS2:  return 1u;
    case INA219_ADDRESS3:  return 2u;
    case INA219_ADDRESS4:  return 3u;
  }
  return 0u; // Some default
}

boolean Plugin_027(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_027;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_027);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_027));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_027));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_027));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choiceMode = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String optionsMode[3];
        optionsMode[0] = F("32V, 2A");
        optionsMode[1] = F("32V, 1A");
        optionsMode[2] = F("16V, 0.4A");
        int optionValuesMode[3];
        optionValuesMode[0] = 0;
        optionValuesMode[1] = 1;
        optionValuesMode[2] = 2;
        addFormSelector(F("Measure range"), F("p027_range"), 3, optionsMode, optionValuesMode, choiceMode);

        addFormSelectorI2C(F("p027_i2c"), 4, Plugin_27_i2c_addresses, Plugin_027_i2c_addr(event));

        byte choiceMeasureType = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        String options[4] = { F("Voltage"), F("Current"), F("Power"), F("Voltage/Current/Power") };
        addFormSelector(F("Measurement Type"), F("p027_measuretype"), 4, options, NULL, choiceMeasureType );

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p027_range"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("p027_i2c"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("p027_measuretype"));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
      	const uint8_t i2caddr =  Plugin_027_i2c_addr(event);
        const uint8_t idx = Plugin_027_device_index(i2caddr);
        _ina219_data[idx].currentDivider_mA = 0;
        String log = F("INA219 0x");
        log += String(i2caddr,HEX);
        log += F(" setting Range to: ");
        switch (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
        {
      		case 0:
      		{
            log += F("32V, 2A");
      		  Plugin_027_setCalibration_32V_2A(i2caddr);
      			break;
      		}
      		case 1:
      		{
            log += F("32V, 1A");
      			Plugin_027_setCalibration_32V_1A(i2caddr);
      			break;
      		}
      		case 2:
      		{
            log += F("16V, 400mA");
      			Plugin_027_setCalibration_16V_400mA(i2caddr);
      			break;
      		}
        }
        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // shuntvoltage = Plugin_027_getShuntVoltage_mV();
        // busvoltage = Plugin_027_getBusVoltage_V();
        // current_mA = Plugin_027_getCurrent_mA();
        // loadvoltage = Plugin_027_getBusVoltage_V() + (Plugin_027_getShuntVoltage_mV() / 1000);
        const uint8_t i2caddr =  Plugin_027_i2c_addr(event);

				float voltage = Plugin_027_getBusVoltage_V(i2caddr) + (Plugin_027_getShuntVoltage_mV(i2caddr) / 1000);
				float current = Plugin_027_getCurrent_mA(i2caddr)/1000;
				float power = voltage * current;

        UserVar[event->BaseVarIndex] = voltage;
      	UserVar[event->BaseVarIndex + 1] = current;
      	UserVar[event->BaseVarIndex + 2] = power;

      	String log = F("INA219 0x");
      	log += String(i2caddr,HEX);

      	// for backward compability we allow the user to select if only one measurement should be returned
      	// or all 3 measurement at once
        switch (Settings.TaskDevicePluginConfig[event->TaskIndex][2])
        {
          case 0:
          {
            event->sensorType = SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = voltage;
          	log += F(": Voltage: ");
          	log += voltage;
            break;
          }
          case 1:
          {
            event->sensorType = SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = current;
          	log += F(" Current: ");
          	log += current;
            break;
          }
          case 2:
          {
            event->sensorType = SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = power;
          	log += F(" Power: ");
          	log += power;
            break;
          }
          case 3:
          {
            event->sensorType = SENSOR_TYPE_TRIPLE;
            UserVar[event->BaseVarIndex] = voltage;
            UserVar[event->BaseVarIndex+1] = current;
            UserVar[event->BaseVarIndex+2] = power;
          	log += F(": Voltage: ");
          	log += voltage;
          	log += F(" Current: ");
          	log += current;
          	log += F(" Power: ");
          	log += power;
            break;
          }
        }

        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }
  }
  return success;
}

//**************************************************************************/
// Sends a single command byte over I2C
//**************************************************************************/
void Plugin_027_wireWriteRegister (uint8_t i2caddr, uint8_t reg, uint16_t value)
{
  Wire.beginTransmission(i2caddr);
  Wire.write(reg);                       // Register
  Wire.write((value >> 8) & 0xFF);       // Upper 8-bits
  Wire.write(value & 0xFF);              // Lower 8-bits
  Wire.endTransmission();
}

//**************************************************************************/
// Reads a 16 bit values over I2C
//**************************************************************************/
void Plugin_027_wireReadRegister(uint8_t i2caddr, uint8_t reg, uint16_t *value)
{

  Wire.beginTransmission(i2caddr);
  Wire.write(reg);                       // Register
  Wire.endTransmission();

  delay(1); // Max 12-bit conversion time is 586us per sample

  Wire.requestFrom(i2caddr, (uint8_t)2);
  // Shift values to create properly formed integer
  *value = ((Wire.read() << 8) | Wire.read());
}

//**************************************************************************/
// Configures to INA219 to be able to measure up to 32V and 2A
/**************************************************************************/
void Plugin_027_setCalibration_32V_2A(uint8_t i2caddr) {
  const uint8_t idx = Plugin_027_device_index(i2caddr);
  _ina219_data[idx].calValue = 4027;

  // Set multipliers to convert raw current/power values
  _ina219_data[idx].currentDivider_mA = 10;  // Current LSB = 100uA per bit (1000/100 = 10)

  // Set Calibration register to 'Cal' calculated above
  Plugin_027_wireWriteRegister(i2caddr, INA219_REG_CALIBRATION, _ina219_data[idx].calValue);

  // Set Config register to take into account the settings above
  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
                    INA219_CONFIG_GAIN_8_320MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
  Plugin_027_wireWriteRegister(i2caddr, INA219_REG_CONFIG, config);
}

//**************************************************************************/
// Configures to INA219 to be able to measure up to 32V and 1A
//**************************************************************************/
void Plugin_027_setCalibration_32V_1A(uint8_t i2caddr) {
  const uint8_t idx = Plugin_027_device_index(i2caddr);
  _ina219_data[idx].calValue = 10240;

  // Set multipliers to convert raw current/power values
  _ina219_data[idx].currentDivider_mA = 25;      // Current LSB = 40uA per bit (1000/40 = 25)

  // Set Calibration register to 'Cal' calculated above
  Plugin_027_wireWriteRegister(i2caddr, INA219_REG_CALIBRATION, _ina219_data[idx].calValue);

  // Set Config register to take into account the settings above
  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
                    INA219_CONFIG_GAIN_8_320MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
  Plugin_027_wireWriteRegister(i2caddr, INA219_REG_CONFIG, config);
}

//**************************************************************************/
// Configures to INA219 to be able to measure up to 16V and 400mA
//**************************************************************************/
void Plugin_027_setCalibration_16V_400mA(uint8_t i2caddr) {
  const uint8_t idx = Plugin_027_device_index(i2caddr);

  _ina219_data[idx].calValue = 8192;

  // Set multipliers to convert raw current/power values
  _ina219_data[idx].currentDivider_mA = 20;  // Current LSB = 50uA per bit (1000/50 = 20)

  // Set Calibration register to 'Cal' calculated above
  Plugin_027_wireWriteRegister(i2caddr, INA219_REG_CALIBRATION, _ina219_data[idx].calValue);

  // Set Config register to take into account the settings above
  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
                    INA219_CONFIG_GAIN_1_40MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
  Plugin_027_wireWriteRegister(i2caddr, INA219_REG_CONFIG, config);
}


//**************************************************************************/
// Gets the raw bus voltage (16-bit signed integer, so +-32767)
//**************************************************************************/
int16_t Plugin_027_getBusVoltage_raw(uint8_t i2caddr) {
  uint16_t value;
  Plugin_027_wireReadRegister(i2caddr, INA219_REG_BUSVOLTAGE, &value);

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
  return (int16_t)((value >> 3) * 4);
}

//**************************************************************************/
// Gets the raw shunt voltage (16-bit signed integer, so +-32767)
//**************************************************************************/
int16_t Plugin_027_getShuntVoltage_raw(uint8_t i2caddr) {
  uint16_t value;
  Plugin_027_wireReadRegister(i2caddr, INA219_REG_SHUNTVOLTAGE, &value);
  return (int16_t)value;
}

//**************************************************************************/
// Gets the raw current value (16-bit signed integer, so +-32767)
//**************************************************************************/
int16_t Plugin_027_getCurrent_raw(uint8_t i2caddr) {
  const uint8_t idx = Plugin_027_device_index(i2caddr);
  uint16_t value;

  // Sometimes a sharp load will reset the INA219, which will
  // reset the cal register, meaning CURRENT and POWER will
  // not be available ... avoid this by always setting a cal
  // value even if it's an unfortunate extra step
  Plugin_027_wireWriteRegister(i2caddr, INA219_REG_CALIBRATION, _ina219_data[idx].calValue);

  // Now we can safely read the CURRENT register!
  Plugin_027_wireReadRegister(i2caddr, INA219_REG_CURRENT, &value);

  return (int16_t)value;
}

//**************************************************************************/
// Gets the shunt voltage in mV (so +-327mV)
//**************************************************************************/
float Plugin_027_getShuntVoltage_mV(uint8_t i2caddr) {
  int16_t value;
  value = Plugin_027_getShuntVoltage_raw(i2caddr);
  return value * 0.01;
}

//**************************************************************************/
// Gets the shunt voltage in volts
//**************************************************************************/
float Plugin_027_getBusVoltage_V(uint8_t i2caddr) {
  int16_t value = Plugin_027_getBusVoltage_raw(i2caddr);
  return value * 0.001;
}

//**************************************************************************/
// Gets the current value in mA, taking into account the
//            config settings and current LSB
//**************************************************************************/
float Plugin_027_getCurrent_mA(uint8_t i2caddr) {
  const uint8_t idx = Plugin_027_device_index(i2caddr);
  float valueDec = Plugin_027_getCurrent_raw(i2caddr);
  valueDec /= _ina219_data[idx].currentDivider_mA;
  return valueDec;
}
#endif // USES_P027

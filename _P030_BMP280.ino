//#######################################################################################################
//#################### Plugin 030 BMP280 I2C Temp/Barometric Pressure Sensor      #######################
//#######################################################################################################

#define PLUGIN_030
#define PLUGIN_ID_030        30
#define PLUGIN_NAME_030       "Temperature & Pressure - BMP280"
#define PLUGIN_VALUENAME1_030 "Temperature"
#define PLUGIN_VALUENAME2_030 "Pressure"

enum
{
  BMP280_REGISTER_DIG_T1              = 0x88,
  BMP280_REGISTER_DIG_T2              = 0x8A,
  BMP280_REGISTER_DIG_T3              = 0x8C,

  BMP280_REGISTER_DIG_P1              = 0x8E,
  BMP280_REGISTER_DIG_P2              = 0x90,
  BMP280_REGISTER_DIG_P3              = 0x92,
  BMP280_REGISTER_DIG_P4              = 0x94,
  BMP280_REGISTER_DIG_P5              = 0x96,
  BMP280_REGISTER_DIG_P6              = 0x98,
  BMP280_REGISTER_DIG_P7              = 0x9A,
  BMP280_REGISTER_DIG_P8              = 0x9C,
  BMP280_REGISTER_DIG_P9              = 0x9E,

  BMP280_REGISTER_CHIPID             = 0xD0,
  BMP280_REGISTER_VERSION            = 0xD1,
  BMP280_REGISTER_SOFTRESET          = 0xE0,

  BMP280_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0

  BMP280_REGISTER_CONTROL            = 0xF4,
  BMP280_REGISTER_CONFIG             = 0xF5,
  BMP280_REGISTER_PRESSUREDATA       = 0xF7,
  BMP280_REGISTER_TEMPDATA           = 0xFA,

  BMP280_CONTROL_SETTING             = 0x57, // Oversampling: 16x P, 2x T, normal mode
  BMP280_CONFIG_SETTING              = 0xE0, // Tstandby 1000ms, filter 16, 3-wire SPI Disable
};

typedef struct
{
  uint16_t dig_T1;
  int16_t  dig_T2;
  int16_t  dig_T3;

  uint16_t dig_P1;
  int16_t  dig_P2;
  int16_t  dig_P3;
  int16_t  dig_P4;
  int16_t  dig_P5;
  int16_t  dig_P6;
  int16_t  dig_P7;
  int16_t  dig_P8;
  int16_t  dig_P9;
} bmp280_calib_data;

bmp280_calib_data _bmp280_calib[2];

uint8_t bmp280_i2caddr;
int32_t bmp280_sensorID;
int32_t bmp280_t_fine;

uint8_t Plugin_030_read8(byte reg, bool * is_ok = NULL); // Declaration

boolean Plugin_030_init[2] = {false, false};

boolean Plugin_030(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_030;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_TEMP_BARO;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_030);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_030));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_030));
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[2];
        options[0] = F("0x76 - default settings (SDO Low)");
        options[1] = F("0x77 - alternate settings (SDO HIGH)");
        int optionValues[2];
        optionValues[0] = 0x76;
        optionValues[1] = 0x77;
        string += F("<TR><TD>I2C Address:<TD><select name='plugin_030_bmp280_i2c'>");
        for (byte x = 0; x < 2; x++)
        {
          string += F("<option value='");
          string += optionValues[x];
          string += "'";
          if (choice == optionValues[x])
            string += F(" selected");
          string += ">";
          string += options[x];
          string += F("</option>");
        }
        string += F("</select>");
        string += F("<TR><TD>Altitude [m]:<TD><input type='text' title='Set Altitude to 0 to get measurement without altitude adjustment' name='");
        string += F("plugin_030_bmp280_elev' value='");
        string += Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        string += F("'>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_030_bmp280_i2c");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        String elev = WebServer.arg("plugin_030_bmp280_elev");
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = elev.toInt();
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        uint8_t idx = Settings.TaskDevicePluginConfig[event->TaskIndex][0] & 0x1; //Addresses are 0x76 and 0x77 so we may use it this way
        Plugin_030_init[idx] &= Plugin_030_check(Settings.TaskDevicePluginConfig[event->TaskIndex][0]); // Check id device is present
        Plugin_030_init[idx] &=  (Plugin_030_read8(BMP280_REGISTER_CONTROL) == BMP280_CONTROL_SETTING); // Check if the coefficients are still valid

        if (!Plugin_030_init[idx])
        {
          Plugin_030_init[idx] = Plugin_030_begin(Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
          delay(65); // Ultra high resolution for BMP280 is 43.2 ms, add some extra time
        }

        if (Plugin_030_init[idx])
        {
          UserVar[event->BaseVarIndex] = Plugin_030_readTemperature(idx);
          int elev = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
          if (elev)
          {
             UserVar[event->BaseVarIndex + 1] = Plugin_030_pressureElevation((float)Plugin_030_readPressure(idx) / 100, elev);
          } else {
             UserVar[event->BaseVarIndex + 1] = ((float)Plugin_030_readPressure(idx)) / 100;
          }

          String log = F("BMP280  : Address: 0x");
          log += String(bmp280_i2caddr,HEX);
          addLog(LOG_LEVEL_INFO, log);
          log = F("BMP280  : Temperature: ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
          log = F("BMP280  : Barometric Pressure: ");
          log += UserVar[event->BaseVarIndex + 1];
          addLog(LOG_LEVEL_INFO, log);
/*
          log = F("BMP280  : Coefficients [T]: ");
          log += _bmp280_calib[idx].dig_T1;
          log += ", ";
          log += _bmp280_calib[idx].dig_T2;
          log += ", ";
          log += _bmp280_calib[idx].dig_T3;
          addLog(LOG_LEVEL_INFO, log);
          log = F("BMP280  : Coefficients [P]: ");
          log += _bmp280_calib[idx].dig_P1;
          log += ", ";
          log += _bmp280_calib[idx].dig_P2;
          log += ", ";
          log += _bmp280_calib[idx].dig_P3;
          log += ", ";
          log += _bmp280_calib[idx].dig_P4;
          log += ", ";
          log += _bmp280_calib[idx].dig_P5;
          log += ", ";
          log += _bmp280_calib[idx].dig_P6;
          log += ", ";
          log += _bmp280_calib[idx].dig_P7;
          log += ", ";
          log += _bmp280_calib[idx].dig_P8;
          log += ", ";
          log += _bmp280_calib[idx].dig_P9;
          addLog(LOG_LEVEL_INFO, log);
*/
          success = true;
        }
        break;
      }

  }
  return success;
}

//**************************************************************************/
// Check BMP280 presence
//**************************************************************************/
bool Plugin_030_check(uint8_t a) {
  bmp280_i2caddr = a?a:0x76;
  bool wire_status = false;
  if (Plugin_030_read8(BMP280_REGISTER_CHIPID, &wire_status) != 0x58) {
      return false;
  } else {
      return wire_status;
  }
}

//**************************************************************************/
// Initialize BMP280
//**************************************************************************/
bool Plugin_030_begin(uint8_t a) {
  if (! Plugin_030_check(a))
    return false;

  Plugin_030_readCoefficients(a & 0x1);
  Plugin_030_write8(BMP280_REGISTER_CONTROL, BMP280_CONTROL_SETTING);
  Plugin_030_write8(BMP280_REGISTER_CONFIG, BMP280_CONFIG_SETTING);
  return true;
}

//**************************************************************************/
// Writes an 8 bit value over I2C/SPI
//**************************************************************************/
void Plugin_030_write8(byte reg, byte value)
{
  Wire.beginTransmission((uint8_t)bmp280_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}

//**************************************************************************/
// Reads an 8 bit value over I2C
//**************************************************************************/
uint8_t Plugin_030_read8(byte reg, bool * is_ok)
{
  uint8_t value;

  Wire.beginTransmission((uint8_t)bmp280_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  byte count = Wire.requestFrom((uint8_t)bmp280_i2caddr, (byte)1);
  if (is_ok != NULL) { *is_ok = (count == 1); }
  value = Wire.read();
  Wire.endTransmission();
  return value;
}

//**************************************************************************/
// Reads a 16 bit value over I2C
//**************************************************************************/
uint16_t Plugin_030_read16(byte reg)
{
  uint16_t value;

  Wire.beginTransmission((uint8_t)bmp280_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)bmp280_i2caddr, (byte)2);
  value = (Wire.read() << 8) | Wire.read();
  Wire.endTransmission();

  return value;
}

//**************************************************************************/
// Reads a 24 bit value over I2C
//**************************************************************************/
int32_t Plugin_030_read24(byte reg)
{
  int32_t value;

  Wire.beginTransmission((uint8_t)bmp280_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)bmp280_i2caddr, (byte)3);
  value = (((int32_t)Wire.read()) << 16) | (Wire.read() << 8) | Wire.read();
  Wire.endTransmission();

  return value;
}

//**************************************************************************/
// Reads a 16 bit value over I2C
//**************************************************************************/
uint16_t Plugin_030_read16_LE(byte reg) {
  uint16_t temp = Plugin_030_read16(reg);
  return (temp >> 8) | (temp << 8);

}

//**************************************************************************/
// Reads a signed 16 bit value over I2C
//**************************************************************************/
int16_t Plugin_030_readS16(byte reg)
{
  return (int16_t)Plugin_030_read16(reg);

}

int16_t Plugin_030_readS16_LE(byte reg)
{
  return (int16_t)Plugin_030_read16_LE(reg);

}

//**************************************************************************/
// Reads the factory-set coefficients
//**************************************************************************/
void Plugin_030_readCoefficients(uint8_t idx)
{
  _bmp280_calib[idx].dig_T1 = Plugin_030_read16_LE(BMP280_REGISTER_DIG_T1);
  _bmp280_calib[idx].dig_T2 = Plugin_030_readS16_LE(BMP280_REGISTER_DIG_T2);
  _bmp280_calib[idx].dig_T3 = Plugin_030_readS16_LE(BMP280_REGISTER_DIG_T3);

  _bmp280_calib[idx].dig_P1 = Plugin_030_read16_LE(BMP280_REGISTER_DIG_P1);
  _bmp280_calib[idx].dig_P2 = Plugin_030_readS16_LE(BMP280_REGISTER_DIG_P2);
  _bmp280_calib[idx].dig_P3 = Plugin_030_readS16_LE(BMP280_REGISTER_DIG_P3);
  _bmp280_calib[idx].dig_P4 = Plugin_030_readS16_LE(BMP280_REGISTER_DIG_P4);
  _bmp280_calib[idx].dig_P5 = Plugin_030_readS16_LE(BMP280_REGISTER_DIG_P5);
  _bmp280_calib[idx].dig_P6 = Plugin_030_readS16_LE(BMP280_REGISTER_DIG_P6);
  _bmp280_calib[idx].dig_P7 = Plugin_030_readS16_LE(BMP280_REGISTER_DIG_P7);
  _bmp280_calib[idx].dig_P8 = Plugin_030_readS16_LE(BMP280_REGISTER_DIG_P8);
  _bmp280_calib[idx].dig_P9 = Plugin_030_readS16_LE(BMP280_REGISTER_DIG_P9);
}

//**************************************************************************/
// Read temperature
//**************************************************************************/
float Plugin_030_readTemperature(uint8_t idx)
{
  int32_t var1, var2;

  int32_t adc_T = Plugin_030_read24(BMP280_REGISTER_TEMPDATA);
  adc_T >>= 4;

  var1  = ((((adc_T >> 3) - ((int32_t)_bmp280_calib[idx].dig_T1 << 1))) *
           ((int32_t)_bmp280_calib[idx].dig_T2)) >> 11;

  var2  = (((((adc_T >> 4) - ((int32_t)_bmp280_calib[idx].dig_T1)) *
             ((adc_T >> 4) - ((int32_t)_bmp280_calib[idx].dig_T1))) >> 12) *
           ((int32_t)_bmp280_calib[idx].dig_T3)) >> 14;

  bmp280_t_fine = var1 + var2;

  float T  = (bmp280_t_fine * 5 + 128) >> 8;
  return T / 100;
}

//**************************************************************************/
// Read pressure
//**************************************************************************/
float Plugin_030_readPressure(uint8_t idx) {
  int64_t var1, var2, p;

  int32_t adc_P = Plugin_030_read24(BMP280_REGISTER_PRESSUREDATA);
  adc_P >>= 4;

  var1 = ((int64_t)bmp280_t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)_bmp280_calib[idx].dig_P6;
  var2 = var2 + ((var1 * (int64_t)_bmp280_calib[idx].dig_P5) << 17);
  var2 = var2 + (((int64_t)_bmp280_calib[idx].dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)_bmp280_calib[idx].dig_P3) >> 8) +
         ((var1 * (int64_t)_bmp280_calib[idx].dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)_bmp280_calib[idx].dig_P1) >> 33;

  if (var1 == 0) {
    return 0;  // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)_bmp280_calib[idx].dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)_bmp280_calib[idx].dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)_bmp280_calib[idx].dig_P7) << 4);
  return (float)p / 256;
}

//**************************************************************************/
// Calculates the altitude (in meters) from the specified atmospheric
//    pressure (in hPa), and sea-level pressure (in hPa).
//    @param  seaLevel      Sea-level pressure in hPa
//    @param  atmospheric   Atmospheric pressure in hPa
//**************************************************************************/
float Plugin_030_readAltitude(float seaLevel)
{
  float atmospheric = Plugin_030_readPressure(bmp280_i2caddr & 0x01) / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}

//**************************************************************************/
// MSL pressure formula
//**************************************************************************/
float Plugin_030_pressureElevation(float atmospheric, int altitude) {
  return atmospheric / pow(1.0 - (altitude/44330.0), 5.255);
}


//#######################################################################################################
//#################### Plugin 028 BME280 I2C Temp/Hum/Barometric Pressure Sensor  #######################
//#######################################################################################################

#define PLUGIN_028
#define PLUGIN_ID_028        28
#define PLUGIN_NAME_028       "Temperature & Humidity & Pressure - BME280"
#define PLUGIN_VALUENAME1_028 "Temperature"
#define PLUGIN_VALUENAME2_028 "Humidity"
#define PLUGIN_VALUENAME3_028 "Pressure"

enum
{
  BME280_REGISTER_DIG_T1              = 0x88,
  BME280_REGISTER_DIG_T2              = 0x8A,
  BME280_REGISTER_DIG_T3              = 0x8C,

  BME280_REGISTER_DIG_P1              = 0x8E,
  BME280_REGISTER_DIG_P2              = 0x90,
  BME280_REGISTER_DIG_P3              = 0x92,
  BME280_REGISTER_DIG_P4              = 0x94,
  BME280_REGISTER_DIG_P5              = 0x96,
  BME280_REGISTER_DIG_P6              = 0x98,
  BME280_REGISTER_DIG_P7              = 0x9A,
  BME280_REGISTER_DIG_P8              = 0x9C,
  BME280_REGISTER_DIG_P9              = 0x9E,

  BME280_REGISTER_DIG_H1              = 0xA1,
  BME280_REGISTER_DIG_H2              = 0xE1,
  BME280_REGISTER_DIG_H3              = 0xE3,
  BME280_REGISTER_DIG_H4              = 0xE4,
  BME280_REGISTER_DIG_H5              = 0xE5,
  BME280_REGISTER_DIG_H6              = 0xE7,

  BME280_REGISTER_CHIPID             = 0xD0,
  BME280_REGISTER_VERSION            = 0xD1,
  BME280_REGISTER_SOFTRESET          = 0xE0,

  BME280_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0

  BME280_REGISTER_CONTROLHUMID       = 0xF2,
  BME280_REGISTER_CONTROL            = 0xF4,
  BME280_REGISTER_CONFIG             = 0xF5,
  BME280_REGISTER_PRESSUREDATA       = 0xF7,
  BME280_REGISTER_TEMPDATA           = 0xFA,
  BME280_REGISTER_HUMIDDATA          = 0xFD,
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

  uint8_t  dig_H1;
  int16_t  dig_H2;
  uint8_t  dig_H3;
  int16_t  dig_H4;
  int16_t  dig_H5;
  int8_t   dig_H6;
} bme280_calib_data;

bme280_calib_data _bme280_calib;

uint8_t _i2caddr;
int32_t _sensorID;
int32_t t_fine;

boolean Plugin_028_init = false;

boolean Plugin_028(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_028;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_TEMP_HUM_BARO;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_028);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_028));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_028));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_028));
        break;
      }

    case PLUGIN_READ:
      {
        if (!Plugin_028_init)
        {
          Plugin_028_init = true;
          Plugin_028_begin(0x76);
        }
        UserVar[event->BaseVarIndex] = Plugin_028_readTemperature();
        UserVar[event->BaseVarIndex + 1] = ((float)Plugin_028_readHumidity());
        UserVar[event->BaseVarIndex + 2] = ((float)Plugin_028_readPressure()) / 100;
        String log = F("BME  : Temperature: ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        log = F("BME  : Humidity: ");
        log += UserVar[event->BaseVarIndex + 1];
        addLog(LOG_LEVEL_INFO, log);
        log = F("BME  : Barometric Pressure: ");
        log += UserVar[event->BaseVarIndex + 2];
        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }

  }
  return success;
}

//**************************************************************************/
// Initialize BME280
//**************************************************************************/
bool Plugin_028_begin(uint8_t a) {
  _i2caddr = a;

  if (Plugin_028_read8(BME280_REGISTER_CHIPID) != 0x60)
    return false;

  Plugin_028_readCoefficients();
  Plugin_028_write8(BME280_REGISTER_CONTROLHUMID, 0x03);
  Plugin_028_write8(BME280_REGISTER_CONTROL, 0x3F);
  return true;
}

//**************************************************************************/
// Writes an 8 bit value over I2C/SPI
//**************************************************************************/
void Plugin_028_write8(byte reg, byte value)
{
  Wire.beginTransmission((uint8_t)_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}

//**************************************************************************/
// Reads an 8 bit value over I2C
//**************************************************************************/
uint8_t Plugin_028_read8(byte reg)
{
  uint8_t value;

  Wire.beginTransmission((uint8_t)_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)_i2caddr, (byte)1);
  value = Wire.read();
  Wire.endTransmission();
  return value;
}

//**************************************************************************/
// Reads a 16 bit value over I2C
//**************************************************************************/
uint16_t Plugin_028_read16(byte reg)
{
  uint16_t value;

  Wire.beginTransmission((uint8_t)_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)_i2caddr, (byte)2);
  value = (Wire.read() << 8) | Wire.read();
  Wire.endTransmission();

  return value;
}

//**************************************************************************/
// Reads a 16 bit value over I2C
//**************************************************************************/
uint16_t Plugin_028_read16_LE(byte reg) {
  uint16_t temp = Plugin_028_read16(reg);
  return (temp >> 8) | (temp << 8);

}

//**************************************************************************/
// Reads a signed 16 bit value over I2C
//**************************************************************************/
int16_t Plugin_028_readS16(byte reg)
{
  return (int16_t)Plugin_028_read16(reg);

}

int16_t Plugin_028_readS16_LE(byte reg)
{
  return (int16_t)Plugin_028_read16_LE(reg);

}

//**************************************************************************/
// Reads the factory-set coefficients
//**************************************************************************/
void Plugin_028_readCoefficients(void)
{
  _bme280_calib.dig_T1 = Plugin_028_read16_LE(BME280_REGISTER_DIG_T1);
  _bme280_calib.dig_T2 = Plugin_028_readS16_LE(BME280_REGISTER_DIG_T2);
  _bme280_calib.dig_T3 = Plugin_028_readS16_LE(BME280_REGISTER_DIG_T3);

  _bme280_calib.dig_P1 = Plugin_028_read16_LE(BME280_REGISTER_DIG_P1);
  _bme280_calib.dig_P2 = Plugin_028_readS16_LE(BME280_REGISTER_DIG_P2);
  _bme280_calib.dig_P3 = Plugin_028_readS16_LE(BME280_REGISTER_DIG_P3);
  _bme280_calib.dig_P4 = Plugin_028_readS16_LE(BME280_REGISTER_DIG_P4);
  _bme280_calib.dig_P5 = Plugin_028_readS16_LE(BME280_REGISTER_DIG_P5);
  _bme280_calib.dig_P6 = Plugin_028_readS16_LE(BME280_REGISTER_DIG_P6);
  _bme280_calib.dig_P7 = Plugin_028_readS16_LE(BME280_REGISTER_DIG_P7);
  _bme280_calib.dig_P8 = Plugin_028_readS16_LE(BME280_REGISTER_DIG_P8);
  _bme280_calib.dig_P9 = Plugin_028_readS16_LE(BME280_REGISTER_DIG_P9);

  _bme280_calib.dig_H1 = Plugin_028_read8(BME280_REGISTER_DIG_H1);
  _bme280_calib.dig_H2 = Plugin_028_readS16_LE(BME280_REGISTER_DIG_H2);
  _bme280_calib.dig_H3 = Plugin_028_read8(BME280_REGISTER_DIG_H3);
  _bme280_calib.dig_H4 = (Plugin_028_read8(BME280_REGISTER_DIG_H4) << 4) | (Plugin_028_read8(BME280_REGISTER_DIG_H4 + 1) & 0xF);
  _bme280_calib.dig_H5 = (Plugin_028_read8(BME280_REGISTER_DIG_H5 + 1) << 4) | (Plugin_028_read8(BME280_REGISTER_DIG_H5) >> 4);
  _bme280_calib.dig_H6 = (int8_t)Plugin_028_read8(BME280_REGISTER_DIG_H6);
}

//**************************************************************************/
// Read temperature
//**************************************************************************/
float Plugin_028_readTemperature(void)
{
  int32_t var1, var2;

  int32_t adc_T = Plugin_028_read16(BME280_REGISTER_TEMPDATA);
  adc_T <<= 8;
  adc_T |= Plugin_028_read8(BME280_REGISTER_TEMPDATA + 2);
  adc_T >>= 4;

  var1  = ((((adc_T >> 3) - ((int32_t)_bme280_calib.dig_T1 << 1))) *
           ((int32_t)_bme280_calib.dig_T2)) >> 11;

  var2  = (((((adc_T >> 4) - ((int32_t)_bme280_calib.dig_T1)) *
             ((adc_T >> 4) - ((int32_t)_bme280_calib.dig_T1))) >> 12) *
           ((int32_t)_bme280_calib.dig_T3)) >> 14;

  t_fine = var1 + var2;

  float T  = (t_fine * 5 + 128) >> 8;
  return T / 100;
}

//**************************************************************************/
// Read pressure
//**************************************************************************/
float Plugin_028_readPressure(void) {
  int64_t var1, var2, p;

  int32_t adc_P = Plugin_028_read16(BME280_REGISTER_PRESSUREDATA);
  adc_P <<= 8;
  adc_P |= Plugin_028_read8(BME280_REGISTER_PRESSUREDATA + 2);
  adc_P >>= 4;

  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)_bme280_calib.dig_P6;
  var2 = var2 + ((var1 * (int64_t)_bme280_calib.dig_P5) << 17);
  var2 = var2 + (((int64_t)_bme280_calib.dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)_bme280_calib.dig_P3) >> 8) +
         ((var1 * (int64_t)_bme280_calib.dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)_bme280_calib.dig_P1) >> 33;

  if (var1 == 0) {
    return 0;  // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)_bme280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)_bme280_calib.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)_bme280_calib.dig_P7) << 4);
  return (float)p / 256;
}

//**************************************************************************/
// Read humidity
//**************************************************************************/
float Plugin_028_readHumidity(void) {

  int32_t adc_H = Plugin_028_read16(BME280_REGISTER_HUMIDDATA);

  int32_t v_x1_u32r;

  v_x1_u32r = (t_fine - ((int32_t)76800));

  v_x1_u32r = (((((adc_H << 14) - (((int32_t)_bme280_calib.dig_H4) << 20) -
                  (((int32_t)_bme280_calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
               (((((((v_x1_u32r * ((int32_t)_bme280_calib.dig_H6)) >> 10) *
                    (((v_x1_u32r * ((int32_t)_bme280_calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                  ((int32_t)2097152)) * ((int32_t)_bme280_calib.dig_H2) + 8192) >> 14));

  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                             ((int32_t)_bme280_calib.dig_H1)) >> 4));

  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  float h = (v_x1_u32r >> 12);
  return  h / 1024.0;
}

//**************************************************************************/
// Calculates the altitude (in meters) from the specified atmospheric
//    pressure (in hPa), and sea-level pressure (in hPa).
//    @param  seaLevel      Sea-level pressure in hPa
//    @param  atmospheric   Atmospheric pressure in hPa
//**************************************************************************/
float Plugin_028_readAltitude(float seaLevel)
{
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  float atmospheric = Plugin_028_readPressure() / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}


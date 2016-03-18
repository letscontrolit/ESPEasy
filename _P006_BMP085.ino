//#######################################################################################################
//######################## Plugin 006 BMP0685 I2C Barometric Pressure Sensor  ###########################
//#######################################################################################################

#define PLUGIN_006
#define PLUGIN_ID_006        6
#define PLUGIN_NAME_006       "Temperature & Pressure - BMP085"
#define PLUGIN_VALUENAME1_006 "Temperature"
#define PLUGIN_VALUENAME2_006 "Pressure"

boolean Plugin_006_init = false;

boolean Plugin_006(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_006;
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
        string = F(PLUGIN_NAME_006);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_006));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_006));
        break;
      }

    case PLUGIN_READ:
      {
        if (!Plugin_006_init)
        {
          if (Plugin_006_bmp085_begin())
            Plugin_006_init = true;
        }

        if (Plugin_006_init)
        {
          UserVar[event->BaseVarIndex] = Plugin_006_bmp085_readTemperature();
          UserVar[event->BaseVarIndex + 1] = ((float)Plugin_006_bmp085_readPressure()) / 100;
          String log = F("BMP  : Temperature: ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
          log = F("BMP  : Barometric Pressure: ");
          log += UserVar[event->BaseVarIndex + 1];
          addLog(LOG_LEVEL_INFO, log);
          success = true;
        }
        break;
      }

  }
  return success;
}

#define BMP085_I2CADDR           0x77
#define BMP085_ULTRAHIGHRES         3
#define BMP085_CAL_AC1           0xAA  // R   Calibration data (16 bits)
#define BMP085_CAL_AC2           0xAC  // R   Calibration data (16 bits)
#define BMP085_CAL_AC3           0xAE  // R   Calibration data (16 bits)    
#define BMP085_CAL_AC4           0xB0  // R   Calibration data (16 bits)
#define BMP085_CAL_AC5           0xB2  // R   Calibration data (16 bits)
#define BMP085_CAL_AC6           0xB4  // R   Calibration data (16 bits)
#define BMP085_CAL_B1            0xB6  // R   Calibration data (16 bits)
#define BMP085_CAL_B2            0xB8  // R   Calibration data (16 bits)
#define BMP085_CAL_MB            0xBA  // R   Calibration data (16 bits)
#define BMP085_CAL_MC            0xBC  // R   Calibration data (16 bits)
#define BMP085_CAL_MD            0xBE  // R   Calibration data (16 bits)
#define BMP085_CONTROL           0xF4
#define BMP085_TEMPDATA          0xF6
#define BMP085_PRESSUREDATA      0xF6
#define BMP085_READTEMPCMD       0x2E
#define BMP085_READPRESSURECMD   0x34

uint8_t oversampling = BMP085_ULTRAHIGHRES;
int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
uint16_t ac4, ac5, ac6;

/*********************************************************************/
boolean Plugin_006_bmp085_begin()
/*********************************************************************/
{
  if (Plugin_006_bmp085_read8(0xD0) != 0x55) return false;

  /* read calibration data */
  ac1 = Plugin_006_bmp085_read16(BMP085_CAL_AC1);
  ac2 = Plugin_006_bmp085_read16(BMP085_CAL_AC2);
  ac3 = Plugin_006_bmp085_read16(BMP085_CAL_AC3);
  ac4 = Plugin_006_bmp085_read16(BMP085_CAL_AC4);
  ac5 = Plugin_006_bmp085_read16(BMP085_CAL_AC5);
  ac6 = Plugin_006_bmp085_read16(BMP085_CAL_AC6);

  b1 = Plugin_006_bmp085_read16(BMP085_CAL_B1);
  b2 = Plugin_006_bmp085_read16(BMP085_CAL_B2);

  mb = Plugin_006_bmp085_read16(BMP085_CAL_MB);
  mc = Plugin_006_bmp085_read16(BMP085_CAL_MC);
  md = Plugin_006_bmp085_read16(BMP085_CAL_MD);
}

/*********************************************************************/
uint16_t Plugin_006_bmp085_readRawTemperature(void)
/*********************************************************************/
{
  Plugin_006_bmp085_write8(BMP085_CONTROL, BMP085_READTEMPCMD);
  delay(5);
  return Plugin_006_bmp085_read16(BMP085_TEMPDATA);
}

/*********************************************************************/
uint32_t Plugin_006_bmp085_readRawPressure(void)
/*********************************************************************/
{
  uint32_t raw;

  Plugin_006_bmp085_write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (oversampling << 6));

  delay(26);

  raw = Plugin_006_bmp085_read16(BMP085_PRESSUREDATA);
  raw <<= 8;
  raw |= Plugin_006_bmp085_read8(BMP085_PRESSUREDATA + 2);
  raw >>= (8 - oversampling);

  return raw;
}

/*********************************************************************/
int32_t Plugin_006_bmp085_readPressure(void)
/*********************************************************************/
{
  int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
  uint32_t B4, B7;

  UT = Plugin_006_bmp085_readRawTemperature();
  UP = Plugin_006_bmp085_readRawPressure();

  // do temperature calculations
  X1 = (UT - (int32_t)(ac6)) * ((int32_t)(ac5)) / pow(2, 15);
  X2 = ((int32_t)mc * pow(2, 11)) / (X1 + (int32_t)md);
  B5 = X1 + X2;

  // do pressure calcs
  B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ( (B6 * B6) >> 12 )) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)ac1 * 4 + X3) << oversampling) + 2) / 4;

  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oversampling );

  if (B7 < 0x80000000)
  {
    p = (B7 * 2) / B4;
  }
  else
  {
    p = (B7 / B4) * 2;
  }
  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

  p = p + ((X1 + X2 + (int32_t)3791) >> 4);
  return p;
}

/*********************************************************************/
float Plugin_006_bmp085_readTemperature(void)
/*********************************************************************/
{
  int32_t UT, X1, X2, B5;     // following ds convention
  float temp;

  UT = Plugin_006_bmp085_readRawTemperature();

  // step 1
  X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) / pow(2, 15);
  X2 = ((int32_t)mc * pow(2, 11)) / (X1 + (int32_t)md);
  B5 = X1 + X2;
  temp = (B5 + 8) / pow(2, 4);
  temp /= 10;

  return temp;
}

/*********************************************************************/
uint8_t Plugin_006_bmp085_read8(uint8_t a)
/*********************************************************************/
{
  uint8_t ret;

  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(); // end transmission

  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device
  Wire.requestFrom(BMP085_I2CADDR, 1);// send data n-bytes read
  ret = Wire.read(); // receive DATA
  Wire.endTransmission(); // end transmission

  return ret;
}

/*********************************************************************/
uint16_t Plugin_006_bmp085_read16(uint8_t a)
/*********************************************************************/
{
  uint16_t ret;

  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(); // end transmission

  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device
  Wire.requestFrom(BMP085_I2CADDR, 2);// send data n-bytes read
  ret = Wire.read(); // receive DATA
  ret <<= 8;
  ret |= Wire.read(); // receive DATA
  Wire.endTransmission(); // end transmission

  return ret;
}

/*********************************************************************/
boolean Plugin_006_bmp085_write8(uint8_t a, uint8_t d)
/*********************************************************************/
{
  Wire.beginTransmission(BMP085_I2CADDR); // start transmission to device
  Wire.write(a); // sends register address to read from
  Wire.write(d);  // write data
  if(Wire.endTransmission() != 0)
    return false;

  return true;
}

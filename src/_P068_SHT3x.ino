#ifdef USES_P068

// #######################################################################################################
// ################ Plugin 68: SHT30/SHT31/SHT35 Temperature and Humidity Sensor (I2C) ###################
// #######################################################################################################
// ######################## Library source code for Arduino by WeMos, 2016 ###############################
// #######################################################################################################
// ###################### Plugin for ESP Easy by B.E.I.C. ELECTRONICS, 2017 ##############################
// ############################### http://www.beicelectronics.com ########################################
// #######################################################################################################
// ########################## Adapted to ESPEasy 2.0 by Jochen Krapf #####################################
// #######################################################################################################


#define PLUGIN_068
#define PLUGIN_ID_068         68
#define PLUGIN_NAME_068       "Environment - SHT30/31/35 [TESTING]"
#define PLUGIN_VALUENAME1_068 "Temperature"
#define PLUGIN_VALUENAME2_068 "Humidity"

// ==============================================
// SHT3X LIBRARY - SHT3X.h
// =============================================
#ifndef SHT3X_H
# define SHT3X_H
# include "_Plugin_Helper.h"

class SHT3X : public PluginTaskData_base {
public:

  SHT3X(uint8_t addr);

  void readFromSensor(void);
  bool CRC8(uint8_t MSB,
            uint8_t LSB,
            uint8_t CRC);

  float tmp = 0;
  float hum = 0;

private:

  uint8_t _i2c_device_address;
};

#endif // ifndef SHT3X_H

// ==============================================
// SHT3X LIBRARY - SHT3X.cpp
// =============================================
SHT3X::SHT3X(uint8_t addr)
{
  _i2c_device_address = addr;

  // Wire.begin();   called in ESPEasy framework

  // Set to periodic mode
  I2C_write8_reg(
    _i2c_device_address,
    0x20, // periodic 0.5mps
    0x32  // repeatability high
    );
}

void SHT3X::readFromSensor()
{
  uint16_t data[6];

  I2C_write8_reg(
    _i2c_device_address,
    0xE0, // fetch data command
    0x00
    );

  // FIXME TD-er: Currently the I2Cdev::readBytes does not support writing 2 bytes before reading.
  Wire.requestFrom(_i2c_device_address, (uint8_t)6);

  if (Wire.available() == 6)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read();

    // TODO: check CRC (data[2] and data[5])
    if (CRC8(data[0], data[1], data[2]) &&
        CRC8(data[3], data[4], data[5]))
    {
      tmp = ((((data[0] << 8) | data[1]) * 175.0) / 65535.0) - 45.0;
      hum = ((((data[3] << 8) | data[4]) * 100.0) / 65535.0);
    }
  }
  else
  {
    tmp = NAN;
    hum = NAN;

    // Set to periodic mode
    Wire.beginTransmission(_i2c_device_address);
    Wire.write(0x20); // periodic 0.5mps
    Wire.write(0x32); // repeatability high
    Wire.endTransmission();
  }
}

// FIXME TD-er: Try to make some collection of used CRC algorithms
// See http://reveng.sourceforge.net/crc-catalogue/1-15.htm#crc.cat.crc-8-dvb-s2
bool SHT3X::CRC8(uint8_t MSB, uint8_t LSB, uint8_t CRC)
{
  /*
   *	Name           : CRC-8
   * Polynomial     : 0x31 (x8 + x5 + x4 + 1)
   * Initialization : 0xFF
   * Reflect input  : False
   * Reflect output : False
   * Final          : XOR 0x00
   *	Example        : CRC8( 0xBE, 0xEF, 0x92) should be true
   */
  uint8_t crc = 0xFF;

  for (uint8_t bytenr = 0; bytenr < 2; ++bytenr) {
    crc ^= (bytenr == 0) ? MSB : LSB;

    for (uint8_t i = 0; i < 8; ++i) {
      crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }
  }
  return crc == CRC;
}

// ==============================================
// PLUGIN
// =============================================

boolean Plugin_068(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_068;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = SENSOR_TYPE_TEMP_HUM;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_068);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_068));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_068));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      int optionValues[2] = { 0x44, 0x45 };
      addFormSelectorI2C(F("i2c_addr"), 2, optionValues, PCONFIG(0));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) SHT3X(PCONFIG(0)));
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      SHT3X *sht3x = static_cast<SHT3X *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == sht3x) {
        addLog(LOG_LEVEL_ERROR, F("SHT3x: not initialised!"));
        return success;
      }

      sht3x->readFromSensor();
      UserVar[event->BaseVarIndex + 0] = sht3x->tmp;
      UserVar[event->BaseVarIndex + 1] = sht3x->hum;
      String log = F("SHT3x: Temperature: ");
      log += UserVar[event->BaseVarIndex + 0];
      addLog(LOG_LEVEL_INFO, log);
      log  = F("SHT3x: Humidity: ");
      log += UserVar[event->BaseVarIndex + 1];
      addLog(LOG_LEVEL_INFO, log);
      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P068

/********************************************************************************************\
* Initialize specific hardware setings (only global ones, others are set through devices)
\*********************************************************************************************/

void hardwareInit()
{
  // configure hardware pins according to eeprom settings.
  if (Settings.Pin_i2c_sda != -1)
    {
      String log = F("INIT : I2C");
      addLog(LOG_LEVEL_INFO,log);
      Wire.begin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
    }
}


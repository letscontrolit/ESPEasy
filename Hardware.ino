void hardwareInit()
{
  // configure hardware pins according to eeprom settings.
  if (Settings.Pin_i2c_sda != -1)
    {
      Serial.println(F("INIT : I2C"));
      Wire.begin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
    }
}


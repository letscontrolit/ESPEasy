void hardwareInit()
{
  // configure hardware pins according to eeprom settings.
  if (Settings.Pin_i2c_sda != -1)
    {
      Serial.println(F("INIT : I2C"));
      Wire.begin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
    }

  if (Settings.Pin_wired_in_1 != -1)
    {
      Serial.println(F("INIT : Input 1"));
      pinMode(Settings.Pin_wired_in_1, INPUT_PULLUP);
    }
    
  if (Settings.Pin_wired_in_2 != -1)
    {
      Serial.println(F("INIT : Input 2"));
      pinMode(Settings.Pin_wired_in_2, INPUT_PULLUP);
    }

  if (Settings.Pin_wired_out_1 != -1)
    {
      Serial.println(F("INIT : Output 1"));
      pinMode(Settings.Pin_wired_out_1, OUTPUT);
    }

  if (Settings.Pin_wired_out_2 != -1)
    {
      Serial.println(F("INIT : Output 2"));
      pinMode(Settings.Pin_wired_out_2, OUTPUT);
    }

  if (Settings.RFID > 0)
    rfidinit(Settings.Pin_wired_in_1, Settings.Pin_wired_in_2);

  if (Settings.Pulse1 > 0)
    pulseinit(Settings.Pin_wired_in_1);

}


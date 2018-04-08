/********************************************************************************************\
* Initialize specific hardware settings (only global ones, others are set through devices)
\*********************************************************************************************/

void hardwareInit()
{

  // set GPIO pins state if not set to default
  for (byte x=0; x < 17; x++)
    if (Settings.PinBootStates[x] != 0)
      switch(Settings.PinBootStates[x])
      {
        case 1:
          pinMode(x,OUTPUT);
          digitalWrite(x,LOW);
          setPinState(1, x, PIN_MODE_OUTPUT, LOW);
          break;
        case 2:
          pinMode(x,OUTPUT);
          digitalWrite(x,HIGH);
          setPinState(1, x, PIN_MODE_OUTPUT, HIGH);
          break;
        case 3:
          pinMode(x,INPUT_PULLUP);
          setPinState(1, x, PIN_MODE_INPUT, 0);
          break;
      }

  if (Settings.Pin_Reset != -1)
    pinMode(Settings.Pin_Reset,INPUT_PULLUP);
 
  // configure hardware pins according to eeprom settings.
  if (Settings.Pin_i2c_sda != -1)
  {
    String log = F("INIT : I2C");
    addLog(LOG_LEVEL_INFO, log);
    Wire.begin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
      if(Settings.WireClockStretchLimit)
      {
        String log = F("INIT : I2C custom clockstretchlimit:");
        log += Settings.WireClockStretchLimit;
        addLog(LOG_LEVEL_INFO, log);
        #if defined(ESP8266)
          Wire.setClockStretchLimit(Settings.WireClockStretchLimit);
        #endif        
      }
  }

  // I2C Watchdog boot status check
  if (Settings.WDI2CAddress != 0)
  {
    delay(500);
    Wire.beginTransmission(Settings.WDI2CAddress);
    Wire.write(0x83);             // command to set pointer
    Wire.write(17);               // pointer value to status byte
    Wire.endTransmission();

    Wire.requestFrom(Settings.WDI2CAddress, (uint8_t)1);
    if (Wire.available())
    {
      byte status = Wire.read();
      if (status & 0x1)
      {
        String log = F("INIT : Reset by WD!");
        addLog(LOG_LEVEL_ERROR, log);
        lastBootCause = BOOT_CAUSE_EXT_WD;
      }
    }
  }

  // SPI Init
  if (Settings.InitSPI)
  {
    SPI.setHwCs(false);
    SPI.begin();
    String log = F("INIT : SPI Init (without CS)");
    addLog(LOG_LEVEL_INFO, log);
  }
  else
  {
    String log = F("INIT : SPI not enabled");
    addLog(LOG_LEVEL_INFO, log);
  }

#ifdef FEATURE_SD
  if (Settings.Pin_sd_cs >= 0)
  {
    if (SD.begin(Settings.Pin_sd_cs))
    {
      String log = F("SD   : Init OK");
      addLog(LOG_LEVEL_INFO, log);
    }
    else
    {
      String log = F("SD   : Init failed");
      addLog(LOG_LEVEL_ERROR, log);
    }
  }
#endif

}

void checkResetFactoryPin(){
  static byte factoryResetCounter=0;
  if (Settings.Pin_Reset == -1)
    return;

  if (digitalRead(Settings.Pin_Reset) == 0){ // active low reset pin  
    factoryResetCounter++; // just count every second
  }
  else
  { // reset pin released
    if (factoryResetCounter > 9) // factory reset and reboot
      ResetFactory();
    if (factoryResetCounter > 3) // normal reboot
    #if defined(ESP8266)
      ESP.reset();
    #endif
    #if defined(ESP32)
      ESP.restart();
    #endif    

    factoryResetCounter = 0; // count was < 3, reset counter
  }
}


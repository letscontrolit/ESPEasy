/********************************************************************************************\
* Initialize specific hardware settings (only global ones, others are set through devices)
\*********************************************************************************************/

void hardwareInit()
{
  // set GPIO pins state if not set to default
  for (byte gpio = 0; gpio < 17; ++gpio) {
    bool serialPinConflict = (Settings.UseSerial && (gpio == 1 || gpio == 3));
    if (!serialPinConflict && Settings.PinBootStates[gpio] != 0) {
      switch(Settings.PinBootStates[gpio])
      {
        case 1:
          pinMode(gpio,OUTPUT);
          digitalWrite(gpio,LOW);
          setPinState(1, gpio, PIN_MODE_OUTPUT, LOW);
          break;
        case 2:
          pinMode(gpio,OUTPUT);
          digitalWrite(gpio,HIGH);
          setPinState(1, gpio, PIN_MODE_OUTPUT, HIGH);
          break;
        case 3:
          pinMode(gpio,INPUT_PULLUP);
          setPinState(1, gpio, PIN_MODE_INPUT, 0);
          break;
      }
    }
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

/********************************************************************************************\
  Hardware specific configurations
  \*********************************************************************************************/
String getDeviceModelString(DeviceModel model) {
  switch (model) {
    case DeviceModel_Sonoff_Basic:   return F("Sonoff Basic");
    case DeviceModel_Sonoff_TH1x:    return F("Sonoff TH1x");
    case DeviceModel_Sonoff_S2x:     return F("Sonoff S2x");
    case DeviceModel_Sonoff_TouchT1: return F("Sonoff TouchT1");
    case DeviceModel_Sonoff_TouchT2: return F("Sonoff TouchT2");
    case DeviceModel_Sonoff_TouchT3: return F("Sonoff TouchT3");
    case DeviceModel_Sonoff_4ch:     return F("Sonoff 4ch");
    case DeviceModel_Sonoff_POW:     return F("Sonoff POW");
    case DeviceModel_Sonoff_POWr2:   return F("Sonoff POW-r2");
    case DeviceModel_Shelly1:        return F("Shelly1");

    //case DeviceModel_default:
    default:        return F("default");
  }
}

bool modelMatchingFlashSize(DeviceModel model) {
  uint32_t size_MB = getFlashRealSizeInBytes() >> 20;
  // TODO TD-er: Add checks for ESP8266/ESP8285/ESP32
  switch (model) {
    case DeviceModel_Sonoff_Basic:
    case DeviceModel_Sonoff_TH1x:
    case DeviceModel_Sonoff_S2x:
    case DeviceModel_Sonoff_TouchT1:
    case DeviceModel_Sonoff_TouchT2:
    case DeviceModel_Sonoff_TouchT3:
    case DeviceModel_Sonoff_4ch:     return size_MB == 1;
    case DeviceModel_Sonoff_POW:
    case DeviceModel_Sonoff_POWr2:   return size_MB == 4;
    case DeviceModel_Shelly1:        return size_MB == 2;

    // case DeviceModel_default:
    default:  return true;
  }
  return true;
}

void setFactoryDefault(DeviceModel model) {
  ResetFactoryDefaultPreference.setDeviceModel(model);
}

/********************************************************************************************\
  Add pre defined plugins and rules.
  \*********************************************************************************************/
void addSwitchPlugin(byte taskIndex, byte gpio, const String& name, bool activeLow) {
  setTaskDevice_to_TaskIndex(1, taskIndex);
  setBasicTaskValues(
    taskIndex,
    0,            // taskdevicetimer
    true,         // enabled
    name,         // name
    gpio,         // pin1
    -1,            // pin2
    -1);           // pin3
  Settings.TaskDevicePin1PullUp[taskIndex] = true;
  if (activeLow)
    Settings.TaskDevicePluginConfig[taskIndex][2] = 1; // PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW;
}

void addPredefinedPlugins(const GpioFactorySettingsStruct& gpio_settings) {
  byte taskIndex = 0;
  for (int i = 0; i < 4; ++i) {
    if (gpio_settings.button[i] >= 0) {
      String label = F("Button");
      label += (i+1);
      addSwitchPlugin(taskIndex, gpio_settings.button[i], label, true);
      ++taskIndex;
    }
    if (gpio_settings.relais[i] >= 0) {
      String label = F("Relay");
      label += (i+1);
      addSwitchPlugin(taskIndex, gpio_settings.relais[i], label, false);
      ++taskIndex;
    }
  }
}


void addButtonRelayRule(byte buttonNumber, byte relay_gpio) {
  Settings.UseRules = true;
  String fileName;
  #if defined(ESP32)
    fileName += '/';
  #endif
  fileName += F("rules1.txt");
  String rule = F("on ButtonBNR#switch do\n  if [ButtonBNR#switch]=1\n    gpio,GNR,1\n  else\n    gpio,GNR,0\n  endif\nendon\n");
  rule.replace(F("BNR"), String(buttonNumber));
  rule.replace(F("GNR"), String(relay_gpio));
  String result = appendLineToFile(fileName, rule);
  if (result.length() > 0) {
    addLog(LOG_LEVEL_ERROR, result);
  }
}

void addPredefinedRules(const GpioFactorySettingsStruct& gpio_settings) {
  for (int i = 0; i < 4; ++i) {
    if (gpio_settings.button[i] >= 0 && gpio_settings.relais[i] >= 0) {
      addButtonRelayRule((i+1), gpio_settings.relais[i]);
    }
  }
}

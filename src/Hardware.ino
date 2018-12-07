/********************************************************************************************\
* Initialize specific hardware settings (only global ones, others are set
through devices)
\*********************************************************************************************/
bool applyGpioPinBootState(byte gpio_pin, byte PinBootState) {
  if (!checkValidGpioBootStatePin(gpio_pin))
    return false;

  bool serialPinConflict =
      (Settings.UseSerial && (gpio_pin == 1 || gpio_pin == 3));
  if (serialPinConflict)
    return false;

  int nodemcu_pinnr = -1;
  bool input, output, warning;
  getGpioInfo(gpio_pin, nodemcu_pinnr, input, output, warning);
  const uint32_t key = createInternalGpioKey(gpio_pin);
  switch (PinBootState) {
  case 0:
    if (!input)
      return false;
    pinMode(gpio_pin, INPUT);
    globalMapPortStatus[key].mode = PIN_MODE_INPUT;
    globalMapPortStatus[key].portstatus_init = 1;
    read_GPIO_state(gpio_pin, PIN_MODE_INPUT);
    // setPinState(1, gpio_pin, PIN_MODE_OUTPUT, LOW);
    break;
  case 1:
    if (!output)
      return false;
    pinMode(gpio_pin, OUTPUT);
    digitalWrite(gpio_pin, LOW);
    globalMapPortStatus[key].state = LOW;
    globalMapPortStatus[key].mode = PIN_MODE_OUTPUT;
    globalMapPortStatus[key].portstatus_init = 1;
    // setPinState(1, gpio_pin, PIN_MODE_OUTPUT, LOW);
    break;
  case 2:
    if (!output)
      return false;
    pinMode(gpio_pin, OUTPUT);
    digitalWrite(gpio_pin, HIGH);
    globalMapPortStatus[key].state = HIGH;
    globalMapPortStatus[key].mode = PIN_MODE_OUTPUT;
    globalMapPortStatus[key].portstatus_init = 1;
    // setPinState(1, gpio_pin, PIN_MODE_OUTPUT, HIGH);
    break;
  case 3:
    if (!input)
      return false;
    pinMode(gpio_pin, INPUT_PULLUP);
    globalMapPortStatus[key].mode = PIN_MODE_INPUT_PULLUP;
    globalMapPortStatus[key].portstatus_init = 1;
    read_GPIO_state(gpio_pin, PIN_MODE_INPUT_PULLUP);
    // setPinState(1, gpio_pin, PIN_MODE_INPUT, 0);
    break;
  }
  return true;
}

void hardwareInit() {
  // set GPIO pins state if not set to default
  for (byte gpio_pin = 0; gpio_pin < PIN_D_MAX; ++gpio_pin) {
    if (Settings.PinBootStates[gpio_pin] != 0) {
      // Do not change at boot when set to default.
      applyGpioPinBootState(gpio_pin, Settings.PinBootStates[gpio_pin]);
    }
  }

  if (Settings.Pin_Reset != -1)
    pinMode(Settings.Pin_Reset, INPUT_PULLUP);

  // configure hardware pins according to eeprom settings.
  if (Settings.Pin_i2c_sda != -1) {
    String log = F("INIT : I2C");
    addLog(LOG_LEVEL_INFO, log);
    Wire.begin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
    if (Settings.WireClockStretchLimit) {
      String log = F("INIT : I2C custom clockstretchlimit:");
      log += Settings.WireClockStretchLimit;
      addLog(LOG_LEVEL_INFO, log);
#if defined(ESP8266)
      Wire.setClockStretchLimit(Settings.WireClockStretchLimit);
#endif
    }
  }

  // I2C Watchdog boot status check
  if (Settings.WDI2CAddress != 0) {
    delay(500);
    Wire.beginTransmission(Settings.WDI2CAddress);
    Wire.write(0x83); // command to set pointer
    Wire.write(17);   // pointer value to status byte
    Wire.endTransmission();

    Wire.requestFrom(Settings.WDI2CAddress, (uint8_t)1);
    if (Wire.available()) {
      byte status = Wire.read();
      if (status & 0x1) {
        String log = F("INIT : Reset by WD!");
        addLog(LOG_LEVEL_ERROR, log);
        lastBootCause = BOOT_CAUSE_EXT_WD;
      }
    }
  }

  // SPI Init
  if (Settings.InitSPI) {
    SPI.setHwCs(false);
    SPI.begin();
    String log = F("INIT : SPI Init (without CS)");
    addLog(LOG_LEVEL_INFO, log);
  } else {
    String log = F("INIT : SPI not enabled");
    addLog(LOG_LEVEL_INFO, log);
  }

#ifdef FEATURE_SD
  if (Settings.Pin_sd_cs >= 0) {
    if (SD.begin(Settings.Pin_sd_cs)) {
      String log = F("SD   : Init OK");
      addLog(LOG_LEVEL_INFO, log);
    } else {
      String log = F("SD   : Init failed");
      addLog(LOG_LEVEL_ERROR, log);
    }
  }
#endif
}

void checkResetFactoryPin() {
  static byte factoryResetCounter = 0;
  if (Settings.Pin_Reset == -1)
    return;

  if (digitalRead(Settings.Pin_Reset) == 0) { // active low reset pin
    factoryResetCounter++;                    // just count every second
  } else {                                    // reset pin released
    if (factoryResetCounter > 9) {
      // factory reset and reboot
      ResetFactory();
    }
    if (factoryResetCounter > 3) {
      // normal reboot
      reboot();
    }
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
  case DeviceModel_Sonoff_4ch:
    return size_MB == 1;
  case DeviceModel_Sonoff_POW:
  case DeviceModel_Sonoff_POWr2:
    return size_MB == 4;
  case DeviceModel_Shelly1:
    return size_MB == 2;

  // case DeviceModel_default:
  default:
    return true;
  }
  return true;
}

void setFactoryDefault(DeviceModel model) {
  ResetFactoryDefaultPreference.setDeviceModel(model);
}

/********************************************************************************************\
  Add pre defined plugins and rules.
  \*********************************************************************************************/
void addSwitchPlugin(byte taskIndex, byte gpio_pin, const String &name,
                     bool activeLow) {
  setTaskDevice_to_TaskIndex(1, taskIndex);
  setBasicTaskValues(taskIndex,
                     0,        // taskdevicetimer
                     true,     // enabled
                     name,     // name
                     gpio_pin, // pin1
                     -1,       // pin2
                     -1);      // pin3
  Settings.TaskDevicePin1PullUp[taskIndex] = true;
  if (activeLow)
    Settings.TaskDevicePluginConfig[taskIndex][2] =
        1; // PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW;
}

void addPredefinedPlugins(const GpioFactorySettingsStruct &gpio_settings) {
  byte taskIndex = 0;
  for (int i = 0; i < 4; ++i) {
    if (gpio_settings.button[i] >= 0) {
      String label = F("Button");
      label += (i + 1);
      addSwitchPlugin(taskIndex, gpio_settings.button[i], label, true);
      ++taskIndex;
    }
    if (gpio_settings.relais[i] >= 0) {
      String label = F("Relay");
      label += (i + 1);
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
  String rule =
      F("on ButtonBNR#switch do\n  if [ButtonBNR#switch]=1\n    "
        "gpio_pin,GNR,1\n  else\n    gpio_pin,GNR,0\n  endif\nendon\n");
  rule.replace(F("BNR"), String(buttonNumber));
  rule.replace(F("GNR"), String(relay_gpio));
  String result = appendLineToFile(fileName, rule);
  if (result.length() > 0) {
    addLog(LOG_LEVEL_ERROR, result);
  }
}

void addPredefinedRules(const GpioFactorySettingsStruct &gpio_settings) {
  for (int i = 0; i < 4; ++i) {
    if (gpio_settings.button[i] >= 0 && gpio_settings.relais[i] >= 0) {
      addButtonRelayRule((i + 1), gpio_settings.relais[i]);
    }
  }
}

bool checkValidGpioBootStatePin(byte gpio_pin) {
#ifdef ESP8266
  // GPIO 16 is a strange pin, pulled to GND and when used to wake from deep
  // sleep,
  // it will trigger a reset when changed.
  if (gpio_pin == 16)
    return false;
#endif
  return checkValidGpioPin(gpio_pin);
}

//********************************************************************************
// Get info of a specific GPIO pin.
//********************************************************************************
#ifdef ESP32
// return true when pin can be used.
bool getGpioInfo(int gpio_pin, int& nodemcu_pinnr, bool& input, bool& output, bool& warning) {
  nodemcu_pinnr = -1; // ESP32 does not label the pins, they just use the GPIO number.

  // Input GPIOs:  0-19, 21-23, 25-27, 32-39
  // Output GPIOs: 0-19, 21-23, 25-27, 32-33
  input = gpio_pin <= 39;
  output = gpio_pin <= 33;
  if (gpio_pin < 0 || gpio_pin == 20 || gpio_pin == 24 || (gpio_pin > 27 && gpio_pin < 32)) {
    input = false;
    output = false;
  }
  if (input == false && output == false) {
    return false;
  }

  // GPIO 0 & 2 can't be used as an input. State during boot is dependent on boot mode.
  warning = (gpio_pin == 0 || gpio_pin == 2);
  if (gpio_pin == 12) {
    // If driven High, flash voltage (VDD_SDIO) is 1.8V not default 3.3V.
    // Has internal pull-down, so unconnected = Low = 3.3V.
    // May prevent flashing and/or booting if 3.3V flash is used and this pin is
    // pulled high, causing the flash to brownout.
    // See the ESP32 datasheet for more details.
    warning = true;
  }
  if (gpio_pin == 15) {
    // If driven Low, silences boot messages printed by the ROM bootloader.
    // Has an internal pull-up, so unconnected = High = normal output.
    warning = true;
  }
  return true;
};
#else
// return true when pin can be used.
bool getGpioInfo(int gpio_pin, int& nodemcu_pinnr, bool& input, bool& output, bool& warning) {
  nodemcu_pinnr = -1;
  input = true;
  output = true;
  // GPIO 0, 2 & 15 can't be used as an input. State during boot is dependent on boot mode.
  // GPIO 16 is a strange pin, pulled to GND and when used to wake from deep sleep,
  // it will trigger a reset when changed.
  warning = (gpio_pin == 0 || gpio_pin == 2 || gpio_pin == 15 || gpio_pin == 16);
  switch (gpio_pin) {
    case  0: nodemcu_pinnr =  3; break;
    case  1: nodemcu_pinnr = 10; break;
    case  2: nodemcu_pinnr =  4; break;
    case  3: nodemcu_pinnr =  9; break;
    case  4: nodemcu_pinnr =  2; break;
    case  5: nodemcu_pinnr =  1; break;
    case  6: // GPIO 6 .. 8  is used for flash
    case  7:
    case  8: nodemcu_pinnr = -1; break;
    case  9: nodemcu_pinnr = 11; break; // On ESP8266 used for flash
    case 10: nodemcu_pinnr = 12; break; // On ESP8266 used for flash
    case 11: nodemcu_pinnr = -1; break;
    case 12: nodemcu_pinnr =  6; break;
    case 13: nodemcu_pinnr =  7; break;
    case 14: nodemcu_pinnr =  5; break;
    // GPIO-15 Can't be used as an input. There is an external pull-down on this pin.
    case 15: nodemcu_pinnr =  8; input = false; break;
    case 16: nodemcu_pinnr =  0; break; // This is used by the deep-sleep mechanism
  }
  #ifndef ESP8285
  if (gpio_pin == 9 || gpio_pin == 10) {
    // On ESP8266 used for flash
    warning = true;
  }
#endif
  if (nodemcu_pinnr < 0) {
    input = false;
    output = false;
    return false;
  }
  return true;
}
#endif

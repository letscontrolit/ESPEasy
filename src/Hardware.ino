#include "src/DataStructs/PinMode.h"
#include "src/Globals/ResetFactoryDefaultPref.h"

/********************************************************************************************\
 * Initialize specific hardware settings (only global ones, others are set through devices)
 \*********************************************************************************************/
void hardwareInit()
{
  // set GPIO pins state if not set to default
  constexpr byte maxStates = sizeof(Settings.PinBootStates)/sizeof(Settings.PinBootStates[0]);
  for (byte gpio = 0; gpio < PIN_D_MAX; ++gpio) {
    bool serialPinConflict = (Settings.UseSerial && (gpio == 1 || gpio == 3));
    const int8_t bootState = (gpio < maxStates) ? Settings.PinBootStates[gpio] : 0;

    if (!serialPinConflict && (bootState != 0)) {
      const uint32_t key = createKey(1, gpio);

      switch (bootState)
      {
        case 1:
          pinMode(gpio, OUTPUT);
          digitalWrite(gpio, LOW);
          globalMapPortStatus[key].state = LOW;
          globalMapPortStatus[key].mode  = PIN_MODE_OUTPUT;
          globalMapPortStatus[key].init  = 1;

          // setPinState(1, gpio, PIN_MODE_OUTPUT, LOW);
          break;
        case 2:
          pinMode(gpio, OUTPUT);
          digitalWrite(gpio, HIGH);
          globalMapPortStatus[key].state = HIGH;
          globalMapPortStatus[key].mode  = PIN_MODE_OUTPUT;
          globalMapPortStatus[key].init  = 1;

          // setPinState(1, gpio, PIN_MODE_OUTPUT, HIGH);
          break;
        case 3:
          pinMode(gpio, INPUT_PULLUP);
          globalMapPortStatus[key].state = 0;
          globalMapPortStatus[key].mode  = PIN_MODE_INPUT_PULLUP;
          globalMapPortStatus[key].init  = 1;

          // setPinState(1, gpio, PIN_MODE_INPUT, 0);
          break;
      }
    }
  }

  if (Settings.Pin_Reset != -1) {
    pinMode(Settings.Pin_Reset, INPUT_PULLUP);
  }

  initI2C();

  // SPI Init
  if (Settings.InitSPI>0)
  {
    SPI.setHwCs(false);
    
    //MFD: for ESP32 enable the SPI on HSPI as the default is VSPI
    #ifdef ESP32 
    if (Settings.InitSPI==2)
    {
      #define HSPI_MISO   12
      #define HSPI_MOSI   13
      #define HSPI_SCLK   14
      #define HSPI_SS     15
      SPI.begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI); //HSPI
    }
    else
     SPI.begin(); //VSPI
    #else
    SPI.begin();
    #endif
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
#endif // ifdef FEATURE_SD
}

void initI2C() {
  // configure hardware pins according to eeprom settings.
  if (Settings.Pin_i2c_sda != -1)
  {
    addLog(LOG_LEVEL_INFO, F("INIT : I2C"));
    Wire.setClock(Settings.I2C_clockSpeed);
    Wire.begin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);

    if (Settings.WireClockStretchLimit)
    {
      String log = F("INIT : I2C custom clockstretchlimit:");
      log += Settings.WireClockStretchLimit;
      addLog(LOG_LEVEL_INFO, log);
        #if defined(ESP8266)
      Wire.setClockStretchLimit(Settings.WireClockStretchLimit);
        #endif // if defined(ESP8266)
    }
  }

  // I2C Watchdog boot status check
  if (Settings.WDI2CAddress != 0)
  {
    delay(500);
    Wire.beginTransmission(Settings.WDI2CAddress);
    Wire.write(0x83); // command to set pointer
    Wire.write(17);   // pointer value to status byte
    Wire.endTransmission();

    Wire.requestFrom(Settings.WDI2CAddress, (uint8_t)1);

    if (Wire.available())
    {
      byte status = Wire.read();

      if (status & 0x1)
      {
        addLog(LOG_LEVEL_ERROR, F("INIT : Reset by WD!"));
        lastBootCause = BOOT_CAUSE_EXT_WD;
      }
    }
  }
}

void checkResetFactoryPin() {
  static byte factoryResetCounter = 0;

  if (Settings.Pin_Reset == -1) {
    return;
  }

  if (digitalRead(Settings.Pin_Reset) == 0) { // active low reset pin
    factoryResetCounter++;                    // just count every second
  }
  else
  {                                           // reset pin released
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


#ifdef ESP8266
int espeasy_analogRead(int pin) {
  if (!wifiConnectInProgress) {
    lastADCvalue = analogRead(A0);
  }
  return lastADCvalue;
}
#endif

#ifdef ESP32
int espeasy_analogRead(int pin) {
  return espeasy_analogRead(pin, false);
}

int espeasy_analogRead(int pin, bool readAsTouch) {
  int value = 0;
  int adc, ch, t;
  if (getADC_gpio_info(pin, adc, ch, t)) {
    bool canread = false;
    switch (adc) {
      case 0:
        value = hallRead();
        break;
      case 1:
        canread = true;
        break;
      case 2:
        if (WiFi.getMode() == WIFI_OFF) {
          // See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html#configuration-and-reading-adc
          // ADC2 is shared with WiFi, so don't read ADC2 when WiFi is on.
          canread = true;
        }
        break;
    }
    if (canread) {
      if (readAsTouch && t >= 0) {
        value = touchRead(pin);
      } else {
        value = analogRead(pin);
      }
    }
  }
  return value;
}
#endif


/********************************************************************************************\
   Hardware specific configurations
 \*********************************************************************************************/
String getDeviceModelBrandString(DeviceModel model) {
  switch (model) {
    case DeviceModel_Sonoff_Basic:
    case DeviceModel_Sonoff_TH1x:
    case DeviceModel_Sonoff_S2x:
    case DeviceModel_Sonoff_TouchT1:
    case DeviceModel_Sonoff_TouchT2:
    case DeviceModel_Sonoff_TouchT3:
    case DeviceModel_Sonoff_4ch:
    case DeviceModel_Sonoff_POW:
    case DeviceModel_Sonoff_POWr2:   return F("Sonoff");
    case DeviceModel_Shelly1:
    case DeviceModel_ShellyPLUG_S:   return F("Shelly");
    case DeviceMode_Olimex_ESP32_PoE: return F("Olimex");

    // case DeviceModel_default:
    default:        return "";
  }
}

String getDeviceModelString(DeviceModel model) {
  String result;

  result.reserve(16);
  result = getDeviceModelBrandString(model);

  switch (model) {
    case DeviceModel_Sonoff_Basic:   result += F(" Basic");   break;
    case DeviceModel_Sonoff_TH1x:    result += F(" TH1x");    break;
    case DeviceModel_Sonoff_S2x:     result += F(" S2x");     break;
    case DeviceModel_Sonoff_TouchT1: result += F(" TouchT1"); break;
    case DeviceModel_Sonoff_TouchT2: result += F(" TouchT2"); break;
    case DeviceModel_Sonoff_TouchT3: result += F(" TouchT3"); break;
    case DeviceModel_Sonoff_4ch:     result += F(" 4ch");     break;
    case DeviceModel_Sonoff_POW:     result += F(" POW");     break;
    case DeviceModel_Sonoff_POWr2:   result += F(" POW-r2");  break;
    case DeviceModel_Shelly1:        result += '1';           break;
    case DeviceModel_ShellyPLUG_S:   result += F(" PLUG S");  break;
    case DeviceMode_Olimex_ESP32_PoE: result += F(" ESP32-PoE"); break;

    // case DeviceModel_default:
    default:    result += F("default");
  }
  return result;
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
    case DeviceModel_Shelly1:     
    case DeviceModel_ShellyPLUG_S:   return size_MB == 2;
    case DeviceMode_Olimex_ESP32_PoE:return size_MB == 4;

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
void addSwitchPlugin(taskIndex_t taskIndex, byte gpio, const String& name, bool activeLow) {
  setTaskDevice_to_TaskIndex(1, taskIndex);
  setBasicTaskValues(
    taskIndex,
    0,    // taskdevicetimer
    true, // enabled
    name, // name
    gpio, // pin1
    -1,   // pin2
    -1);  // pin3
  Settings.TaskDevicePin1PullUp[taskIndex] = true;

  if (activeLow) {
    Settings.TaskDevicePluginConfig[taskIndex][2] = 1; // PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW;
  }
  Settings.TaskDevicePluginConfig[taskIndex][3] = 1;   // "Send Boot state" checked.
}

void addPredefinedPlugins(const GpioFactorySettingsStruct& gpio_settings) {
  taskIndex_t taskIndex = 0;

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
  #endif // if defined(ESP32)
  fileName += F("rules1.txt");
  String rule = F("on ButtonBNR#state do\n  if [RelayBNR#state]=0\n    gpio,GNR,1\n  else\n    gpio,GNR,0\n  endif\nendon\n");
  rule.replace(F("BNR"), String(buttonNumber));
  rule.replace(F("GNR"), String(relay_gpio));
  String result = appendLineToFile(fileName, rule);

  if (result.length() > 0) {
    addLog(LOG_LEVEL_ERROR, result);
  }
}

void addPredefinedRules(const GpioFactorySettingsStruct& gpio_settings) {
  for (int i = 0; i < 4; ++i) {
    if ((gpio_settings.button[i] >= 0) && (gpio_settings.relais[i] >= 0)) {
      addButtonRelayRule((i + 1), gpio_settings.relais[i]);
    }
  }
}

#ifdef ESP32

// ********************************************************************************
// Get info of a specific GPIO pin.
// ********************************************************************************
bool getGpioInfo(int gpio, int& pinnr, bool& input, bool& output, bool& warning) {
  pinnr = -1; // ESP32 does not label the pins, they just use the GPIO number.

  // Input GPIOs:  0-19, 21-23, 25-27, 32-39
  // Output GPIOs: 0-19, 21-23, 25-27, 32-33
  input  = gpio <= 39;
  output = gpio <= 33;

  if ((gpio < 0) || (gpio == 20) || (gpio == 24) || ((gpio > 27) && (gpio < 32))) {
    input  = false;
    output = false;
  }

  if ((input == false) && (output == false)) {
    return false;
  }

  // GPIO 0 & 2 can't be used as an input. State during boot is dependent on boot mode.
  warning = (gpio == 0 || gpio == 2);

  if (gpio == 12) {
    // If driven High, flash voltage (VDD_SDIO) is 1.8V not default 3.3V.
    // Has internal pull-down, so unconnected = Low = 3.3V.
    // May prevent flashing and/or booting if 3.3V flash is used and this pin is
    // pulled high, causing the flash to brownout.
    // See the ESP32 datasheet for more details.
    warning = true;
  }

  if (gpio == 15) {
    // If driven Low, silences boot messages printed by the ROM bootloader.
    // Has an internal pull-up, so unconnected = High = normal output.
    warning = true;
  }
  return true;
}

#else // ifdef ESP32

// return true when pin can be used.
bool getGpioInfo(int gpio, int& pinnr, bool& input, bool& output, bool& warning) {
  pinnr  = -1;
  input  = true;
  output = true;

  // GPIO 0, 2 & 15 can't be used as an input. State during boot is dependent on boot mode.
  warning = (gpio == 0 || gpio == 2 || gpio == 15);

  switch (gpio) {
    case  0: pinnr =  3; break;
    case  1: pinnr = 10; break;
    case  2: pinnr =  4; break;
    case  3: pinnr =  9; break;
    case  4: pinnr =  2; break;
    case  5: pinnr =  1; break;
    case  6:                    // GPIO 6 .. 8  is used for flash
    case  7:
    case  8: pinnr = -1; break;
    case  9: pinnr = 11; break; // On ESP8266 used for flash
    case 10: pinnr = 12; break; // On ESP8266 used for flash
    case 11: pinnr = -1; break;
    case 12: pinnr =  6; break;
    case 13: pinnr =  7; break;
    case 14: pinnr =  5; break;

    // GPIO-15 Can't be used as an input. There is an external pull-down on this pin.
    case 15: pinnr =  8; input = false; break;
    case 16: pinnr =  0; break; // This is used by the deep-sleep mechanism
  }
  # ifndef ESP8285

  if ((gpio == 9) || (gpio == 10)) {
    // On ESP8266 used for flash
    warning = true;
  }
  # endif // ifndef ESP8285

  if (pinnr < 0) {
    input  = false;
    output = false;
    return false;
  }
  return true;
}

#endif // ifdef ESP32


#ifdef ESP32

// Get ADC related info for a given GPIO pin
// @param gpio_pin   GPIO pin number
// @param adc        Number of ADC unit (0 == Hall effect)
// @param ch         Channel number on ADC unit
// @param t          index of touch pad ID
bool getADC_gpio_info(int gpio_pin, int& adc, int& ch, int& t)
{
  t = -1;
  switch (gpio_pin) {
    case -1: adc = 0; break; // Hall effect Sensor
    case 36: adc = 1; ch = 0; break;
    case 37: adc = 1; ch = 1; break;
    case 38: adc = 1; ch = 2; break;
    case 39: adc = 1; ch = 3; break;
    case 32: adc = 1; ch = 4; t = 9; break;
    case 33: adc = 1; ch = 5; t = 8; break;
    case 34: adc = 1; ch = 6; break;
    case 35: adc = 1; ch = 7; break;
    case 4:  adc = 2; ch = 0; t = 0; break;
    case 0:  adc = 2; ch = 1; t = 1; break;
    case 2:  adc = 2; ch = 2; t = 2; break;
    case 15: adc = 2; ch = 3; t = 3; break;
    case 13: adc = 2; ch = 4; t = 4; break;
    case 12: adc = 2; ch = 5; t = 5; break;
    case 14: adc = 2; ch = 6; t = 6; break;
    case 27: adc = 2; ch = 7; t = 7; break;
    case 25: adc = 2; ch = 8; break;
    case 26: adc = 2; ch = 9; break;
    default:
      return false;
  }
  return true;
}

int touchPinToGpio(int touch_pin)
{
  switch(touch_pin) {
    case 0: return T0;
    case 1: return T1;
    case 2: return T2;
    case 3: return T3;
    case 4: return T4;
    case 5: return T5;
    case 6: return T6;
    case 7: return T7;
    case 8: return T8;
    case 9: return T9;
    default:
    break;    
  }
  return -1;
}


#endif
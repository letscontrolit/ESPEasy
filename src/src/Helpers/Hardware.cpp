#include "Hardware.h"

#include "../Commands/GPIO.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Globals/Device.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"
#include "../Globals/GlobalMapPortStatus.h"

#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Misc.h"
#include "../Helpers/PortStatus.h"
#include "../Helpers/StringConverter.h"

//#include "../../ESPEasy-Globals.h"

#ifdef ESP32
#include <soc/soc.h>
#include <soc/efuse_reg.h>
#endif

/********************************************************************************************\
 * Initialize specific hardware settings (only global ones, others are set through devices)
 \*********************************************************************************************/
void hardwareInit()
{
  // set GPIO pins state if not set to default
  bool hasPullUp, hasPullDown;

  for (int gpio = 0; gpio <= PIN_D_MAX; ++gpio) {
    const bool serialPinConflict = (Settings.UseSerial && (gpio == 1 || gpio == 3));
    if (!serialPinConflict) {
      const uint32_t key = createKey(1, gpio);
      if (getGpioPullResistor(gpio, hasPullUp, hasPullDown)) {
        const PinBootState bootState = Settings.getPinBootState(gpio);
        if (bootState != PinBootState::Default_state) {
          int8_t state = -1;
          uint8_t mode = PIN_MODE_UNDEFINED;
          int8_t init = 0;
          switch (bootState)
          {
            case PinBootState::Default_state:
              // At startup, pins are configured as INPUT
              break;
            case PinBootState::Output_low:
              pinMode(gpio, OUTPUT);
              digitalWrite(gpio, LOW);
              state = LOW;
              mode  = PIN_MODE_OUTPUT;
              init  = 1;

              // setPinState(1, gpio, PIN_MODE_OUTPUT, LOW);
              break;
            case PinBootState::Output_high:
              pinMode(gpio, OUTPUT);
              digitalWrite(gpio, HIGH);
              state = HIGH;
              mode  = PIN_MODE_OUTPUT;
              init  = 1;

              // setPinState(1, gpio, PIN_MODE_OUTPUT, HIGH);
              break;
            case PinBootState::Input_pullup:
              if (hasPullUp) {
                pinMode(gpio, INPUT_PULLUP);
                state = 0;
                mode  = PIN_MODE_INPUT_PULLUP;
                init  = 1;
              }
              break;
            case PinBootState::Input_pulldown:
              if (hasPullDown) {
                #ifdef ESP8266
                if (gpio == 16) {
                  pinMode(gpio, INPUT_PULLDOWN_16);
                }
                #endif
                #ifdef ESP32
                pinMode(gpio, INPUT_PULLDOWN);
                #endif
                state = 0;
                mode  = PIN_MODE_INPUT_PULLDOWN;
                init  = 1;
              }
              break;
            case PinBootState::Input:
              pinMode(gpio, INPUT);
              state = 0;
              mode  = PIN_MODE_INPUT;
              init  = 1;
              break;

          }
          if (init == 1) {
            globalMapPortStatus[key].state = state;
            globalMapPortStatus[key].mode  = mode;
            globalMapPortStatus[key].init  = init;
          }
        }
      }
    }
  }

  if (getGpioPullResistor(Settings.Pin_Reset, hasPullUp, hasPullDown)) {
    if (hasPullUp) {
      pinMode(Settings.Pin_Reset, INPUT_PULLUP);
    }
  }

  initI2C();

  // SPI Init
  if (Settings.InitSPI > 0)
  {
    SPI.setHwCs(false);

    // MFD: for ESP32 enable the SPI on HSPI as the default is VSPI
    #ifdef ESP32

    if (Settings.InitSPI == 2)
    {
      # define HSPI_MISO   12
      # define HSPI_MOSI   13
      # define HSPI_SCLK   14
      # define HSPI_SS     15
      SPI.begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI); // HSPI
    }
    else {
      SPI.begin();                                // VSPI
    }
    #else // ifdef ESP32
    SPI.begin();
    #endif // ifdef ESP32
    addLog(LOG_LEVEL_INFO, F("INIT : SPI Init (without CS)"));
  }
  else
  {
    addLog(LOG_LEVEL_INFO, F("INIT : SPI not enabled"));
  }

#ifdef FEATURE_SD

  if (Settings.Pin_sd_cs >= 0)
  {
    if (SD.begin(Settings.Pin_sd_cs))
    {
      addLog(LOG_LEVEL_INFO, F("SD   : Init OK"));
    }
    else
    {
      addLog(LOG_LEVEL_ERROR, F("SD   : Init failed"));
    }
  }
#endif // ifdef FEATURE_SD
}

void initI2C() {
  // configure hardware pins according to eeprom settings.
  if (Settings.Pin_i2c_sda != -1 && Settings.Pin_i2c_scl != -1)
  {
    addLog(LOG_LEVEL_INFO, F("INIT : I2C"));
    I2CSelectClockSpeed(false); // Set normal clock speed
    Wire.begin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);

    if (Settings.WireClockStretchLimit)
    {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("INIT : I2C custom clockstretchlimit:");
        log += Settings.WireClockStretchLimit;
        addLog(LOG_LEVEL_INFO, log);
      }
        #if defined(ESP8266)
      Wire.setClockStretchLimit(Settings.WireClockStretchLimit);
        #endif // if defined(ESP8266)
    }

#ifdef FEATURE_I2CMULTIPLEXER

    if (Settings.I2C_Multiplexer_ResetPin != -1) { // Initialize Reset pin to High if configured
      pinMode(Settings.I2C_Multiplexer_ResetPin, OUTPUT);
      digitalWrite(Settings.I2C_Multiplexer_ResetPin, HIGH);
    }
#endif // ifdef FEATURE_I2CMULTIPLEXER
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

void I2CSelectClockSpeed(bool setLowSpeed) {
  static uint32_t lastI2CClockSpeed = 0;
  const uint32_t newI2CClockSpeed = setLowSpeed ? Settings.I2C_clockSpeed_Slow : Settings.I2C_clockSpeed;
  if (newI2CClockSpeed == lastI2CClockSpeed) {
    // No need to change the clock speed.
    return;
  }
  lastI2CClockSpeed = newI2CClockSpeed;  
  Wire.setClock(newI2CClockSpeed);
}

#ifdef FEATURE_I2CMULTIPLEXER

// Check if the I2C Multiplexer is enabled
bool isI2CMultiplexerEnabled() {
  return Settings.I2C_Multiplexer_Type != I2C_MULTIPLEXER_NONE
         && Settings.I2C_Multiplexer_Addr != -1;
}

// Reset the I2C Multiplexer, if a pin is assigned for that. Pulled to low to force a reset.
void I2CMultiplexerReset() {
  if (Settings.I2C_Multiplexer_ResetPin != -1) {
    digitalWrite(Settings.I2C_Multiplexer_ResetPin, LOW);
    delay(1); // minimum requirement of low for a proper reset seems to be about 6 nsec, so 1 msec should be more than sufficient
    digitalWrite(Settings.I2C_Multiplexer_ResetPin, HIGH);
  }
}

// Shift the bit in the right position when selecting a single channel
byte I2CMultiplexerShiftBit(uint8_t i) {
  byte toWrite = 0;

  switch (Settings.I2C_Multiplexer_Type) {
    case I2C_MULTIPLEXER_TCA9543A: // TCA9543/6/8 addressing
    case I2C_MULTIPLEXER_TCA9546A:
    case I2C_MULTIPLEXER_TCA9548A:
      toWrite = (1 << i);
      break;
    case I2C_MULTIPLEXER_PCA9540: // PCA9540 needs bit 2 set to write the channel
      toWrite = 0b00000100;

      if (i == 1) {
        toWrite |= 0b00000010; // And bit 0 not set when selecting channel 0...
      }
      break;
  }
  return toWrite;
}

// As initially constructed by krikk in PR#254, quite adapted
// utility method for the I2C multiplexer
// select the multiplexer port given as parameter, if taskIndex < 0 then take that abs value as the port to select (to allow I2C scanner)
void I2CMultiplexerSelectByTaskIndex(taskIndex_t taskIndex) {
  if (!validTaskIndex(taskIndex)) { return; }
  if (!I2CMultiplexerPortSelectedForTask(taskIndex)) { return; }

  byte toWrite = 0;

  if (!bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_MUX_MULTICHANNEL)) {
    uint8_t i = Settings.I2C_Multiplexer_Channel[taskIndex];

    if (i > 7) { return; }
    toWrite = I2CMultiplexerShiftBit(i);
  } else {
    toWrite = Settings.I2C_Multiplexer_Channel[taskIndex]; // Bitpattern is already correctly stored
  }

  SetI2CMultiplexer(toWrite);
}

void I2CMultiplexerSelect(uint8_t i) {
  if (i > 7) { return; }

  byte toWrite = I2CMultiplexerShiftBit(i);
  SetI2CMultiplexer(toWrite);
}

void I2CMultiplexerOff() {
  SetI2CMultiplexer(0); // no channel selected
}

void SetI2CMultiplexer(byte toWrite) {
  if (isI2CMultiplexerEnabled()) {
    // FIXME TD-er: Must check to see if we can cache the value so only change it when needed.
    Wire.beginTransmission(Settings.I2C_Multiplexer_Addr);
    Wire.write(toWrite);
    Wire.endTransmission();
    // FIXME TD-er: We must check if the chip needs some time to set the output. (delay?)
  }
}

byte I2CMultiplexerMaxChannels() {
  uint channels = 0;

  switch (Settings.I2C_Multiplexer_Type) {
    case I2C_MULTIPLEXER_TCA9548A:  channels = 8; break; // TCA9548A has 8 channels
    case I2C_MULTIPLEXER_TCA9546A:  channels = 4; break; // TCA9546A has 4 channels
    case I2C_MULTIPLEXER_PCA9540:   channels = 2; break; // PCA9540 has 2 channels
    case I2C_MULTIPLEXER_TCA9543A:  channels = 2; break; // TCA9543A has 2 channels
  }
  return channels;
}

// Has this taskIndex a channel selected? Checks for both Single channel and Multiple channel mode
// taskIndex must already be validated! (0..MAX_TASKS)
bool I2CMultiplexerPortSelectedForTask(taskIndex_t taskIndex) {
  if (!validTaskIndex(taskIndex)) { return false; }
  if (!isI2CMultiplexerEnabled()) { return false; }
  return (!bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_MUX_MULTICHANNEL) && Settings.I2C_Multiplexer_Channel[taskIndex] != -1)
         || (bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_MUX_MULTICHANNEL) && Settings.I2C_Multiplexer_Channel[taskIndex] !=  0);
}

#endif // ifdef FEATURE_I2CMULTIPLEXER

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
      reboot(ESPEasy_Scheduler::IntendedRebootReason_e::ResetFactoryPinActive);
    }
    factoryResetCounter = 0; // count was < 3, reset counter
  }
}

#ifdef ESP8266
int lastADCvalue = 0;

int espeasy_analogRead(int pin) {
  if (!WiFiEventData.wifiConnectInProgress) {
    lastADCvalue = analogRead(A0);
  }
  return lastADCvalue;
}

#endif // ifdef ESP8266

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
          // See:
          // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html#configuration-and-reading-adc
          // ADC2 is shared with WiFi, so don't read ADC2 when WiFi is on.
          canread = true;
        }
        break;
    }

    if (canread) {
      if (readAsTouch && (t >= 0)) {
        value = touchRead(pin);
      } else {
        value = analogRead(pin);
      }
    }
  }
  return value;
}

#endif // ifdef ESP32


/********************************************************************************************\
   Hardware information
 \*********************************************************************************************/
uint32_t getFlashChipId() {
  uint32_t flashChipId = 0;
  #ifdef ESP32
  //esp_flash_read_id(nullptr, &flashChipId);
  #elif defined(ESP8266)
  flashChipId = ESP.getFlashChipId();
  #endif
  return flashChipId;
}

uint32_t getFlashRealSizeInBytes() {
  #if defined(ESP32)
  return ESP.getFlashChipSize();
  #else // if defined(ESP32)
  return ESP.getFlashChipRealSize(); // ESP.getFlashChipSize();
  #endif // if defined(ESP32)
}


bool puyaSupport() {
  bool supported = false;

#ifdef PUYA_SUPPORT

  // New support starting core 2.5.0
  if (PUYA_SUPPORT) { supported = true; }
#endif // ifdef PUYA_SUPPORT
#ifdef PUYASUPPORT

  // Old patch
  supported = true;
#endif // ifdef PUYASUPPORT
  return supported;
}

uint8_t getFlashChipVendorId() {
#ifdef PUYA_SUPPORT
  return ESP.getFlashChipVendorId();
#else // ifdef PUYA_SUPPORT
  # if defined(ESP8266)
    uint32_t flashChipId = ESP.getFlashChipId();
    return flashChipId & 0x000000ff;
  # elif defined(ESP32)
  
  # endif // if defined(ESP8266)
#endif // ifdef PUYA_SUPPORT
  return 0xFF; // Not an existing function for ESP32
}

bool flashChipVendorPuya() {
  uint8_t vendorId = getFlashChipVendorId();

  return vendorId == 0x85; // 0x146085 PUYA
}

uint32_t getChipId() {
  uint32_t chipId = 0;

#ifdef ESP8266
  chipId = ESP.getChipId();
#endif
#ifdef ESP32
  for(int i=0; i<17; i=i+8) {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
#endif

  return chipId;
}

uint8_t getChipCores() {
  uint8_t cores = 1;
  #ifdef ESP32
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cores = chip_info.cores;
  #endif
  return cores;
}

String getChipModel() {
#ifdef ESP32
  {
    uint32_t chip_ver = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
    uint32_t pkg_ver = chip_ver & 0x7;
    switch (pkg_ver) {
      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ6 :
        return F("ESP32-D0WDQ6");
      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ5 :
        return F("ESP32-D0WDQ5");
      case EFUSE_RD_CHIP_VER_PKG_ESP32D2WDQ5 :
        return F("ESP32-D2WDQ5");
      case EFUSE_RD_CHIP_VER_PKG_ESP32PICOD2 :
        return F("ESP32-PICO-D2");
      case EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4 :
        return F("ESP32-PICO-D4");
      default:
        break;
    }
  }
#elif defined(ESP8285)
  return F("ESP8285");
#elif defined(ESP8266)
  return F("ESP8266");
#endif
  return F("Unknown");
}

uint8_t getChipRevision() {
  uint8_t rev = 0;
  #ifdef ESP32
    rev = ESP.getChipRevision();
  #endif
  return rev;
}

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
    case DeviceMode_Olimex_ESP32_PoE:
    case DeviceMode_Olimex_ESP32_EVB:
    case DeviceMode_Olimex_ESP32_GATEWAY:  
    #ifdef ESP32
      return F("Olimex");
    #endif
    case DeviceMode_wESP32:
    #ifdef ESP32
      return F("wESP32");
    #endif
    case DeviceMode_WT32_ETH01:
    #ifdef ESP32
      return F("WT32-ETH01");
    #endif
    case DeviceModel_default:
    case DeviceModel_MAX:      break;

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return "";
}

String getDeviceModelString(DeviceModel model) {
  String result;

  result.reserve(16);
  result = getDeviceModelBrandString(model);

  switch (model) {
#if defined(ESP8266) && !defined(LIMIT_BUILD_SIZE)
    case DeviceModel_Sonoff_Basic:   result      += F(" Basic");   break;
    case DeviceModel_Sonoff_TH1x:    result      += F(" TH1x");    break;
    case DeviceModel_Sonoff_S2x:     result      += F(" S2x");     break;
    case DeviceModel_Sonoff_TouchT1: result      += F(" TouchT1"); break;
    case DeviceModel_Sonoff_TouchT2: result      += F(" TouchT2"); break;
    case DeviceModel_Sonoff_TouchT3: result      += F(" TouchT3"); break;
    case DeviceModel_Sonoff_4ch:     result      += F(" 4ch");     break;
    case DeviceModel_Sonoff_POW:     result      += F(" POW");     break;
    case DeviceModel_Sonoff_POWr2:   result      += F(" POW-r2");  break;
    case DeviceModel_Shelly1:        result      += '1';           break;
    case DeviceModel_ShellyPLUG_S:   result      += F(" PLUG S");  break;
#else
    case DeviceModel_Sonoff_Basic:
    case DeviceModel_Sonoff_TH1x:
    case DeviceModel_Sonoff_S2x:
    case DeviceModel_Sonoff_TouchT1:
    case DeviceModel_Sonoff_TouchT2:
    case DeviceModel_Sonoff_TouchT3:
    case DeviceModel_Sonoff_4ch:
    case DeviceModel_Sonoff_POW:
    case DeviceModel_Sonoff_POWr2:
    case DeviceModel_Shelly1:
    case DeviceModel_ShellyPLUG_S:
      result += F("default");  break;
#endif
#ifdef ESP32
    case DeviceMode_Olimex_ESP32_PoE: result     += F(" ESP32-PoE"); break;
    case DeviceMode_Olimex_ESP32_EVB: result     += F(" ESP32-EVB"); break;
    case DeviceMode_Olimex_ESP32_GATEWAY: result += F(" ESP32-GATEWAY"); break;
    case DeviceMode_wESP32:           break;
    case DeviceMode_WT32_ETH01:           result += F(" add-on"); break;
#else
    case DeviceMode_Olimex_ESP32_PoE:
    case DeviceMode_Olimex_ESP32_EVB:
    case DeviceMode_Olimex_ESP32_GATEWAY:
    case DeviceMode_wESP32:
    case DeviceMode_WT32_ETH01:
#endif

    case DeviceModel_default:
    case DeviceModel_MAX:            result += F("default");  break;

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return result;
}

bool modelMatchingFlashSize(DeviceModel model) {
  uint32_t size_MB = getFlashRealSizeInBytes() >> 20;

  // TD-er: This also checks for ESP8266/ESP8285/ESP32
  switch (model) {
    case DeviceModel_Sonoff_Basic:
    case DeviceModel_Sonoff_TH1x:
    case DeviceModel_Sonoff_S2x:
    case DeviceModel_Sonoff_TouchT1:
    case DeviceModel_Sonoff_TouchT2:
    case DeviceModel_Sonoff_TouchT3:
    case DeviceModel_Sonoff_4ch:
#ifdef ESP8266    
      return size_MB == 1;
#else
      return false;
#endif

    case DeviceModel_Sonoff_POW:
    case DeviceModel_Sonoff_POWr2:   
#ifdef ESP8266    
      return size_MB == 4;
#else
      return false;
#endif

    case DeviceModel_Shelly1:
    case DeviceModel_ShellyPLUG_S:
#ifdef ESP8266    
      return size_MB == 2;
#else
      return false;
#endif

    // These Olimex boards all have Ethernet
    case DeviceMode_Olimex_ESP32_PoE:
    case DeviceMode_Olimex_ESP32_EVB:
    case DeviceMode_Olimex_ESP32_GATEWAY:
    case DeviceMode_wESP32:
    case DeviceMode_WT32_ETH01:
#if  defined(ESP32) && defined(HAS_ETHERNET)
      return size_MB == 4;
#else
      return false;
#endif

    case DeviceModel_default:
    case DeviceModel_MAX:
      return true;

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return true;
}

void setFactoryDefault(DeviceModel model) {
  ResetFactoryDefaultPreference.setDeviceModel(model);
}

/********************************************************************************************\
   Add pre defined plugins and rules.
 \*********************************************************************************************/
void addSwitchPlugin(taskIndex_t taskIndex, int gpio, const String& name, bool activeLow) {
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

void addButtonRelayRule(byte buttonNumber, int relay_gpio) {
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

  # ifdef HAS_ETHERNET

  // Check pins used for RMII Ethernet PHY
  if (NetworkMedium_t::Ethernet == Settings.NetworkMedium) {
    switch (gpio) {
      case 0:
      case 21:
      case 19:
      case 22:
      case 25:
      case 26:
      case 27:
        warning = true;
        break;
    }

    // FIXME TD-er: Must we also check for pins used for MDC/MDIO and Eth PHY power?
  }


  # endif // ifdef HAS_ETHERNET
  return true;
}

bool getGpioPullResistor(int gpio, bool& hasPullUp, bool& hasPullDown) {
  hasPullDown = false;
  hasPullUp = false;

  int pinnr;
  bool input;
  bool output;
  bool warning;
  if (!getGpioInfo(gpio, pinnr, input, output, warning)) {
    return false;
  }
  if (gpio >= 34) {
    // For GPIO 34 .. 39, no pull-up nor pull-down.
  } else if (gpio == 12) {
    // No Pull-up on GPIO12
    // compatible with the SDIO protocol.
    // Just connect GPIO12 to VDD via a 10 kOhm resistor.
  } else {
    hasPullUp = true;
    hasPullDown = true;
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
  if (isFlashInterfacePin(gpio)) {
    #ifdef ESP8285
    
    if ((gpio == 9) || (gpio == 10)) {
      // Usable on ESP8285
    } else {
      warning = true;
    }

    #else

    warning = true;
    // On ESP8266 GPIO 9 & 10 are only usable if not connected to flash 
    if (gpio == 9) {
      // GPIO9 is internally used to control the flash memory.
      input  = false;
      output = false;
    } else if (gpio == 10) {
      // GPIO10 can be used as input only.
      output = false;
    }

    #endif
  }

  if (pinnr < 0 || pinnr > 16) {
    input  = false;
    output = false;
    return false;
  }
  return true;
}

bool getGpioPullResistor(int gpio, bool& hasPullUp, bool& hasPullDown) {
  hasPullDown = false;
  hasPullUp = false;

  int pinnr;
  bool input;
  bool output;
  bool warning;
  if (!getGpioInfo(gpio, pinnr, input, output, warning)) {
    return false;
  }
  if (gpio == 16) {
    hasPullDown = true;
  } else {
    hasPullUp = true;
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
  switch (touch_pin) {
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

#endif // ifdef ESP32

// ********************************************************************************
// Manage PWM state of GPIO pins.
// ********************************************************************************
void initAnalogWrite()
{
  #if defined(ESP32)
  for(byte x = 0; x < 16; x++) {
    ledcSetup(x, 0, 10); // Clear the channel
    ledChannelPin[x] = -1;
    ledChannelFreq[x] = 0;
  }
  #endif
  #ifdef ESP8266
  // See https://github.com/esp8266/Arduino/commit/a67986915512c5304bd7c161cf0d9c65f66e0892
  analogWriteRange(1023);
  #endif
}

#if defined(ESP32)
int8_t ledChannelPin[16];
uint32_t ledChannelFreq[16];


int8_t attachLedChannel(int pin, uint32_t frequency)
{
  // find existing channel if this pin has been used before
  int8_t ledChannel = -1;
  bool mustSetup = false;
  for (byte x = 0; x < 16; x++) {
    if (ledChannelPin[x] == pin) {
      ledChannel = x;
    }
  }

  if (ledChannel == -1)             // no channel set for this pin
  {
    for (byte x = 0; x < 16; x++) { // find free channel
      if (ledChannelPin[x] == -1)
      {
        if (!ledcRead(x)) {
          // Channel is not used by some other piece of code.
          ledChannel = x;
          mustSetup = true;
          break;
        }
      }
    }
  }
  if (ledChannel == -1) return ledChannel;
  if (frequency != 0) {
    if (ledChannelFreq[ledChannel] != frequency)
    {
      // Frequency is given and has changed
      mustSetup = true;
    }
    ledChannelFreq[ledChannel] = frequency;
  } else if (ledChannelFreq[ledChannel] == 0) {
    mustSetup = true;
    // Set some default frequency
    ledChannelFreq[ledChannel] = 1000;
  }

  if (mustSetup) {
    // setup channel to 10 bit and set frequency.
    ledChannelFreq[ledChannel] = ledcSetup(ledChannel, ledChannelFreq[ledChannel], 10);
    ledChannelPin[ledChannel] = pin; // store pin nr
    ledcAttachPin(pin, ledChannel);  // attach to this pin
  }

  return ledChannel;
}

void detachLedChannel(int pin)
{
  int8_t ledChannel = -1;

  for (byte x = 0; x < 16; x++) {
    if (ledChannelPin[x] == pin) {
      ledChannel = x;
    }
  }

  if (ledChannel != -1) {
    ledcWrite(ledChannel, 0);
    ledcDetachPin(pin);
    ledChannelPin[ledChannel] = -1;
    ledChannelFreq[ledChannel] = 0;
  }
}


uint32_t analogWriteESP32(int pin, int value, uint32_t frequency)
{
  if (value == 0) {
    detachLedChannel(pin);
    return 0;
  }

  // find existing channel if this pin has been used before
  int8_t ledChannel = attachLedChannel(pin, frequency);

  if (ledChannel != -1) {
    ledcWrite(ledChannel, value);
    return ledChannelFreq[ledChannel];
  }
  return 0;
}

#endif // if defined(ESP32)

bool set_Gpio_PWM_pct(int gpio, float dutyCycle_f, uint32_t frequency) {
  uint32_t dutyCycle = dutyCycle_f * 10.23f;
  return set_Gpio_PWM(gpio, dutyCycle, frequency);
}

bool set_Gpio_PWM(int gpio, uint32_t dutyCycle, uint32_t frequency) {
  uint32_t key;
  return set_Gpio_PWM(gpio, dutyCycle, 0, frequency, key);
}

bool set_Gpio_PWM(int gpio, uint32_t dutyCycle, uint32_t fadeDuration_ms, uint32_t& frequency, uint32_t& key)
{
  // For now, we only support the internal GPIO pins.
  byte   pluginID  = PLUGIN_GPIO;
  if (!checkValidPortRange(pluginID, gpio)) {
    return false;
  }
  portStatusStruct tempStatus;

  // FIXME TD-er: PWM values cannot be stored very well in the portStatusStruct.
  key = createKey(pluginID, gpio);

  // WARNING: operator [] creates an entry in the map if key does not exist
  // So the next command should be part of each command:
  tempStatus = globalMapPortStatus[key];

        #if defined(ESP8266)
  pinMode(gpio, OUTPUT);
        #endif // if defined(ESP8266)

  if ((frequency > 0) && (frequency <= 40000)) {
        #if defined(ESP8266)
    analogWriteFreq(frequency);
        #endif // if defined(ESP8266)
  }

  if (fadeDuration_ms != 0)
  {
    const int32_t resolution_factor = (1 << 12);
    const byte prev_mode  = tempStatus.mode;
    int32_t   prev_value = tempStatus.getDutyCycle();

    // getPinState(pluginID, gpio, &prev_mode, &prev_value);
    if (prev_mode != PIN_MODE_PWM) {
      prev_value = 0;
    }

    const int32_t step_value = ((static_cast<int32_t>(dutyCycle) - prev_value) * resolution_factor) / static_cast<int32_t>(fadeDuration_ms);
    int32_t curr_value = prev_value * resolution_factor;

    int i = fadeDuration_ms;

    while (i--) {
      curr_value += step_value;
      const int16_t new_value = curr_value / resolution_factor;
            #if defined(ESP8266)
      analogWrite(gpio, new_value);
            #endif // if defined(ESP8266)
            #if defined(ESP32)
      analogWriteESP32(gpio, new_value);
            #endif // if defined(ESP32)
      delay(1);
    }
  }

        #if defined(ESP8266)
  analogWrite(gpio, dutyCycle);
        #endif // if defined(ESP8266)
        #if defined(ESP32)
  frequency = analogWriteESP32(gpio, dutyCycle, frequency);
        #endif // if defined(ESP32)

  // setPinState(pluginID, gpio, PIN_MODE_PWM, dutyCycle);
  tempStatus.mode      = PIN_MODE_PWM;
  tempStatus.dutyCycle = dutyCycle;
  tempStatus.command   = 1; // set to 1 in order to display the status in the PinStatus page

  savePortStatus(key, tempStatus);
  return true;
}


// ********************************************************************************
// change of device: cleanup old device and reset default settings
// ********************************************************************************
void setTaskDevice_to_TaskIndex(pluginID_t taskdevicenumber, taskIndex_t taskIndex) {
  struct EventStruct TempEvent(taskIndex);
  String dummy;

  // let the plugin do its cleanup by calling PLUGIN_EXIT with this TaskIndex
  PluginCall(PLUGIN_EXIT, &TempEvent, dummy);
  taskClear(taskIndex, false); // clear settings, but do not save
  ClearCustomTaskSettings(taskIndex);

  Settings.TaskDeviceNumber[taskIndex] = taskdevicenumber;
  if (validPluginID_fullcheck(taskdevicenumber)) // set default values if a new device has been selected
  {
    // FIXME TD-er: Must check if this is working (e.g. need to set nr. decimals?)
    ExtraTaskSettings.clear();
    ExtraTaskSettings.TaskIndex = taskIndex;

    // NOTE: do not enable task by default. allow user to enter sensible valus first and let him enable it when ready.
    PluginCall(PLUGIN_SET_DEFAULTS,         &TempEvent, dummy);
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummy); // the plugin should populate ExtraTaskSettings with its default values.
  } else {
    // New task is empty task, thus save config now.
    taskClear(taskIndex, true);                                 // clear settings, and save
  }
}

// ********************************************************************************
// Initialize task with some default values applicable for almost all tasks
// ********************************************************************************
void setBasicTaskValues(taskIndex_t taskIndex, unsigned long taskdevicetimer,
                        bool enabled, const String& name, int pin1, int pin2, int pin3) {
  if (!validTaskIndex(taskIndex)) { return; }
  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);

  if (!validDeviceIndex(DeviceIndex)) { return; }

  LoadTaskSettings(taskIndex); // Make sure ExtraTaskSettings are up-to-date

  if (taskdevicetimer > 0) {
    Settings.TaskDeviceTimer[taskIndex] = taskdevicetimer;
  } else {
    if (!Device[DeviceIndex].TimerOptional) { // Set default delay, unless it's optional...
      Settings.TaskDeviceTimer[taskIndex] = Settings.Delay;
    }
    else {
      Settings.TaskDeviceTimer[taskIndex] = 0;
    }
  }
  Settings.TaskDeviceEnabled[taskIndex] = enabled;
  safe_strncpy(ExtraTaskSettings.TaskDeviceName, name.c_str(), sizeof(ExtraTaskSettings.TaskDeviceName));

  // FIXME TD-er: Check for valid GPIO pin (and  -1 for "not set")
  Settings.TaskDevicePin1[taskIndex] = pin1;
  Settings.TaskDevicePin2[taskIndex] = pin2;
  Settings.TaskDevicePin3[taskIndex] = pin3;
  SaveTaskSettings(taskIndex);
}

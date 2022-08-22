#include "../Helpers/Hardware.h"

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataTypes/SPI_options.h"
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


// #include "../../ESPEasy-Globals.h"

#ifdef ESP32
  # include <soc/soc.h>
  # include <soc/efuse_reg.h>
  # include <soc/spi_reg.h>
  # include <soc/rtc.h>

  # if ESP_IDF_VERSION_MAJOR > 3      // IDF 4+
    #  if CONFIG_IDF_TARGET_ESP32     // ESP32/PICO-D4
      #   include <esp32/rom/spi_flash.h>
      #   include <esp32/spiram.h>
    #  elif CONFIG_IDF_TARGET_ESP32S2 // ESP32-S2
      #   include <esp32s2/rom/spi_flash.h>
      #   include <esp32s2/spiram.h>
    #  elif CONFIG_IDF_TARGET_ESP32C3 // ESP32-C3
      #   include <esp32c3/rom/spi_flash.h>
      #   include <esp32c3/spiram.h>
    #  else // if CONFIG_IDF_TARGET_ESP32
      #   error Target CONFIG_IDF_TARGET is not supported
    #  endif // if CONFIG_IDF_TARGET_ESP32
  # else // ESP32 Before IDF 4.0
    #  include <rom/spi_flash.h>
  # endif    // if ESP_IDF_VERSION_MAJOR > 3

#endif       // ifdef ESP32


#if FEATURE_SD
# include <SD.h>
#endif // if FEATURE_SD


#include <SPI.h>
#include <Wire.h>

/********************************************************************************************\
 * Initialize specific hardware settings (only global ones, others are set through devices)
 \*********************************************************************************************/
void hardwareInit()
{
  // set GPIO pins state if not set to default
  bool hasPullUp, hasPullDown;

  for (int gpio = 0; gpio <= MAX_GPIO; ++gpio) {
    const bool serialPinConflict = (Settings.UseSerial && (gpio == 1 || gpio == 3));

    if (!serialPinConflict) {
      const uint32_t key = createKey(1, gpio);
      #ifdef ESP32
      checkAndClearPWM(key);
      #endif // ifdef ESP32

      if (getGpioPullResistor(gpio, hasPullUp, hasPullDown)) {
        PinBootState bootState = Settings.getPinBootState(gpio);
      #if FEATURE_ETHERNET

        if (Settings.ETH_Pin_power == gpio)
        {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("ETH  : Reset ETH module on pin ");
            log += Settings.ETH_Pin_power;
            addLog(LOG_LEVEL_INFO, log);
          }
          bootState = PinBootState::Output_low;
        }

      #endif // if FEATURE_ETHERNET

        if (bootState != PinBootState::Default_state) {
          int8_t  state = -1;
          uint8_t mode  = PIN_MODE_UNDEFINED;
          int8_t  init  = 0;

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
                #endif // ifdef ESP8266
                #ifdef ESP32
                pinMode(gpio, INPUT_PULLDOWN);
                #endif // ifdef ESP32
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
  if (Settings.isSPI_valid())
  {
    SPI.setHwCs(false);

    // MFD: for ESP32 enable the SPI on HSPI as the default is VSPI
    #ifdef ESP32

    const SPI_Options_e SPI_selection = static_cast<SPI_Options_e>(Settings.InitSPI);

    switch (SPI_selection) {
      case SPI_Options_e::Hspi:
      {
        # define HSPI_MISO   12
        # define HSPI_MOSI   13
        # define HSPI_SCLK   14
        # define HSPI_SS     15
        SPI.begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI); // HSPI
        break;
      }
      case SPI_Options_e::UserDefined:
      {
        SPI.begin(Settings.SPI_SCLK_pin,
                  Settings.SPI_MISO_pin,
                  Settings.SPI_MOSI_pin); // User-defined SPI
        break;
      }
      case SPI_Options_e::Vspi:
      {
        SPI.begin(); // VSPI
        break;
      }
      case SPI_Options_e::None:
        break;
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

#if FEATURE_SD

  if (Settings.Pin_sd_cs >= 0)
  {
    if (SD.begin(Settings.Pin_sd_cs))
    {
      addLog(LOG_LEVEL_INFO, F("SD   : Init OK"));
    }
    else
    {
      SD.end();
      addLog(LOG_LEVEL_ERROR, F("SD   : Init failed"));
    }
  }
#endif // if FEATURE_SD
}

void initI2C() {
  // configure hardware pins according to eeprom settings.
  if (!Settings.isI2CEnabled())
  {
    return;
  }
  addLog(LOG_LEVEL_INFO, F("INIT : I2C"));
  I2CSelectHighClockSpeed(); // Set normal clock speed

  if (Settings.WireClockStretchLimit)
  {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("INIT : I2C custom clockstretchlimit:");
      log += Settings.WireClockStretchLimit;
      addLogMove(LOG_LEVEL_INFO, log);
    }
      #if defined(ESP8266)
    Wire.setClockStretchLimit(Settings.WireClockStretchLimit);
      #endif // if defined(ESP8266)
      #ifdef ESP32
    Wire.setTimeOut(Settings.WireClockStretchLimit);
      #endif // ifdef ESP32S2
  }

  #if FEATURE_I2CMULTIPLEXER

  if (validGpio(Settings.I2C_Multiplexer_ResetPin)) { // Initialize Reset pin to High if configured
    pinMode(Settings.I2C_Multiplexer_ResetPin, OUTPUT);
    digitalWrite(Settings.I2C_Multiplexer_ResetPin, HIGH);
  }
  #endif // if FEATURE_I2CMULTIPLEXER

  // I2C Watchdog boot status check
  if (Settings.WDI2CAddress != 0)
  {
    delay(500);
    Wire.beginTransmission(Settings.WDI2CAddress);
    Wire.write(0x83); // command to set pointer
    Wire.write(17);   // pointer value to status uint8_t
    Wire.endTransmission();

    Wire.requestFrom(Settings.WDI2CAddress, (uint8_t)1);

    if (Wire.available())
    {
      uint8_t status = Wire.read();

      if (status & 0x1)
      {
        addLog(LOG_LEVEL_ERROR, F("INIT : Reset by WD!"));
        lastBootCause = BOOT_CAUSE_EXT_WD;
      }
    }
  }
}

void I2CSelectHighClockSpeed() {
  I2CSelectClockSpeed(Settings.I2C_clockSpeed);
}

void I2CSelectLowClockSpeed() {
  I2CSelectClockSpeed(Settings.I2C_clockSpeed_Slow);
}

void I2CSelect_Max100kHz_ClockSpeed() {
  if (Settings.I2C_clockSpeed <= 100000) {
    I2CSelectHighClockSpeed();
  } else if (Settings.I2C_clockSpeed_Slow <= 100000) {
    I2CSelectLowClockSpeed();
  } else {
    I2CSelectClockSpeed(100000);
  }
}

void I2CSelectClockSpeed(uint32_t clockFreq) {
  I2CBegin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl, clockFreq);
}

void I2CForceResetBus_swap_pins(uint8_t address) {
  if (!Settings.EnableClearHangingI2Cbus()) { return; }

  // As a final work-around, we temporary swap SDA and SCL, perform a scan and return pin order.
  I2CBegin(Settings.Pin_i2c_scl, Settings.Pin_i2c_sda, 100000);
  Wire.beginTransmission(address);
  Wire.endTransmission();
  delay(1);

  // Now we switch back to the correct pins
  I2CSelectClockSpeed(100000);
}

void I2CBegin(int8_t sda, int8_t scl, uint32_t clockFreq) {
  #ifdef ESP32
  uint32_t lastI2CClockSpeed = Wire.getClock();
  #else // ifdef ESP32
  static uint32_t lastI2CClockSpeed = 0;
  #endif // ifdef ESP32
  static int8_t last_sda = -1;
  static int8_t last_scl = -1;

  if ((clockFreq == lastI2CClockSpeed) && (sda == last_sda) && (scl == last_scl)) {
    // No need to change the clock speed.
    return;
  }
  #ifdef ESP32

  if ((sda != last_sda) || (scl != last_scl)) {
    Wire.end();
  }
  #endif // ifdef ESP32
  lastI2CClockSpeed = clockFreq;
  last_scl          = scl;
  last_sda          = sda;

  #ifdef ESP32
  Wire.begin(sda, scl, clockFreq); // Will only set the clock when not yet initialized.
  Wire.setClock(clockFreq);
  #else // ifdef ESP32
  Wire.begin(sda, scl);
  Wire.setClock(clockFreq);
  #endif // ifdef ESP32
}

#if FEATURE_I2CMULTIPLEXER

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
uint8_t I2CMultiplexerShiftBit(uint8_t i) {
  uint8_t toWrite = 0;

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

  uint8_t toWrite = 0;

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

  uint8_t toWrite = I2CMultiplexerShiftBit(i);

  SetI2CMultiplexer(toWrite);
}

void I2CMultiplexerOff() {
  SetI2CMultiplexer(0); // no channel selected
}

void SetI2CMultiplexer(uint8_t toWrite) {
  if (isI2CMultiplexerEnabled()) {
    // FIXME TD-er: Must check to see if we can cache the value so only change it when needed.
    Wire.beginTransmission(Settings.I2C_Multiplexer_Addr);
    Wire.write(toWrite);
    Wire.endTransmission();

    // FIXME TD-er: We must check if the chip needs some time to set the output. (delay?)
  }
}

uint8_t I2CMultiplexerMaxChannels() {
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

#endif // if FEATURE_I2CMULTIPLEXER

void checkResetFactoryPin() {
  static uint8_t factoryResetCounter = 0;

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

// ESP32 ADC calibration datatypes.
esp_adc_cal_value_t adc1_calibration_type = ESP_ADC_CAL_VAL_NOT_SUPPORTED;
esp_adc_cal_characteristics_t adc_chars[ADC_ATTEN_MAX];

void initADC() {
  # ifndef DEFAULT_VREF
  #  define DEFAULT_VREF 1100
  # endif // ifndef DEFAULT_VREF
  const adc_bits_width_t adc_bit_width = static_cast<adc_bits_width_t>(ADC_WIDTH_MAX - 1);

  for (size_t atten = 0; atten < ADC_ATTEN_MAX; ++atten) {
    adc1_calibration_type =
      esp_adc_cal_characterize(ADC_UNIT_1, static_cast<adc_atten_t>(atten), adc_bit_width, DEFAULT_VREF, &adc_chars[atten]);
  }
}

bool hasADC_factory_calibration() {
  return esp_adc_cal_check_efuse(adc1_calibration_type) == ESP_OK;
}

const __FlashStringHelper* getADC_factory_calibration_type() {
  switch (adc1_calibration_type) {
    case ESP_ADC_CAL_VAL_EFUSE_VREF:   return F("V_ref in eFuse");
    case ESP_ADC_CAL_VAL_EFUSE_TP:     return F("Two Point values in eFuse");
    case ESP_ADC_CAL_VAL_DEFAULT_VREF: return F("Default reference voltage");
    case ESP_ADC_CAL_VAL_EFUSE_TP_FIT: return F("Two Point values and fitting curve in eFuse");
    case ESP_ADC_CAL_VAL_NOT_SUPPORTED:
      break;
  }
  return F("Unknown");
}

int getADC_num_for_gpio(int pin) {
  int adc, ch, t;

  if (getADC_gpio_info(pin, adc, ch, t)) {
    return adc;
  }
  return -1;
}

int espeasy_analogRead(int pin, bool readAsTouch) {
  int value = 0;
  int adc, ch, t;

  if (getADC_gpio_info(pin, adc, ch, t)) {
    bool canread = false;

    switch (adc) {
      case 0:
      # ifndef ESP32S2
        value = hallRead();
      # endif // ifndef ESP32S2
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
  // Cache since size does not change
  static uint32_t flashChipId = 0;

  if (flashChipId == 0) {
  #ifdef ESP32
    uint32_t tmp = g_rom_flashchip.device_id;

    for (int i = 0; i < 3; ++i) {
      flashChipId  = flashChipId << 8;
      flashChipId |= (tmp & 0xFF);
      tmp          = tmp >> 8;
    }

    //    esp_flash_read_id(nullptr, &flashChipId);
  #elif defined(ESP8266)
    flashChipId = ESP.getFlashChipId();
  #endif // ifdef ESP32
  }
  return flashChipId;
}

uint32_t getFlashRealSizeInBytes() {
  // Cache since size does not change
  static uint32_t res = 0;

  if (res == 0) {
    #if defined(ESP32)
    res = (1 << ((getFlashChipId() >> 16) & 0xFF));
    #else // if defined(ESP32)
    res = ESP.getFlashChipRealSize(); // ESP.getFlashChipSize();
    #endif // if defined(ESP32)
  }
  return res;
}

#ifdef ESP32
uint32_t getXtalFrequencyMHz() {
  return rtc_clk_xtal_freq_get();
}
#endif // ifdef ESP32


uint32_t getFlashChipSpeed() {
  #ifdef ESP8266
  return ESP.getFlashChipSpeed();
  #else // ifdef ESP8266
  const uint32_t spi_clock = REG_READ(SPI_CLOCK_REG(1));

  if (spi_clock & BIT(31)) {
    // spi_clk is equal to system clock
    return getApbFrequency();
  }
  return spiClockDivToFrequency(spi_clock);
  #endif // ifdef ESP8266
}

const __FlashStringHelper* getFlashChipMode() {
#ifdef ESP8266

  switch (ESP.getFlashChipMode()) {
    case FM_QIO:   return F("QIO");
    case FM_QOUT:  return F("QOUT");
    case FM_DIO:   return F("DIO");
    case FM_DOUT:  return F("DOUT");
  }
  return F("Unknown");
#else // ifdef ESP8266

  // Source: https://github.com/letscontrolit/ESPEasy/pull/4200#issuecomment-1221607332
  // + discussion: https://github.com/espressif/arduino-esp32/issues/7140#issuecomment-1222274417
  const uint32_t spi_ctrl = REG_READ(PERIPHS_SPI_FLASH_CTRL);

  # if ESP_IDF_VERSION_MAJOR > 3      // IDF 4+
    #  if CONFIG_IDF_TARGET_ESP32     // ESP32/PICO-D4
      if (spi_ctrl & SPI_FREAD_QIO) {  
        return F("QIO");
      } else if (spi_ctrl & SPI_FREAD_QUAD) { 
        return F("QOUT");
      } else if (spi_ctrl & SPI_FREAD_DIO) {
        return F("DIO");
      } else if (spi_ctrl & SPI_FREAD_DUAL) {
        return F("DOUT");
      } else if (spi_ctrl & SPI_FASTRD_MODE) {
        return F("Fast");
      }
      return F("Slow");
    #  elif CONFIG_IDF_TARGET_ESP32S2 // ESP32-S2
      if (spi_ctrl & SPI_FREAD_OCT) {  
        return F("OCT");
      } else if (spi_ctrl & SPI_FREAD_QUAD) { 
        return F("QIO");
      } else if (spi_ctrl & SPI_FREAD_DUAL) {
        return F("DIO");
      }
      return F("DOUT");
    #  elif CONFIG_IDF_TARGET_ESP32C3 // ESP32-C3
      if (spi_ctrl & SPI_FREAD_QUAD) { 
        return F("QIO");
      } else if (spi_ctrl & SPI_FREAD_DUAL) {
        return F("DIO");
      }
      return F("DOUT");
    #  endif // if CONFIG_IDF_TARGET_ESP32
  # else // ESP32 Before IDF 4.0
    if (spi_ctrl & (BIT(24))) {  
      return F("QIO");
    } else if (spi_ctrl & (BIT(20))) { 
      return F("QOUT");
    } else if (spi_ctrl & (BIT(23))) {
      return F("DIO");
    } else if (spi_ctrl & (BIT(14))) {
      return F("DOUT");
    } else if (spi_ctrl & (BIT(13))) {
      return F("Fast");
    }
    return F("Slow");
  # endif    // if ESP_IDF_VERSION_MAJOR > 3
#endif // ifdef ESP8266
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

  // Cache since size does not change
  static uint32_t flashChipId = ESP.getFlashChipId();
  return flashChipId & 0x000000ff;
  # elif defined(ESP32)
  return 0xFF; // Not an existing function for ESP32
  # endif // if defined(ESP8266)
#endif // ifdef PUYA_SUPPORT
}

bool flashChipVendorPuya() {
  const uint8_t vendorId = getFlashChipVendorId();

  return vendorId == 0x85; // 0x146085 PUYA
}

uint32_t getChipId() {
  uint32_t chipId = 0;

#ifdef ESP8266
  chipId = ESP.getChipId();
#endif // ifdef ESP8266
#ifdef ESP32

  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
#endif // ifdef ESP32

  return chipId;
}

uint8_t getChipCores() {
  #ifdef ESP8266
  return 1;
  #else // ifdef ESP8266
  static uint8_t cores = 0;

  if (cores == 0) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cores = chip_info.cores;
  }
  return cores;
  #endif // ifdef ESP8266
}

const __FlashStringHelper* getChipModel() {
#ifdef ESP32

  // https://www.espressif.com/en/products/socs
  // https://github.com/arendst/Tasmota/blob/1e6b78a957be538cf494f0e2dc49060d1cb0fe8b/tasmota/support_esp.ino#L579

  /*
     Source: esp-idf esp_system.h and esptool
     typedef enum {
      CHIP_ESP32   = 1,  //!< ESP32
      CHIP_ESP32S2 = 2,  //!< ESP32-S2
      CHIP_ESP32S3 = 4,  //!< ESP32-S3
      CHIP_ESP32C3 = 5,  //!< ESP32-C3
     } esp_chip_model_t;
     // Chip feature flags, used in esp_chip_info_t
   #define CHIP_FEATURE_EMB_FLASH      BIT(0)      //!< Chip has embedded flash memory
   #define CHIP_FEATURE_WIFI_BGN       BIT(1)      //!< Chip has 2.4GHz WiFi
   #define CHIP_FEATURE_BLE            BIT(4)      //!< Chip has Bluetooth LE
   #define CHIP_FEATURE_BT             BIT(5)      //!< Chip has Bluetooth Classic
     // The structure represents information about the chip
     typedef struct {
      esp_chip_model_t model;  //!< chip model, one of esp_chip_model_t
      uint32_t features;       //!< bit mask of CHIP_FEATURE_x feature flags
      uint8_t cores;           //!< number of CPU cores
      uint8_t revision;        //!< chip revision number
     } esp_chip_info_t;
   */
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  uint32_t chip_model    = chip_info.model;
  uint32_t chip_revision = chip_info.revision;

  //  uint32_t chip_revision = ESP.getChipRevision();
  bool rev3 = (3 == chip_revision);

  //  bool single_core = (1 == ESP.getChipCores());
  bool single_core = (1 == chip_info.cores);

  if (chip_model < 2) { // ESP32
# ifdef CONFIG_IDF_TARGET_ESP32

    /* esptool:
        def get_pkg_version(self):
            word3 = self.read_efuse(3)
            pkg_version = (word3 >> 9) & 0x07
            pkg_version += ((word3 >> 2) & 0x1) << 3
            return pkg_version
     */
    uint32_t chip_ver    = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
    uint32_t pkg_version = chip_ver & 0x7;

    //    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("HDW: ESP32 Model %d, Revision %d, Core %d, Package %d"), chip_info.model, chip_revision,
    // chip_info.cores, chip_ver);

    switch (pkg_version) {
      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ6:

        if (single_core) { return F("ESP32-S0WDQ6"); } // Max 240MHz, Single core, QFN 6*6
        else if (rev3)   { return F("ESP32-D0WDQ6-V3"); }  // Max 240MHz, Dual core, QFN 6*6
        else {             return F("ESP32-D0WDQ6"); } // Max 240MHz, Dual core, QFN 6*6
      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ5:

        if (single_core) { return F("ESP32-S0WD"); }   // Max 160MHz, Single core, QFN 5*5, ESP32-SOLO-1, ESP32-DevKitC
        else if (rev3)   { return F("ESP32-D0WDQ5-V3"); }  // Max 240MHz, Dual core, QFN 5*5, ESP32-WROOM-32E, ESP32_WROVER-E, ESP32-DevKitC
        else {             return F("ESP32-D0WDQ5"); } // Max 240MHz, Dual core, QFN 5*5, ESP32-WROOM-32D, ESP32_WROVER-B, ESP32-DevKitC
      case EFUSE_RD_CHIP_VER_PKG_ESP32D2WDQ5:
        return F("ESP32-D2WDQ5");                      // Max 160MHz, Dual core, QFN 5*5, 2MB embedded flash
      case 3:

        if (single_core) { return F("ESP32-S0WD-OEM"); } // Max 160MHz, Single core, QFN 5*5, Xiaomi Yeelight
        else {             return F("ESP32-D0WD-OEM"); } // Max 240MHz, Dual core, QFN 5*5
      case EFUSE_RD_CHIP_VER_PKG_ESP32U4WDH:
        return F("ESP32-U4WDH");                         // Max 160MHz, Single core, QFN 5*5, 4MB embedded flash, ESP32-MINI-1,
      // ESP32-DevKitM-1
      case EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4:

        if (rev3)        { return F("ESP32-PICO-V3"); } // Max 240MHz, Dual core, LGA 7*7, ESP32-PICO-V3-ZERO, ESP32-PICO-V3-ZERO-DevKit
        else {             return F("ESP32-PICO-D4"); } // Max 240MHz, Dual core, LGA 7*7, 4MB embedded flash, ESP32-PICO-KIT
      case EFUSE_RD_CHIP_VER_PKG_ESP32PICOV302:
        return F("ESP32-PICO-V3-02");                    // Max 240MHz, Dual core, LGA 7*7, 8MB embedded flash, 2MB embedded PSRAM,
                                                         // ESP32-PICO-MINI-02, ESP32-PICO-DevKitM-2
    }
# endif // CONFIG_IDF_TARGET_ESP32
    return F("ESP32");
  }
  else if (2 == chip_model) { // ESP32-S2
# ifdef CONFIG_IDF_TARGET_ESP32S2

    /* esptool:
        def get_pkg_version(self):
            num_word = 3
            block1_addr = self.EFUSE_BASE + 0x044
            word3 = self.read_reg(block1_addr + (4 * num_word))
            pkg_version = (word3 >> 21) & 0x0F
            return pkg_version
     */
    uint32_t chip_ver    = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_PKG_VERSION);
    uint32_t pkg_version = chip_ver & 0x7;

    //    uint32_t pkg_version = esp_efuse_get_pkg_ver();

    //    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("HDW: ESP32 Model %d, Revision %d, Core %d, Package %d"), chip_info.model, chip_revision,
    // chip_info.cores, chip_ver);

    switch (pkg_version) {
      case 0:              return F("ESP32-S2");      // Max 240MHz, Single core, QFN 7*7, ESP32-S2-WROOM, ESP32-S2-WROVER,
      // ESP32-S2-Saola-1, ESP32-S2-Kaluga-1
      case 1:              return F("ESP32-S2FH2");   // Max 240MHz, Single core, QFN 7*7, 2MB embedded flash, ESP32-S2-MINI-1,
      // ESP32-S2-DevKitM-1
      case 2:              return F("ESP32-S2FH4");   // Max 240MHz, Single core, QFN 7*7, 4MB embedded flash
      case 3:              return F("ESP32-S2FN4R2"); // Max 240MHz, Single core, QFN 7*7, 4MB embedded flash, 2MB embedded PSRAM, ,
                                                      // ESP32-S2-MINI-1U, ESP32-S2-DevKitM-1U
    }
# endif // CONFIG_IDF_TARGET_ESP32S2
    return F("ESP32-S2");
  }
  else if (4 == chip_model) { // ESP32-S3
    return F("ESP32-S3");     // Max 240MHz, Dual core, QFN 7*7, ESP32-S3-WROOM-1, ESP32-S3-DevKitC-1
  }
  else if (5 == chip_model) { // ESP32-C3
# ifdef CONFIG_IDF_TARGET_ESP32C3

    /* esptool:
        def get_pkg_version(self):
            num_word = 3
            block1_addr = self.EFUSE_BASE + 0x044
            word3 = self.read_reg(block1_addr + (4 * num_word))
            pkg_version = (word3 >> 21) & 0x0F
            return pkg_version
     */
    uint32_t chip_ver    = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_PKG_VERSION);
    uint32_t pkg_version = chip_ver & 0x7;

    //    uint32_t pkg_version = esp_efuse_get_pkg_ver();

    //    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("HDW: ESP32 Model %d, Revision %d, Core %d, Package %d"), chip_info.model, chip_revision,
    // chip_info.cores, chip_ver);

    switch (pkg_version) {
      case 0:              return F("ESP32-C3");    // Max 160MHz, Single core, QFN 5*5, ESP32-C3-WROOM-02, ESP32-C3-DevKitC-02
      case 1:              return F("ESP32-C3FH4"); // Max 160MHz, Single core, QFN 5*5, 4MB embedded flash, ESP32-C3-MINI-1,
                                                    // ESP32-C3-DevKitM-1
    }
# endif // CONFIG_IDF_TARGET_ESP32C3
    return F("ESP32-C3");
  }
  else if (6 == chip_model) { // ESP32-S3(beta3)
    return F("ESP32-S3");
  }
  else if (7 == chip_model) { // ESP32-C6(beta)
# ifdef CONFIG_IDF_TARGET_ESP32C6

    /* esptool:
        def get_pkg_version(self):
            num_word = 3
            block1_addr = self.EFUSE_BASE + 0x044
            word3 = self.read_reg(block1_addr + (4 * num_word))
            pkg_version = (word3 >> 21) & 0x0F
            return pkg_version
     */
    uint32_t chip_ver    = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_PKG_VERSION);
    uint32_t pkg_version = chip_ver & 0x7;

    //    uint32_t pkg_version = esp_efuse_get_pkg_ver();

    //    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("HDW: ESP32 Model %d, Revision %d, Core %d, Package %d"), chip_info.model, chip_revision,
    // chip_info.cores, chip_ver);

    switch (pkg_version) {
      case 0:              return F("ESP32-C6");
    }
# endif // CONFIG_IDF_TARGET_ESP32C6
    return F("ESP32-C6");
  }
  else if (10 == chip_model) {  // ESP32-H2
# ifdef CONFIG_IDF_TARGET_ESP32H2

    /* esptool:
        def get_pkg_version(self):
            num_word = 3
            block1_addr = self.EFUSE_BASE + 0x044
            word3 = self.read_reg(block1_addr + (4 * num_word))
            pkg_version = (word3 >> 21) & 0x0F
            return pkg_version
     */
    uint32_t chip_ver    = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_PKG_VERSION);
    uint32_t pkg_version = chip_ver & 0x7;

    //    uint32_t pkg_version = esp_efuse_get_pkg_ver();

    //    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("HDW: ESP32 Model %d, Revision %d, Core %d, Package %d"), chip_info.model, chip_revision,
    // chip_info.cores, chip_ver);

    switch (pkg_version) {
      case 0:              return F("ESP32-H2");
    }
# endif // CONFIG_IDF_TARGET_ESP32H2
    return F("ESP32-H2");
  }
  return F("ESP32");
#elif defined(ESP8266)
  return isESP8285() ? F("ESP8285") : F("ESP8266");
#endif // ifdef ESP32
  return F("Unknown");
}

bool isESP8285() {
  #ifdef ESP8266
  const uint32_t efuse_blocks[4] {
    READ_PERI_REG(0x3ff00050),
    READ_PERI_REG(0x3ff00054),
    READ_PERI_REG(0x3ff00058),
    READ_PERI_REG(0x3ff0005c)
  };

  return
    (efuse_blocks[0] & (1 << 4))
    || (efuse_blocks[2] & (1 << 16))
  ;
  #else // ifdef ESP8266
  return false;
  #endif // ifdef ESP8266
}

uint8_t getChipRevision() {
  uint8_t rev = 0;

  #ifdef ESP32
  rev = ESP.getChipRevision();
  #endif // ifdef ESP32
  return rev;
}

uint32_t getSketchSize() {
  // Cache the value as this never changes during run time.
  static uint32_t res = ESP.getSketchSize();

  return res;
}

uint32_t getFreeSketchSpace() {
  // Cache the value as this never changes during run time.
  static uint32_t res = ESP.getFreeSketchSpace();

  return res;
}

/********************************************************************************************\
   PSRAM support
 \*********************************************************************************************/

#ifdef ESP32

// this function is a replacement for `psramFound()`.
// `psramFound()` can return true even if no PSRAM is actually installed
// This new version also checks `esp_spiram_is_initialized` to know if the PSRAM is initialized
// Original Tasmota:
// https://github.com/arendst/Tasmota/blob/1e6b78a957be538cf494f0e2dc49060d1cb0fe8b/tasmota/support_esp.ino#L470
bool FoundPSRAM() {
# if CONFIG_IDF_TARGET_ESP32C3
  return psramFound();
# else // if CONFIG_IDF_TARGET_ESP32C3
  return psramFound() && esp_spiram_is_initialized();
# endif // if CONFIG_IDF_TARGET_ESP32C3
}

// new function to check whether PSRAM is present and supported (i.e. required pacthes are present)
bool UsePSRAM() {
  static bool can_use_psram = CanUsePSRAM();

  return FoundPSRAM() && can_use_psram;
}

/*
 * ESP32 v1 and v2 needs some special patches to use PSRAM.
 * Original function used from Tasmota:
 * https://github.com/arendst/Tasmota/blob/1e6b78a957be538cf494f0e2dc49060d1cb0fe8b/tasmota/support_esp.ino#L762
 *
 * If using ESP32 v1, please add: `-mfix-esp32-psram-cache-issue -lc-psram-workaround -lm-psram-workaround`
 *
 * This function returns true if the chip supports PSRAM natively (v3) or if the
 * patches are present.
 */
bool CanUsePSRAM() {
  if (!FoundPSRAM()) { return false; }
# ifdef HAS_PSRAM_FIX
  return true;
# endif // ifdef HAS_PSRAM_FIX
# ifdef CONFIG_IDF_TARGET_ESP32
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  if ((CHIP_ESP32 == chip_info.model) && (chip_info.revision < 3)) {
    return false;
  }
#  if ESP_IDF_VERSION_MAJOR < 4
  uint32_t chip_ver    = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
  uint32_t pkg_version = chip_ver & 0x7;

  if ((CHIP_ESP32 == chip_info.model) && (pkg_version >= 6)) {
    return false; // support for embedded PSRAM of ESP32-PICO-V3-02 requires esp-idf 4.4
  }
#  endif // ESP_IDF_VERSION_MAJOR < 4

# endif // CONFIG_IDF_TARGET_ESP32
  return true;
}

#endif // ESP32


/*********************************************************************************************\
* High entropy hardware random generator
* Thanks to DigitalAlchemist
\*********************************************************************************************/

// Based on code from https://raw.githubusercontent.com/espressif/esp-idf/master/components/esp32/hw_random.c
// https://github.com/arendst/Tasmota/blob/1e6b78a957be538cf494f0e2dc49060d1cb0fe8b/tasmota/support_esp.ino#L805
uint32_t HwRandom() {
#if ESP8266

  // https://web.archive.org/web/20160922031242/http://esp8266-re.foogod.com/wiki/Random_Number_Generator
  # define _RAND_ADDR 0x3FF20E44UL
#endif // ESP8266
#ifdef ESP32
  # define _RAND_ADDR 0x3FF75144UL
#endif // ESP32
  static uint32_t last_ccount = 0;
  uint32_t ccount;
  uint32_t result = 0;

  do {
    ccount  = ESP.getCycleCount();
    result ^= *(volatile uint32_t *)_RAND_ADDR;     // -V566
  } while (ccount - last_ccount < 64);
  last_ccount = ccount;
  return result ^ *(volatile uint32_t *)_RAND_ADDR; // -V566
#undef _RAND_ADDR
}

#ifdef ESP8266
void readBootCause() {
  lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;
  const rst_info *resetInfo = ESP.getResetInfoPtr();

  if (resetInfo != nullptr) {
    switch (resetInfo->reason) {
      // normal startup by power on
      case REASON_DEFAULT_RST:      lastBootCause = BOOT_CAUSE_COLD_BOOT; break;

      // hardware watch dog reset
      case REASON_WDT_RST:          lastBootCause = BOOT_CAUSE_EXT_WD; break;

      // exception reset, GPIO status won’t change
      case REASON_EXCEPTION_RST:    lastBootCause = BOOT_CAUSE_EXCEPTION; break;

      // software watch dog reset, GPIO status won’t change
      case REASON_SOFT_WDT_RST:     lastBootCause = BOOT_CAUSE_SW_WATCHDOG; break;

      // software restart ,system_restart , GPIO status won’t change
      case REASON_SOFT_RESTART:     lastBootCause = BOOT_CAUSE_SOFT_RESTART; break;

      // wake up from deep-sleep
      case REASON_DEEP_SLEEP_AWAKE: lastBootCause = BOOT_CAUSE_DEEP_SLEEP; break;

      // external system reset
      case REASON_EXT_SYS_RST:      lastBootCause = BOOT_CAUSE_MANUAL_REBOOT; break;
      default:
        break;
    }
  }
}

#endif // ifdef ESP8266

#ifdef ESP32
void readBootCause() {
  lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;

  switch (rtc_get_reset_reason(0)) {
    case NO_MEAN:           break;
    case POWERON_RESET:     lastBootCause = BOOT_CAUSE_MANUAL_REBOOT; break;
    # ifndef ESP32S2
    case SW_RESET:          lastBootCause = BOOT_CAUSE_SOFT_RESTART; break;
    case OWDT_RESET:        lastBootCause = BOOT_CAUSE_SW_WATCHDOG; break;
    # endif // ifndef ESP32S2
    case DEEPSLEEP_RESET:   lastBootCause = BOOT_CAUSE_DEEP_SLEEP; break;
    # ifndef ESP32S2
    case SDIO_RESET:        lastBootCause = BOOT_CAUSE_MANUAL_REBOOT; break;
    # endif // ifndef ESP32S2
    case TG0WDT_SYS_RESET:
    case TG1WDT_SYS_RESET:
    case RTCWDT_SYS_RESET:  lastBootCause = BOOT_CAUSE_EXT_WD; break;
    # ifndef ESP32S2
    case SW_CPU_RESET:
    case TGWDT_CPU_RESET:
    # endif // ifndef ESP32S2
    case INTRUSION_RESET:   lastBootCause = BOOT_CAUSE_SOFT_RESTART; break;  // Both call to ESP.reset() and on exception crash
    case RTCWDT_CPU_RESET:  lastBootCause = BOOT_CAUSE_EXT_WD; break;
    # ifndef ESP32S2
    case EXT_CPU_RESET:     lastBootCause = BOOT_CAUSE_MANUAL_REBOOT; break; // reset button or cold boot, only for core 1
    # endif // ifndef ESP32S2
    case RTCWDT_BROWN_OUT_RESET: lastBootCause = BOOT_CAUSE_POWER_UNSTABLE; break;
    case RTCWDT_RTC_RESET:  lastBootCause      = BOOT_CAUSE_COLD_BOOT; break;
    # ifdef ESP32S2
    case RTC_SW_SYS_RESET: lastBootCause  = BOOT_CAUSE_SOFT_RESTART; break;
    case TG0WDT_CPU_RESET: lastBootCause  = BOOT_CAUSE_EXT_WD; break;
    case RTC_SW_CPU_RESET: lastBootCause  = BOOT_CAUSE_SOFT_RESTART; break;
    case TG1WDT_CPU_RESET: lastBootCause  = BOOT_CAUSE_EXT_WD; break;
    case SUPER_WDT_RESET:   lastBootCause = BOOT_CAUSE_EXT_WD; break;
    case GLITCH_RTC_RESET:  lastBootCause = BOOT_CAUSE_POWER_UNSTABLE; break; // FIXME TD-er: Does this need a different reason?
    case EFUSE_RESET:       break;                                            // FIXME TD-er: No idea what may cause this reset reason.
    # endif // ifdef ESP32S2
  }
}

#endif // ifdef ESP32


/********************************************************************************************\
   Hardware specific configurations
 \*********************************************************************************************/
const __FlashStringHelper* getDeviceModelBrandString(DeviceModel model) {
  switch (model) {
    case DeviceModel::DeviceModel_Sonoff_Basic:
    case DeviceModel::DeviceModel_Sonoff_TH1x:
    case DeviceModel::DeviceModel_Sonoff_S2x:
    case DeviceModel::DeviceModel_Sonoff_TouchT1:
    case DeviceModel::DeviceModel_Sonoff_TouchT2:
    case DeviceModel::DeviceModel_Sonoff_TouchT3:
    case DeviceModel::DeviceModel_Sonoff_4ch:
    case DeviceModel::DeviceModel_Sonoff_POW:
    case DeviceModel::DeviceModel_Sonoff_POWr2:   return F("Sonoff");
    case DeviceModel::DeviceModel_Shelly1:
    case DeviceModel::DeviceModel_ShellyPLUG_S:   return F("Shelly");
    case DeviceModel::DeviceModel_Olimex_ESP32_PoE:
    case DeviceModel::DeviceModel_Olimex_ESP32_EVB:
    case DeviceModel::DeviceModel_Olimex_ESP32_GATEWAY:
    #ifdef ESP32
      return F("Olimex");
    #endif // ifdef ESP32
    case DeviceModel::DeviceModel_wESP32:
    #ifdef ESP32
      return F("wESP32");
    #endif // ifdef ESP32
    case DeviceModel::DeviceModel_WT32_ETH01:
    #ifdef ESP32
      return F("WT32-ETH01");
    #endif // ifdef ESP32
    case DeviceModel::DeviceModel_default:
    case DeviceModel::DeviceModel_MAX:      break;

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("");
}

const __FlashStringHelper* getDeviceModelTypeString(DeviceModel model)
{
  switch (model) {
#if defined(ESP8266) && !defined(LIMIT_BUILD_SIZE)
    case DeviceModel::DeviceModel_Sonoff_Basic:   return F(" Basic");
    case DeviceModel::DeviceModel_Sonoff_TH1x:    return F(" TH1x");
    case DeviceModel::DeviceModel_Sonoff_S2x:     return F(" S2x");
    case DeviceModel::DeviceModel_Sonoff_TouchT1: return F(" TouchT1");
    case DeviceModel::DeviceModel_Sonoff_TouchT2: return F(" TouchT2");
    case DeviceModel::DeviceModel_Sonoff_TouchT3: return F(" TouchT3");
    case DeviceModel::DeviceModel_Sonoff_4ch:     return F(" 4ch");
    case DeviceModel::DeviceModel_Sonoff_POW:     return F(" POW");
    case DeviceModel::DeviceModel_Sonoff_POWr2:   return F(" POW-r2");
    case DeviceModel::DeviceModel_Shelly1:        return F("1");
    case DeviceModel::DeviceModel_ShellyPLUG_S:   return F(" PLUG S");
#else // if defined(ESP8266) && !defined(LIMIT_BUILD_SIZE)
    case DeviceModel::DeviceModel_Sonoff_Basic:
    case DeviceModel::DeviceModel_Sonoff_TH1x:
    case DeviceModel::DeviceModel_Sonoff_S2x:
    case DeviceModel::DeviceModel_Sonoff_TouchT1:
    case DeviceModel::DeviceModel_Sonoff_TouchT2:
    case DeviceModel::DeviceModel_Sonoff_TouchT3:
    case DeviceModel::DeviceModel_Sonoff_4ch:
    case DeviceModel::DeviceModel_Sonoff_POW:
    case DeviceModel::DeviceModel_Sonoff_POWr2:
    case DeviceModel::DeviceModel_Shelly1:
    case DeviceModel::DeviceModel_ShellyPLUG_S:
      return F("default");
#endif // if defined(ESP8266) && !defined(LIMIT_BUILD_SIZE)
#ifdef ESP32
    case DeviceModel::DeviceModel_Olimex_ESP32_PoE:      return F(" ESP32-PoE");
    case DeviceModel::DeviceModel_Olimex_ESP32_EVB:      return F(" ESP32-EVB");
    case DeviceModel::DeviceModel_Olimex_ESP32_GATEWAY:  return F(" ESP32-GATEWAY");
    case DeviceModel::DeviceModel_wESP32:                break;
    case DeviceModel::DeviceModel_WT32_ETH01:            return F(" add-on");
#else // ifdef ESP32
    case DeviceModel::DeviceModel_Olimex_ESP32_PoE:
    case DeviceModel::DeviceModel_Olimex_ESP32_EVB:
    case DeviceModel::DeviceModel_Olimex_ESP32_GATEWAY:
    case DeviceModel::DeviceModel_wESP32:
    case DeviceModel::DeviceModel_WT32_ETH01:
#endif // ifdef ESP32

    case DeviceModel::DeviceModel_default:
    case DeviceModel::DeviceModel_MAX:             return F("default");

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("");
}

String getDeviceModelString(DeviceModel model) {
  String result = getDeviceModelBrandString(model);

  result += getDeviceModelTypeString(model);
  return result;
}

bool modelMatchingFlashSize(DeviceModel model) {
#if defined(ESP8266) || (defined(ESP32) && FEATURE_ETHERNET)
  const uint32_t size_MB = getFlashRealSizeInBytes() >> 20;
#endif // if defined(ESP8266) || (defined(ESP32) && FEATURE_ETHERNET)

  // TD-er: This also checks for ESP8266/ESP8285/ESP32
  switch (model) {
    case DeviceModel::DeviceModel_Sonoff_Basic:
    case DeviceModel::DeviceModel_Sonoff_TH1x:
    case DeviceModel::DeviceModel_Sonoff_S2x:
    case DeviceModel::DeviceModel_Sonoff_TouchT1:
    case DeviceModel::DeviceModel_Sonoff_TouchT2:
    case DeviceModel::DeviceModel_Sonoff_TouchT3:
    case DeviceModel::DeviceModel_Sonoff_4ch:
#ifdef ESP8266
      return size_MB == 1;
#else // ifdef ESP8266
      return false;
#endif // ifdef ESP8266

    case DeviceModel::DeviceModel_Sonoff_POW:
    case DeviceModel::DeviceModel_Sonoff_POWr2:
#ifdef ESP8266
      return size_MB == 4;
#else // ifdef ESP8266
      return false;
#endif // ifdef ESP8266

    case DeviceModel::DeviceModel_Shelly1:
    case DeviceModel::DeviceModel_ShellyPLUG_S:
#ifdef ESP8266
      return size_MB == 2;
#else // ifdef ESP8266
      return false;
#endif // ifdef ESP8266

    // These Olimex boards all have Ethernet
    case DeviceModel::DeviceModel_Olimex_ESP32_PoE:
    case DeviceModel::DeviceModel_Olimex_ESP32_EVB:
    case DeviceModel::DeviceModel_Olimex_ESP32_GATEWAY:
    case DeviceModel::DeviceModel_wESP32:
    case DeviceModel::DeviceModel_WT32_ETH01:
#if  defined(ESP32) && FEATURE_ETHERNET
      return size_MB == 4;
#else // if  defined(ESP32) && FEATURE_ETHERNET
      return false;
#endif // if  defined(ESP32) && FEATURE_ETHERNET

    case DeviceModel::DeviceModel_default:
    case DeviceModel::DeviceModel_MAX:
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

void addButtonRelayRule(uint8_t buttonNumber, int relay_gpio) {
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
    addLogMove(LOG_LEVEL_ERROR, result);
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
// Get info of a specific GPIO pin
// ********************************************************************************
bool getGpioInfo(int gpio, int& pinnr, bool& input, bool& output, bool& warning) {
  pinnr = -1; // ESP32 does not label the pins, they just use the GPIO number.

# ifdef ESP32S2

  // Input GPIOs:  0-21, 26, 33-46
  // Output GPIOs: 0-21, 26, 33-45
  input  = gpio <= 46;
  output = gpio <= 45;

  if ((gpio < 0) || ((gpio > 21) && (gpio < 26)) || ((gpio > 26) && (gpio < 33))) {
    input  = false;
    output = false;
  }

  if (gpio == 26) {
    // Pin shared with the flash memory and/or PSRAM.
    // Cannot be used as regular GPIO
    input   = false;
    output  = false;
    warning = true;
  }

  if ((gpio > 26) && (gpio < 33)) {
    // SPIHD, SPIWP, SPICS0, SPICLK, SPIQ, SPID pins of ESP32-S2FH2 and ESP32-S2FH4
    // are connected to embedded flash and not recommended for other uses.
    warning = true;
  }


  if ((input == false) && (output == false)) {
    return false;
  }

  if (gpio == 45) {
    // VDD_SPI can work as the power supply for the external device at either
    // 1.8 V (when GPIO45 is 1 during boot), or
    // 3.3 V (when GPIO45 is 0 and at default state during boot).
    warning = true;
  }

  // GPIO 0  State during boot determines boot mode.
  warning = gpio == 0;


  if (gpio == 46) {
    // Part of the boot strapping pins.
    warning = true;
  }

  /*
   # if FEATURE_ETHERNET

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


   # endif // if FEATURE_ETHERNET

   */
# else // ifdef ESP32S2

  // ESP32 classic

  // Input GPIOs:  0-19, 21-23, 25-27, 32-39
  // Output GPIOs: 0-19, 21-23, 25-27, 32-33
  input  = gpio <= 39;
  output = gpio <= 33;

  if ((gpio < 0) || (gpio == 20) || (gpio == 24) || ((gpio > 27) && (gpio < 32))) {
    input  = false;
    output = false;
  }

  if ((gpio == 37) || (gpio == 38)) {
    // Pins are not present on the ESP32
    input  = true;
    output = false;
  }

  if ((gpio >= 6) && (gpio <= 11)) {
    // Connected to the integrated SPI flash.
    input   = false;
    output  = false;
    warning = true;
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

  #  if FEATURE_ETHERNET

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


  #  endif // if FEATURE_ETHERNET

# endif    // ifdef ESP32S2

  if (UsePSRAM()) {
    // PSRAM can use GPIO 16 and 17
    // There will be a high frequency signal on those pins (flash frequency)
    // which makes them unusable for other purposes.
    // WROVER does not even have these pins made available on the outside.
    switch (gpio) {
      case 16:
      case 17:
        warning = true;
        break;
    }
  }
  return true;
}

bool getGpioPullResistor(int gpio, bool& hasPullUp, bool& hasPullDown) {
  hasPullDown = false;
  hasPullUp   = false;

  int pinnr;
  bool input;
  bool output;
  bool warning;

  if (!getGpioInfo(gpio, pinnr, input, output, warning)) {
    return false;
  }

# ifdef ESP32S2

  if (gpio <= 45) {
    hasPullUp   = true;
    hasPullDown = true;
  }
# else // ifdef ESP32S2

  // ESP32 classic
  if (gpio >= 34) {
    // For GPIO 34 .. 39, no pull-up nor pull-down.
  } else if (gpio == 12) {
    // No Pull-up on GPIO12
    // compatible with the SDIO protocol.
    // Just connect GPIO12 to VDD via a 10 kOhm resistor.
  } else {
    hasPullUp   = true;
    hasPullDown = true;
  }

# endif // ifdef ESP32S2
  return true;
}

#endif // ifdef ESP32

#ifdef ESP8266

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
    if (isESP8285()) {
      if ((gpio == 9) || (gpio == 10)) {
        // Usable on ESP8285
      } else {
        warning = true;
      }
    } else {
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
    }
  }

  if ((pinnr < 0) || (pinnr > 16)) {
    input  = false;
    output = false;
  }
  return input || output;
}

bool getGpioPullResistor(int gpio, bool& hasPullUp, bool& hasPullDown) {
  hasPullDown = false;
  hasPullUp   = false;

  if (!validGpio(gpio)) {
    return false;
  }

  if (gpio == 16) {
    hasPullDown = true;
  } else {
    hasPullUp = true;
  }
  return true;
}

#endif // ifdef ESP8266

bool validGpio(int gpio) {
  if ((gpio < 0) || (gpio > MAX_GPIO)) { return false; }
  int pinnr;
  bool input;
  bool output;
  bool warning;

  return getGpioInfo(gpio, pinnr, input, output, warning);
}

#ifdef ESP32

// Get ADC related info for a given GPIO pin
// @param gpio_pin   GPIO pin number
// @param adc        Number of ADC unit (0 == Hall effect)
// @param ch         Channel number on ADC unit
// @param t          index of touch pad ID
bool getADC_gpio_info(int gpio_pin, int& adc, int& ch, int& t)
{
  adc = -1;
  ch  = -1;
  t   = -1;

# ifdef ESP32S2

  switch (gpio_pin) {
    case 1: adc  = 1; ch = 0; t = 1; break;
    case 2: adc  = 1; ch = 1; t = 2; break;
    case 3: adc  = 1; ch = 2; t = 3; break;
    case 4: adc  = 1; ch = 3; t = 4; break;
    case 5: adc  = 1; ch = 4; t = 5; break;
    case 6: adc  = 1; ch = 5; t = 6; break;
    case 7: adc  = 1; ch = 6; t = 7; break;
    case 8: adc  = 1; ch = 7; t = 8; break;
    case 9: adc  = 1; ch = 8; t = 9; break;
    case 10: adc = 1; ch = 9; t = 10; break;
    case 11: adc = 2; ch = 0; t = 11; break;
    case 12: adc = 2; ch = 1; t = 12; break;
    case 13: adc = 2; ch = 2; t = 13; break;
    case 14: adc = 2; ch = 3; t = 14; break;
    case 15: adc = 2; ch = 4;  break;
    case 16: adc = 2; ch = 5;  break;
    case 17: adc = 2; ch = 6;  break;
    case 18: adc = 2; ch = 7;  break;
    case 19: adc = 2; ch = 8;  break;
    case 20: adc = 2; ch = 9;  break;
    default:
      return false;
  }
# else // ifdef ESP32S2

  // Classic ESP32
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
# endif // ifdef ESP32S2
  return true;
}

int touchPinToGpio(int touch_pin)
{
# ifdef ESP32S2

  switch (touch_pin) {
    case 1: return T1;
    case 2: return T2;
    case 3: return T3;
    case 4: return T4;
    case 5: return T5;
    case 6: return T6;
    case 7: return T7;
    case 8: return T8;
    case 9: return T9;
    case 10: return T10;
    case 11: return T11;
    case 12: return T12;
    case 13: return T13;
    case 14: return T14;
    default:
      break;
  }
# else // ifdef ESP32S2

  // ESP32 classic
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
# endif // ifdef ESP32S2
  return -1;
}

#endif // ifdef ESP32

// ********************************************************************************
// Manage PWM state of GPIO pins.
// ********************************************************************************
void initAnalogWrite()
{
  #if defined(ESP32)

  for (uint8_t x = 0; x < 16; x++) {
    ledChannelPin[x]  = -1;
    ledChannelFreq[x] = ledcSetup(x, 1000, 10); // Clear the channel
  }
  #endif // if defined(ESP32)
  #ifdef ESP8266

  // See https://github.com/esp8266/Arduino/commit/a67986915512c5304bd7c161cf0d9c65f66e0892
  analogWriteRange(1023);
  #endif // ifdef ESP8266
}

#if defined(ESP32)
int8_t ledChannelPin[16];
uint32_t ledChannelFreq[16] = { 0 };


int8_t attachLedChannel(int pin, uint32_t frequency)
{
  static bool initialized = false;

  if (!initialized) {
    for (uint8_t x = 0; x < 16; x++) {
      ledChannelPin[x]  = -1;
      ledChannelFreq[x] = 0;
    }
    initialized = true;
  }


  // find existing channel if this pin has been used before
  int8_t ledChannel = -1;
  bool mustSetup    = false;

  for (uint8_t x = 0; x < 16; x++) {
    if (ledChannelPin[x] == pin) {
      ledChannel = x;
    }
  }

  if (ledChannel == -1)                                    // no channel set for this pin
  {
    for (uint8_t x = 0; x < 16 && ledChannel == -1; ++x) { // find free channel
      if (ledChannelPin[x] == -1)
      {
        if (static_cast<uint32_t>(ledcReadFreq(x)) == ledChannelFreq[x]) {
          // Channel is not used by some other piece of code.
          ledChannel = x;
          mustSetup  = true;
          break;
        }
      }
    }
  }

  if (ledChannel == -1) { return ledChannel; }

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
    ledChannelPin[ledChannel]  = pin; // store pin nr
    ledcAttachPin(pin, ledChannel);   // attach to this pin
    //    pinMode(pin, OUTPUT);
  }

  return ledChannel;
}

void detachLedChannel(int pin)
{
  int8_t ledChannel = -1;

  for (uint8_t x = 0; x < 16; x++) {
    if (ledChannelPin[x] == pin) {
      ledChannel = x;
    }
  }

  if (ledChannel != -1) {
    ledcWrite(ledChannel, 0);
    ledcDetachPin(pin);
    ledChannelPin[ledChannel]  = -1;
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
  uint8_t   pluginID = PLUGIN_GPIO;

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

  #if defined(ESP8266)

  if ((frequency > 0) && (frequency <= 40000)) {
    analogWriteFreq(frequency);
  }
  #endif // if defined(ESP8266)

  if (fadeDuration_ms != 0)
  {
    const int32_t resolution_factor = (1 << 12);
    const uint8_t prev_mode         = tempStatus.mode;
    int32_t   prev_value            = tempStatus.getDutyCycle();

    // getPinState(pluginID, gpio, &prev_mode, &prev_value);
    if (prev_mode != PIN_MODE_PWM) {
      prev_value = 0;
    }

    const int32_t step_value = ((static_cast<int32_t>(dutyCycle) - prev_value) * resolution_factor) / static_cast<int32_t>(fadeDuration_ms);
    int32_t curr_value       = prev_value * resolution_factor;

    int i = fadeDuration_ms;

    while (i--) {
      curr_value += step_value;
      const int16_t new_value = curr_value / resolution_factor;
            #if defined(ESP8266)
      analogWrite(gpio, new_value);
            #endif // if defined(ESP8266)
            #if defined(ESP32)
      frequency = analogWriteESP32(gpio, new_value, frequency);
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
}

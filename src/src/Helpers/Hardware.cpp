#include "../Helpers/Hardware.h"

#include "../Commands/GPIO.h"
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
#include "../Helpers/FS_Helper.h"
#include "../Helpers/Hardware_device_info.h"
#include "../Helpers/Hardware_GPIO.h"
#include "../Helpers/Hardware_I2C.h"
#include "../Helpers/I2C_access.h"
#include "../Helpers/Misc.h"
#include "../Helpers/PortStatus.h"
#include "../Helpers/StringConverter.h"


#if defined(ESP8266)
  # include <ESP8266WiFi.h>
#endif // if defined(ESP8266)
#if defined(ESP32)
  # include <WiFi.h>
#endif // if defined(ESP32)

// #include "../../ESPEasy-Globals.h"

#ifdef ESP32
  # include <soc/soc.h>
  # include <soc/efuse_reg.h>
  # include <hal/efuse_hal.h>
  # include <soc/spi_reg.h>
  # include <soc/rtc.h>

  # if ESP_IDF_VERSION_MAJOR == 4
    #  if CONFIG_IDF_TARGET_ESP32S3   // ESP32-S3
      #   include <esp32s3/rom/spi_flash.h>
      #   include <esp32s3/spiram.h>
      #   include <esp32s3/rom/rtc.h>
    #  elif CONFIG_IDF_TARGET_ESP32S2   // ESP32-S2
      #   include <esp32s2/rom/spi_flash.h>
      #   include <esp32s2/spiram.h>
      #   include <esp32s2/rom/rtc.h>
    #  elif CONFIG_IDF_TARGET_ESP32C3 // ESP32-C3
      #   include <esp32c3/rom/spi_flash.h>
      #   include <esp32c3/rom/rtc.h>
    #  elif CONFIG_IDF_TARGET_ESP32   // ESP32/PICO-D4
      #   include <esp32/rom/spi_flash.h>
      #   include <esp32/rom/rtc.h>
      #   include <esp32/spiram.h>
    #  else // if CONFIG_IDF_TARGET_ESP32S3
      #   error Target CONFIG_IDF_TARGET is not supported
    #  endif // if CONFIG_IDF_TARGET_ESP32S3
  # else // ESP32 IDF 5.x and later
    #  include <rom/spi_flash.h>
    #  include <rom/rtc.h>
    #  include <bootloader_common.h>
  # endif // if ESP_IDF_VERSION_MAJOR == 4


# if CONFIG_IDF_TARGET_ESP32S3   // ESP32-S3
  #  define HAS_HALL_EFFECT_SENSOR  0
  #  define HAS_TOUCH_GPIO 1
# elif CONFIG_IDF_TARGET_ESP32S2 // ESP32-S2
  #  define HAS_HALL_EFFECT_SENSOR  0
  #  define HAS_TOUCH_GPIO 1
# elif CONFIG_IDF_TARGET_ESP32C6 // ESP32-C6
  #  define HAS_HALL_EFFECT_SENSOR  0
  #  define HAS_TOUCH_GPIO  0
# elif CONFIG_IDF_TARGET_ESP32C3 // ESP32-C3
  #  define HAS_HALL_EFFECT_SENSOR  0
  #  define HAS_TOUCH_GPIO  0
# elif CONFIG_IDF_TARGET_ESP32C2 // ESP32-C2
  #  define HAS_HALL_EFFECT_SENSOR  0
  #  define HAS_TOUCH_GPIO  0
# elif CONFIG_IDF_TARGET_ESP32   // ESP32/PICO-D4
  #  if ESP_IDF_VERSION_MAJOR < 5
  #   define HAS_HALL_EFFECT_SENSOR  1
  #  else // if ESP_IDF_VERSION_MAJOR < 5

// Support for Hall Effect sensor was removed in ESP_IDF 5.x
  #   define HAS_HALL_EFFECT_SENSOR  0
  #  endif // if ESP_IDF_VERSION_MAJOR < 5
  #  define HAS_TOUCH_GPIO 1
# else // if CONFIG_IDF_TARGET_ESP32S3
  #  error Target CONFIG_IDF_TARGET is not supported
# endif // if CONFIG_IDF_TARGET_ESP32S3


# ifndef HAS_TOUCH_GPIO
#  define HAS_TOUCH_GPIO 0
# endif // ifndef HAS_TOUCH_GPIO


# if ESP_IDF_VERSION_MAJOR >= 5

#  include <esp_chip_info.h>
#  include <soc/soc.h>
#  include <driver/ledc.h>
#  include <esp_psram.h>

// #include <hal/ledc_hal.h>

# endif // if ESP_IDF_VERSION_MAJOR >= 5

# include "../Helpers/Hardware_ADC_cali.h"

#endif // ifdef ESP32


#if FEATURE_SD
# include <SD.h>
#endif // if FEATURE_SD


#include <SPI.h>


# define GPIO_PLUGIN_ID  1

/********************************************************************************************\
 * Initialize specific hardware settings (only global ones, others are set through devices)
 \*********************************************************************************************/
void hardwareInit()
{
  // set GPIO pins state if not set to default
  bool hasPullUp, hasPullDown;

  for (int gpio = 0; gpio <= MAX_GPIO; ++gpio) {
    const bool serialPinConflict = isSerialConsolePin(gpio);

    if (!serialPinConflict) {
      const uint32_t key = createKey(PLUGIN_GPIO, gpio);
      #ifdef ESP32
      checkAndClearPWM(key);
      #endif // ifdef ESP32

      if (getGpioPullResistor(gpio, hasPullUp, hasPullDown)) {
        PinBootState bootState = Settings.getPinBootState(gpio);
      #if FEATURE_ETHERNET
/*
        if (Settings.ETH_Pin_power_rst == gpio)
        {
                  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                    String log = F("ETH  : Reset ETH module on pin ");
                    log += Settings.ETH_Pin_power_rst;
                    addLog(LOG_LEVEL_INFO, log);
                  }
                  bootState = PinBootState::Output_low;
                }
         */
      #endif // if FEATURE_ETHERNET

        #ifdef ESP32
        if (bootState != PinBootState::Default_state) {
          gpio_reset_pin(static_cast<gpio_num_t>(gpio));
        }
        #endif
          
        switch (bootState)
        {
          case PinBootState::Default_state:
            // At startup, pins are configured as INPUT
            break;
          case PinBootState::Output_low:
            createAndSetPortStatus_Mode_State(key, PIN_MODE_OUTPUT, 0);
            GPIO_Write(PLUGIN_GPIO, gpio, LOW, PIN_MODE_OUTPUT);

            // setPinState(1, gpio, PIN_MODE_OUTPUT, LOW);
            break;
          case PinBootState::Output_high:
            createAndSetPortStatus_Mode_State(key, PIN_MODE_OUTPUT, 0);
            GPIO_Write(PLUGIN_GPIO, gpio, HIGH, PIN_MODE_OUTPUT);

            // setPinState(1, gpio, PIN_MODE_OUTPUT, HIGH);
            break;
          case PinBootState::Input_pullup:

            if (hasPullUp) {
              createAndSetPortStatus_Mode_State(key, PIN_MODE_INPUT_PULLUP, 0);
              pinMode(gpio, INPUT_PULLUP);
            }
            break;
          case PinBootState::Input_pulldown:

            if (hasPullDown) {
              createAndSetPortStatus_Mode_State(key, PIN_MODE_INPUT_PULLDOWN, 0);

              #ifdef ESP8266

              if (gpio == 16) {
                pinMode(gpio, INPUT_PULLDOWN_16);
              }
              #endif // ifdef ESP8266
              #ifdef ESP32
              pinMode(gpio, INPUT_PULLDOWN);
              #endif // ifdef ESP32
            }
            break;
          case PinBootState::Input:
            createAndSetPortStatus_Mode_State(key, PIN_MODE_INPUT, 0);
            pinMode(gpio, INPUT);
            break;
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

  #if FEATURE_PLUGIN_PRIORITY
  String dummy;
  PluginCall(PLUGIN_PRIORITY_INIT_ALL, nullptr, dummy);
  #endif // if FEATURE_PLUGIN_PRIORITY

  bool tryInitSPI = true;
#if FEATURE_ETHERNET
  if ((Settings.NetworkMedium == NetworkMedium_t::Ethernet) &&
      isValid(Settings.ETH_Phy_Type) && 
      isSPI_EthernetType(Settings.ETH_Phy_Type)) 
  {
#if !ETH_SPI_SUPPORTS_CUSTOM
      tryInitSPI = false;
#endif
  }
#endif


  // SPI Init
  bool SPI_initialized = false;
  if (tryInitSPI && Settings.isSPI_valid())
  {
    SPI.setHwCs(false);

    // MFD: for ESP32 enable the SPI on HSPI as the default is VSPI
    #ifdef ESP32

    const SPI_Options_e SPI_selection = static_cast<SPI_Options_e>(Settings.InitSPI);
    int8_t spi_gpios[3]               = {};

    if (Settings.getSPI_pins(spi_gpios)) {
      if (SPI_selection == SPI_Options_e::Vspi_Fspi) {
        SPI.begin(); // Default SPI bus
      } else {
        SPI.begin(spi_gpios[0], spi_gpios[1], spi_gpios[2]);
      }
      SPI_initialized = true;
    }
    #else // ifdef ESP32
    SPI.begin();
    SPI_initialized = true;
    #endif // ifdef ESP32
  }

  if (SPI_initialized)
  {
    addLog(LOG_LEVEL_INFO, F("INIT : SPI Init (without CS)"));
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
  } else {
    addLog(LOG_LEVEL_INFO, F("INIT : SPI not enabled"));
  }
}


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
      reboot(IntendedRebootReason_e::ResetFactoryPinActive);
    }
    factoryResetCounter = 0; // count was < 3, reset counter
  }
}

#ifdef ESP8266
int lastADCvalue = 0;

int espeasy_analogRead(int pin) {
  if (!WiFiEventData.wifiConnectInProgress) {
    #if FEATURE_ADC_VCC
      lastADCvalue = ESP.getVcc();
    #else
      lastADCvalue = analogRead(A0);
    #endif // if FEATURE_ADC_VCC
  }
  return lastADCvalue;
}

#endif // ifdef ESP8266

float mapADCtoFloat(float float_value,
                    float adc1,
                    float adc2,
                    float out1,
                    float out2)
{
  if (!approximatelyEqual(adc1, adc2))
  {
    const float normalized = (float_value - adc1) / (adc2 - adc1);
    float_value = normalized * (out2 - out1) + out1;
  }
  return float_value;
}


#ifdef ESP32

// ESP32 ADC calibration datatypes.


// FIXME TD-er: For now keep a local array of the adc calibration 
#if ESP_IDF_VERSION_MAJOR < 5
Hardware_ADC_cali_t ESP32_ADC_cali[ADC_ATTEN_MAX];
#else
Hardware_ADC_cali_t ESP32_ADC_cali[ADC_ATTENDB_MAX];
#endif


void initADC() {
  for (size_t atten = 0; atten < NR_ELEMENTS(ESP32_ADC_cali); ++atten) {
    if (!ESP32_ADC_cali[atten].initialized()) {
      // FIXME TD-er: For now fake some pin which is connected to ADC1
      #ifdef ESP32_CLASSIC
      const int pin = 36;
      #else 
      const int pin = 1;
      #endif
      ESP32_ADC_cali[atten].init(pin, static_cast<adc_atten_t>(atten));
    }
  }
}

float applyADCFactoryCalibration(float raw_value, adc_atten_t attenuation)
{
  if (attenuation < NR_ELEMENTS(ESP32_ADC_cali)) {
    return ESP32_ADC_cali[attenuation].applyFactoryCalibration(raw_value);
  }
  return raw_value;
}

bool hasADC_factory_calibration() {
  return ESP32_ADC_cali[0].useFactoryCalibration();
}

const __FlashStringHelper* getADC_factory_calibration_type()
{
  return ESP32_ADC_cali[0].getADC_factory_calibration_type();
}

float getADC_factory_calibrated_min(adc_atten_t attenuation)
{
  if (attenuation < NR_ELEMENTS(ESP32_ADC_cali)) {
    return ESP32_ADC_cali[attenuation].getMinOut();
  }
  return 0.0f;
}

float getADC_factory_calibrated_max(adc_atten_t attenuation)
{
  if (attenuation < NR_ELEMENTS(ESP32_ADC_cali)) {
    return ESP32_ADC_cali[attenuation].getMaxOut();
  }
  return MAX_ADC_VALUE;
}

int getADC_num_for_gpio(int pin) {
  int ch;

  return getADC_num_for_gpio(pin, ch);
}

int getADC_num_for_gpio(int pin, int& channel)
{
  int adc, t;

  if (getADC_gpio_info(pin, adc, channel, t)) {
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
      # if HAS_HALL_EFFECT_SENSOR
        value = hallRead();
      # endif // if HAS_HALL_EFFECT_SENSOR
        break;
      case 1:
        canread = true;
        break;
      case 2:
#if ESP_IDF_VERSION_MAJOR < 5
        if (WiFi.getMode() == WIFI_OFF) {
          // See:
          // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html#configuration-and-reading-adc
          // ADC2 is shared with WiFi, so don't read ADC2 when WiFi is on.
          canread = true;
        }
#else
        canread = true;
#endif
        break;
    }

    if (canread) {
      if (readAsTouch && (t >= 0)) {
        # if HAS_TOUCH_GPIO
        value = touchRead(pin);
        # endif // if HAS_TOUCH_GPIO
      } else {
        value = analogRead(pin);
      }
    }
  }
  return value;
}


int  getCPU_MaxFreqMHz()
{
#if CONFIG_IDF_TARGET_ESP32
    return static_cast<int>(efuse_hal_get_rated_freq_mhz());
#elif CONFIG_IDF_TARGET_ESP32C2
    return 120;
#elif CONFIG_IDF_TARGET_ESP32C3
    return 160;
#elif CONFIG_IDF_TARGET_ESP32C6
    return 160;
#elif CONFIG_IDF_TARGET_ESP32H2
    //IDF-6570
    return 96;
#elif CONFIG_IDF_TARGET_ESP32P4
    return 400;
#elif CONFIG_IDF_TARGET_ESP32S2
    return 240;
#elif CONFIG_IDF_TARGET_ESP32S3
    return 240;

#  else 
  #   error Target CONFIG_IDF_TARGET is not supported
  return 160;
#  endif
}

int  getCPU_MinFreqMHz()
{
  // TODO TD-er: May differ on some ESPs and also some allow less but only without WiFi
  return 80;
}


#endif // ifdef ESP32



/*********************************************************************************************\
* High entropy hardware random generator
* Thanks to DigitalAlchemist
\*********************************************************************************************/

#if ESP_IDF_VERSION_MAJOR >= 5
#include <esp_random.h>
#endif

uint32_t HwRandom() {
#if ESP_IDF_VERSION_MAJOR >= 5
  // See for more info on the HW RNG:
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/api-reference/system/random.html
  return esp_random();
#else

// Based on code from https://raw.githubusercontent.com/espressif/esp-idf/master/components/esp32/hw_random.c
// https://github.com/arendst/Tasmota/blob/1e6b78a957be538cf494f0e2dc49060d1cb0fe8b/tasmota/support_esp.ino#L805
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
#endif
}

long HwRandom(long howbig) {
    if(howbig == 0) {
        return 0;
    }
    return HwRandom() % howbig;
}

long HwRandom(long howsmall, long howbig) {
    if(howsmall >= howbig) {
        return howsmall;
    }
    long diff = howbig - howsmall;
    return HwRandom(diff) + howsmall;
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

  #ifdef ESP32S2

  switch (rtc_get_reset_reason(0)) {
    case NO_MEAN                : break;
    case POWERON_RESET          : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<1, Vbat power on reset*/
    case RTC_SW_SYS_RESET       : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<3, Software reset digital core*/
    case DEEPSLEEP_RESET        : lastBootCause = BOOT_CAUSE_DEEP_SLEEP;       break; /**<5, Deep Sleep reset digital core*/
    case TG0WDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<7, Timer Group0 Watch dog reset digital core*/
    case TG1WDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<8, Timer Group1 Watch dog reset digital core*/
    case RTCWDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<9, RTC Watch dog Reset digital core*/
    case INTRUSION_RESET        : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<10, Instrusion tested to reset CPU*/
    case TG0WDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<11, Time Group0 reset CPU*/
    case RTC_SW_CPU_RESET       : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<12, Software reset CPU*/
    case RTCWDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<13, RTC Watch dog Reset CPU*/
    case RTCWDT_BROWN_OUT_RESET : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<15, Reset when the vdd voltage is not stable*/
    case RTCWDT_RTC_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<16, RTC Watch dog reset digital core and rtc module*/
    case TG1WDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<17, Time Group1 reset CPU*/
    case SUPER_WDT_RESET        : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<18, super watchdog reset digital core and rtc module*/
    case GLITCH_RTC_RESET       : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<19, glitch reset digital core and rtc module*/
    case EFUSE_RESET            : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<20, efuse reset digital core*/
  }



#elif defined(ESP32S3)
  switch (rtc_get_reset_reason(0)) {
    case NO_MEAN                : break;
    case POWERON_RESET          : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<1, Vbat power on reset*/
    case RTC_SW_SYS_RESET       : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<3, Software reset digital core*/
    case DEEPSLEEP_RESET        : lastBootCause = BOOT_CAUSE_DEEP_SLEEP;       break; /**<5, Deep Sleep reset digital core*/
    case TG0WDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<7, Timer Group0 Watch dog reset digital core*/
    case TG1WDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<8, Timer Group1 Watch dog reset digital core*/
    case RTCWDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<9, RTC Watch dog Reset digital core*/
    case INTRUSION_RESET        : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<10, Instrusion tested to reset CPU*/
    case TG0WDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<11, Time Group0 reset CPU*/
    case RTC_SW_CPU_RESET       : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<12, Software reset CPU*/
    case RTCWDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<13, RTC Watch dog Reset CPU*/
    case RTCWDT_BROWN_OUT_RESET : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<15, Reset when the vdd voltage is not stable*/
    case RTCWDT_RTC_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<16, RTC Watch dog reset digital core and rtc module*/
    case TG1WDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<17, Time Group1 reset CPU*/
    case SUPER_WDT_RESET        : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<18, super watchdog reset digital core and rtc module*/
    case GLITCH_RTC_RESET       : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<19, glitch reset digital core and rtc module*/
    case EFUSE_RESET            : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<20, efuse reset digital core*/
    case USB_UART_CHIP_RESET    : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<21, usb uart reset digital core */
    case USB_JTAG_CHIP_RESET    : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<22, usb jtag reset digital core */
    case POWER_GLITCH_RESET     : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<23, power glitch reset digital core and rtc module*/
  }


#elif defined(ESP32C2)
  switch (rtc_get_reset_reason(0)) {
    case NO_MEAN                : break;
    case POWERON_RESET          : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<1, Vbat power on reset*/
    case RTC_SW_SYS_RESET       : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<3, Software reset digital core*/
    case DEEPSLEEP_RESET        : lastBootCause = BOOT_CAUSE_DEEP_SLEEP;       break; /**<3, Deep Sleep reset digital core*/
    case TG0WDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<7, Timer Group0 Watch dog reset digital core*/
    case RTCWDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<9, RTC Watch dog Reset digital core*/
    case INTRUSION_RESET        : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<10, Instrusion tested to reset CPU*/
    case TG0WDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<11, Time Group0 reset CPU*/
    case RTC_SW_CPU_RESET       : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<12, Software reset CPU*/
    case RTCWDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<13, RTC Watch dog Reset CPU*/
    case RTCWDT_BROWN_OUT_RESET : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<15, Reset when the vdd voltage is not stable*/
    case RTCWDT_RTC_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<16, RTC Watch dog reset digital core and rtc module*/
    case SUPER_WDT_RESET        : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<11, super watchdog reset digital core and rtc module*/
    case GLITCH_RTC_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<19, glitch reset digital core and rtc module*/
    case EFUSE_RESET            : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<20, efuse reset digital core*/
    case JTAG_RESET             : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<24, jtag reset CPU*/
  }


#elif defined(ESP32C3)
  switch (rtc_get_reset_reason(0)) {
    case NO_MEAN                : break;
    case POWERON_RESET          : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<1, Vbat power on reset*/
    case RTC_SW_SYS_RESET       : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<3, Software reset digital core*/
    case DEEPSLEEP_RESET        : lastBootCause = BOOT_CAUSE_DEEP_SLEEP;       break; /**<5, Deep Sleep reset digital core*/
    case TG0WDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<7, Timer Group0 Watch dog reset digital core*/
    case TG1WDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<8, Timer Group1 Watch dog reset digital core*/
    case RTCWDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<9, RTC Watch dog Reset digital core*/
    case INTRUSION_RESET        : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<10, Instrusion tested to reset CPU*/
    case TG0WDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<11, Time Group0 reset CPU*/
    case RTC_SW_CPU_RESET       : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<12, Software reset CPU*/
    case RTCWDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<13, RTC Watch dog Reset CPU*/
    case RTCWDT_BROWN_OUT_RESET : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<15, Reset when the vdd voltage is not stable*/
    case RTCWDT_RTC_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<16, RTC Watch dog reset digital core and rtc module*/
    case TG1WDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<17, Time Group1 reset CPU*/
    case SUPER_WDT_RESET        : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<18, super watchdog reset digital core and rtc module*/
    case GLITCH_RTC_RESET       : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<19, glitch reset digital core and rtc module*/
    case EFUSE_RESET            : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<20, efuse reset digital core*/
    case USB_UART_CHIP_RESET    : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<21, usb uart reset digital core */
    case USB_JTAG_CHIP_RESET    : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<22, usb jtag reset digital core */
    case POWER_GLITCH_RESET     : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<23, power glitch reset digital core and rtc module*/
  }

#elif defined(ESP32C6)
  switch (rtc_get_reset_reason(0)) {
    case NO_MEAN                : break;
    case POWERON_RESET          : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<1, Vbat power on reset*/
    case RTC_SW_SYS_RESET       : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<3, Software reset digital core*/
    case DEEPSLEEP_RESET        : lastBootCause = BOOT_CAUSE_DEEP_SLEEP;       break; /**<5, Deep Sleep reset digital core*/
    case SDIO_RESET             : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<6, Reset by SLC module, reset digital core (hp system)*/
    case TG0WDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<7, Timer Group0 Watch dog reset digital core*/
    case TG1WDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<8, Timer Group1 Watch dog reset digital core*/
    case RTCWDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<9, RTC Watch dog Reset digital core*/
    case TG0WDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<11, Time Group0 reset CPU*/
    case RTC_SW_CPU_RESET       : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<12, Software reset CPU*/
    case RTCWDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<13, RTC Watch dog Reset CPU*/
    case RTCWDT_BROWN_OUT_RESET : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<15, Reset when the vdd voltage is not stable*/
    case RTCWDT_RTC_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<16, RTC Watch dog reset digital core and rtc module*/
    case TG1WDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<17, Time Group1 reset CPU*/
    case SUPER_WDT_RESET        : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<18, super watchdog reset digital core and rtc module*/
    case EFUSE_RESET            : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<20, efuse reset digital core*/
    case USB_UART_CHIP_RESET    : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<21, usb uart reset digital core */
    case USB_JTAG_CHIP_RESET    : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<22, usb jtag reset digital core */
    case JTAG_RESET             : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<24, jtag reset CPU*/
  }

# elif defined(ESP32_CLASSIC)
  switch (rtc_get_reset_reason(0)) {
    case NO_MEAN                : break;
    case POWERON_RESET          : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<1, Vbat power on reset*/
    case SW_RESET               : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<3, Software reset digital core*/
    case OWDT_RESET             : lastBootCause = BOOT_CAUSE_SW_WATCHDOG;      break; /**<4, Legacy watch dog reset digital core*/
    case DEEPSLEEP_RESET        : lastBootCause = BOOT_CAUSE_DEEP_SLEEP;       break; /**<3, Deep Sleep reset digital core*/
    case SDIO_RESET             : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<6, Reset by SLC module, reset digital core*/
    case TG0WDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<7, Timer Group0 Watch dog reset digital core*/
    case TG1WDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<8, Timer Group1 Watch dog reset digital core*/
    case RTCWDT_SYS_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<9, RTC Watch dog Reset digital core*/
    case INTRUSION_RESET        : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<10, Instrusion tested to reset CPU*/
    case TGWDT_CPU_RESET        : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<11, Time Group reset CPU*/
    case SW_CPU_RESET           : lastBootCause = BOOT_CAUSE_SOFT_RESTART;     break; /**<12, Software reset CPU*/
    case RTCWDT_CPU_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<13, RTC Watch dog Reset CPU*/
    case EXT_CPU_RESET          : lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;    break; /**<14, for APP CPU, reseted by PRO CPU*/
    case RTCWDT_BROWN_OUT_RESET : lastBootCause = BOOT_CAUSE_POWER_UNSTABLE;   break; /**<15, Reset when the vdd voltage is not stable*/
    case RTCWDT_RTC_RESET       : lastBootCause = BOOT_CAUSE_EXT_WD;           break; /**<16, RTC Watch dog reset digital core and rtc module*/
  }

  # else

    static_assert(false, "Implement processor architecture");

  #endif
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
# if CONFIG_ETH_USE_ESP32_EMAC
    case DeviceModel::DeviceModel_Olimex_ESP32_PoE:
    case DeviceModel::DeviceModel_Olimex_ESP32_EVB:
    case DeviceModel::DeviceModel_Olimex_ESP32_GATEWAY:
    #ifdef ESP32_CLASSIC
      return F("Olimex");
    #endif // ifdef ESP32_CLASSIC
    case DeviceModel::DeviceModel_wESP32:
    #ifdef ESP32_CLASSIC
      return F("wESP32");
    #endif // ifdef ESP32_CLASSIC
    case DeviceModel::DeviceModel_WT32_ETH01:
    #ifdef ESP32_CLASSIC
      return F("WT32-ETH01");
    #endif // ifdef ESP32_CLASSIC
#endif
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
#if CONFIG_ETH_USE_ESP32_EMAC
    case DeviceModel::DeviceModel_Olimex_ESP32_PoE:      return F(" ESP32-PoE");
    case DeviceModel::DeviceModel_Olimex_ESP32_EVB:      return F(" ESP32-EVB");
    case DeviceModel::DeviceModel_Olimex_ESP32_GATEWAY:  return F(" ESP32-GATEWAY");
    case DeviceModel::DeviceModel_wESP32:                break;
    case DeviceModel::DeviceModel_WT32_ETH01:            return F(" add-on");
#endif // if CONFIG_ETH_USE_ESP32_EMAC

    case DeviceModel::DeviceModel_default:
    case DeviceModel::DeviceModel_MAX:             return F("default");

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("");
}

String getDeviceModelString(DeviceModel model) {
  return concat(
    getDeviceModelBrandString(model),
    getDeviceModelTypeString(model));
}

bool modelMatchingFlashSize(DeviceModel model) {
#if defined(ESP8266) || (defined(ESP32_CLASSIC) && FEATURE_ETHERNET)
  const uint32_t size_MB = getFlashRealSizeInBytes() >> 20;
#endif // if defined(ESP8266) || (defined(ESP32_CLASSIC) && FEATURE_ETHERNET)

  // TD-er: This also checks for ESP8266/ESP8285/ESP32_CLASSIC
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
#if CONFIG_ETH_USE_ESP32_EMAC
    case DeviceModel::DeviceModel_Olimex_ESP32_PoE:
    case DeviceModel::DeviceModel_Olimex_ESP32_EVB:
    case DeviceModel::DeviceModel_Olimex_ESP32_GATEWAY:
    case DeviceModel::DeviceModel_wESP32:
    case DeviceModel::DeviceModel_WT32_ETH01:
# if  defined(ESP32_CLASSIC) && FEATURE_ETHERNET
      return size_MB == 4;
# else // if  defined(ESP32_CLASSIC) && FEATURE_ETHERNET
      return false;
# endif // if  defined(ESP32_CLASSIC) && FEATURE_ETHERNET
#endif // if CONFIG_ETH_USE_ESP32_EMAC
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
  setTaskDevice_to_TaskIndex(PLUGIN_GPIO, taskIndex);
  const int pins[] = { gpio, -1, -1 };

  setBasicTaskValues(
    taskIndex,
    0,    // taskdevicetimer
    true, // enabled
    name, // name
    pins);
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

  Settings.TaskDeviceNumber[taskIndex] = taskdevicenumber.value;

  //  Settings.getPluginID_for_task(taskIndex) = taskdevicenumber;

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
                        bool enabled, const String& name, const int pins[3]) {
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
  //Settings.TaskDeviceEnabled[taskIndex].enabled = enabled;
  safe_strncpy(ExtraTaskSettings.TaskDeviceName, name.c_str(), sizeof(ExtraTaskSettings.TaskDeviceName));

  // FIXME TD-er: Check for valid GPIO pin (and  -1 for "not set")
  Settings.TaskDevicePin1[taskIndex] = pins[0];
  Settings.TaskDevicePin2[taskIndex] = pins[1];
  Settings.TaskDevicePin3[taskIndex] = pins[2];
}

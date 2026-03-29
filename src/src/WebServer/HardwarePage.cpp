#include "../WebServer/HardwarePage.h"

#ifdef WEBSERVER_HARDWARE

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../CustomBuild/ESPEasyLimits.h"

#include "../DataStructs/DeviceStruct.h"

#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware_GPIO.h"
#include "../Helpers/I2C_access.h"
#include "../Helpers/SPI_Helper.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"

# ifdef FEATURE_PIN_WAKEUP
#include "../../_Plugin_Helper.h"
#include "driver/rtc_io.h"
# endif 

// ********************************************************************************
// Web Interface hardware page
// ********************************************************************************
void handle_hardware() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_hardware"));
  #endif

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_HARDWARE;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  if (isFormItem(F("pled"))) {
    String error;
    Settings.Pin_status_led           = getFormItemInt(F("pled"));
    Settings.Pin_status_led_Inversed  = isFormItemChecked(F("pledi"));
    Settings.Pin_Reset                = getFormItemInt(F("pres"));
    #if FEATURE_I2C_MULTIPLE
    if (isFormItem(F("pi2cbuspcf"))) {
      set3BitToUL(Settings.I2C_peripheral_bus, I2C_PERIPHERAL_BUS_PCFMCP, getFormItemInt(F("pi2cbuspcf")));
    }
    #endif // if FEATURE_I2C_MULTIPLE
    #if defined(ESP32) && FEATURE_SD
    Settings.setSPIBusForSDCard(getFormItemInt(F("sdspibus"), 0));
    #endif // if defined(ESP32) && FEATURE_SD
    Settings.Pin_sd_cs                = getFormItemInt(F("sd"));
    int gpio = 0;

    while (gpio <= MAX_GPIO) {
      if (isSerialConsolePin(gpio)) {
        // do not add the pin state select for these pins.
      } else {
        if (validGpio(gpio)) {
          String int_pinlabel('p');
          int_pinlabel       += gpio;
          Settings.setPinBootState(gpio, static_cast<PinBootState>(getFormItemInt(int_pinlabel)));
        }
      }
      ++gpio;
    }
    
    #  if FEATURE_PIN_WAKEUP
    gpio = 0;
    uint64_t wakeGpioMask = 0;
    Settings.wakeOnHigh() = isFormItemChecked(F("WoHi")); // Wake on HIGH or LOW
    while (gpio <= MAX_GPIO) {
      if (esp_sleep_is_valid_wakeup_gpio((gpio_num_t)gpio)) {
        char checkboxId[8]; // "WoL" + max 2 digits + null terminator
        snprintf(checkboxId, sizeof(checkboxId), "WoL%d", gpio);
         // if the checkbox is checked, set the corresponding bit in wakeMask
        if (isFormItemChecked(checkboxId)) {
          wakeGpioMask |= (1ULL << gpio);
        }
      }
      ++gpio;
    }
    Settings.setWakeGpioMask(wakeGpioMask); // save the bitmask
    setupGpioWakeup(wakeGpioMask); //attach gpios for wakeup
    #  endif // if FEATURE_PIN_WAKEUP

    error += SaveSettings();
    addHtmlError(error);
  }

  addHtml(F("<form  method='post'>"));
  html_table_class_normal();
  addFormHeader(F("Hardware Settings"), F(""), F("Hardware/Hardware.html"));

  addFormSubHeader(F("Wifi Status LED"));
  addFormPinSelect(PinSelectPurpose::Status_led, formatGpioName_output(F("LED")), F("pled"), Settings.Pin_status_led);
  addFormCheckBox(F("Inversed LED"), F("pledi"), Settings.Pin_status_led_Inversed);
  addFormNote(F("Use &rsquo;GPIO-2"
#ifdef ESP8266
    " (D4)"
#endif
    "&rsquo; with &rsquo;Inversed&rsquo; checked for onboard LED"));

  addFormSubHeader(F("Reset Pin"));
  addFormPinSelect(PinSelectPurpose::Reset_pin, formatGpioName_input(F("Switch")), F("pres"), Settings.Pin_Reset);
  addFormNote(F("Press about 10s for factory reset"));

# if FEATURE_I2C_MULTIPLE
  const uint8_t i2cMaxBusCount = Settings.getNrConfiguredI2C_buses();

  if (i2cMaxBusCount > 1) {
    addFormSubHeader(F("PCF &amp; MCP Direct I/O"));
    const uint8_t i2cBus = Settings.getI2CInterfacePCFMCP();
    I2CInterfaceSelector(F("I2C Bus"),
                         F("pi2cbuspcf"),
                         i2cBus,
                         false);

  }
# endif // if FEATURE_I2C_MULTIPLE

#if FEATURE_SD
  addFormSubHeader(F("SD Card"));
  #ifdef ESP32
  if (getSPIBusCount() > 1 && (Settings.getNrConfiguredSPI_buses() != 0)) {
    uint8_t spiBus = Settings.getSPIBusForSDCard();
    SPIInterfaceSelector(F("SPI Bus"),
                        F("sdspibus"),
                        spiBus);
  }
  #endif // ifdef ESP32
  addFormPinSelect(PinSelectPurpose::SD_Card, formatGpioName_output(F("SD Card CS")), F("sd"), Settings.Pin_sd_cs);
#endif // if FEATURE_SD

  addFormSubHeader(F("GPIO boot states"));
  addFormDetailsStart(0);
  
  for (int gpio = 0; gpio <= MAX_GPIO; ++gpio) {
    addFormPinStateSelect(gpio, static_cast<int>(Settings.getPinBootState(gpio)));
  }
  addFormDetailsEnd();

 #  if FEATURE_PIN_WAKEUP
  #if FEATURE_PIN_WAKEUP == 1
  addFormSubHeader(F("EXT1 Wake-up Pins"));
  #else
  addFormSubHeader(F("GPIO Wake-up"));
  #endif
  addFormDetailsStart(0);
  addFormCheckBox(F("Wake on HIGH"), F("WoHi"), Settings.wakeOnHigh());
  #if FEATURE_PIN_WAKEUP == 1
  addFormNote(F("(default: Wake on LOW) Add an external Pull-Resistor if needed!"));
  #else
  addFormNote(F("(default: Wake on LOW) No external pull-up/down resistors are needed"));
  #endif
  for (int gpio = 0; gpio <= MAX_GPIO; ++gpio) {
    addFormPinWakeSelect(gpio, Settings.getWakeGpioMask());
  }
  addFormDetailsEnd();
  #  endif // if FEATURE_PIN_WAKEUP

  addFormSeparator(2);

  html_TR_TD();
  html_TD();
  addSubmitButton();
  html_TR_TD();
  html_end_table();
  html_end_form();

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // ifdef WEBSERVER_HARDWARE

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
#include "../Helpers/SPI_Helper.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"


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

  for (int gpio = 0; gpio <= MAX_GPIO; ++gpio) {
    addFormPinStateSelect(gpio, static_cast<int>(Settings.getPinBootState(gpio)));
  }
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

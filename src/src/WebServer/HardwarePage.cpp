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
//#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"


#if FEATURE_EEPROM_EXTERNAL
#include "../../ESPEasy/eeprom/Helpers/EEPROMExternal.h"
#include "../WebServer/DevicesPage.h" // For using ShowI2CMultiplexerUI() and GetI2CMultiplexerFromPage()
#endif // if FEATURE_EEPROM_EXTERNAL

// ********************************************************************************
// Web Interface hardware page
// ********************************************************************************
void handle_hardware() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_hardware"));
  #endif

  if (!startStream_send_stdTemplate(MENU_INDEX_HARDWARE)) { return; }

  html_add_form();

  if (isFormItem(F("pled"))) {
    String error;
    Settings.Pin_status_led           = getFormItemInt(F("pled"));
    Settings.Pin_status_led_Inversed  = isFormItemChecked(F("pledi"));
    Settings.Pin_Reset                = getFormItemInt(F("pres"));
    #if FEATURE_I2C_MULTIPLE
    if (isFormItem(F("pi2cbuspcf"))) {
      set3BitToUL(Settings.I2C_peripheral_bus, I2C_PERIPHERAL_BUS_PCFMCP, getFormItemInt(F("pi2cbuspcf")));
    }
    // EEPROM settings
    # if FEATURE_EEPROM_EXTERNAL
    const uint8_t i2cBus = getFormItemInt(F("pi2cbuseeprom"), 0);
    set3BitToUL(Settings.I2C_peripheral_bus, I2C_PERIPHERAL_BUS_EEPROM, i2cBus);
    # endif // if FEATURE_EEPROM_EXTERNAL

    #if FEATURE_I2CMULTIPLEXER && !FEATURE_I2C_MULTIPLE && FEATURE_EEPROM_EXTERNAL
    constexpr uint8_t i2cBus = 0;
    #endif // if FEATURE_I2CMULTIPLEXER && !FEATURE_I2C_MULTIPLE && FEATURE_EEPROM_EXTERNAL

    #if FEATURE_EEPROM_EXTERNAL
    Settings.EEPROMExternalType(getFormItemInt(F("eepromtype"),
                                static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C256)));
    Settings.EEPROMExternalI2CAddress(getFormItemInt(F("i2c_eeprom"), 0));

    # if FEATURE_I2CMULTIPLEXER

    bool muxPortsOption{};
    int selectedPorts{};
    GetI2CMultiplexerFromPage(i2cBus, muxPortsOption, selectedPorts);
    uint16_t muxFlags{};
    bitWrite(muxFlags, EEPROM_MUX_FLAGS_MULTI, muxPortsOption);
    set8BitToUL(muxFlags, EEPROM_MUX_FLAGS_PORT, selectedPorts);
    Settings.EEPROMExternalI2CMultiplexerFlags(muxFlags);
    # endif // if FEATURE_I2CMULTIPLEXER

    #endif // if FEATURE_EEPROM_EXTERNAL

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

  #if FEATURE_EEPROM_EXTERNAL
  {
    addFormSubHeader(F("External I2C EEPROM"));
    const __FlashStringHelper*eepromOptions[] = {
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C256),
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C512),
      #if EEPROM_SUPPORT_AT24C1024
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C1024),
      #endif // if EEPROM_SUPPORT_AT24C1024
      #if EEPROM_SUPPORT_AT24C2048
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C2048),
      #endif // if EEPROM_SUPPORT_AT24C2048
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C32),
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C64),
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C128),
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC256),
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC512),
      #if EEPROM_SUPPORT_AT24C1024
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC1M),
      #endif // if EEPROM_SUPPORT_AT24C1024
      #if EEPROM_SUPPORT_AT24C2048
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC2M),
      #endif // if EEPROM_SUPPORT_AT24C2048
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC32),
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC64),
      getEEPROMName(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC128),
    };
    const int eepromTypes[] = {
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C256),
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C512),
      #if EEPROM_SUPPORT_AT24C1024
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C1024),
      #endif // if EEPROM_SUPPORT_AT24C1024
      #if EEPROM_SUPPORT_AT24C2048
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C2048),
      #endif // if EEPROM_SUPPORT_AT24C2048
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C32),
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C64),
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::AT24C128),
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC256),
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC512),
      #if EEPROM_SUPPORT_AT24C1024
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC1M),
      #endif // if EEPROM_SUPPORT_AT24C1024
      #if EEPROM_SUPPORT_AT24C2048
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC2M),
      #endif // if EEPROM_SUPPORT_AT24C2048
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC32),
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC64),
      static_cast<int>(ESPEasy::eeprom::EEPROMExternal_Type_e::MB85RC128),
    };
    constexpr uint8_t eepromSizeCount = NR_ELEMENTS(eepromTypes);
    FormSelectorOptions eepromSizeSelector(eepromSizeCount, eepromOptions, eepromTypes);
    eepromSizeSelector.addFormSelector(F("EEPROM Model/size"), F("eepromtype"), Settings.EEPROMExternalType());

    const uint8_t i2cAddressValues[] = { 0, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57 };
    constexpr int nrAddressOptions   = NR_ELEMENTS(i2cAddressValues);

    addFormSelectorI2C(F("i2c_eeprom"), nrAddressOptions, i2cAddressValues, Settings.EEPROMExternalI2CAddress());

    #if FEATURE_I2C_MULTIPLE
    const uint8_t i2cBus = Settings.getI2CInterfaceEEPROM();
    if (i2cMaxBusCount > 1) {
      I2CInterfaceSelector(F("I2C Bus"),
                          F("pi2cbuseeprom"),
                          i2cBus,
                          false);

    }
    #endif // if FEATURE_I2C_MULTIPLE

    #if FEATURE_I2CMULTIPLEXER && !FEATURE_I2C_MULTIPLE && FEATURE_EEPROM_EXTERNAL
    constexpr uint8_t i2cBus = 0;
    #endif // if FEATURE_I2CMULTIPLEXER && !FEATURE_I2C_MULTIPLE && FEATURE_EEPROM_EXTERNAL

    #if FEATURE_I2CMULTIPLEXER
    const uint16_t eepromMux = Settings.EEPROMExternalI2CMultiplexerFlags();
    ShowI2CMultiplexerUI(i2cBus,
                         bitRead(eepromMux, EEPROM_MUX_FLAGS_MULTI),
                         get8BitFromUL(eepromMux, EEPROM_MUX_FLAGS_PORT)); // Re-used from DevicesPage
    #endif // if FEATURE_I2CMULTIPLEXER

    const bool eepromChecked = ESPEasy::eeprom::checkEEPROMEnabled() > 0;
    addRowLabel(F("EEPROM Enabled"));
    addEnabled(eepromChecked);
    if (eepromChecked && ESPEasy::eeprom::isEEPROMExternalWriteProtected()) {
      addHtml(F(" Write-protected!"));
    } 
    if (eepromChecked && !ESPEasy::eeprom::isEEPROMExternalWriteProtected()) {
      addRowLabel(F("'WriteEE' slots available"));
      addHtmlInt(ESPEasy::eeprom::getEEPROMMaxSlots());
    }
  }
  #endif // if FEATURE_EEPROM_EXTERNAL

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

  sendTail_stdtemplate();
}

#endif // ifdef WEBSERVER_HARDWARE

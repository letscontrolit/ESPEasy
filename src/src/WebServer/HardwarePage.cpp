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
#include "../Helpers/Hardware.h"
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
    #if FEATURE_PLUGIN_PRIORITY
    if (!isI2CPriorityTaskActive())
    #endif //if FEATURE_PLUGIN_PRIORITY
    {
      Settings.Pin_i2c_sda            = getFormItemInt(F("psda"));
      Settings.Pin_i2c_scl            = getFormItemInt(F("pscl"));
    }
    Settings.I2C_clockSpeed           = getFormItemInt(F("pi2csp"), DEFAULT_I2C_CLOCK_SPEED);
    Settings.I2C_clockSpeed_Slow      = getFormItemInt(F("pi2cspslow"), DEFAULT_I2C_CLOCK_SPEED_SLOW);
    #if FEATURE_I2CMULTIPLEXER
    Settings.I2C_Multiplexer_Type     = getFormItemInt(F("pi2cmuxtype"));
    if (Settings.I2C_Multiplexer_Type != I2C_MULTIPLEXER_NONE) {
      Settings.I2C_Multiplexer_Addr   = getFormItemInt(F("pi2cmuxaddr"));
    } else {
      Settings.I2C_Multiplexer_Addr   = -1;
    }
    Settings.I2C_Multiplexer_ResetPin = getFormItemInt(F("pi2cmuxreset"));
    #endif // if FEATURE_I2CMULTIPLEXER
    #ifdef ESP32
      Settings.InitSPI                = getFormItemInt(F("initspi"), static_cast<int>(SPI_Options_e::None));
      if (Settings.InitSPI == static_cast<int>(SPI_Options_e::UserDefined)) { // User-define SPI GPIO pins
        Settings.SPI_SCLK_pin         = getFormItemInt(F("spipinsclk"), -1);
        Settings.SPI_MISO_pin         = getFormItemInt(F("spipinmiso"), -1);
        Settings.SPI_MOSI_pin         = getFormItemInt(F("spipinmosi"), -1);
        if (!Settings.isSPI_valid()) { // Checks
          error += F("User-defined SPI pins not configured correctly!\n");
        }
      }
    #else //for ESP8266 we keep the old UI
      Settings.InitSPI                = isFormItemChecked(F("initspi")); // SPI Init
    #endif
    Settings.Pin_sd_cs                = getFormItemInt(F("sd"));
    #if FEATURE_ETHERNET
    Settings.ETH_Phy_Addr             = getFormItemInt(F("ethphy"));
    Settings.ETH_Pin_mdc              = getFormItemInt(F("ethmdc"));
    Settings.ETH_Pin_mdio             = getFormItemInt(F("ethmdio"));
    Settings.ETH_Pin_power            = getFormItemInt(F("ethpower"));
    Settings.ETH_Phy_Type             = static_cast<EthPhyType_t>(getFormItemInt(F("ethtype")));
    Settings.ETH_Clock_Mode           = static_cast<EthClockMode_t>(getFormItemInt(F("ethclock")));
    Settings.NetworkMedium            = static_cast<NetworkMedium_t>(getFormItemInt(F("ethwifi")));
    #endif // if FEATURE_ETHERNET
    int gpio = 0;

    while (gpio <= MAX_GPIO) {
      if (Settings.UseSerial && ((gpio == 1) || (gpio == 3))) {
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
    if (error.isEmpty()) {
      // Apply I2C settings.
      initI2C();
    }
  }

  addHtml(F("<form  method='post'>"));
  html_table_class_normal();
  addFormHeader(F("Hardware Settings"), F("ESPEasy#Hardware_page"), F("Hardware/Hardware.html"));

  addFormSubHeader(F("Wifi Status LED"));
  addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output(F("LED")), F("pled"), Settings.Pin_status_led);
  addFormCheckBox(F("Inversed LED"), F("pledi"), Settings.Pin_status_led_Inversed);
  addFormNote(F("Use &rsquo;GPIO-2 (D4)&rsquo; with &rsquo;Inversed&rsquo; checked for onboard LED"));

  addFormSubHeader(F("Reset Pin"));
  addFormPinSelect(PinSelectPurpose::Generic_input, formatGpioName_input(F("Switch")), F("pres"), Settings.Pin_Reset);
  addFormNote(F("Press about 10s for factory reset"));

  addFormSubHeader(F("I2C Interface"));
  #if FEATURE_PLUGIN_PRIORITY
  if (isI2CPriorityTaskActive()) {
    int  pinnr = -1;
    bool input, output, warning = false;
    addFormNote(F("I2C GPIO pins can't be changed when an I2C Priority task is configured."));
    addRowLabel(formatGpioName_bidirectional(F("SDA")));
    getGpioInfo(Settings.Pin_i2c_sda, pinnr, input, output, warning);
    addHtml(createGPIO_label(Settings.Pin_i2c_sda, pinnr, true, true, false));
    addRowLabel(formatGpioName_output(F("SCL")));
    getGpioInfo(Settings.Pin_i2c_scl, pinnr, input, output, warning);
    addHtml(createGPIO_label(Settings.Pin_i2c_scl, pinnr, true, true, false));
  } else
  #endif // if FEATURE_PLUGIN_PRIORITY
  {
    addFormPinSelectI2C(formatGpioName_bidirectional(F("SDA")), F("psda"), Settings.Pin_i2c_sda);
    addFormPinSelectI2C(formatGpioName_output(F("SCL")),        F("pscl"), Settings.Pin_i2c_scl);
  }
  addFormNumericBox(F("Clock Speed"), F("pi2csp"), Settings.I2C_clockSpeed, 100, 3400000);
  addUnit(F("Hz"));
  addFormNote(F("Use 100 kHz for old I2C devices, 400 kHz is max for most."));
  addFormNumericBox(F("Slow device Clock Speed"), F("pi2cspslow"), Settings.I2C_clockSpeed_Slow, 100, 3400000);
  addUnit(F("Hz"));
  #if FEATURE_I2CMULTIPLEXER
  addFormSubHeader(F("I2C Multiplexer"));
  // Select the type of multiplexer to use
  {
    # define I2C_MULTIPLEXER_OPTIONCOUNT  5 // Nr. of supported devices + 'None'
    const __FlashStringHelper *i2c_muxtype_options[] = {
      F("- None -"),
      F("TCA9548a - 8 channel"),
      F("TCA9546a - 4 channel"),
      F("TCA9543a - 2 channel"),
      F("PCA9540 - 2 channel (experimental)")
    };
    const int i2c_muxtype_choices[] = {
      -1,
      I2C_MULTIPLEXER_TCA9548A,
      I2C_MULTIPLEXER_TCA9546A,
      I2C_MULTIPLEXER_TCA9543A,
      I2C_MULTIPLEXER_PCA9540
    };
    addFormSelector(F("I2C Multiplexer type"), F("pi2cmuxtype"), I2C_MULTIPLEXER_OPTIONCOUNT,
                    i2c_muxtype_options, i2c_muxtype_choices, Settings.I2C_Multiplexer_Type);
  }
  // Select the I2C address for a port multiplexer
  {
    String  i2c_mux_options[9];
    int     i2c_mux_choices[9];
    uint8_t mux_opt = 0;
    i2c_mux_options[mux_opt] = F("- None -");
    i2c_mux_choices[mux_opt] = I2C_MULTIPLEXER_NONE;
    for (int8_t x = 0; x < 8; x++) {
      mux_opt++;
      i2c_mux_options[mux_opt] = formatToHex_decimal(0x70 + x);
      if (x == 0) { // PCA9540 has a fixed address 0f 0x70
        i2c_mux_options[mux_opt] += F(" [TCA9543a/6a/8a, PCA9540]");
      } else if (x < 4) {
        i2c_mux_options[mux_opt] += F(" [TCA9543a/6a/8a]");
      } else {
        i2c_mux_options[mux_opt] += F(" [TCA9546a/8a]");
      }
      i2c_mux_choices[mux_opt] = 0x70 + x;
    }
    addFormSelector(F("I2C Multiplexer address"), F("pi2cmuxaddr"), mux_opt + 1, i2c_mux_options, i2c_mux_choices, Settings.I2C_Multiplexer_Addr);
  }
  addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("Reset")), F("pi2cmuxreset"), Settings.I2C_Multiplexer_ResetPin);
  addFormNote(F("Will be pulled low to force a reset. Reset is not available on PCA9540."));
  #endif // if FEATURE_I2CMULTIPLEXER

  // SPI Init
  addFormSubHeader(F("SPI Interface"));
  #ifdef ESP32
  {
    // Script to show GPIO pins for User-defined SPI GPIOs
    // html_add_script(F("function spiOptionChanged(elem) {var spipinstyle = elem.value == 9 ? '' : 'none';document.getElementById('tr_spipinsclk').style.display = spipinstyle;document.getElementById('tr_spipinmiso').style.display = spipinstyle;document.getElementById('tr_spipinmosi').style.display = spipinstyle;}"),
    // Minified:
    html_add_script(F("function spiOptionChanged(e){var i=9==e.value?'':'none';"
                      "document.getElementById('tr_spipinsclk').style.display=i,"
                      "document.getElementById('tr_spipinmiso').style.display=i,"
                      "document.getElementById('tr_spipinmosi').style.display=i}"),
                    false);
    const __FlashStringHelper * spi_options[] = {
      getSPI_optionToString(SPI_Options_e::None), 
      getSPI_optionToString(SPI_Options_e::Vspi), 
      getSPI_optionToString(SPI_Options_e::Hspi), 
      getSPI_optionToString(SPI_Options_e::UserDefined)};
    const int spi_index[] = {
      static_cast<int>(SPI_Options_e::None),
      static_cast<int>(SPI_Options_e::Vspi),
      static_cast<int>(SPI_Options_e::Hspi),
      static_cast<int>(SPI_Options_e::UserDefined)
    };
    addFormSelector_script(F("Init SPI"), F("initspi"), 4, spi_options, spi_index, nullptr, Settings.InitSPI, F("spiOptionChanged(this)"));
    // User-defined pins
    addFormPinSelect(PinSelectPurpose::SPI, formatGpioName_output(F("CLK")),  F("spipinsclk"), Settings.SPI_SCLK_pin);
    addFormPinSelect(PinSelectPurpose::SPI_MISO, formatGpioName_input(F("MISO")),  F("spipinmiso"), Settings.SPI_MISO_pin);
    addFormPinSelect(PinSelectPurpose::SPI, formatGpioName_output(F("MOSI")), F("spipinmosi"), Settings.SPI_MOSI_pin);
    html_add_script(F("document.getElementById('initspi').onchange();"), false); // Initial trigger onchange script
    addFormNote(F("Changing SPI settings requires to press the hardware-reset button or power off-on!"));
  }
  #else //for ESP8266 we keep the existing UI
  addFormCheckBox(F("Init SPI"), F("initspi"), Settings.InitSPI > static_cast<int>(SPI_Options_e::None));
  addFormNote(F("CLK=GPIO-14 (D5), MISO=GPIO-12 (D6), MOSI=GPIO-13 (D7)"));
  #endif
  addFormNote(F("Chip Select (CS) config must be done in the plugin"));
  
#if FEATURE_SD
  addFormSubHeader(F("SD Card"));
  addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output(F("SD Card CS")), F("sd"), Settings.Pin_sd_cs);
#endif // if FEATURE_SD
  
#if FEATURE_ETHERNET
  addFormSubHeader(F("Ethernet"));
  addRowLabel_tr_id(F("Preferred network medium"), F("ethwifi"));
  {
    const __FlashStringHelper * ethWifiOptions[2] = {
      toString(NetworkMedium_t::WIFI), 
      toString(NetworkMedium_t::Ethernet) 
      };
    addSelector(F("ethwifi"), 2, ethWifiOptions, nullptr, nullptr, static_cast<int>(Settings.NetworkMedium), false, true);
  }
  addFormNote(F("Change Switch between WiFi and Ethernet requires reboot to activate"));
  addRowLabel_tr_id(F("Ethernet PHY type"), F("ethtype"));
  {
  #if ESP_IDF_VERSION_MAJOR > 3
    const uint32_t nrItems = 5;
  #else
    const uint32_t nrItems = 2;
  #endif
    const __FlashStringHelper * ethPhyTypes[nrItems] = { 
      toString(EthPhyType_t::LAN8710), 
      toString(EthPhyType_t::TLK110)
  #if ESP_IDF_VERSION_MAJOR > 3
      ,
      toString(EthPhyType_t::RTL8201),
      toString(EthPhyType_t::DP83848),
      toString(EthPhyType_t::DM9051) 
  #endif
      };
    const int ethPhyTypes_index[] = {
      static_cast<int>(EthPhyType_t::LAN8710),
      static_cast<int>(EthPhyType_t::TLK110)
  #if ESP_IDF_VERSION_MAJOR > 3
      ,
      static_cast<int>(EthPhyType_t::RTL8201),
      static_cast<int>(EthPhyType_t::DP83848),
      static_cast<int>(EthPhyType_t::DM9051)
  #endif
    };

    addSelector(F("ethtype"), nrItems, ethPhyTypes, ethPhyTypes_index, nullptr, static_cast<int>(Settings.ETH_Phy_Type), false, true);
  }
  addFormNumericBox(F("Ethernet PHY Address"), F("ethphy"), Settings.ETH_Phy_Addr, -1, 127);
  addFormNote(F("I&sup2;C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110, -1 autodetect)"));
  addFormPinSelect(PinSelectPurpose::Ethernet, formatGpioName_output(F("Ethernet MDC pin")), F("ethmdc"), Settings.ETH_Pin_mdc);
  addFormPinSelect(PinSelectPurpose::Ethernet, formatGpioName_input(F("Ethernet MIO pin")), F("ethmdio"), Settings.ETH_Pin_mdio);
  addFormPinSelect(PinSelectPurpose::Ethernet, formatGpioName_output(F("Ethernet Power pin")), F("ethpower"), Settings.ETH_Pin_power);
  addRowLabel_tr_id(F("Ethernet Clock"), F("ethclock"));
  {
    const __FlashStringHelper * ethClockOptions[4] = { 
      toString(EthClockMode_t::Ext_crystal_osc),
      toString(EthClockMode_t::Int_50MHz_GPIO_0),
      toString(EthClockMode_t::Int_50MHz_GPIO_16),
      toString(EthClockMode_t::Int_50MHz_GPIO_17_inv)
      };
    addSelector(F("ethclock"), 4, ethClockOptions, nullptr, nullptr, static_cast<int>(Settings.ETH_Clock_Mode), false, true);
  }
#endif // if FEATURE_ETHERNET

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

#if FEATURE_PLUGIN_PRIORITY
bool isI2CPriorityTaskActive() {
  bool hasI2CPriorityTask = false;
  for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX && !hasI2CPriorityTask; taskIndex++) {
    hasI2CPriorityTask |= isPluginI2CPowerManager_from_TaskIndex(taskIndex);
  }
  return hasI2CPriorityTask;
}
#endif // if FEATURE_PLUGIN_PRIORITY

#endif // ifdef WEBSERVER_HARDWARE

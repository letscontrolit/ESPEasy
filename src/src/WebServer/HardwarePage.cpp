#include "../WebServer/HardwarePage.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../DataStructs/DeviceStruct.h"

#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"

#ifdef WEBSERVER_HARDWARE

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

  if (isFormItem(F("psda")))
  {
    Settings.Pin_status_led           = getFormItemInt(F("pled"));
    Settings.Pin_status_led_Inversed  = isFormItemChecked(F("pledi"));
    Settings.Pin_Reset                = getFormItemInt(F("pres"));
    Settings.Pin_i2c_sda              = getFormItemInt(F("psda"));
    Settings.Pin_i2c_scl              = getFormItemInt(F("pscl"));
    Settings.I2C_clockSpeed           = getFormItemInt(F("pi2csp"), DEFAULT_I2C_CLOCK_SPEED);
    Settings.I2C_clockSpeed_Slow      = getFormItemInt(F("pi2cspslow"), DEFAULT_I2C_CLOCK_SPEED_SLOW);
#ifdef FEATURE_I2CMULTIPLEXER
    Settings.I2C_Multiplexer_Type     = getFormItemInt(F("pi2cmuxtype"));
    if (Settings.I2C_Multiplexer_Type != I2C_MULTIPLEXER_NONE) {
      Settings.I2C_Multiplexer_Addr   = getFormItemInt(F("pi2cmuxaddr"));
    } else {
      Settings.I2C_Multiplexer_Addr   = -1;
    }
    Settings.I2C_Multiplexer_ResetPin = getFormItemInt(F("pi2cmuxreset"));
#endif
    #ifdef ESP32
      Settings.InitSPI                = getFormItemInt(F("initspi"), 0);
    #else //for ESP8266 we keep the old UI
      Settings.InitSPI                = isFormItemChecked(F("initspi")); // SPI Init
    #endif
    Settings.Pin_sd_cs                = getFormItemInt(F("sd"));
#ifdef HAS_ETHERNET
    Settings.ETH_Phy_Addr             = getFormItemInt(F("ethphy"));
    Settings.ETH_Pin_mdc              = getFormItemInt(F("ethmdc"));
    Settings.ETH_Pin_mdio             = getFormItemInt(F("ethmdio"));
    Settings.ETH_Pin_power            = getFormItemInt(F("ethpower"));
    Settings.ETH_Phy_Type             = static_cast<EthPhyType_t>(getFormItemInt(F("ethtype")));
    Settings.ETH_Clock_Mode           = static_cast<EthClockMode_t>(getFormItemInt(F("ethclock")));
    Settings.NetworkMedium            = static_cast<NetworkMedium_t>(getFormItemInt(F("ethwifi")));
#endif
    int gpio = 0;

    while (gpio <= MAX_GPIO) {
      if (Settings.UseSerial && ((gpio == 1) || (gpio == 3))) {
        // do not add the pin state select for these pins.
      } else {
        int  pinnr = -1;
        bool input, output, warning;

        if (getGpioInfo(gpio, pinnr, input, output, warning)) {
          String int_pinlabel = "p";
          int_pinlabel       += gpio;
          Settings.setPinBootState(gpio, static_cast<PinBootState>(getFormItemInt(int_pinlabel)));
        }
      }
      ++gpio;
    }
    String error = SaveSettings();
    addHtmlError(error);
    if (error.length() == 0) {
      // Apply I2C settings.
      initI2C();
    }
  }

  addHtml(F("<form  method='post'>"));
  html_table_class_normal();
  addFormHeader(F("Hardware Settings"), F("ESPEasy#Hardware_page"), F("Hardware/Hardware.html"));

  addFormSubHeader(F("Wifi Status LED"));
  addFormPinSelect(formatGpioName_output("LED"), "pled", Settings.Pin_status_led);
  addFormCheckBox(F("Inversed LED"), F("pledi"), Settings.Pin_status_led_Inversed);
  addFormNote(F("Use &rsquo;GPIO-2 (D4)&rsquo; with &rsquo;Inversed&rsquo; checked for onboard LED"));

  addFormSubHeader(F("Reset Pin"));
  addFormPinSelect(formatGpioName_input(F("Switch")), "pres", Settings.Pin_Reset);
  addFormNote(F("Press about 10s for factory reset"));

  addFormSubHeader(F("I2C Interface"));
  addFormPinSelectI2C(formatGpioName_bidirectional("SDA"), F("psda"), Settings.Pin_i2c_sda);
  addFormPinSelectI2C(formatGpioName_output("SCL"),        F("pscl"), Settings.Pin_i2c_scl);
  addFormNumericBox(F("Clock Speed"), F("pi2csp"), Settings.I2C_clockSpeed, 100, 3400000);
  addUnit(F("Hz"));
  addFormNote(F("Use 100 kHz for old I2C devices, 400 kHz is max for most."));
  addFormNumericBox(F("Slow device Clock Speed"), F("pi2cspslow"), Settings.I2C_clockSpeed_Slow, 100, 3400000);
  addUnit(F("Hz"));
#ifdef FEATURE_I2CMULTIPLEXER
  addFormSubHeader(F("I2C Multiplexer"));
  // Select the type of multiplexer to use
  {
    String i2c_muxtype_options[5];
    int    i2c_muxtype_choices[5];
    i2c_muxtype_options[0] = F("- None -");
    i2c_muxtype_choices[0] = -1;
    i2c_muxtype_options[1] = F("TCA9548a - 8 channel");
    i2c_muxtype_choices[1] = I2C_MULTIPLEXER_TCA9548A;
    i2c_muxtype_options[2] = F("TCA9546a - 4 channel");
    i2c_muxtype_choices[2] = I2C_MULTIPLEXER_TCA9546A;
    i2c_muxtype_options[3] = F("TCA9543a - 2 channel");
    i2c_muxtype_choices[3] = I2C_MULTIPLEXER_TCA9543A;
    i2c_muxtype_options[4] = F("PCA9540 - 2 channel (experimental)");
    i2c_muxtype_choices[4] = I2C_MULTIPLEXER_PCA9540;
    addFormSelector(F("I2C Multiplexer type"), F("pi2cmuxtype"), 5, i2c_muxtype_options, i2c_muxtype_choices, Settings.I2C_Multiplexer_Type);
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
  addFormPinSelect(formatGpioName_output_optional("Reset"), F("pi2cmuxreset"), Settings.I2C_Multiplexer_ResetPin);
  addFormNote(F("Will be pulled low to force a reset. Reset is not available on PCA9540."));
#endif

  // SPI Init
  addFormSubHeader(F("SPI Interface"));
  #ifdef ESP32
    String spi_options[3] = { F("Disabled"), F("VSPI: CLK=GPIO-18, MISO=GPIO-19, MOSI=GPIO-23"), F("HSPI: CLK=GPIO-14, MISO=GPIO-12, MOSI=GPIO-13")};
    addFormSelector(F("Init SPI"), F("initspi"), 3, spi_options, NULL, Settings.InitSPI);
    addFormNote(F("Changing SPI settings requires to manualy restart"));
  #else //for ESP8266 we keep the existing UI
  addFormCheckBox(F("Init SPI"), F("initspi"), Settings.InitSPI>0);
  addFormNote(F("CLK=GPIO-14 (D5), MISO=GPIO-12 (D6), MOSI=GPIO-13 (D7)"));
  #endif
  addFormNote(F("Chip Select (CS) config must be done in the plugin"));
  
#ifdef FEATURE_SD
  addFormPinSelect(formatGpioName_output("SD Card CS"), "sd", Settings.Pin_sd_cs);
#endif // ifdef FEATURE_SD
  
#ifdef HAS_ETHERNET
  addFormSubHeader(F("Ethernet"));
  addRowLabel_tr_id(F("Preferred network medium"), "ethwifi");
  String ethWifiOptions[2] = {
    toString(NetworkMedium_t::WIFI), 
    toString(NetworkMedium_t::Ethernet) 
    };
  addSelector("ethwifi", 2, ethWifiOptions, NULL, NULL, static_cast<int>(Settings.NetworkMedium), false, true);
  addFormNote(F("Change Switch between WiFi and Ethernet requires reboot to activate"));
  addRowLabel_tr_id(F("Ethernet PHY type"), "ethtype");
  String ethPhyTypes[2] = { 
    toString(EthPhyType_t::LAN8710), 
    toString(EthPhyType_t::TLK110) };
  addSelector("ethtype", 2, ethPhyTypes, NULL, NULL, static_cast<int>(Settings.ETH_Phy_Type), false, true);
  addFormNumericBox(F("Ethernet PHY Address"), "ethphy", Settings.ETH_Phy_Addr, 0, 255);
  addFormNote(F("I&sup2;C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)"));
  addFormPinSelect(formatGpioName_output("Ethernet MDC pin"), "ethmdc", Settings.ETH_Pin_mdc);
  addFormPinSelect(formatGpioName_input("Ethernet MIO pin"), "ethmdio", Settings.ETH_Pin_mdio);
  addFormPinSelect(formatGpioName_output("Ethernet Power pin"), "ethpower", Settings.ETH_Pin_power);
  addRowLabel_tr_id(F("Ethernet Clock"), "ethclock");
  String ethClockOptions[4] = { 
    toString(EthClockMode_t::Ext_crystal_osc),
    toString(EthClockMode_t::Int_50MHz_GPIO_0),
    toString(EthClockMode_t::Int_50MHz_GPIO_16),
    toString(EthClockMode_t::Int_50MHz_GPIO_17_inv)
     };
  addSelector("ethclock", 4, ethClockOptions, NULL, NULL, static_cast<int>(Settings.ETH_Clock_Mode), false, true);
#endif // ifdef HAS_ETHERNET

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

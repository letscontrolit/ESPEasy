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
#include "../Helpers/Hardware_I2C.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"

#if FEATURE_I2C_MULTIPLE
#include "../Helpers/I2C_access.h"
#include "../Helpers/Hardware_device_info.h"
#endif // if FEATURE_I2C_MULTIPLE

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
    if (!isI2CPriorityTaskActive(0))
    #endif //if FEATURE_PLUGIN_PRIORITY
    {
      update_whenset_FormItemInt(F("psda0"), Settings.Pin_i2c_sda);
      update_whenset_FormItemInt(F("pscl0"), Settings.Pin_i2c_scl);
    }
    Settings.I2C_clockSpeed           = getFormItemInt(F("pi2csp0"), DEFAULT_I2C_CLOCK_SPEED);
    Settings.I2C_clockSpeed_Slow      = getFormItemInt(F("pi2cspslow0"), DEFAULT_I2C_CLOCK_SPEED_SLOW);
    Settings.WireClockStretchLimit    = getFormItemInt(F("wirestretch"));
    #if FEATURE_I2CMULTIPLEXER
    Settings.I2C_Multiplexer_Type     = getFormItemInt(F("pi2cmuxtype0"));
    if (Settings.I2C_Multiplexer_Type != I2C_MULTIPLEXER_NONE) {
      Settings.I2C_Multiplexer_Addr   = getFormItemInt(F("pi2cmuxaddr0"));
    } else {
      Settings.I2C_Multiplexer_Addr   = -1;
    }
    Settings.I2C_Multiplexer_ResetPin = getFormItemInt(F("pi2cmuxreset0"));
    #endif // if FEATURE_I2CMULTIPLEXER
    #if FEATURE_I2C_MULTIPLE // No loop used here, to avoid adding setters to the SettingsStruct template code
    if (getI2CBusCount() > 1) {
      #if FEATURE_PLUGIN_PRIORITY
      if (!isI2CPriorityTaskActive(1))
      #endif //if FEATURE_PLUGIN_PRIORITY
      {
        update_whenset_FormItemInt(F("psda1"), Settings.Pin_i2c2_sda);
        update_whenset_FormItemInt(F("pscl1"), Settings.Pin_i2c2_scl);
      }
      Settings.I2C2_clockSpeed            = getFormItemInt(F("pi2csp1"),     DEFAULT_I2C_CLOCK_SPEED);
      Settings.I2C2_clockSpeed_Slow       = getFormItemInt(F("pi2cspslow1"), DEFAULT_I2C_CLOCK_SPEED_SLOW);
      #if FEATURE_I2CMULTIPLEXER
      Settings.I2C2_Multiplexer_Type      = getFormItemInt(F("pi2cmuxtype1"));
      if (Settings.I2C2_Multiplexer_Type != I2C_MULTIPLEXER_NONE) {
        Settings.I2C2_Multiplexer_Addr    = getFormItemInt(F("pi2cmuxaddr1"));
      } else {
        Settings.I2C2_Multiplexer_Addr    = -1;
      }
      Settings.I2C2_Multiplexer_ResetPin  = getFormItemInt(F("pi2cmuxreset1"));
      #endif // if FEATURE_I2CMULTIPLEXER
    }
    #if FEATURE_I2C_INTERFACE_3
    if (getI2CBusCount() > 2) {
      #if FEATURE_PLUGIN_PRIORITY
      if (!isI2CPriorityTaskActive(2))
      #endif //if FEATURE_PLUGIN_PRIORITY
      {
        update_whenset_FormItemInt(F("psda2"), Settings.Pin_i2c3_sda);
        update_whenset_FormItemInt(F("pscl2"), Settings.Pin_i2c3_scl);
      }
      Settings.I2C3_clockSpeed            = getFormItemInt(F("pi2csp2"),     DEFAULT_I2C_CLOCK_SPEED);
      Settings.I2C3_clockSpeed_Slow       = getFormItemInt(F("pi2cspslow2"), DEFAULT_I2C_CLOCK_SPEED_SLOW);
      #if FEATURE_I2CMULTIPLEXER
      Settings.I2C3_Multiplexer_Type      = getFormItemInt(F("pi2cmuxtype2"));
      if (Settings.I2C3_Multiplexer_Type != I2C_MULTIPLEXER_NONE) {
        Settings.I2C3_Multiplexer_Addr    = getFormItemInt(F("pi2cmuxaddr2"));
      } else {
        Settings.I2C3_Multiplexer_Addr    = -1;
      }
      Settings.I2C3_Multiplexer_ResetPin  = getFormItemInt(F("pi2cmuxreset2"));
      #endif // if FEATURE_I2CMULTIPLEXER
    }
    #endif // if FEATURE_I2C_INTERFACE_3
    set3BitToUL(Settings.I2C_peripheral_bus, I2C_PERIPHERAL_BUS_PCFMCP, getFormItemInt(F("pi2cbuspcf")));
    #endif // if FEATURE_I2C_MULTIPLE
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
    Settings.ETH_Pin_mdc_cs              = getFormItemInt(F("ethmdc"));
    Settings.ETH_Pin_mdio_irq             = getFormItemInt(F("ethmdio"));
    Settings.ETH_Pin_power_rst            = getFormItemInt(F("ethpower"));
    Settings.ETH_Phy_Type             = static_cast<EthPhyType_t>(getFormItemInt(F("ethtype")));
#if CONFIG_ETH_USE_ESP32_EMAC
    Settings.ETH_Clock_Mode           = static_cast<EthClockMode_t>(getFormItemInt(F("ethclock")));
#endif
    Settings.NetworkMedium            = static_cast<NetworkMedium_t>(getFormItemInt(F("ethwifi")));
    #endif // if FEATURE_ETHERNET
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
    if (error.isEmpty()) {
      // Apply I2C settings.
      initI2C();
    }
  }

  addHtml(F("<form  method='post'>"));
  html_table_class_normal();
  addFormHeader(F("Hardware Settings"), F(""), F("Hardware/Hardware.html"));

  addFormSubHeader(F("Wifi Status LED"));
  addFormPinSelect(PinSelectPurpose::Status_led, formatGpioName_output(F("LED")), F("pled"), Settings.Pin_status_led);
  addFormCheckBox(F("Inversed LED"), F("pledi"), Settings.Pin_status_led_Inversed);
  addFormNote(F("Use &rsquo;GPIO-2 (D4)&rsquo; with &rsquo;Inversed&rsquo; checked for onboard LED"));

  addFormSubHeader(F("Reset Pin"));
  addFormPinSelect(PinSelectPurpose::Reset_pin, formatGpioName_input(F("Switch")), F("pres"), Settings.Pin_Reset);
  addFormNote(F("Press about 10s for factory reset"));

  #if FEATURE_I2CMULTIPLEXER
  const __FlashStringHelper *i2c_muxtype_options[] = {
    F("- None -"),
    F("TCA9548a - 8 channel"),
    F("TCA9546a - 4 channel"),
    F("TCA9543a - 2 channel"),
    F("PCA9540 - 2 channel (experimental)")
  };
  const int i2c_muxtype_choices[] = {
    I2C_MULTIPLEXER_NONE,
    I2C_MULTIPLEXER_TCA9548A,
    I2C_MULTIPLEXER_TCA9546A,
    I2C_MULTIPLEXER_TCA9543A,
    I2C_MULTIPLEXER_PCA9540
  };
  const FormSelectorOptions muxSelector(NR_ELEMENTS(i2c_muxtype_choices),
                  i2c_muxtype_options, i2c_muxtype_choices);

// Select the I2C address for a port multiplexer

  String  i2c_mux_options[9];
  int     i2c_mux_choices[9];
  uint8_t mux_opt = 0;
  i2c_mux_options[mux_opt] = F("- None -");
  i2c_mux_choices[mux_opt] = I2C_MULTIPLEXER_NONE;
  mux_opt++;
  for (int8_t x = 0; x < 8; x++) {
    i2c_mux_options[mux_opt] = formatToHex_decimal(0x70 + x);
    if (x == 0) { // PCA9540 has a fixed address 0f 0x70
      i2c_mux_options[mux_opt] += F(" [TCA9543a/6a/8a, PCA9540]");
    } else if (x < 4) {
      i2c_mux_options[mux_opt] += F(" [TCA9543a/6a/8a]");
    } else {
      i2c_mux_options[mux_opt] += F(" [TCA9546a/8a]");
    }
    i2c_mux_choices[mux_opt] = 0x70 + x;
    mux_opt++;
  }
  const FormSelectorOptions addrSelector(mux_opt, i2c_mux_options, i2c_mux_choices);
  #endif // if FEATURE_I2CMULTIPLEXER

  uint8_t i2cBus = 0;
  #if FEATURE_I2C_MULTIPLE
  for (uint8_t i2cBus = 0; i2cBus < getI2CBusCount(); ++i2cBus)
  #endif // if FEATURE_I2C_MULTIPLE
  {
    #if !FEATURE_I2C_MULTIPLE
    addFormSubHeader(F("I2C Bus"));
    #else // if !FEATURE_I2C_MULTIPLE
    addFormSubHeader(strformat(F("I2C Bus %u"), i2cBus));
    #endif // if !FEATURE_I2C_MULTIPLE
    #if FEATURE_PLUGIN_PRIORITY
    if (isI2CPriorityTaskActive(i2cBus)) {
      I2CShowSdaSclReadonly(Settings.getI2CSdaPin(i2cBus), Settings.getI2CSclPin(i2cBus), i2cBus);
    } else
    #endif // if FEATURE_PLUGIN_PRIORITY
    {
      addFormPinSelectI2C(formatGpioName_bidirectional(F("SDA")), strformat(F("psda%u"), i2cBus), i2cBus, Settings.getI2CSdaPin(i2cBus));
      addFormPinSelectI2C(formatGpioName_output(F("SCL")),        strformat(F("pscl%u"), i2cBus), i2cBus, Settings.getI2CSclPin(i2cBus));
    }
    addFormNumericBox(F("Clock Speed"), strformat(F("pi2csp%u"), i2cBus), Settings.getI2CClockSpeed(i2cBus), 100, 3400000);
    addUnit(F("Hz"));
    addFormNote(F("Use 100 kHz for old I2C devices, 400 kHz is max for most."));
    addFormNumericBox(F("Slow device Clock Speed"), strformat(F("pi2cspslow%u"), i2cBus), Settings.getI2CClockSpeedSlow(i2cBus), 100, 3400000);
    addUnit(F("Hz"));

    if (0 == i2cBus) { // Only support Clock-stretching on Bus 1
      addFormNumericBox(F("I2C ClockStretchLimit"), F("wirestretch"), Settings.WireClockStretchLimit, 0);
      #ifdef ESP8266
      addUnit(F("usec"));
      #endif
      #ifdef ESP32
      addUnit(F("1/80 usec"));
      #endif
    }

    #if FEATURE_I2CMULTIPLEXER
    #if !FEATURE_I2C_MULTIPLE
    addFormSubHeader(F("I2C Multiplexer"));
    #else // if !FEATURE_I2C_MULTIPLE
    addFormSubHeader(strformat(F("I2C Multiplexer %u"), i2cBus));
    #endif // if FEATURE_I2C_MULTIPLE
    // Select the type of multiplexer to use
    {
      muxSelector.addFormSelector(F("I2C Multiplexer type"), strformat(F("pi2cmuxtype%u"), i2cBus), Settings.getI2CMultiplexerType(i2cBus));
      addrSelector.addFormSelector(F("I2C Multiplexer address"), strformat(F("pi2cmuxaddr%u"), i2cBus),  Settings.getI2CMultiplexerAddr(i2cBus));
      // addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("Reset")), strformat(F("pi2cmuxreset%u"), i2cBus), Settings.getI2CMultiplexerResetPin(i2cBus));
      const String id = strformat(F("pi2cmuxreset%u"), i2cBus);
      addRowLabel_tr_id(formatGpioName_output_optional(F("Reset")), id);
      addPinSelect(PinSelectPurpose::Generic_output, id, Settings.getI2CMultiplexerResetPin(i2cBus));
      addFormNote(F("Will be pulled low to force a reset. Reset is not available on PCA9540."));
    }
    #endif // if FEATURE_I2CMULTIPLEXER
  }
  #if FEATURE_I2C_MULTIPLE
  const uint8_t i2cMaxBusCount = (getI2CBusCount() > 1
                                  ? ((Settings.isI2CEnabled(1) ? 1 : 0)
                                    # if FEATURE_I2C_INTERFACE_3
                                     + (Settings.isI2CEnabled(2) ? 1 : 0)
                                    # endif // if FEATURE_I2C_INTERFACE_3
                                     )
                                  : 0) + (Settings.isI2CEnabled(0) ? 1 : 0);
  if (i2cMaxBusCount > 1) {
    addFormSubHeader(F("PCF &amp; MCP Direct I/O"));
    const uint8_t i2cBus = Settings.getI2CInterfacePCFMCP();
    I2CInterfaceSelector(F("I2C Bus"),
                        F("pi2cbuspcf"),
                        i2cBus,
                        false);

  }
  #endif // if FEATURE_I2C_MULTIPLE

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
      getSPI_optionToString(SPI_Options_e::Vspi_Fspi), 
      #ifdef ESP32_CLASSIC
      getSPI_optionToString(SPI_Options_e::Hspi), 
      #endif
      getSPI_optionToString(SPI_Options_e::UserDefined)};
    const int spi_index[] = {
      static_cast<int>(SPI_Options_e::None),
      static_cast<int>(SPI_Options_e::Vspi_Fspi),
      #ifdef ESP32_CLASSIC
      static_cast<int>(SPI_Options_e::Hspi),
      #endif
      static_cast<int>(SPI_Options_e::UserDefined)
    };
    constexpr size_t nrOptions = NR_ELEMENTS(spi_index);
    FormSelectorOptions selector(nrOptions, spi_options, spi_index);
    selector.onChangeCall = F("spiOptionChanged(this)");
    selector.addFormSelector(F("Init SPI"), F("initspi"), Settings.InitSPI);
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
  addFormPinSelect(PinSelectPurpose::SD_Card, formatGpioName_output(F("SD Card CS")), F("sd"), Settings.Pin_sd_cs);
#endif // if FEATURE_SD
  
#if FEATURE_ETHERNET
  addFormSubHeader(F("Ethernet"));
  addRowLabel_tr_id(F("Preferred network medium"), F("ethwifi"));
  {
    const __FlashStringHelper * ethWifiOptions[2] = {
      toString(NetworkMedium_t::WIFI), 
      toString(NetworkMedium_t::Ethernet) 
      };
    const FormSelectorOptions selector(2, ethWifiOptions);
    selector.addSelector(F("ethwifi"), static_cast<int>(Settings.NetworkMedium));
  }
  addFormNote(F("Change Switch between WiFi and Ethernet requires reboot to activate"));
  {
    const __FlashStringHelper * ethPhyTypes[] = { 
      toString(EthPhyType_t::notSet),			  

# if CONFIG_ETH_USE_ESP32_EMAC
      toString(EthPhyType_t::LAN8720),			  
      toString(EthPhyType_t::TLK110),				  
#if ESP_IDF_VERSION_MAJOR > 3
      toString(EthPhyType_t::RTL8201),				
#if ETH_TYPE_JL1101_SUPPORTED
      toString(EthPhyType_t::JL1101),				  
#endif
      toString(EthPhyType_t::DP83848),				
      toString(EthPhyType_t::KSZ8041),				
      toString(EthPhyType_t::KSZ8081),				
#endif
# endif // if CONFIG_ETH_USE_ESP32_EMAC

#if ESP_IDF_VERSION_MAJOR >= 5
# if CONFIG_ETH_SPI_ETHERNET_DM9051
      toString(EthPhyType_t::DM9051),				  
# endif // if CONFIG_ETH_SPI_ETHERNET_DM9051
# if CONFIG_ETH_SPI_ETHERNET_W5500
      toString(EthPhyType_t::W5500),				  
# endif // if CONFIG_ETH_SPI_ETHERNET_W5500
# if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
      toString(EthPhyType_t::KSZ8851),				
# endif // if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
#endif
      };
    const int ethPhyTypes_index[] = {
      static_cast<int>(EthPhyType_t::notSet),			  

# if CONFIG_ETH_USE_ESP32_EMAC
      static_cast<int>(EthPhyType_t::LAN8720),			  
      static_cast<int>(EthPhyType_t::TLK110),				  
#if ESP_IDF_VERSION_MAJOR > 3
      static_cast<int>(EthPhyType_t::RTL8201),			
#if ETH_TYPE_JL1101_SUPPORTED	
      static_cast<int>(EthPhyType_t::JL1101),				  
#endif
      static_cast<int>(EthPhyType_t::DP83848),				
      static_cast<int>(EthPhyType_t::KSZ8041),				
      static_cast<int>(EthPhyType_t::KSZ8081),				
#endif
# endif // if CONFIG_ETH_USE_ESP32_EMAC

#if ESP_IDF_VERSION_MAJOR >= 5
# if CONFIG_ETH_SPI_ETHERNET_DM9051
      static_cast<int>(EthPhyType_t::DM9051),				  
# endif // if CONFIG_ETH_SPI_ETHERNET_DM9051
# if CONFIG_ETH_SPI_ETHERNET_W5500
      static_cast<int>(EthPhyType_t::W5500),				  
# endif // if CONFIG_ETH_SPI_ETHERNET_W5500
# if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
      static_cast<int>(EthPhyType_t::KSZ8851),				
# endif // if CONFIG_ETH_SPI_ETHERNET_KSZ8851SNL
#endif
    };

    constexpr unsigned nrItems = NR_ELEMENTS(ethPhyTypes_index);


    const int choice = isValid(Settings.ETH_Phy_Type) 
      ? static_cast<int>(Settings.ETH_Phy_Type) 
      : static_cast<int>(EthPhyType_t::notSet);

    const FormSelectorOptions selector(      
      nrItems, 
      ethPhyTypes, 
      ethPhyTypes_index);
    selector.addFormSelector(
      F("Ethernet PHY type"), 
      F("ethtype"),
      choice);
  }

#if CONFIG_ETH_USE_SPI_ETHERNET && CONFIG_ETH_USE_ESP32_EMAC
#define MDC_CS_PIN_DESCR  "Ethernet MDC/CS pin"
#define MIO_IRQ_PIN_DESCR  "Ethernet MDIO/IRQ pin"
#define PWR_RST_PIN_DESCR  "Ethernet Power/RST pin"
#elif CONFIG_ETH_USE_SPI_ETHERNET
#define MDC_CS_PIN_DESCR  "Ethernet CS pin"
#define MIO_IRQ_PIN_DESCR  "Ethernet IRQ pin"
#define PWR_RST_PIN_DESCR  "Ethernet RST pin"
#else // #elif CONFIG_ETH_USE_ESP32_EMAC
#define MDC_CS_PIN_DESCR  "Ethernet MDC pin"
#define MIO_IRQ_PIN_DESCR  "Ethernet MIO pin"
#define PWR_RST_PIN_DESCR  "Ethernet Power pin"
#endif

  addFormNumericBox(F("Ethernet PHY Address"), F("ethphy"), Settings.ETH_Phy_Addr, -1, 127);
  addFormNote(F("I&sup2;C-address of Ethernet PHY"
#if CONFIG_ETH_USE_ESP32_EMAC
  " (0 or 1 for LAN8720, 31 for TLK110, -1 autodetect)"
#endif
  ));
  addFormPinSelect(PinSelectPurpose::Ethernet, formatGpioName_output(
    F(MDC_CS_PIN_DESCR)), 
    F("ethmdc"), Settings.ETH_Pin_mdc_cs);
  addFormPinSelect(PinSelectPurpose::Ethernet, formatGpioName_input(
    F(MIO_IRQ_PIN_DESCR)), 
    F("ethmdio"), Settings.ETH_Pin_mdio_irq);
  addFormPinSelect(PinSelectPurpose::Ethernet, formatGpioName_output(
    F(PWR_RST_PIN_DESCR)), 
    F("ethpower"), Settings.ETH_Pin_power_rst);
#if CONFIG_ETH_USE_ESP32_EMAC
  addRowLabel_tr_id(F("Ethernet Clock"), F("ethclock"));
  {
    const __FlashStringHelper * ethClockOptions[] = { 
      toString(EthClockMode_t::Ext_crystal_osc),
      toString(EthClockMode_t::Int_50MHz_GPIO_0),
      toString(EthClockMode_t::Int_50MHz_GPIO_16),
      toString(EthClockMode_t::Int_50MHz_GPIO_17_inv)
      };
    const FormSelectorOptions selector(NR_ELEMENTS(ethClockOptions), ethClockOptions);
    selector.addSelector(F("ethclock"), static_cast<int>(Settings.ETH_Clock_Mode));
  }
#endif
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
bool isI2CPriorityTaskActive(uint8_t i2cBus) {
  bool hasI2CPriorityTask = false;
  for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX && !hasI2CPriorityTask; taskIndex++) {
    hasI2CPriorityTask |= isPluginI2CPowerManager_from_TaskIndex(taskIndex, i2cBus);
  }
  return hasI2CPriorityTask;
}

void I2CShowSdaSclReadonly(int8_t i2c_sda, int8_t i2c_scl, uint8_t i2cBus) {
  int  pinnr = -1;
  bool input, output, warning = false;
  addFormNote(strformat(F("I2C (%d) GPIO pins can't be changed when an I2C Priority task is configured."), i2cBus));
  addRowLabel(formatGpioName_bidirectional(F("SDA")));
  getGpioInfo(i2c_sda, pinnr, input, output, warning);
  addHtml(createGPIO_label(i2c_sda, pinnr, true, true, false));
  addRowLabel(formatGpioName_output(F("SCL")));
  getGpioInfo(i2c_scl, pinnr, input, output, warning);
  addHtml(createGPIO_label(i2c_scl, pinnr, true, true, false));
}
#endif // if FEATURE_PLUGIN_PRIORITY

#endif // ifdef WEBSERVER_HARDWARE

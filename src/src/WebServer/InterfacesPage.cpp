#include "../WebServer/InterfacesPage.h"

#ifdef WEBSERVER_INTERFACES

# include "../WebServer/ESPEasy_WebServer.h"
# include "../WebServer/HTML_wrappers.h"
# include "../WebServer/Markup.h"
# include "../WebServer/Markup_Buttons.h"
# include "../WebServer/Markup_Forms.h"

# include "../CustomBuild/ESPEasyLimits.h"

# include "../DataStructs/DeviceStruct.h"

# include "../Globals/Settings.h"

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Hardware_GPIO.h"
# include "../Helpers/Hardware_I2C.h"
# include "../Helpers/Hardware_SPI.h"
# include "../Helpers/SPI_Helper.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/StringGenerator_GPIO.h"

# if FEATURE_I2C_MULTIPLE
#  include "../Helpers/I2C_access.h"
#  include "../Helpers/Hardware_device_info.h"
# endif // if FEATURE_I2C_MULTIPLE


void save_interfaces() {
    
  String error;
  bool   updated{};

  if (isFormItem(F("pi2csp0"))) {
    updated = true;
# if FEATURE_PLUGIN_PRIORITY

    if (!isI2CPriorityTaskActive(0))
# endif // if FEATURE_PLUGIN_PRIORITY
    {
      update_whenset_FormItemInt(F("psda0"), Settings.Pin_i2c_sda);
      update_whenset_FormItemInt(F("pscl0"), Settings.Pin_i2c_scl);
    }
    Settings.I2C_clockSpeed        = getFormItemInt(F("pi2csp0"), DEFAULT_I2C_CLOCK_SPEED);
    Settings.I2C_clockSpeed_Slow   = getFormItemInt(F("pi2cspslow0"), DEFAULT_I2C_CLOCK_SPEED_SLOW);
    Settings.WireClockStretchLimit = getFormItemInt(F("wirestretch"));
# if FEATURE_I2CMULTIPLEXER
    Settings.I2C_Multiplexer_Type = getFormItemInt(F("pi2cmuxtype0"));

    if (Settings.I2C_Multiplexer_Type != I2C_MULTIPLEXER_NONE) {
      Settings.I2C_Multiplexer_Addr = getFormItemInt(F("pi2cmuxaddr0"));
    } else {
      Settings.I2C_Multiplexer_Addr = -1;
    }
    Settings.I2C_Multiplexer_ResetPin = getFormItemInt(F("pi2cmuxreset0"));
# endif // if FEATURE_I2CMULTIPLEXER
  }
# if FEATURE_I2C_MULTIPLE // No loop used here, to avoid adding setters to the SettingsStruct template code

  if ((getI2CBusCount() > 1) && isFormItem(F("pi2csp1"))) {
    updated = true;
#  if FEATURE_PLUGIN_PRIORITY

    if (!isI2CPriorityTaskActive(1))
#  endif // if FEATURE_PLUGIN_PRIORITY
    {
      update_whenset_FormItemInt(F("psda1"), Settings.Pin_i2c2_sda);
      update_whenset_FormItemInt(F("pscl1"), Settings.Pin_i2c2_scl);
    }
    Settings.I2C2_clockSpeed      = getFormItemInt(F("pi2csp1"),     DEFAULT_I2C_CLOCK_SPEED);
    Settings.I2C2_clockSpeed_Slow = getFormItemInt(F("pi2cspslow1"), DEFAULT_I2C_CLOCK_SPEED_SLOW);
#  if FEATURE_I2CMULTIPLEXER
    Settings.I2C2_Multiplexer_Type = getFormItemInt(F("pi2cmuxtype1"));

    if (Settings.I2C2_Multiplexer_Type != I2C_MULTIPLEXER_NONE) {
      Settings.I2C2_Multiplexer_Addr = getFormItemInt(F("pi2cmuxaddr1"));
    } else {
      Settings.I2C2_Multiplexer_Addr = -1;
    }
    Settings.I2C2_Multiplexer_ResetPin = getFormItemInt(F("pi2cmuxreset1"));
#  endif // if FEATURE_I2CMULTIPLEXER
  }


#  if FEATURE_I2C_INTERFACE_3

  if ((getI2CBusCount() > 2) && isFormItem(F("pi2csp2"))) {
    updated = true;
#   if FEATURE_PLUGIN_PRIORITY

    if (!isI2CPriorityTaskActive(2))
#   endif // if FEATURE_PLUGIN_PRIORITY
    {
      update_whenset_FormItemInt(F("psda2"), Settings.Pin_i2c3_sda);
      update_whenset_FormItemInt(F("pscl2"), Settings.Pin_i2c3_scl);
    }
    Settings.I2C3_clockSpeed      = getFormItemInt(F("pi2csp2"),     DEFAULT_I2C_CLOCK_SPEED);
    Settings.I2C3_clockSpeed_Slow = getFormItemInt(F("pi2cspslow2"), DEFAULT_I2C_CLOCK_SPEED_SLOW);
#   if FEATURE_I2CMULTIPLEXER
    Settings.I2C3_Multiplexer_Type = getFormItemInt(F("pi2cmuxtype2"));

    if (Settings.I2C3_Multiplexer_Type != I2C_MULTIPLEXER_NONE) {
      Settings.I2C3_Multiplexer_Addr = getFormItemInt(F("pi2cmuxaddr2"));
    } else {
      Settings.I2C3_Multiplexer_Addr = -1;
    }
    Settings.I2C3_Multiplexer_ResetPin = getFormItemInt(F("pi2cmuxreset2"));
#   endif // if FEATURE_I2CMULTIPLEXER
  }
#  endif // if FEATURE_I2C_INTERFACE_3

  if (isFormItem(F("pi2cbuspcf"))) {
    updated = true;
    set3BitToUL(Settings.I2C_peripheral_bus, I2C_PERIPHERAL_BUS_PCFMCP, getFormItemInt(F("pi2cbuspcf")));
  }
# endif // if FEATURE_I2C_MULTIPLE

  {
# ifdef ESP32
    bool SPI_updated{};

    if (update_whenset_FormItemInt(F("initspi0"), Settings.InitSPI)) {
      SPI_updated = true;

      // User-defined SPI bus 0 GPIO pins
      Settings.SPI_SCLK_pin = getFormItemInt(F("spipinsclk0"), -1);
      Settings.SPI_MISO_pin = getFormItemInt(F("spipinmiso0"), -1);
      Settings.SPI_MOSI_pin = getFormItemInt(F("spipinmosi0"), -1);
    }

    if (update_whenset_FormItemInt(F("initspi1"), Settings.InitSPI1)) {
      SPI_updated = true;

      // User-defined SPI bus 1 GPIO pins
      Settings.SPI1_SCLK_pin = getFormItemInt(F("spipinsclk1"), -1);
      Settings.SPI1_MISO_pin = getFormItemInt(F("spipinmiso1"), -1);
      Settings.SPI1_MOSI_pin = getFormItemInt(F("spipinmosi1"), -1);
    }

    if (SPI_updated) {
      updated = true;

      for (uint8_t spi_bus = 0; spi_bus < getSPIBusCount(); ++spi_bus) {
        if (Settings.isSPI_enabled(spi_bus) && !Settings.isSPI_valid(spi_bus)) { // Checks
          error += strformat(F("SPI bus %u pins not configured correctly!<BR>"), spi_bus);
        }
      }
    }
# else // for ESP8266 we keep the old UI
    Settings.InitSPI = isFormItemChecked(F("initspi")); // SPI Init
    updated          = true;
# endif // ifdef ESP32
  }

  if (updated) {
    error += SaveSettings();
    addHtmlError(error);

    if (error.isEmpty()) {
      // Apply I2C settings.
      initI2C();

      // Apply SPI settings
      initializeSPIBuses();
    }
  }
}


// ********************************************************************************
// Web Interface hardware page
// ********************************************************************************
void handle_interfaces() {
# ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_interfaces"));
# endif

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_INTERFACES;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  save_interfaces();


  addHtml(F("<form  method='post'>"));
  html_table_class_normal();
  addFormHeader(F("Interfaces Settings"), F(""), F("Interfaces/Interfaces.html"));


# if FEATURE_I2CMULTIPLEXER
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
# endif // if FEATURE_I2CMULTIPLEXER

  uint8_t i2cBus = 0;
# if FEATURE_I2C_MULTIPLE

  for (uint8_t i2cBus = 0; i2cBus < getI2CBusCount(); ++i2cBus)
# endif // if FEATURE_I2C_MULTIPLE
  {
# if !FEATURE_I2C_MULTIPLE
    addFormSubHeader(F("I2C Bus"));
# else
    addFormSubHeader(strformat(F("I2C Bus %u"), i2cBus));
# endif // if !FEATURE_I2C_MULTIPLE
# if FEATURE_PLUGIN_PRIORITY

    if (isI2CPriorityTaskActive(i2cBus)) {
      I2CShowSdaSclReadonly(Settings.getI2CSdaPin(i2cBus), Settings.getI2CSclPin(i2cBus), i2cBus);
    } else
# endif // if FEATURE_PLUGIN_PRIORITY
    {
      addFormPinSelectI2C(formatGpioName_bidirectional(F("SDA")), strformat(F("psda%u"), i2cBus), i2cBus, Settings.getI2CSdaPin(i2cBus));
      addFormPinSelectI2C(formatGpioName_output(F("SCL")),        strformat(F("pscl%u"), i2cBus), i2cBus, Settings.getI2CSclPin(i2cBus));
    }
    addFormNumericBox(F("Clock Speed"), strformat(F("pi2csp%u"), i2cBus), Settings.getI2CClockSpeed(i2cBus), 100, 3400000);
    addUnit(F("Hz"));
    addFormNote(F("Use 100 kHz for old I2C devices, 400 kHz is max for most."));
    addFormNumericBox(F("Slow device Clock Speed"), strformat(F("pi2cspslow%u"), i2cBus), Settings.getI2CClockSpeedSlow(i2cBus), 100,
                      3400000);
    addUnit(F("Hz"));

    if (0 == i2cBus) { // Only support Clock-stretching on Bus 1
      addFormNumericBox(F("I2C ClockStretchLimit"), F("wirestretch"), Settings.WireClockStretchLimit, 0);
# ifdef ESP8266
      addUnit(F("usec"));
# endif
# ifdef ESP32
      addUnit(F("1/80 usec"));
# endif
    }

# if FEATURE_I2CMULTIPLEXER
#  if !FEATURE_I2C_MULTIPLE
    addFormSubHeader(F("I2C Multiplexer"));
#  else // if !FEATURE_I2C_MULTIPLE
    addFormSubHeader(strformat(F("I2C Multiplexer %u"), i2cBus));
#  endif // if !FEATURE_I2C_MULTIPLE

    // Select the type of multiplexer to use
    {
      muxSelector.addFormSelector(F("I2C Multiplexer type"), strformat(F("pi2cmuxtype%u"), i2cBus), Settings.getI2CMultiplexerType(i2cBus));
      addrSelector.addFormSelector(F("I2C Multiplexer address"), strformat(F("pi2cmuxaddr%u"), i2cBus),
                                   Settings.getI2CMultiplexerAddr(i2cBus));

      // addFormPinSelect(PinSelectPurpose::Generic_output, formatGpioName_output_optional(F("Reset")), strformat(F("pi2cmuxreset%u"),
      // i2cBus), Settings.getI2CMultiplexerResetPin(i2cBus));
      const String id = strformat(F("pi2cmuxreset%u"), i2cBus);
      addRowLabel_tr_id(formatGpioName_output_optional(F("Reset")), id);
      addPinSelect(PinSelectPurpose::Generic_output, id, Settings.getI2CMultiplexerResetPin(i2cBus));
      addFormNote(F("Will be pulled low to force a reset. Reset is not available on PCA9540."));
    }
# endif // if FEATURE_I2CMULTIPLEXER
  }
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

# ifdef ESP32

  for (uint8_t spi_bus = 0; spi_bus < getSPIBusCount(); ++spi_bus) {
    // SPI Init
    addFormSubHeader(concat(F("SPI Bus "), spi_bus));
    {
      // Script to show GPIO pins for User-defined SPI GPIOs
      // html_add_script(F("function spiOptionChanged(elem) {var spipinstyle = elem.value == 9 ? '' :
      // 'none';document.getElementById('tr_spipinsclk').style.display = spipinstyle;document.getElementById('tr_spipinmiso').style.display
      // = spipinstyle;document.getElementById('tr_spipinmosi').style.display = spipinstyle;}"),
      // Minified:
      html_add_script(strformat(F("function spi%uOptionChanged(e){var i=9==e.value?'':'none';"
                                  "document.getElementById('tr_spipinsclk%u').style.display=i,"
                                  "document.getElementById('tr_spipinmiso%u').style.display=i,"
                                  "document.getElementById('tr_spipinmosi%u').style.display=i"
                                  "}"), spi_bus, spi_bus, spi_bus, spi_bus, spi_bus),
                      false);
      const __FlashStringHelper *spi_options[] = {
        getSPI_optionToString(SPI_Options_e::None),
        getSPI_optionToString(SPI_Options_e::Vspi_Fspi),
#  ifdef ESP32_CLASSIC
        getSPI_optionToString(SPI_Options_e::Hspi),
#  endif
        getSPI_optionToString(SPI_Options_e::UserDefined_VSPI)
#  if SOC_SPI_PERIPH_NUM > 2
        , getSPI_optionToString(SPI_Options_e::UserDefined_HSPI)
#  endif
      };
      const int spi_index[] = {
        static_cast<int>(SPI_Options_e::None),
        static_cast<int>(SPI_Options_e::Vspi_Fspi),
#  ifdef ESP32_CLASSIC
        static_cast<int>(SPI_Options_e::Hspi),
#  endif
        static_cast<int>(SPI_Options_e::UserDefined_VSPI)
#  if SOC_SPI_PERIPH_NUM > 2
        , static_cast<int>(SPI_Options_e::UserDefined_HSPI)
#  endif
      };
      constexpr size_t nrOptions = NR_ELEMENTS(spi_index);
      FormSelectorOptions selector(nrOptions, spi_options, spi_index);
      selector.onChangeCall = strformat(F("spi%uOptionChanged(this)"), spi_bus);
      selector.addFormSelector(concat(F("Init SPI Bus "), spi_bus),
                               concat(F("initspi"), spi_bus),
                               0 == spi_bus ? Settings.InitSPI : Settings.InitSPI1);
      int8_t spi_gpio[3];
      Settings.getSPI_pins(spi_gpio, spi_bus, true); // Load GPIO pins even if wrongly configured
      // User-defined pins
      addFormPinSelect(PinSelectPurpose::SPI,      formatGpioName_output(F("CLK")),  concat(F("spipinsclk"), spi_bus), spi_gpio[0]);
      addFormPinSelect(PinSelectPurpose::SPI_MISO, formatGpioName_input(F("MISO")),  concat(F("spipinmiso"), spi_bus), spi_gpio[1]);
      addFormPinSelect(PinSelectPurpose::SPI,      formatGpioName_output(F("MOSI")), concat(F("spipinmosi"), spi_bus), spi_gpio[2]);
      html_add_script(strformat(F("document.getElementById('initspi%u').onchange();"), spi_bus), false); // Initial trigger onchange script
      // addFormNote(F("Changing SPI settings requires to press the hardware-reset button or power off-on!"));
      addFormNote(F("Chip Select (CS) config must be done in the plugin"));
    }
  }
# else // for ESP8266 we keep the existing UI
  addFormSubHeader(F("SPI Bus 0"));
  addFormCheckBox(F("Init SPI"), F("initspi"), Settings.InitSPI > static_cast<int>(SPI_Options_e::None));
  addFormNote(F("CLK=GPIO-14 (D5), MISO=GPIO-12 (D6), MOSI=GPIO-13 (D7)"));
  addFormNote(F("Chip Select (CS) config must be done in the plugin"));
# endif // ifdef ESP32

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

# if FEATURE_PLUGIN_PRIORITY

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

# endif // if FEATURE_PLUGIN_PRIORITY

#endif // ifdef WEBSERVER_INTERFACES

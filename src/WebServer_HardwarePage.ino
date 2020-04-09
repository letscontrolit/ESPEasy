#ifdef WEBSERVER_HARDWARE

// ********************************************************************************
// Web Interface hardware page
// ********************************************************************************
void handle_hardware() {
  checkRAM(F("handle_hardware"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_HARDWARE;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  if (isFormItem(F("psda")))
  {
    Settings.Pin_status_led          = getFormItemInt(F("pled"));
    Settings.Pin_status_led_Inversed = isFormItemChecked(F("pledi"));
    Settings.Pin_Reset               = getFormItemInt(F("pres"));
    Settings.Pin_i2c_sda             = getFormItemInt(F("psda"));
    Settings.Pin_i2c_scl             = getFormItemInt(F("pscl"));
    Settings.I2C_clockSpeed          = getFormItemInt(F("pi2csp"), DEFAULT_I2C_CLOCK_SPEED);
    Settings.InitSPI                 = isFormItemChecked(F("initspi")); // SPI Init
    Settings.Pin_sd_cs               = getFormItemInt(F("sd"));
    int gpio = 0;

    // FIXME TD-er: Max of 17 is a limit in the Settings.PinBootStates array
    while (gpio < MAX_GPIO  && gpio < 17) {
      if (Settings.UseSerial && ((gpio == 1) || (gpio == 3))) {
        // do not add the pin state select for these pins.
      } else {
        int  pinnr = -1;
        bool input, output, warning;

        if (getGpioInfo(gpio, pinnr, input, output, warning)) {
          String int_pinlabel = "p";
          int_pinlabel                += gpio;
          Settings.PinBootStates[gpio] = getFormItemInt(int_pinlabel);
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
  addFormHeader(F("Hardware Settings"), F("ESPEasy#Hardware_page"));

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

  // SPI Init
  addFormSubHeader(F("SPI Interface"));
  addFormCheckBox(F("Init SPI"), F("initspi"), Settings.InitSPI);
  addFormNote(F("CLK=GPIO-14 (D5), MISO=GPIO-12 (D6), MOSI=GPIO-13 (D7)"));
  addFormNote(F("Chip Select (CS) config must be done in the plugin"));
#ifdef FEATURE_SD
  addFormPinSelect(formatGpioName_output("SD Card CS"), "sd", Settings.Pin_sd_cs);
#endif // ifdef FEATURE_SD

  addFormSubHeader(F("GPIO boot states"));
  int gpio = 0;

  // FIXME TD-er: Max of 17 is a limit in the Settings.PinBootStates array
  while (gpio < MAX_GPIO  && gpio < 17) {
    bool enabled = true;

    if (Settings.UseSerial && ((gpio == 1) || (gpio == 3))) {
      // do not add the pin state select for these pins.
      enabled = false;
    }
    int  pinnr = -1;
    bool input, output, warning;

    if (getGpioInfo(gpio, pinnr, input, output, warning)) {
      String label;
      label.reserve(32);
      label  = F("Pin mode ");
      label += createGPIO_label(gpio, pinnr, input, output, warning);
      String int_pinlabel = "p";
      int_pinlabel += gpio;
      addFormPinStateSelect(label, int_pinlabel, Settings.PinBootStates[gpio], enabled);
    }
    ++gpio;
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

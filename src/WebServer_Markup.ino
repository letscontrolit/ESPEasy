// ********************************************************************************
// Add Selector
// ********************************************************************************
void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 boolean       reloadonchange,
                 bool          enabled)
{
  // FIXME TD-er Change boolean to disabled
  addSelector_Head(id, reloadonchange, !enabled);
  addSelector_options(optionCount, options, indices, attr, selectedIndex);
  addSelector_Foot();
}

void addSelector_options(int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex)
{
  int index;

  for (byte x = 0; x < optionCount; x++)
  {
    if (indices) {
      index = indices[x];
    }
    else {
      index = x;
    }
    TXBuffer += F("<option value=");
    TXBuffer += index;

    if (selectedIndex == index) {
      TXBuffer += F(" selected");
    }

    if (attr)
    {
      TXBuffer += ' ';
      TXBuffer += attr[x];
    }
    TXBuffer += '>';
    TXBuffer += options[x];
    TXBuffer += F("</option>");
  }
}

void addSelector_Head(const String& id, boolean reloadonchange) {
  addSelector_Head(id, reloadonchange, false);
}

void addSelector_Head(const String& id, boolean reloadonchange, bool disabled)
{
  if (reloadonchange) {
    addSelector_Head(id, (const String)F("return dept_onchange(frmselect)"), disabled);
  } else {
    addSelector_Head(id, (const String)"", disabled);
  }
}

void addSelector_Head(const String& id, const String& onChangeCall, bool disabled)
{
  TXBuffer += F("<select class='wide' name='");
  TXBuffer += id;
  TXBuffer += F("' id='");
  TXBuffer += id;
  TXBuffer += '\'';

  if (disabled) {
    addDisabled();
  }

  if (onChangeCall.length() > 0) {
    TXBuffer += F(" onchange='");
    TXBuffer += onChangeCall;
    TXBuffer += '\'';
  }
  TXBuffer += '>';
}

void addSelector_Item(const String& option, int index, boolean selected, boolean disabled, const String& attr)
{
  TXBuffer += F("<option value=");
  TXBuffer += index;

  if (selected) {
    TXBuffer += F(" selected");
  }

  if (disabled) {
    addDisabled();
  }

  if (attr && (attr.length() > 0))
  {
    TXBuffer += ' ';
    TXBuffer += attr;
  }
  TXBuffer += '>';
  TXBuffer += option;
  TXBuffer += F("</option>");
}

void addSelector_Foot()
{
  TXBuffer += F("</select>");
}

void addUnit(const String& unit)
{
  TXBuffer += F(" [");
  TXBuffer += unit;
  TXBuffer += "]";
}

void addRowLabel_tr_id(const String& label, const String& id)
{
  String tr_id = F("tr_");
  tr_id += id;
  addRowLabel(label, tr_id);
}

void addRowLabel(const String& label, const String& id)
{
  if (id.length() > 0) {
    TXBuffer += F("<TR id='");
    TXBuffer += id;
    TXBuffer += F("'><TD>");
  } else {
    html_TR_TD();
  }
  if (label.length() != 0) {
    TXBuffer += label;
    TXBuffer += ':';
  }
  html_TD();
}

// Add a row label and mark it with copy markers to copy it to clipboard.
void addRowLabel_copy(const String& label) {
  TXBuffer += F("<TR>");
  html_copyText_TD();
  TXBuffer += label;
  TXBuffer += ':';
  html_copyText_marker();
  html_copyText_TD();
}

void addRowLabelValue(LabelType::Enum label) {
  addRowLabel(getLabel(label));
  TXBuffer += getValue(label);
}

void addRowLabelValue_copy(LabelType::Enum label) {
  addRowLabel_copy(getLabel(label));
  TXBuffer += getValue(label);
}

// ********************************************************************************
// Add a header
// ********************************************************************************
void addTableSeparator(const String& label, int colspan, int h_size, const String& helpButton) {
  TXBuffer += F("<TR><TD colspan=");
  TXBuffer += colspan;
  TXBuffer += "><H";
  TXBuffer += h_size;
  TXBuffer += '>';
  TXBuffer += label;

  if (helpButton.length() > 0) {
    addHelpButton(helpButton);
  }
  TXBuffer += "</H";
  TXBuffer += h_size;
  TXBuffer += F("></TD></TR>");
}

void addFormHeader(const String& header, const String& helpButton)
{
  html_TR();
  html_table_header(header, helpButton, 225);
  html_table_header("");
}


// ********************************************************************************
// Add a sub header
// ********************************************************************************
void addFormSubHeader(const String& header)
{
  addTableSeparator(header, 2, 3);
}

// ********************************************************************************
// Add a checkbox
// ********************************************************************************
void addCheckBox(const String& id, boolean checked, bool disabled)
{
  TXBuffer += F("<label class='container'>&nbsp;");
  TXBuffer += F("<input type='checkbox' id='");
  TXBuffer += id;
  TXBuffer += F("' name='");
  TXBuffer += id;
  TXBuffer += '\'';

  if (checked) {
    TXBuffer += F(" checked");
  }

  if (disabled) { addDisabled(); }
  TXBuffer += F("><span class='checkmark");

  if (disabled) { addDisabled(); }
  TXBuffer += F("'></span></label>");
}

// ********************************************************************************
// Add a numeric box
// ********************************************************************************
void addNumericBox(const String& id, int value, int min, int max)
{
  TXBuffer += F("<input class='widenumber' type='number' name='");
  TXBuffer += id;
  TXBuffer += '\'';

  if (min != INT_MIN)
  {
    TXBuffer += F(" min=");
    TXBuffer += min;
  }

  if (max != INT_MAX)
  {
    TXBuffer += F(" max=");
    TXBuffer += max;
  }
  TXBuffer += F(" value=");
  TXBuffer += value;
  TXBuffer += '>';
}

void addFloatNumberBox(const String& id, float value, float min, float max)
{
  TXBuffer += F("<input type='number' name='");
  TXBuffer += id;
  TXBuffer += '\'';
  TXBuffer += F(" min=");
  TXBuffer += min;
  TXBuffer += F(" max=");
  TXBuffer += max;
  TXBuffer += F(" step=0.01");
  TXBuffer += F(" style='width:5em;' value=");
  TXBuffer += value;
  TXBuffer += '>';
}

// ********************************************************************************
// Add Textbox
// ********************************************************************************
void addTextBox(const String& id, const String&  value, int maxlength, bool readonly, bool required, const String& pattern)
{
  TXBuffer += F("<input class='wide' type='text' name='");
  TXBuffer += id;
  TXBuffer += F("' maxlength=");
  TXBuffer += maxlength;
  TXBuffer += F(" value='");
  TXBuffer += value;
  TXBuffer += '\'';

  if (readonly) {
    TXBuffer += F(" readonly ");
  }

  if (required) {
    TXBuffer += F(" required ");
  }

  if (pattern.length() > 0) {
    TXBuffer += F("pattern = '");
    TXBuffer += pattern;
    TXBuffer += '\'';
  }
  TXBuffer += '>';
}

// ********************************************************************************
// Add Textarea
// ********************************************************************************
void addTextArea(const String& id, const String& value, int maxlength, int rows, int columns, bool readonly, bool required)
{
  TXBuffer += F("<textarea class='wide' type='text' name='");
  TXBuffer += id;
  TXBuffer += F("' maxlength=");
  TXBuffer += maxlength;
  TXBuffer += F("' rows=");
  TXBuffer += rows;
  TXBuffer += F("' cols=");
  TXBuffer += columns;
  TXBuffer += '\'';

  if (readonly) {
    TXBuffer += F(" readonly ");
  }

  if (required) {
    TXBuffer += F(" required ");
  }
  TXBuffer += '>';
  TXBuffer += value;
  TXBuffer += F("</textarea>");
}

// ********************************************************************************
// Add Help Buttons
// ********************************************************************************

// adds a Help Button with points to the the given Wiki Subpage
// If url starts with "RTD", it will be considered as a Read-the-docs link
void addHelpButton(const String& url) {
  if (url.startsWith("RTD")) {
    addRTDHelpButton(url.substring(3));
  } else {
    addHelpButton(url, false);
  }
}

void addRTDHelpButton(const String& url)
{
  addHelpButton(url, true);
}

void addHelpButton(const String& url, bool isRTD)
{
  addHtmlLink(
    F("button help"),
    makeDocLink(url, isRTD),
    isRTD ? F("&#8505;") : F("&#10068;"));
}

void addRTDPluginButton(int taskDeviceNumber) {
  String url;

  url.reserve(16);
  url = F("Plugin/P");

  if (taskDeviceNumber < 100) { url += '0'; }

  if (taskDeviceNumber < 10) { url += '0'; }
  url += String(taskDeviceNumber);
  url += F(".html");
  addRTDHelpButton(url);

  switch (taskDeviceNumber) {
    case 76:
    case 77:
      addHtmlLink(
        F("button help"),
        makeDocLink(F("Reference/Safety.html"), true),
        F("&#9889;")); // High voltage sign
      break;
  }
}

String makeDocLink(const String& url, bool isRTD) {
  String result;

  if (!url.startsWith(F("http"))) {
    if (isRTD) {
      result += F("https://espeasy.readthedocs.io/en/latest/");
    } else {
      result += F("http://www.letscontrolit.com/wiki/index.php/");
    }
  }
  result += url;
  return result;
}

// ********************************************************************************
// Add a GPIO pin select dropdown list for 8266, 8285 or ESP32
// ********************************************************************************
String createGPIO_label(int gpio, int pinnr, bool input, bool output, bool warning) {
  if (gpio < 0) { return F("- None -"); }
  String result;
  result.reserve(24);
  result  = F("GPIO-");
  result += gpio;

  if (pinnr >= 0) {
    result += F(" (D");
    result += pinnr;
    result += ')';
  }

  if (input != output) {
    result += ' ';
    result += input ? F(HTML_SYMBOL_INPUT) : F(HTML_SYMBOL_OUTPUT);
  }

  if (warning) {
    result += ' ';
    result += F(HTML_SYMBOL_WARNING);
  }
  bool serialPinConflict = (Settings.UseSerial && (gpio == 1 || gpio == 3));

  if (serialPinConflict) {
    if (gpio == 1) { result += F(" TX0"); }

    if (gpio == 3) { result += F(" RX0"); }
  }
  return result;
}

void addPinSelect(boolean forI2C, String id,  int choice)
{
  #ifdef ESP32
    # define NR_ITEMS_PIN_DROPDOWN  35 // 34 GPIO + 1
  #else // ifdef ESP32
    # define NR_ITEMS_PIN_DROPDOWN  14 // 13 GPIO + 1
  #endif // ifdef ESP32

  String *gpio_labels  = new String[NR_ITEMS_PIN_DROPDOWN];
  int    *gpio_numbers = new int[NR_ITEMS_PIN_DROPDOWN];

  // At i == 0 && gpio == -1, add the "- None -" option first
  int i    = 0;
  int gpio = -1;

  while (i < NR_ITEMS_PIN_DROPDOWN && gpio <= MAX_GPIO) {
    int  pinnr = -1;
    bool input, output, warning;

    if (getGpioInfo(gpio, pinnr, input, output, warning) || (i == 0)) {
      gpio_labels[i]  = createGPIO_label(gpio, pinnr, input, output, warning);
      gpio_numbers[i] = gpio;
      ++i;
    }
    ++gpio;
  }
  renderHTMLForPinSelect(gpio_labels, gpio_numbers, forI2C, id, choice, NR_ITEMS_PIN_DROPDOWN);
  delete[] gpio_numbers;
  delete[] gpio_labels;
  #undef NR_ITEMS_PIN_DROPDOWN
}

// ********************************************************************************
// Helper function actually rendering dropdown list for addPinSelect()
// ********************************************************************************
void renderHTMLForPinSelect(String options[], int optionValues[], boolean forI2C, const String& id,  int choice, int count) {
  addSelector_Head(id, false);

  for (byte x = 0; x < count; x++)
  {
    boolean disabled = false;

    if (optionValues[x] != -1) // empty selection can never be disabled...
    {
      if (!forI2C && ((optionValues[x] == Settings.Pin_i2c_sda) || (optionValues[x] == Settings.Pin_i2c_scl))) {
        disabled = true;
      }

      if (Settings.UseSerial && ((optionValues[x] == 1) || (optionValues[x] == 3))) {
        disabled = true;
      }
    }
    addSelector_Item(options[x],
                     optionValues[x],
                     choice == optionValues[x],
                     disabled,
                     "");
  }
  addSelector_Foot();
}

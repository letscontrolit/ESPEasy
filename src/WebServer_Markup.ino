// ********************************************************************************

// Add Selector
// ********************************************************************************
void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex)
{
  addSelector(id, optionCount, options, indices, attr, selectedIndex, false, true, F("wide"));
}

void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 boolean       reloadonchange,
                 bool          enabled)
{
  addSelector(id, optionCount, options, indices, attr, selectedIndex, reloadonchange, enabled, F("wide"));
}

void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 boolean       reloadonchange,
                 bool          enabled,
                 const String& classname)
{
  // FIXME TD-er Change boolean to disabled
  if (reloadonchange)
  {
    addSelector_Head_reloadOnChange(id, classname, !enabled);
  } else {
    do_addSelector_Head(id, classname, "", !enabled);
  }
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
    String attr_str;
    if (attr)
    {
      attr_str = attr[x];
    }
    addSelector_option(index, options[x], attr_str, selectedIndex == index);
  }
}

void addSelector_option(const int index, const String& option, const String& attr, bool isSelected)
{
  String html;
  html.reserve(36 + option.length() + attr.length());
  html += F("<option value=");
  html += index;

  if (isSelected) {
    html += F(" selected");
  }

  if (attr.length() != 0)
  {
    addHtml(" ");
    html += attr;
  }
  html += '>';
  html += option;
  html += F("</option>");
  addHtml(html);
}


void addSelector_Head(const String& id) {
  do_addSelector_Head(id, F("wide"), "", false);
}

void addSelector_Head_reloadOnChange(const String& id) {
  addSelector_Head_reloadOnChange(id, F("wide"), false);
}

void addSelector_Head_reloadOnChange(const String& id, const String& classname, bool disabled) {
  do_addSelector_Head(id, classname, F("return dept_onchange(frmselect)"), disabled);
}

void addSelector_Head_reloadOnChange(const String& id, const String& classname, const String& onChangeCall, bool disabled) {
  do_addSelector_Head(id, classname, onChangeCall, disabled);
}

void do_addSelector_Head(const String& id, const String& classname, const String& onChangeCall, const bool& disabled)
{
  {
    String html;
    html.reserve(32 + id.length());
    html += F("<select class='");
    html += classname;
    html += F("' name='");
    html += id;
    html += F("' id='");
    html += id;
    html += '\'';
    addHtml(html);
  }

  if (disabled) {
    addDisabled();
  }

  {
    String html;
    html.reserve(16 + onChangeCall.length());

    if (onChangeCall.length() > 0) {
      html += F(" onchange='");
      html += onChangeCall;
      html += '\'';
    }
    html += '>';
    addHtml(html);
  }
}

void addSelector_Item(const String& option, int index, boolean selected, boolean disabled, const String& attr)
{
  addHtml(F("<option value="));
  addHtml(String(index));

  if (selected) {
    addHtml(F(" selected"));
  }

  if (disabled) {
    addDisabled();
  }
  String html;
  html.reserve(48 + option.length() + attr.length());

  if (attr && (attr.length() > 0))
  {
    html += ' ';
    html += attr;
  }
  html += '>';
  html += option;
  html += F("</option>");
  addHtml(html);
}

void addSelector_Foot()
{
  addHtml(F("</select>"));
}

void addUnit(const String& unit)
{
  String html;

  html += F(" [");
  html += unit;
  html += "]";
  addHtml(html);
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
    String html;
    html += F("<TR id='");
    html += id;
    html += F("'><TD>");
    addHtml(html);
  } else {
    html_TR_TD();
  }

  if (label.length() != 0) {
    String html;
    html += label;
    html += ':';
    addHtml(html);
  }
  html_TD();
}

// Add a row label and mark it with copy markers to copy it to clipboard.
void addRowLabel_copy(const String& label) {
  addHtml(F("<TR>"));
  html_copyText_TD();
  String html;
  html += label;
  html += ':';
  addHtml(html);
  html_copyText_marker();
  html_copyText_TD();
}

void addRowLabelValue(LabelType::Enum label) {
  addRowLabel(getLabel(label));
  addHtml(getValue(label));
}

void addRowLabelValue_copy(LabelType::Enum label) {
  addRowLabel_copy(getLabel(label));
  addHtml(getValue(label));
}

// ********************************************************************************
// Add a header
// ********************************************************************************
void addTableSeparator(const String& label, int colspan, int h_size, const String& helpButton) {
  {
    String html;
    html.reserve(32 + label.length());
    html += F("<TR><TD colspan=");
    html += colspan;
    html += "><H";
    html += h_size;
    html += '>';
    html += label;
    addHtml(html);
  }

  if (helpButton.length() > 0) {
    addHelpButton(helpButton);
  }
  {
    String html;
    html.reserve(16);
    html += "</H";
    html += h_size;
    html += F("></TD></TR>");
    addHtml(html);
  }
}

void addFormHeader(const String& header, const String& helpButton) {
  addFormHeader(header, helpButton, F(""));
}
void addFormHeader(const String& header, const String& helpButton, const String& rtdHelpButton)
{
  html_TR();
  html_table_header(header, helpButton, rtdHelpButton, 225);
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
  addHtml(F("<label class='container'>&nbsp;"));
  addHtml(F("<input type='checkbox' id='"));
  {
    String html;
    html.reserve(16 + 2 * id.length());
    html += id;
    html += F("' name='");
    html += id;
    html += '\'';

    if (checked) {
      html += F(" checked");
    }
    addHtml(html);
  }

  if (disabled) { addDisabled(); }
  addHtml(F("><span class='checkmark"));

  if (disabled) { addDisabled(); }
  addHtml(F("'></span></label>"));
}

// ********************************************************************************
// Add a numeric box
// ********************************************************************************
void addNumericBox(const String& id, int value, int min, int max)
{
  addHtml(F("<input class='widenumber' type='number' name='"));
  if (value < min) {
    value = min;
  }
  if (value > max) {
    value = max;
  }
  String html;
  html.reserve(32 + id.length());
  html += id;
  html += '\'';

  if (min != INT_MIN)
  {
    html += F(" min=");
    html += min;
  }

  if (max != INT_MAX)
  {
    html += F(" max=");
    html += max;
  }
  html += F(" value=");
  html += value;
  html += '>';
  addHtml(html);
}

void addFloatNumberBox(const String& id, float value, float min, float max)
{
  String html;

  html.reserve(64 + id.length());

  html += F("<input type='number' name='");
  html += id;
  html += '\'';
  html += F(" min=");
  html += min;
  html += F(" max=");
  html += max;
  html += F(" step=0.01");
  html += F(" style='width:5em;' value=");
  html += value;
  html += '>';

  addHtml(html);
}

// ********************************************************************************
// Add Textbox
// ********************************************************************************
void addTextBox(const String& id, const String&  value, int maxlength, bool readonly, bool required, const String& pattern)
{
  addTextBox(id, value, maxlength, readonly, required, pattern, F("wide"));
}

void addTextBox(const String& id, const String&  value, int maxlength, bool readonly, bool required, const String& pattern, const String& classname)
{
  String html;

  html.reserve(96 + id.length() + value.length() + pattern.length());

  html += F("<input class='");
  html += classname;
  html += F("' type='text' name='");
  html += id;
  html += F("' maxlength=");
  html += maxlength;
  html += F(" value='");
  html += value;
  html += '\'';

  if (readonly) {
    html += F(" readonly ");
  }

  if (required) {
    html += F(" required ");
  }

  if (pattern.length() > 0) {
    html += F("pattern = '");
    html += pattern;
    html += '\'';
  }
  html += '>';

  addHtml(html);
}

// ********************************************************************************
// Add Textarea
// ********************************************************************************
void addTextArea(const String& id, const String& value, int maxlength, int rows, int columns, bool readonly, bool required)
{
  String html;

  html.reserve(96 + id.length() + value.length());

  html += F("<textarea class='wide' type='text' name='");
  html += id;
  html += F("' maxlength=");
  html += maxlength;
  html += F("' rows=");
  html += rows;
  html += F("' cols=");
  html += columns;
  html += '\'';

  if (readonly) {
    html += F(" readonly ");
  }

  if (required) {
    html += F(" required ");
  }
  html += '>';
  html += value;
  html += F("</textarea>");

  addHtml(html);
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

void addRTDPluginButton(pluginID_t taskDeviceNumber) {
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


void addPinSelect(boolean forI2C, const String& id,  int choice)
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

#ifdef ESP32
void addADC_PinSelect(bool touchOnly, const String& id,  int choice)
{
  int NR_ITEMS_PIN_DROPDOWN = touchOnly ? 10 : 19;
  String *gpio_labels  = new String[NR_ITEMS_PIN_DROPDOWN];
  int    *gpio_numbers = new int[NR_ITEMS_PIN_DROPDOWN];

  // At i == 0 && gpio == -1, add the "Hall Effect" option first
  int i    = 0;
  int gpio = -1;

  while (i < NR_ITEMS_PIN_DROPDOWN && gpio <= MAX_GPIO) {
    int  pinnr = -1;
    bool input, output, warning;
    if (touchOnly) {
      // For touch only list, sort based on touch number
      // Default sort is on GPIO number.
      gpio = touchPinToGpio(i);
    }

    if (getGpioInfo(gpio, pinnr, input, output, warning) || (i == 0)) {
      int adc,ch, t;
      if (getADC_gpio_info(gpio, adc, ch, t)) {
        if (!touchOnly || t >= 0) {
          gpio_labels[i] = formatGpioName_ADC(gpio);
          if (adc != 0) {
            gpio_labels[i] += F(" / ");
            gpio_labels[i] += createGPIO_label(gpio, pinnr, input, output, warning);
          }
          gpio_numbers[i] = gpio;
          ++i;
        }
      }
    }
    ++gpio;
  }
  bool forI2C = false;
  renderHTMLForPinSelect(gpio_labels, gpio_numbers, forI2C, id, choice, i);
  delete[] gpio_numbers;
  delete[] gpio_labels;
}


#endif


// ********************************************************************************
// Helper function actually rendering dropdown list for addPinSelect()
// ********************************************************************************
void renderHTMLForPinSelect(String options[], int optionValues[], boolean forI2C, const String& id,  int choice, int count) {
  addSelector_Head(id);

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

      if (Settings.InitSPI != 0) {
        #ifdef ESP32
        switch (Settings.InitSPI)
        {
        case 1:
          disabled = (optionValues[x] == 18 || optionValues[x] == 19 || optionValues[x] == 23);
          break;
        case 2:
          disabled = (optionValues[x] == 14 || optionValues[x] == 12 || optionValues[x] == 13);
          break;
        }
        #else // #ifdef ESP32
        disabled = (optionValues[x] == 14 || optionValues[x] == 12 || optionValues[x] == 13);
        #endif
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

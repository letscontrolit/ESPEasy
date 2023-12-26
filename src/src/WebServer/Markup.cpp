
#include "../WebServer/Markup.h"

#include "../WebServer/HTML_wrappers.h"

#include "../CustomBuild/ESPEasyLimits.h"

#include "../Globals/Settings.h"

#include "../Helpers/Convert.h"
#include "../Helpers/Hardware_GPIO.h"
#include "../Helpers/StringConverter_Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"

#include "../../ESPEasy_common.h"

// ********************************************************************************
// Add Selector
// ********************************************************************************
void addSelector(const __FlashStringHelper *id,
                 int                        optionCount,
                 const __FlashStringHelper *options[],
                 const int                  indices[],
                 const String               attr[],
                 int                        selectedIndex,
                 bool                       reloadonchange,
                 bool                       enabled)
{
  addSelector(String(id), optionCount, options, indices, attr, selectedIndex, reloadonchange, enabled, F("wide"));
}

void addSelector(const String             & id,
                 int                        optionCount,
                 const __FlashStringHelper *options[],
                 const int                  indices[],
                 const String               attr[],
                 int                        selectedIndex,
                 bool                       reloadonchange,
                 bool                       enabled)
{
  addSelector(id, optionCount, options, indices, attr, selectedIndex, reloadonchange, enabled, F("wide"));
}

void addSelector(const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 bool          reloadonchange,
                 bool          enabled)
{
  addSelector(id, optionCount, options, indices, attr, selectedIndex, reloadonchange, enabled, F("wide"));
}

void addSelector(const String             & id,
                 int                        optionCount,
                 const __FlashStringHelper *options[],
                 const int                  indices[],
                 const String               attr[],
                 int                        selectedIndex,
                 bool                       reloadonchange,
                 bool                       enabled,
                 const __FlashStringHelper * classname
                 #if FEATURE_TOOLTIPS
                 , const String           & tooltip
                 #endif // if FEATURE_TOOLTIPS
                 )
{
  // FIXME TD-er Change bool    to disabled
  if (reloadonchange)
  {
    addSelector_Head_reloadOnChange(id, classname, !enabled
                                    #if FEATURE_TOOLTIPS
                                    , tooltip
                                    #endif // if FEATURE_TOOLTIPS
                                    );
  } else {
    do_addSelector_Head(id, classname, EMPTY_STRING, !enabled
                        #if FEATURE_TOOLTIPS
                        , tooltip
                        #endif // if FEATURE_TOOLTIPS
                        );
  }
  addSelector_options(optionCount, options, indices, attr, selectedIndex);
  addSelector_Foot();
}

void addSelector_reloadOnChange(
                 const String& id,
                 int           optionCount,
                 const String  options[],
                 const int     indices[],
                 const String  attr[],
                 int           selectedIndex,
                 const String& onChangeCall,
                 bool          enabled,
                 const __FlashStringHelper * classname
                 #if FEATURE_TOOLTIPS
                 ,
                 const String& tooltip
                 #endif // if FEATURE_TOOLTIPS
                 )
{
  // FIXME TD-er Change bool    to disabled
  do_addSelector_Head(id, classname, onChangeCall, !enabled
                      #if FEATURE_TOOLTIPS
                      , tooltip
                      #endif // if FEATURE_TOOLTIPS
                      );
  addSelector_options(optionCount, options, indices, attr, selectedIndex);
  addSelector_Foot();
}


void addSelector(const String  & id,
                 int             optionCount,
                 const String    options[],
                 const int       indices[],
                 const String    attr[],
                 int             selectedIndex,
                 bool            reloadonchange,
                 bool            enabled,
                 const __FlashStringHelper * classname
                 #if FEATURE_TOOLTIPS
                 , const String& tooltip
                 #endif // if FEATURE_TOOLTIPS
                 )
{
  // FIXME TD-er Change bool    to disabled
  if (reloadonchange)
  {
    addSelector_Head_reloadOnChange(id, classname, !enabled
                                    #if FEATURE_TOOLTIPS
                                    , tooltip
                                    #endif // if FEATURE_TOOLTIPS
                                    );
  } else {
    do_addSelector_Head(id, classname, EMPTY_STRING, !enabled
                        #if FEATURE_TOOLTIPS
                        , tooltip
                        #endif // if FEATURE_TOOLTIPS
                        );
  }
  addSelector_options(optionCount, options, indices, attr, selectedIndex);
  addSelector_Foot();
}

void addSelector_options(int optionCount, const __FlashStringHelper *options[], const int indices[], const String attr[], int selectedIndex)
{
  for (uint8_t x = 0; x < optionCount; ++x)
  {
    const int index = indices ? indices[x] : x;
    addSelector_Item(
      options[x], 
      index, 
      selectedIndex == index, 
      false, 
      attr ? attr[x] : EMPTY_STRING);
    if ((x & 0x07) == 0) delay(0);
  }
}

void addSelector_options(int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex)
{
  for (uint8_t x = 0; x < optionCount; ++x)
  {
    const int index = indices ? indices[x] : x;
    addSelector_Item(
      options[x], 
      index, 
      selectedIndex == index, 
      false, 
      attr ? attr[x] : EMPTY_STRING);
    if ((x & 0x07) == 0) delay(0);
  }
}

void addSelector_Head(const String& id) {
  do_addSelector_Head(id, F("wide"), EMPTY_STRING, false
                      #if FEATURE_TOOLTIPS
                      , F("")
                      #endif // if FEATURE_TOOLTIPS
                      );
}

void addSelector_Head_reloadOnChange(const __FlashStringHelper * id) {
  addSelector_Head_reloadOnChange(String(id), F("wide"), false);
}

/*
void addSelector_Head_reloadOnChange(const String& id) {
  addSelector_Head_reloadOnChange(id, F("wide"), false);
}
*/

void addSelector_Head_reloadOnChange(const String& id, 
                                     const __FlashStringHelper * classname, 
                                     bool disabled
                                     #if FEATURE_TOOLTIPS
                                     , const String& tooltip
                                     #endif // if FEATURE_TOOLTIPS
                                     ) {
  do_addSelector_Head(id, classname, F("return dept_onchange(frmselect)"), disabled
                      #if FEATURE_TOOLTIPS
                      , tooltip
                      #endif // if FEATURE_TOOLTIPS
                      );
}

void addSelector_Head_reloadOnChange(const String& id, const __FlashStringHelper * classname, const String& onChangeCall, bool disabled
                                     #if FEATURE_TOOLTIPS
                                     , const String& tooltip
                                     #endif // if FEATURE_TOOLTIPS
                                     ) {
  do_addSelector_Head(id, classname, onChangeCall, disabled
                      #if FEATURE_TOOLTIPS
                      , tooltip
                      #endif // if FEATURE_TOOLTIPS
                      );
}


void do_addSelector_Head(const String& id, const __FlashStringHelper * classname, const String& onChangeCall, const bool& disabled
                         #if FEATURE_TOOLTIPS
                         , const String& tooltip
                         #endif // if FEATURE_TOOLTIPS
                         )
{
  addHtml(F("<select "));
  addHtmlAttribute(F("class"), classname);
  addHtmlAttribute(F("name"),  id);
  addHtmlAttribute(F("id"),    id);

  #if FEATURE_TOOLTIPS

  if (tooltip.length() > 0) {
    addHtmlAttribute(F("title"), tooltip);
  }
  #endif // if FEATURE_TOOLTIPS

  if (disabled) {
    addDisabled();
  }

  if (onChangeCall.length() > 0) {
    addHtmlAttribute(F("onchange"), onChangeCall);
  }
  addHtml('>');
}

void addPinSelector_Item(PinSelectPurpose purpose, const String& gpio_label, int gpio, bool    selected, bool    disabled, const String& attr)
{
  if (gpio != -1) // empty selection can never be disabled...
  {
    int  pinnr = -1;
    bool input, output, warning;

    if (getGpioInfo(gpio, pinnr, input, output, warning)) {
      bool includeI2C = true;
      bool includeSPI = true;
      bool includeSerial = true;
      #if FEATURE_ETHERNET
      bool includeEthernet = true;
      #endif // if FEATURE_ETHERNET
      #if FEATURE_SD
      bool includeSDCard = true;
      #endif // if FEATURE_SD

      switch (purpose) {
        case PinSelectPurpose::SPI:
        case PinSelectPurpose::SPI_MISO:
          includeSPI = false;
          if (purpose == PinSelectPurpose::SPI && !output) {
            return;
          }
          break;
        case PinSelectPurpose::Ethernet:
          #if FEATURE_ETHERNET
          includeEthernet = false;
          #endif // if FEATURE_ETHERNET
          break;
        case PinSelectPurpose::Generic:

          if (!input && !output) {
            return;
          }
          break;

        case PinSelectPurpose::Generic_input:

          if (!input) {
            return;
          }
          break;

        case PinSelectPurpose::Generic_output:
        case PinSelectPurpose::DAC:

          if (!output) {
            return;
          }
          break;

        case PinSelectPurpose::Generic_bidir:
        case PinSelectPurpose::I2C:
          includeI2C = false;

          if (!output || !input) {
            // SDA is obviously bidirectional.
            // SCL is obviously output, but can be held down by a slave device to signal clock stretch limit.
            // Thus both must be capable of input & output.
            return;
          }
          break;
          
        case PinSelectPurpose::Serial_input:
          includeSerial = false;
          if (!input) {
            return;
          }
          break;

        case PinSelectPurpose::Serial_output:
          includeSerial = false;
          if (!output) {
            return;
          }
          break;
        #if FEATURE_SD
        case PinSelectPurpose::SD_Card:
          includeSDCard = false;
          if (!output) {
            return;
          }
          break;
        #endif
      }

      if (includeI2C && Settings.isI2C_pin(gpio)) {
        disabled = true;
      }

      if (includeSerial && isSerialConsolePin(gpio)) {
        disabled = true;
      }

      if (includeSPI && Settings.isSPI_pin(gpio)) {
        disabled = true;
      }

  #if FEATURE_ETHERNET

      if (Settings.isEthernetPin(gpio) || (includeEthernet && Settings.isEthernetPinOptional(gpio))) {
        disabled = true;
      }
  #endif // if FEATURE_ETHERNET

      #if FEATURE_SD
      if (includeSDCard && (Settings.Pin_sd_cs == gpio)) {
        disabled = true;
      }
      #endif
    }
  }

  addSelector_Item(gpio_label,
                   gpio,
                   selected,
                   disabled);
}

void addSelector_Item(const __FlashStringHelper *option, int index, bool    selected, bool    disabled, const String& attr)
{
  addHtml(F("<option "));
  addHtmlAttribute(F("value"), index);

  if (selected) {
    addHtml(F(" selected"));
  }

  if (disabled) {
    addDisabled();
  }

  if (attr.length() > 0)
  {
    addHtml(' ');
    addHtml(attr);
  }
  addHtml('>');
  addHtml(option);
  addHtml(F("</option>"));
}

void addSelector_Item(const String& option, int index, bool    selected, bool    disabled, const String& attr)
{
  addHtml(F("<option "));
  addHtmlAttribute(F("value"), index);

  if (selected) {
    addHtml(F(" selected"));
  }

  if (disabled) {
    addDisabled();
  }

  if (attr.length() > 0)
  {
    addHtml(' ');
    addHtml(attr);
  }
  addHtml('>');
  addHtml(option);
  addHtml(F("</option>"));
}

void addSelector_Foot()
{
  addHtml(F("</select>"));
}

void addUnit(const __FlashStringHelper *unit)
{
  addHtml(F(" ["));
  addHtml(unit);
  addHtml(']');
}

void addUnit(const String& unit)
{
  addHtml(F(" ["));
  addHtml(unit);
  addHtml(']');
}

void addUnit(char unit)
{
  addHtml(F(" ["));
  addHtml(unit);
  addHtml(']');
}

void addRowLabel_tr_id(const __FlashStringHelper *label, const __FlashStringHelper *id)
{
  addRowLabel_tr_id(label, String(id));
}

void addRowLabel_tr_id(const __FlashStringHelper *label, const String& id)
{
  if (id.isEmpty()) {
    addRowLabel(label);
  } else {
    addRowLabel_tr_id(String(label), id);
  }
}

void addRowLabel_tr_id(const String& label, const String& id)
{
  if (id.isEmpty()) {
    addRowLabel(label);
  } else {
    addRowLabel(label, concat(F("tr_"), id));
  }
}

void addRowLabel(const __FlashStringHelper *label)
{
  html_TR_TD();
  addHtml(concat(label, F(":</td>")));
  html_TD();
}

void addRowLabel(const String& label, const String& id)
{
  if (id.length() > 0) {
    addHtml(F("<TR id='"));
    addHtml(id);
    addHtml(F("'><TD>"));
  } else {
    html_TR_TD();
  }

  if (!label.isEmpty()) {
    addHtml(label);
    addHtml(':');
  }
  addHtml(F("</td>"));
  html_TD();
}

// Add a row label and mark it with copy markers to copy it to clipboard.
void addRowLabel_copy(const __FlashStringHelper *label) {
  addHtml(F("<TR>"));
  html_copyText_TD();
  addHtml(label);
  addHtml(':');
  html_copyText_marker();
  html_copyText_TD();
}

void addRowLabel_copy(const String& label) {
  addHtml(F("<TR>"));
  html_copyText_TD();
  addHtml(label);
  addHtml(':');
  html_copyText_marker();
  html_copyText_TD();
}

void addRowLabel(LabelType::Enum label) {
  addRowLabel(getLabel(label));
}

void addRowLabelValue(LabelType::Enum label) {
  addRowLabel(getLabel(label));
  addHtml(getValue(label));
}

void addRowLabelValues(const LabelType::Enum labels[]) {
  size_t i = 0;
  LabelType::Enum cur  = static_cast<const LabelType::Enum>(pgm_read_byte(labels + i));

  while (true) {
    const LabelType::Enum next = static_cast<const LabelType::Enum>(pgm_read_byte(labels + i + 1));
    addRowLabelValue(cur);
    if (next == LabelType::MAX_LABEL) {
      return;
    }
    ++i;
    cur = next;
  }
}

void addRowLabelValue_copy(LabelType::Enum label) {
  addRowLabel_copy(getLabel(label));
  addHtml(getValue(label));
}

// ********************************************************************************
// Add a header
// ********************************************************************************
void addTableSeparator(const __FlashStringHelper *label, int colspan, int h_size)
{
  addHtml(strformat(
    F("<TR><TD colspan=%d><H%d>"),
    colspan, h_size));
  addHtml(label);
  addHtml(strformat(
    F("</H%d></TD></TR>"),
    h_size));
}

void addTableSeparator(const __FlashStringHelper *label, int colspan, int h_size, const __FlashStringHelper *helpButton)
{
  addTableSeparator(String(label), colspan, h_size, String(helpButton));
}

void addTableSeparator(const String& label, int colspan, int h_size, const String& helpButton) {
  addHtml(strformat(
    F("<TR><TD colspan=%d><H%d>"),
    colspan, h_size));
  addHtml(label);

  if (!helpButton.isEmpty()) {
    addHelpButton(helpButton);
  }
  addHtml(strformat(
    F("</H%d></TD></TR>"),
    h_size));
}

void addFormHeader(const __FlashStringHelper *header) {
  addFormHeader(header, F(""), F(""));
}

void addFormHeader(const __FlashStringHelper *header,
                   const __FlashStringHelper *helpButton)
{
  addFormHeader(header, helpButton, F(""));
}

void addFormHeader(const __FlashStringHelper *header,
                   const __FlashStringHelper *helpButton,
                   const __FlashStringHelper *rtdHelpButton)
{
  html_TR();
  html_table_header(header, helpButton, rtdHelpButton, 300);
  html_table_header(F(""));
}

/*
void addFormHeader(const String& header, const String& helpButton) {
  addFormHeader(header, helpButton, EMPTY_STRING);
}

void addFormHeader(const String& header, const String& helpButton, const String& rtdHelpButton)
{
  html_TR();
  html_table_header(header, helpButton, rtdHelpButton, 225);
  html_table_header(F(""));
}
*/

// ********************************************************************************
// Add a sub header
// ********************************************************************************
void addFormSubHeader(const __FlashStringHelper *header) {
  addTableSeparator(header, 2, 3);
}

void addFormSubHeader(const String& header)
{
  addTableSeparator(header, 2, 3);
}

// ********************************************************************************
// Add a checkbox
// ********************************************************************************
void addCheckBox(const __FlashStringHelper *id, bool    checked, bool disabled)
{
  addCheckBox(String(id), checked, disabled);
}

void addCheckBox(const String& id, bool    checked, bool disabled
                 #if FEATURE_TOOLTIPS
                 , const String& tooltip
                 #endif // if FEATURE_TOOLTIPS
                 )
{
  addHtml(F("<label class='container'>&nbsp;"));
  addHtml(F("<input "));
  addHtmlAttribute(F("type"), F("checkbox"));
  addHtmlAttribute(F("id"),   id);
  addHtmlAttribute(F("name"), id);

  if (checked) {
    addHtml(F(" checked"));
  }

  if (disabled) { addDisabled(); }
  addHtml(F("><span class='checkmark"));

  if (disabled) { addDisabled(); }
  addHtml('\'');
  #if FEATURE_TOOLTIPS

  if (tooltip.length() > 0) {
    addHtmlAttribute(F("title"), tooltip);
  }
  #endif // if FEATURE_TOOLTIPS
  addHtml(F("></span></label>"));
}

// ********************************************************************************
// Add a numeric box
// ********************************************************************************
void addNumericBox(const __FlashStringHelper *id, int value, int min, int max, bool disabled)
{
  addNumericBox(String(id), value, min, max, disabled);
}

void addNumericBox(const String& id, int value, int min, int max
                   #if FEATURE_TOOLTIPS
                   , const __FlashStringHelper * classname, const String& tooltip
                   #endif // if FEATURE_TOOLTIPS
                   , bool disabled
                   )
{
  addHtml(F("<input "));
  #if FEATURE_TOOLTIPS
  addHtmlAttribute(F("class"), classname);
  #else // if FEATURE_TOOLTIPS
  addHtmlAttribute(F("class"), F("widenumber"));
  #endif  // if FEATURE_TOOLTIPS
  addHtmlAttribute(F("type"),  F("number"));
  addHtmlAttribute(F("name"),  id);
  addHtmlAttribute(F("id"),    id);

  #if FEATURE_TOOLTIPS

  if (tooltip.length() > 0) {
    addHtmlAttribute(F("title"), tooltip);
  }
  #endif // if FEATURE_TOOLTIPS

  if (disabled) {
    addDisabled();
  }

  if (value < min) {
    value = min;
  }

  if (value > max) {
    value = max;
  }

  if (min != INT_MIN)
  {
    addHtmlAttribute(F("min"), min);
  }

  if (max != INT_MAX)
  {
    addHtmlAttribute(F("max"), max);
  }
  addHtmlAttribute(F("value"), value);
  addHtml('>');
}

#if FEATURE_TOOLTIPS
void addNumericBox(const String& id, int value, int min, int max, bool disabled)
{
  addNumericBox(id, value, min, max, F("widenumber"), EMPTY_STRING, disabled);
}

#endif // if FEATURE_TOOLTIPS

void addFloatNumberBox(const String& id, float value, float min, float max, unsigned int nrDecimals, float stepsize
                       #if FEATURE_TOOLTIPS
                       , const String& tooltip
                       #endif // if FEATURE_TOOLTIPS
                       )
{
  addHtml(strformat(
      F("<input type='number' name='%s' min="),
      id.c_str()));
  addHtmlFloat(min, nrDecimals);
  addHtml(F(" max="));
  addHtmlFloat(max, nrDecimals);
  addHtml(F(" step="));

  if (stepsize <= 0.0f) {
    addHtml('0', '.');

    for (uint8_t i = 1; i < nrDecimals; ++i) {
      addHtml('0');
    }
    addHtml('1');
  } else {
    addHtmlFloat(stepsize, nrDecimals);
  }

  addHtml(F(" style='width:7em;' value="));
  addHtmlFloat(value, nrDecimals);

  #if FEATURE_TOOLTIPS

  if (!tooltip.isEmpty()) {
    addHtml(strformat(
      F("title='%s' "), tooltip.c_str()));
  }
  #endif // if FEATURE_TOOLTIPS
  addHtml('>');
}

// ********************************************************************************
// Add Textbox
// ********************************************************************************
void addTextBox(const __FlashStringHelper * id, const String&  value, int maxlength, bool readonly, bool required, const String& pattern) {
  addTextBox(id, value, maxlength, readonly, required, pattern, F("wide"));
}

void addTextBox(const String& id, const String&  value, int maxlength, bool readonly, bool required, const String& pattern) {
  addTextBox(id, value, maxlength, readonly, required, pattern, F("wide"));
}

void addTextBox(const String  & id,
                const String  & value,
                int             maxlength,
                bool            readonly,
                bool            required,
                const String  & pattern,
                const __FlashStringHelper * classname
                #if FEATURE_TOOLTIPS
                , const String& tooltip
                #endif // if FEATURE_TOOLTIPS
                )
{
  addHtml(F("<input "));
  addHtmlAttribute(F("class"),     classname);
  addHtmlAttribute(F("type"),      F("search"));
  addHtmlAttribute(F("name"),      id);
  addHtmlAttribute(F("id"),        id);
  if (maxlength > 0) {
    addHtmlAttribute(F("maxlength"), maxlength);
  }
  addHtmlAttribute(F("value"),     value);

  if (readonly) {
    addHtml(F(" readonly "));
  }

  if (required) {
    addHtml(F(" required "));
  }

  if (pattern.length() > 0) {
    addHtmlAttribute(F("pattern"), pattern);
  }

  #if FEATURE_TOOLTIPS

  if (tooltip.length() > 0) {
    addHtmlAttribute(F("title"), tooltip);
  }
  #endif // if FEATURE_TOOLTIPS
  addHtml('>');
}



// ********************************************************************************
// Add Textarea
// ********************************************************************************
void addTextArea(const String  & id,
                 const String  & value,
                 int             maxlength,
                 int             rows,
                 int             columns,
                 bool            readonly,
                 bool          required
                 #if FEATURE_TOOLTIPS
                 , const String& tooltip
                 #endif // if FEATURE_TOOLTIPS
                 )
{
  addHtml(F("<textarea "));
  addHtmlAttribute(F("class"),     F("wide"));
  addHtmlAttribute(F("type"),      F("text"));
  addHtmlAttribute(F("name"),      id);
  addHtmlAttribute(F("id"),        id);
  if (maxlength > 0) {
    addHtmlAttribute(F("maxlength"), maxlength);
  }
  addHtmlAttribute(F("rows"),      rows);
  addHtmlAttribute(F("cols"),      columns);

  if (readonly) {
    addHtml(F(" readonly "));
  }

  if (required) {
    addHtml(F(" required "));
  }

  #if FEATURE_TOOLTIPS

  if (tooltip.length() > 0) {
    addHtmlAttribute(F("title"), tooltip);
  }
  #endif // if FEATURE_TOOLTIPS
  addHtml('>');
  addHtml(value);
  addHtml(F("</textarea>"));
}

// ********************************************************************************
// Add Help Buttons
// ********************************************************************************

// adds a Help Button with points to the the given Wiki Subpage
// If url starts with "RTD", it will be considered as a Read-the-docs link
void addHelpButton(const __FlashStringHelper *url) {
  addHelpButton(String(url));
}

void addHelpButton(const String& url) {
#ifndef WEBPAGE_TEMPLATE_HIDE_HELP_BUTTON

  if (url.startsWith("RTD")) {
    addRTDHelpButton(url.substring(3));
  } else {
    addHelpButton(url, false);
  }
#endif // ifndef WEBPAGE_TEMPLATE_HIDE_HELP_BUTTON
}

void addRTDHelpButton(const String& url)
{
  addHelpButton(url, true);
}

void addHelpButton(const String& url, bool isRTD)
{
  #ifndef WEBPAGE_TEMPLATE_HIDE_HELP_BUTTON
  addHtmlLink(
    F("button help"),
    makeDocLink(url, isRTD),
    isRTD ? F("i") : F("?"));
  #endif // ifndef WEBPAGE_TEMPLATE_HIDE_HELP_BUTTON
}

void addRTDPluginButton(pluginID_t pluginID) {
  addRTDHelpButton(
    strformat(
      F("Plugin/%s.html"),
      get_formatted_Plugin_number(pluginID).c_str()));

  constexpr pluginID_t PLUGIN_ID_P076_HLW8012(76);
  constexpr pluginID_t PLUGIN_ID_P077_CSE7766(77);

  if ((pluginID == PLUGIN_ID_P076_HLW8012) || 
      (pluginID == PLUGIN_ID_P077_CSE7766)) {
    addHtmlLink(
      F("button help"),
      makeDocLink(F("Reference/Safety.html"), true),
      F("&#9889;")); // High voltage sign
  }
}

# ifndef LIMIT_BUILD_SIZE
void addRTDControllerButton(cpluginID_t cpluginID) {
  addRTDHelpButton(
    strformat(
      F("Controller/%s.html"),
      get_formatted_Controller_number(cpluginID).c_str()));
}
# endif // ifndef LIMIT_BUILD_SIZE

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

void addPinSelect(PinSelectPurpose purpose, const __FlashStringHelper *id,  int choice)
{
  addPinSelect(purpose, String(id), choice);
}

void addPinSelect(PinSelectPurpose purpose, const String& id,  int choice)
{
  addSelector_Head(id);

  // At i == 0 && gpio == -1, add the "- None -" option first
  int i    = 0;
  int gpio = -1;

  while (gpio <= MAX_GPIO) {
    int  pinnr = -1;
    bool input, output, warning = false;

    // Make sure getGpioInfo is called (compiler may optimize it away if (i == 0))
    const bool UsableGPIO = getGpioInfo(gpio, pinnr, input, output, warning);

    if (UsableGPIO || (i == 0)) {
      addPinSelector_Item(
        purpose,
        concat(
          createGPIO_label(gpio, pinnr, input, output, warning),
          getConflictingUse_wrapped(gpio, purpose)),
        gpio,
        choice == gpio);

      ++i;
    }
    ++gpio;
  }
  addSelector_Foot();
}

#ifdef ESP32
void addADC_PinSelect(AdcPinSelectPurpose purpose, const String& id,  int choice)
{
  addSelector_Head(id);

  // At i == 0 && gpio == -1, add the "Hall Effect" option first
  int i    = 0;
  int gpio = -1;

  if (
#if HAS_HALL_EFFECT_SENSOR
    (purpose == AdcPinSelectPurpose::ADC_Touch_HallEffect) ||
#endif
      (purpose == AdcPinSelectPurpose::ADC_Touch_Optional)) {
    addPinSelector_Item(
      PinSelectPurpose::Generic,
      purpose == AdcPinSelectPurpose::ADC_Touch_Optional ? F("- None -") : formatGpioName_ADC(gpio),
      gpio,
      choice == gpio);
  }

  while (i <= MAX_GPIO && gpio <= MAX_GPIO) {
    int  pinnr = -1;
    bool input, output, warning;

    if (purpose == AdcPinSelectPurpose::TouchOnly) {
      // For touch only list, sort based on touch number
      // Default sort is on GPIO number.
      gpio = touchPinToGpio(i);
    } else {
      ++gpio;
    }

    if (getGpioInfo(gpio, pinnr, input, output, warning)) {
      int adc, ch, t;

      if (getADC_gpio_info(gpio, adc, ch, t)) {
        if ((purpose != AdcPinSelectPurpose::TouchOnly) || (t >= 0)) {
          String gpio_label;
          gpio_label = formatGpioName_ADC(gpio);

          if (adc != 0) {
            gpio_label += F(" / ");
            gpio_label += createGPIO_label(gpio, pinnr, input, output, warning);
            gpio_label += getConflictingUse_wrapped(gpio);
          }
          addPinSelector_Item(
            PinSelectPurpose::Generic,
            gpio_label,
            gpio,
            choice == gpio);
        }
      }
    }
    ++i;
  }
  addSelector_Foot();
}

void addDAC_PinSelect(const String& id,  int choice)
{
  addSelector_Head(id);

  // At i == 0 && gpio == -1, add the "- None -" option first
  int i    = 0;
  int gpio = -1;

  while (gpio <= MAX_GPIO) {
    int  pinnr   = -1;
    bool input   = false;
    bool output  = false;
    bool warning = false;
    int  dac     = 0;

    // Make sure getGpioInfo is called (compiler may optimize it away if (i == 0))
    const bool UsableGPIO = getDAC_gpio_info(gpio, dac); // getGpioInfo(gpio, pinnr, input, output, warning);

    if (UsableGPIO || (i == 0)) {
      if (getGpioInfo(gpio, pinnr, input, output, warning) || (i == 0)) {
        String gpio_label = formatGpioName_DAC(gpio);

        if (dac != 0) {
          gpio_label += F(" / ");
          gpio_label += createGPIO_label(gpio, pinnr, input, output, warning);
          gpio_label += getConflictingUse_wrapped(gpio, PinSelectPurpose::DAC);
        }
        addPinSelector_Item(
          PinSelectPurpose::DAC,
          gpio_label,
          gpio,
          choice == gpio);
      }
      ++i;
    }
    ++gpio;
  }
  addSelector_Foot();
}

#endif // ifdef ESP32

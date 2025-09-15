
#include "../WebServer/Markup.h"

#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup_Forms.h"

#include "../CustomBuild/ESPEasyLimits.h"

#include "../Globals/Settings.h"

#include "../Helpers/Convert.h"
#include "../Helpers/Hardware_GPIO.h"
#include "../Helpers/StringConverter_Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"

#include "../../ESPEasy_common.h"

#ifdef ESP32
# include "soc/soc_caps.h"
#endif


#if FEATURE_TOOLTIPS
void addTooltip(const String& tooltip)
{
  if (tooltip.length() > 0) {
    addHtmlAttribute(F("title"), tooltip);
  }
}
#endif


void addSelector_Head(const String& id) {
  do_addSelector_Head(id, F("wide"), EMPTY_STRING, false);
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
  addSelector_Head_reloadOnChange(
    id, 
    classname, 
    F("return dept_onchange(frmselect)"), 
    disabled
#if FEATURE_TOOLTIPS
    , tooltip
#endif // if FEATURE_TOOLTIPS
  );
}

void addSelector_Head_reloadOnChange(const String& id,
                                     const __FlashStringHelper * classname, 
                                     const String& onChangeCall, 
                                     bool disabled
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
  addTooltip(tooltip);
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
      // bool includeStatusLed = true; // Added as place-holders, see below
      // bool includeResetPin = true;

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

        case PinSelectPurpose::I2C:
#if FEATURE_I2C_MULTIPLE
        case PinSelectPurpose::I2C_2:
#if FEATURE_I2C_INTERFACE_3
        case PinSelectPurpose::I2C_3:
#endif
#endif
          includeI2C = false;
          // fallthrough
        case PinSelectPurpose::Generic_bidir:

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
        
        case PinSelectPurpose::Status_led:
          // includeStatusLed = false; // Placeholder, see below
          if (!output) {
            return;
          }
          break;

        case PinSelectPurpose::Reset_pin:
          // includeResetPin = false; // Placeholder, see below
          if (!input) {
            return;
          }  
          break;
  
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

      // Not blocking these GPIO pins, as they may already be in dual-purpose use, just a place-holder
      // if (includeStatusLed && (Settings.Pin_status_led == gpio)) {
      //   disabled = true;
      // }
      
      // if (includeResetPin && (Settings.Pin_Reset == gpio)) {
      //   disabled = true;
      // }
  
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

void addSelector_OptGroup(const String& label) {
  addHtml(F("<optgroup label=\""));
  addHtml(label);
  addHtml('\"', '>');
}

void addSelector_OptGroupFoot() {
  addHtml(F("</optgroup>"));
}

void addSelector_Foot(bool reloadonchange)
{
  addHtml(F("</select>"));
  if (reloadonchange) {
#if FEATURE_TOOLTIPS
    addHtml(F("<tt"));
    addTooltip(F("Change will submit and reload page"));
    addHtml(F(">&#128260;</tt>"));
#else
    addHtml(F("&#128260;"));
#endif
  }
}

void addUnit(const __FlashStringHelper *unit)
{
  // Needed so we can check whether it is empty
  addUnit(String(unit));
}

void addUnit(const String& unit)
{
  if (unit.isEmpty()) return;
  addHtml(F(" ["));
  addHtml(unit);
  addHtml(']');
}

void addUnit(char unit)
{
  if (unit == '\0') return;
  addHtml(F(" ["));
  addHtml(unit);
  addHtml(']');
}

#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
const char unit_of_measure_list[] PROGMEM = // *** DO NOT CHANGE ORDER, SAVED IN TASK SETTINGS! ***
 "|" // 0 = Empty/none
 "°C|°F|K|" // 1..3
 "%|" // 4
 "Pa|hPa|bar|mbar|inHg|psi|" // 5..10
 "W|kW|" // 11..12
 "V|" // 13
 "Wh|kWh|" // 14..15
 "A|VA|" // 16..17
 "mm|cm|m|km|" // 18..21
 "L|mL|m³|ft³|" // 22..25
 "m³/h|ft³/h|" // 26..27
 "lx|" // 28
 "UV index|" // 29
 "µg/m³|mg/m³|p/m³|ppm|ppb|" // 30..34
 "°|" // 35
 "€|$|¢|" // 36..38
 "μs|ms|s|min|h|d|w|m|y|" // 39..47
 "in|ft|yd|mi|" // 48..51
 "Hz|GHz|" // 52..53
 "gal|fl. oz|" // 54..55
 "m²|" // 56
 "g|kg|mg|µg|" // 57..60
 "oz|lb|" // 61..62
 "µS/cm|" // 63
 "W/m²|" // 64
 "mm/h|" // 65
 "mm/s|in/s|m/s|in/h|km/h|mph|" // 66..71
 "db|dBm|" // 72..73
 "bit|kbit|Mbit|Gbit|B|kB|MB|GB|TB|PB|EB|ZB|YB|KiB|MiB|GiB|TiB|PiB|EiB|ZiB|YiB|" // 74..94
 "bit/s|kbit/s|Mbit/s|Gbit/s|B/s|kB/s|MB/s|GB/s|KiB/s|MiB/s|GiB/s|" // 95..105
 "ft/s|kn|" // 106..107
 "mW|MW|GW|TW|" // 108..111
 "BTU/(h⋅ft²)|" // 112
 "pH|" // 113
 "cbar|mmHg|kPa|" // 114..116
 "mA|µA|mV|µV|kV|" // 117..121
 "cm²|km²|mm²|in²|ft²|yd²|mi²|ac|ha|" // 122..130
 "kHz|MHz|" // 131..132
 "mWh|MWh|GWh|TWh|cal|kcal|Mcal|Gcal|J|kJ|MJ|GJ|" // 133..144
 "var|kvar|varh|kvarh|" // 145..148
 "st|" // 149
 "mg/dL|mmol/L|" // 150..151
 "μSv|μSv/h|" // 152..153
 "m³/s|ft³/min|L/h|L/min|L/s|gal/min|mL/s|" // 154..160
 "g/m³|kWh/100km|Wh/km|mi/kWh|km/kWh|" // 161..165
 "in/d|mm/d" // 166 .. 167
 ; // *** DO NOT CHANGE ORDER, SAVED IN TASK SETTINGS! ***


const char unit_of_measure_labels[] PROGMEM = // Not stored, when UoM index >= 1024 it's a label-index with 1024 subtracted
 "Apparent power|Air quality/CO/CO2|Area|(Atmospheric) Pressure|" // A 1024..1027
 "Blood glucose concentr.|" // B 1028
 "Data rate|Data size|Distance|Duration|" // D 1029..1032
 "Energy distance|Energy(-storage)|" // E 1033..1034
 "Frequency|" // F 1035
 "Gas|" // G 1036
 "Percent Hum./Batt./Moist.|" // H 1037
 "Illuminance|Irradiance|" // I 1038..1039
 "Monetary|" // M 1040
 "Nitrogen (di-/mon-)oxide|" // N 1041
 "Voc/Ozone|" // O 1042
 "Ph|PM/CO/CO2/NO(x)/Voc/Ozone|Power|" // P 1043..1045
 "Radiation|Reactive energy/power|" // R 1046..1047
 "Signal strength|Sound pressure|Speed|" // S 1048..1050
 "Temperature|" // T 1051
 "Voltage/Current|Volume/Water cons.|Volume flow rate|" // V 1052..1054
 "Weight|Wind direction|" // W 1055..1056
 "Various units|" // Additional 1057
 ;

const uint16_t unit_of_measure_map[] PROGMEM = {
  1051, 1, 2, 3, // Temperature
  1037, 4, // Percent Battery, Humidity, Moisture
  1027, 8, 6, 116, 7, 115, 10, 5, 114, 9, // (Atmospheric) Pressure
  1052, 13, 119, 120, 121, 16, 117, 118, // Voltage/Current
  1045, 11, 12, 108, 109, 110, 111, // Power
  1024, 17, // Apparent power
  1047, 145, 146, 147, 148, // Reactive power/energy
  1044, 30, 31, 32, 33, 34, // Particle matter
  1031, 18, 19, 20, 21, 48, 49, 50, 51, // Distance
  1055, 57, 58, 59, 60, 61, 62, 149, // Weight
  1053, 22, 23, 24, 25, 54, 55, // Volume/Water
  1054, 26, 27, 153, 154, 155, 156, 157, 158, 159, 160, // Volume flow rate
  1032, 39, 40, 41, 42, 43, 44, 45, 46, 47, // Duration
  1034, 14, 15, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, // Energy(-storage)
  1033, 162, 163, 164, 165, // Energy distance
  1050, 66, 67, 68, 69, 70, 71, 65, 106, 107, // Speed
  1056, 35, // (Wind) direction
  1038, 28, // Illuminance
  1039, 64, 112, // Irradiance
  1046, 152, 153, // Radiation
  1057, 29, 63, 161, 166, 167, // Various units
  1035, 52, 53, 131, 132, // Frequency
  1043, 113, // Potential hydrogen
  1026, 56, 122, 123, 124, 125, 126, 127, 128, 129, 130, // Area
  1029, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, // Data rate
  1030, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, // Data size
  1049, 72, 73, // Sound pressure
  1028, 150, 151, // Blood glucose
  1040, 36, 37, 38, // Monetary
};

String toUnitOfMeasureName(const uint32_t unitOfMeasureIndex,
                           const String & defUoM) {
  char tmp[26]{};
  String result;

  if (unitOfMeasureIndex < 1024) {
    result = GetTextIndexed(tmp, sizeof(tmp), unitOfMeasureIndex, unit_of_measure_list);
  } else {
    result = GetTextIndexed(tmp, sizeof(tmp), unitOfMeasureIndex - 1024, unit_of_measure_labels);
  }

  return result.isEmpty() ? defUoM : result;
}

int getUnitOfMeasureIndex(const String& uomName) {
  return GetCommandCode(uomName.c_str(), unit_of_measure_list);
}

void addUnitOfMeasureSelector(const String& id,
                              const uint8_t unitOfMeasure) {
  constexpr uint16_t asize = NR_ELEMENTS(unit_of_measure_map);
  bool firstGrp = true;

  do_addSelector_Head(id, F("xwide"), EMPTY_STRING, false
                      #if FEATURE_TOOLTIPS
                      , EMPTY_STRING
                      #endif // if FEATURE_TOOLTIPS
                     );
  addSelector_Item( // Empty first value
    F(""),
    0,
    unitOfMeasure == 0);

  for (uint16_t idx = 0; idx < asize; ++idx) {
    const uint16_t uomIdx = pgm_read_word_near(&unit_of_measure_map[idx]);
    if (uomIdx < 1024) {
      addSelector_Item(
        toUnitOfMeasureName(uomIdx),
        uomIdx,
        unitOfMeasure == uomIdx);
    } else {
      if (!firstGrp) {
        addSelector_OptGroupFoot();
      }
      addSelector_OptGroup(toUnitOfMeasureName(uomIdx));
      firstGrp = false;
    }
    if ((idx & 0x07) == 0) { delay(0); }
  }
  if (!firstGrp) {
    addSelector_OptGroupFoot();
  }
  addSelector_Foot();
}
#endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE

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
  addUnit(getFormUnit(label));
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
  addUnit(getFormUnit(label));
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
  addHtml(F("><span "));
  addHtmlAttribute(F("id"), concat(F("cs"), id)); // cs=checkbox span
  addHtml(F(" class='checkmark"));

  if (disabled) { addDisabled(); }
  addHtml('\'');
  #if FEATURE_TOOLTIPS
  addTooltip(tooltip);
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
  addTooltip(tooltip);
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
  addTooltip(tooltip);
  #endif // if FEATURE_TOOLTIPS
  addHtml('>');
}

// ********************************************************************************
// Add Textbox
// ********************************************************************************
void addTextBox(const __FlashStringHelper * id,
                const String& value,
                int           maxlength,
                const __FlashStringHelper * classname)
{
  addTextBox(String(id), value, maxlength, 
             false, // readonly
             false, // required
             EMPTY_STRING, // pattern
             classname);
}

void addTextBox(const String& id,
                const String& value,
                int           maxlength,
                const __FlashStringHelper * classname)
{
  addTextBox(id, value, maxlength, 
             false, // readonly
             false, // required
             EMPTY_STRING, // pattern
             classname);
}

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
                ,
                const String&   datalist
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
  if (!datalist.isEmpty()) {
    addHtmlAttribute(F("list"),    datalist);
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
  addTooltip(tooltip);
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
  if (rows < 0) {
    rows = count_newlines(value) + 1;
  }
  addHtml(F("<textarea "));
//  addHtmlAttribute(F("class"),     F("wide"));
  addHtmlAttribute(F("type"),      F("text"));
  addHtmlAttribute(F("name"),      id);
  addHtmlAttribute(F("id"),        id);
  if (maxlength > 0) {
    addHtmlAttribute(F("maxlength"), maxlength);
  }
  if (rows > 0) {
    addHtmlAttribute(F("rows"),      rows);
  }
  if (columns > 0) {
    addHtmlAttribute(F("cols"),      columns);
  }

  if (readonly) {
    addHtml(F(" readonly "));
  }

  if (required) {
    addHtml(F(" required "));
  }

  #if FEATURE_TOOLTIPS
  addTooltip(tooltip);
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
        #ifdef ESP32
        concat(
        #endif // ifdef ESP32
          concat(
            createGPIO_label(gpio, pinnr, input, output, warning),
            getConflictingUse_wrapped(gpio, purpose)),
        #ifdef ESP32
            isPSRAMInterfacePin(gpio) ? getConflictingUse_wrapped(gpio, purpose, true) : F("")
        ),
        #endif // ifdef ESP32
        gpio,
        choice == gpio);

      ++i;
    }
    ++gpio;
  }
  addSelector_Foot();
}

#ifdef ESP32
#if SOC_ADC_SUPPORTED
void addADC_PinSelect(AdcPinSelectPurpose purpose, const String& id,  int choice)
{
  addSelector_Head(id);

  // At i == 0 && gpio == -1, add the "Hall Effect" option first
  int i    = 0;
  int gpio = -1;
  bool hasADC2 = false;

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

#if SOC_TOUCH_SENSOR_SUPPORTED
    if (purpose == AdcPinSelectPurpose::TouchOnly) {
      // For touch only list, sort based on touch number
      // Default sort is on GPIO number.
      gpio = touchPinToGpio(i);
    } else {
      ++gpio;
    }
#else
    ++gpio;
#endif

    if (getGpioInfo(gpio, pinnr, input, output, warning)) {
      int adc, ch, t;

      if (getADC_gpio_info(gpio, adc, ch, t)) {
        if ((purpose != AdcPinSelectPurpose::TouchOnly) || (t >= 0)) {
          String gpio_label;
          gpio_label = formatGpioName_ADC(gpio);

          if (adc != 0) {
            if (adc == 2) {
              hasADC2 = true;
            }
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
  if (hasADC2) {
    addFormNote(F("Do not use ADC2 pins with WiFi active"));
  }
}
#endif

#if SOC_DAC_SUPPORTED
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
#endif

#endif // ifdef ESP32

#include "_Plugin_Helper.h"
#ifdef USES_P070

# include "src/PluginStructs/P070_data_struct.h"

// #######################################################################################################
// #################################### Plugin 070: NeoPixel ring clock #################################
// #######################################################################################################

/** Changelog:
 * 2024-xx-xx: Add predefined color selection for hands/marks and hourly lightshow (PCONFIG(7/8), PCONFIG_LONG(0-3))
 * 2024-xx-xx: Add automatic brightness control via BH1750 lux sensor (PCONFIG(6))
 * 2024-xx-xx: Add selectable LED ring mode: 60-LED (standard) or 24-LED via web interface (PCONFIG(5))
 * 2023-10-26 tonhuisman: Apply NeoPixelBus_wrapper as replacement for Adafruit_NeoPixel library
 * 2023-10    tonhuisman: Add changelog.
 */

// A clock that uses a strip/ring of 60 or 24 WS2812 NeoPixel LEDs as display for a classic clock.
// Hand/mark colors are configurable. An optional hourly colour-chase lightshow is available.
// Brightness can be set manually or automatically via a BH1750 lux sensor (Task name: "BH1750").
// Commands: Clock,<Enabled 1/0>,<Hand brightness 0-255>,<Mark brightness 0-255>

# define PLUGIN_070
# define PLUGIN_ID_070         70
# define PLUGIN_NAME_070       "Output - NeoPixel Ring Clock"
# define PLUGIN_VALUENAME1_070 "Enabled"
# define PLUGIN_VALUENAME2_070 "Brightness"
# define PLUGIN_VALUENAME3_070 "Marks"

// Helper: emit a row of color radio buttons for one hand/element
static void addColorRadioRow(const __FlashStringHelper *label,
                             const char               *fieldName,
                             uint8_t                   currentValue)
{
  const char *colorNames[] = { "Red", "Green", "Blue", "Yellow", "Cyan", "Magenta", "White" };
  const char *colorStyles[] = {
    "background:#e00;color:#fff",
    "background:#0c0;color:#fff",
    "background:#00e;color:#fff",
    "background:#cc0;color:#000",
    "background:#0cc;color:#000",
    "background:#c0c;color:#fff",
    "background:#ddd;color:#000"
  };

  addRowLabel(label);
  String html;
  html.reserve(512);

  for (uint8_t i = 0; i < P070_COLOR_COUNT; i++) {
    html += F("<label style='margin-right:6px;padding:2px 7px;border-radius:4px;cursor:pointer;");
    html += colorStyles[i];
    html += F("'><input type='radio' name='");
    html += fieldName;
    html += F("' value='");
    html += String(i);
    html += F("'");
    if (currentValue == i) { html += F(" checked"); }
    html += F("> ");
    html += colorNames[i];
    html += F("</label>");
  }
  addHtml(html);
}


boolean Plugin_070(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_070;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].GlobalSyncOption   = false;
      Device[deviceCount].ExitTaskBeforeSave = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_070);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_070));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_070));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_070));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("LED"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // ---------------------------------------------------------------
      // Section: Ring & display
      // ---------------------------------------------------------------
      addFormSubHeader(F("Ring &amp; Display"));

      {
        String radioHtml = F("<input type='radio' name='led_mode' value='0'");
        if (PCONFIG(5) == P070_LED_MODE_60) { radioHtml += F(" checked"); }
        radioHtml += F("> 60 LEDs&nbsp;&nbsp;&nbsp;");
        radioHtml += F("<input type='radio' name='led_mode' value='1'");
        if (PCONFIG(5) == P070_LED_MODE_24) { radioHtml += F(" checked"); }
        radioHtml += F("> 24 LEDs&nbsp;&nbsp;&nbsp;");
        radioHtml += F("<input type='radio' name='led_mode' value='2'");
        if (PCONFIG(5) == P070_LED_MODE_16) { radioHtml += F(" checked"); }
        radioHtml += F("> 16 LEDs&nbsp;&nbsp;&nbsp;");
        radioHtml += F("<input type='radio' name='led_mode' value='3'");
        if (PCONFIG(5) == P070_LED_MODE_12) { radioHtml += F(" checked"); }
        radioHtml += F("> 12 LEDs");
        addRowLabel(F("LED Ring size"));
        addHtml(radioHtml);
      }
      addFormNote(F("Default: 60 LEDs. Changing LED ring size or type requires a reboot."));

      {
        String typeHtml = F("<input type='radio' name='led_type' value='0'");
        if (PCONFIG(9) == P070_LED_TYPE_RGB)  { typeHtml += F(" checked"); }
        typeHtml += F("> RGB (3-channel)&nbsp;&nbsp;&nbsp;");
        typeHtml += F("<input type='radio' name='led_type' value='1'");
        if (PCONFIG(9) == P070_LED_TYPE_RGBW) { typeHtml += F(" checked"); }
        typeHtml += F("> RGBW (4-channel, dedicated white LED)");
        addRowLabel(F("LED Strip type"));
        addHtml(typeHtml);
      }

      addFormNumericBox(F("12 o'clock LED position"), F("offset"), PCONFIG(3), 0, 59);
      addFormNote(F("Position of the 12 o'clock LED (0-59 for 60-LED, 0-23 for 24-LED)"));
      addFormCheckBox(F("Thick 12 o'clock mark"), F("thick_12_mark"), PCONFIG(4));
      addFormNote(F("3 LEDs mark the 12 o'clock position"));
      addFormCheckBox(F("Clock display enabled"), F("enabled"), PCONFIG(0));

      // ---------------------------------------------------------------
      // Section: Brightness
      // ---------------------------------------------------------------
      addFormSubHeader(F("Brightness"));

      addFormCheckBox(F("Auto brightness (BH1750)"), F("auto_brightness"), PCONFIG(6));
      addFormNote(F("Automatically adjust brightness via BH1750 sensor (Task name must be 'BH1750')"));
      addFormNumericBox(F("LED brightness"), F("brightness"), PCONFIG(1), 0, 255);
      addFormNote(F("Brightness of clock hands (0-255) — used when auto brightness is off"));
      addFormNumericBox(F("Hour mark brightness"), F("marks"), PCONFIG(2), 0, 255);
      addFormNote(F("Brightness of hour marks (0-255) — used when auto brightness is off"));

      // ---------------------------------------------------------------
      // Section: Colors
      // ---------------------------------------------------------------
      addFormSubHeader(F("Hand &amp; Mark Colors"));

      addColorRadioRow(F("Hours hand color"),   "color_hours",   PCONFIG_LONG(0));
      addColorRadioRow(F("Minutes hand color"), "color_minutes", PCONFIG_LONG(1));
      addColorRadioRow(F("Seconds hand color"), "color_seconds", PCONFIG_LONG(2));
      addColorRadioRow(F("Hour marks color"),   "color_marks",   PCONFIG_LONG(3));

      // ---------------------------------------------------------------
      // Section: Hourly lightshow
      // ---------------------------------------------------------------
      addFormSubHeader(F("Hourly Lightshow"));

      {
        String lsHtml = F("<input type='radio' name='lightshow_mode' value='0'");
        if (PCONFIG(7) == P070_LIGHTSHOW_OFF)   { lsHtml += F(" checked"); }
        lsHtml += F("> Aus &nbsp;&nbsp;&nbsp;");
        lsHtml += F("<input type='radio' name='lightshow_mode' value='1'");
        if (PCONFIG(7) == P070_LIGHTSHOW_CHASE) { lsHtml += F(" checked"); }
        lsHtml += F("> Lauflicht (Farbe zuf&auml;llig) &nbsp;&nbsp;&nbsp;");
        lsHtml += F("<input type='radio' name='lightshow_mode' value='2'");
        if (PCONFIG(7) == P070_LIGHTSHOW_FLASH) { lsHtml += F(" checked"); }
        lsHtml += F("> Flash (alle LEDs + Zeiger-Blink)");
        addRowLabel(F("Lightshow Modus"));
        addHtml(lsHtml);
      }
      addFormNote(F("Lightshow startet jede volle Stunde (hh:00:00)"));
      addFormNumericBox(F("Lightshow duration (seconds)"), F("lightshow_duration"), PCONFIG(8), 1, 30);
      addFormNote(F("How many seconds the lightshow runs (1-30)"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      const uint8_t new_led_mode = getFormItemInt(F("led_mode"));
      const uint8_t new_led_type = getFormItemInt(F("led_type"));
      const bool    mode_changed = (new_led_mode != PCONFIG(5)) || (new_led_type != PCONFIG(9));

      PCONFIG(0) = isFormItemChecked(F("enabled"));
      PCONFIG(1) = getFormItemInt(F("brightness"));
      PCONFIG(2) = getFormItemInt(F("marks"));
      PCONFIG(3) = getFormItemInt(F("offset"));
      PCONFIG(4) = isFormItemChecked(F("thick_12_mark"));
      PCONFIG(5) = new_led_mode;
      PCONFIG(6) = isFormItemChecked(F("auto_brightness"));
      PCONFIG(7) = getFormItemInt(F("lightshow_mode"));
      PCONFIG(8) = getFormItemInt(F("lightshow_duration"));
      PCONFIG(9) = new_led_type;

      PCONFIG_LONG(0) = getFormItemInt(F("color_hours"));
      PCONFIG_LONG(1) = getFormItemInt(F("color_minutes"));
      PCONFIG_LONG(2) = getFormItemInt(F("color_seconds"));
      PCONFIG_LONG(3) = getFormItemInt(F("color_marks"));

      P070_data_struct *P070_data = static_cast<P070_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P070_data) {
        if (mode_changed) {
          P070_data->reset();
          P070_data->init(event);
        } else {
          P070_data->set(event);
        }
        P070_data->calculateMarks();
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P070_data_struct());
      P070_data_struct *P070_data = static_cast<P070_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P070_data) {
        return success;
      }
      P070_data->init(event);
      P070_data->calculateMarks();

      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P070_data_struct *P070_data = static_cast<P070_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P070_data) {
        P070_data->Clock_update();
      }
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      const String command = parseString(string, 1);

      P070_data_struct *P070_data = static_cast<P070_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P070_data) && (equals(command, F("clock")))) {
        int32_t val_{};

        if (validIntFromString(parseString(string, 2), val_)) {
          if ((val_ > -1) && (val_ < 2)) {
            P070_data->display_enabled = val_;
            PCONFIG(0)                 = val_;
          }
        }

        if (validIntFromString(parseString(string, 3), val_)) {
          if ((val_ > -1) && (val_ < 256)) {
            P070_data->brightness = val_;
            PCONFIG(1)            = val_;
          }
        }

        if (validIntFromString(parseString(string, 4), val_)) {
          if ((val_ > -1) && (val_ < 256)) {
            P070_data->brightness_hour_marks = val_;
            PCONFIG(2)                       = val_;
          }
        }

        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      P070_data_struct *P070_data = static_cast<P070_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P070_data) {
        UserVar.setFloat(event->TaskIndex, 0, P070_data->display_enabled);
        UserVar.setFloat(event->TaskIndex, 1, P070_data->brightness);
        UserVar.setFloat(event->TaskIndex, 2, P070_data->brightness_hour_marks);

        success = true;
      }
    }
  }
  return success;
}

#endif // USES_P070

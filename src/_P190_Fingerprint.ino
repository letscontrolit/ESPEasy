#include "_Plugin_Helper.h"
#ifdef USES_P190

// #######################################################################################################
// ############################ Plugin 190: Fingerprint reader (Grow R503 / R307) ########################
// #######################################################################################################

/** Changelog:
 * 2026-10-23 alexbn71: Initial version, developed in https://github.com/alexbn71/ESPEasy-P190-Fingerprint
 */

/** Info:
 * Grow fingerprint modules on a serial port: R503 (with RGB LED ring), R307, R502, ZFM-20, AS608, ...
 * A finger on the sensor is searched in the module library: the values ID (-1 = not recognised), Score
 * and Count (templates stored) are sent to the controllers and the rules (event <taskname>#ID).
 * The value Enroll is the progress of the last enrolment (P190_ENROLL_...), readable from /json.
 *
 * Commands:
 *   fingerprint,enroll[,<id>]                          store a new finger at <id>, or at the first free slot
 *                                                      (2 captures); answers {"enroll":<id>} or {"error":"..."}
 *   fingerprint,list                                   answers {"ready","capacity","count","enroll","slot","used":[ids]}
 *   fingerprint,delete,<id>                            delete the finger stored at <id>
 *   fingerprint,deleteall                              delete all fingers
 *   fingerprint,count                                  read the number of stored fingers (event #Count)
 *   fingerprint,led,<mode>,<color>[,<speed>[,<times>]] LED ring: breathing|flash|on|off|fadein|fadeout,
 *                                                      red|blue|purple|green|yellow|cyan|white
 *   fingerprint,cancel                                 stop an enrolment
 *
 * Events: <taskname>#EnrollScan=<n>, #EnrollRemove=<n>, #Enrolled=<id>, #EnrollFailed=<code>,
 *         #Deleted=<id> (-1 = all), #Count=<n>, #Error=<code>
 */

# define PLUGIN_190
# define PLUGIN_ID_190         190
# define PLUGIN_NAME_190       "Security - Fingerprint (Grow R503/R307)"
# define PLUGIN_VALUENAME1_190 "ID"
# define PLUGIN_VALUENAME2_190 "Score"
# define PLUGIN_VALUENAME3_190 "Count"
# define PLUGIN_VALUENAME4_190 "Enroll"

# include "src/PluginStructs/P190_data_struct.h"

boolean Plugin_190(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_190;
      dev.Type           = DEVICE_TYPE_SERIAL_PLUS1;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_190);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[P190_VALUE_ID],    PSTR(PLUGIN_VALUENAME1_190));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[P190_VALUE_SCORE], PSTR(PLUGIN_VALUENAME2_190));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[P190_VALUE_COUNT], PSTR(PLUGIN_VALUENAME3_190));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[P190_VALUE_ENROLL], PSTR(PLUGIN_VALUENAME4_190));

      for (uint8_t i = 0; i < 4; ++i) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
      }
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event);
      event->String3 = formatGpioName_input_optional(F("Touch (WAKEUP, blue)"));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P190_BAUD_MULT     = P190_DEF_BAUD_MULT;
      P190_LED_FEEDBACK  = 1;
      P190_MATCH_COLOR   = P190_proto::COLOR_BLUE;
      P190_NOMATCH_COLOR = P190_proto::COLOR_RED;
      P190_POLL_MS       = P190_DEF_POLL_MS;
      P190_MIN_SCORE     = 0;
      P190_PASSWORD      = 0;
      P190_ADDRESS       = P190_proto::DEFAULT_ADDRESS;
      success            = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      const __FlashStringHelper *options[] = { F("9600"), F("19200"), F("38400"), F("57600"), F("115200") };
      const int optionValues[]             = { 1, 2, 4, 6, 12 };
      const FormSelectorOptions selector(NR_ELEMENTS(optionValues), options, optionValues);
      selector.addFormSelector(F("Baud rate"), F("baud"), P190_BAUD_MULT);
      addFormNote(F("Factory setting: 57600"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNote(F("Wiring R503: red 3.3V, black GND, yellow TX, green RX, blue WAKEUP (touch), white 3.3VT. "
                    "Power with 3.3V only. Without the touch GPIO the module is polled at the scan interval."));

      addFormSubHeader(F("Module"));
      addFormTextBox(F("Password"), F("pwd"), formatToHex(P190_PASSWORD, 8), 10);
      addFormTextBox(F("Address"),  F("addr"), formatToHex(P190_ADDRESS, 8), 10);
      addFormNote(F("Factory setting: password 0x00000000, address 0xFFFFFFFF"));

      addFormSubHeader(F("Scan"));
      addFormNumericBox(F("Scan interval (no touch GPIO)"), F("poll"), P190_POLL_MS, 50, 5000);
      addUnit(F("ms"));
      addFormNumericBox(F("Minimum score"), F("minscore"), P190_MIN_SCORE, 0, 1000);
      addFormNote(F("Matches with a lower score are reported as not recognised (ID -1). 0 = accept every match of the module."));

      addFormSubHeader(F("LED ring (R503)"));
      addFormCheckBox(F("Show result on the LED ring"), F("ledfb"), P190_LED_FEEDBACK != 0);
      P190_colorSelector(F("Recognised"),     F("okcolor"), P190_MATCH_COLOR);
      P190_colorSelector(F("Not recognised"), F("kocolor"), P190_NOMATCH_COLOR);
      addFormNote(F("Green, yellow, cyan and white need the full RGB version of the R503. Enrolment: purple."));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      uint32_t value = 0;

      P190_BAUD_MULT     = getFormItemInt(F("baud"));
      P190_POLL_MS       = getFormItemInt(F("poll"));
      P190_MIN_SCORE     = getFormItemInt(F("minscore"));
      P190_LED_FEEDBACK  = isFormItemChecked(F("ledfb")) ? 1 : 0;
      P190_MATCH_COLOR   = getFormItemInt(F("okcolor"));
      P190_NOMATCH_COLOR = getFormItemInt(F("kocolor"));

      if (validUIntFromString(webArg(F("pwd")), value)) { P190_PASSWORD = value; }

      if (validUIntFromString(webArg(F("addr")), value)) { P190_ADDRESS = value; }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      P190_data_struct *P190_data = static_cast<P190_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P190_data) {
        P190_data->showValues(event);
        success = true; // The task values are written by showValues()
      }
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P190_data_struct(event));
      P190_data_struct *P190_data = static_cast<P190_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = nullptr != P190_data && P190_data->init(event);
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P190_data_struct *P190_data = static_cast<P190_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P190_data) {
        P190_data->loop(event);
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      // Values are sent when a finger is scanned. Returning true here would resend the last ID to
      // controllers and rules (e.g. open the door again) at every task interval.
      success = false;
      break;
    }

    case PLUGIN_WRITE:
    {
      P190_data_struct *P190_data = static_cast<P190_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = nullptr != P190_data && P190_data->plugin_write(event, string);
      break;
    }
  }

  return success;
}

#endif // USES_P190

#include "_Plugin_Helper.h"
#ifdef USES_P109

# include "src/PluginStructs/P109_data_struct.h"

/*##########################################################################################
 ##################### Plugin 109: OLED SSD1306 display for Thermostat ####################
 ##########################################################################################

   This is a modification to Plugin_036 with graphics library provided from squix78 github
   https://github.com/squix78/esp8266-oled-ssd1306

   Features :
    - Displays and use current temperature from specified Device/Value (can be a Dummy for example)
    - Displays and maintains setpoint value
    - on power down/up this plugin maintains and reloads RELAY and SETPOINT values from SPIFFS
    - Supports 3 buttons, LEFT, RIGHT and MODE selection (MODE button cycles modes below,
      LEFT/RIGHT increases-decreases setpoint OR timeout (Mode sensitive)
    - one output relay need to be specified, currently only HIGH level active supported
    - 3 mode is available:
        - 0 or X: set relay permanently off no matter what
        - 1 or A: set relay ON if current temperature below setpoint, and set OFF when
                  temperature+hysteresis reached - comparison made at setted Plugin interval (AUTO MODE)
        - 2 or M: set relay ON for specified time in minutes (MANUAL ON MODE), after timeout, mode switch to "A"

   List of commands :
   - oledframedcmd,[OLED_STATUS]              Inherited command from P036 status can be:
                                              [off/on/low/med/high]
   - thermo,setpoint,[target_temperature]     Target setpoint, only used in Mode "A"
   - thermo,heating,[RELAY_STATUS]            Manually forcing relay status [off/on]
   - thermo,mode,[MODE],[TIMEOUT]             Set to either mode X/A/M, if M selected,
                                              then TIMEOUT can be specified in minutes
   - thermo,left                              Emulate the Left button action
   - thermo,right                             Emulate the Right button action
   - thermo,modebtn                           Emulate the Mode button action

   Command Examples :
   -  /control?cmd=thermo,setpoint,23          Set target setpoint to 23 Celsius
   -  /control?cmd=thermo,mode,1               Set mode to AUTOMATIC so it starts to maintain setpoint temperature
   -  /control?cmd=thermo,mode,2,5             Starts pre-heat for 5 minute, does not care about TEMP, then go to AUTO mode after timeout
   -  /control?cmd=thermo,mode,0               Switch heating off, absolutely do nothing until further notice

   ------------------------------------------------------------------------------------------
   Copyleft Nagy Sándor 2018 - https://bitekmindenhol.blog.hu/
   ------------------------------------------------------------------------------------------
   2022-12-08 tonhuisman: Add Relay invert state option, reorder config option Contrast
                          Add setpoint delay option, switch relay after delay seconds
   2022-10-11 tonhuisman: Fix initialization issue for relay state when switching tasks
   2022-10-10 tonhuisman: Save pending thermo-settings on plugin exit (while waiting for the 30 seconds to have passed)
                          Always force Auto mode on plugin start, and timeout 0, reset timeout to 0 on mode change
                          Display timeout only up to 9:59 hours, longer timeout values are truncated!
   2022-10-09 tonhuisman: ** Structural behavior change: **
                          - Saving of the thermo<tasknr>.dat file is reduced to the absolute minimum, only if the setpoint is different
                            from the last saved value, and we're not in manual mode, it will be saved, but only after 30 seconds.
                          - The relay is only actuated/released 5 seconds after the setpoint has been changed
   2022-10-09 tonhuisman: Deduplicate code by moving the OLed I2C Address check to OLed_helper
   2022-10-03 tonhuisman: On request changed the leftbtn and rightbtn commands to thermo,down and thermo,up to be more descriptive
                          Add option for alternating sysname/SSID in title (or just the sysname)
                          Add option for show taskname instead of sysname in title
   2022-09-21 tonhuisman: Adjust subcommands to leftbtn and rightbtn, to be consistent with modebtn
   2022-09-20 tonhuisman: Add commands for emulating the Left, Right and Mode buttons
   2022-08-28 tonhuisman: Changelog reversed order to newest on top
                          Deduplicate code for displaying text on display
   2022-06-18 tonhuisman: Enable multi-instance use, implement OLed_helper functions,
                          remove P109/Plugin_109 prefixes on variables and methods where appropriate
   2022-06-18 tonhuisman: More optimizations, use #defines where appropriate
   2022-06-17 tonhuisman: Optimizations
   No older changelog recorded.
 */

# define PLUGIN_109
# define PLUGIN_ID_109          109
# define PLUGIN_NAME_109        "Display - OLED SSD1306/SH1106 Thermo"
# define PLUGIN_VALUENAME1_109  "setpoint"
# define PLUGIN_VALUENAME2_109  "heating"
# define PLUGIN_VALUENAME3_109  "mode"
# define PLUGIN_VALUENAME4_109  "timeout"


boolean Plugin_109(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number         = PLUGIN_ID_109;
      Device[deviceCount].Type             = DEVICE_TYPE_I2C;
      Device[deviceCount].VType            = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports            = 0;
      Device[deviceCount].FormulaOption    = true;
      Device[deviceCount].ValueCount       = 4;
      Device[deviceCount].SendDataOption   = true;
      Device[deviceCount].TimerOption      = true;
      Device[deviceCount].GlobalSyncOption = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_109);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_109));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_109));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_109));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_109));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P109_CONFIG_RELAYPIN = -1; // Set to None
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      success = OLedI2CAddressCheck(function, event->Par1, F("pi2caddr"), P109_CONFIG_I2CADDRESS);

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P109_CONFIG_I2CADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    # ifndef LIMIT_BUILD_SIZE
    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = F("Btn L: ");
      string += formatGpioLabel(CONFIG_PIN1, false);
      string += event->String1; // newline
      string += F("Btn R: ");
      string += formatGpioLabel(CONFIG_PIN2, false);
      string += event->String1; // newline
      string += F("Btn M: ");
      string += formatGpioLabel(CONFIG_PIN3, false);
      string += event->String1; // newline
      string += F("Relay: ");
      string += formatGpioLabel(P109_CONFIG_RELAYPIN, false);
      success = true;
      break;
    }
    # endif // ifndef LIMIT_BUILD_SIZE

    case PLUGIN_WEBFORM_LOAD:
    {
      OLedFormController(F("controller"), nullptr, P109_CONFIG_DISPLAYTYPE);

      OLedFormRotation(F("rotate"), P109_CONFIG_ROTATION);

      OLedFormContrast(F("contrast"), P109_CONFIG_CONTRAST);

      {
        P109_data_struct *P109_data = new (std::nothrow) P109_data_struct();

        if (nullptr != P109_data) {
          success = P109_data->plugin_webform_load(event); // Load CustomTaskSettings
          delete P109_data;
        }
      }

      addFormPinSelect(PinSelectPurpose::Generic_input,  F("Button left/down"), F("taskdevicepin1"), CONFIG_PIN1);
      addFormPinSelect(PinSelectPurpose::Generic_input,  F("Button right/up"),  F("taskdevicepin2"), CONFIG_PIN2);
      addFormPinSelect(PinSelectPurpose::Generic_input,  F("Button mode"),      F("taskdevicepin3"), CONFIG_PIN3);

      addFormPinSelect(PinSelectPurpose::Generic_output, F("Relay"),            F("heatrelay"),      P109_CONFIG_RELAYPIN);

      addFormCheckBox(F("Invert relay-state (0=on, 1=off)"), F("invertrelay"), P109_GET_RELAY_INVERT);

      {
        const __FlashStringHelper *options4[] = { F("0.2"), F("0.5"), F("1") };
        const int optionValues4[]             = { 2, 5, 10 };
        addFormSelector(F("Hysteresis"), F("hyst"), 3, options4, optionValues4, static_cast<int>(P109_CONFIG_HYSTERESIS * 10.0f));
      }

      {
        addFormCheckBox(F("Alternate Sysname/SSID in title"), F("palt"),  P109_GET_ALTERNATE_HEADER == 0); // Inverted!

        addFormCheckBox(F("Use Taskname instead of Sysname"), F("ptask"), P109_GET_TASKNAME_IN_TITLE == 1);
      }

      {
        if (P109_CONFIG_SETPOINT_DELAY == 0) { P109_CONFIG_SETPOINT_DELAY = P109_DEFAULT_SETPOINT_DELAY + P109_SETPOINT_OFFSET; }
        addFormNumericBox(F("Delay on setpoint change"), F("setpdelay"), P109_CONFIG_SETPOINT_DELAY - P109_SETPOINT_OFFSET, 1, 10);
        addUnit(F("1..10 sec."));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P109_CONFIG_I2CADDRESS     = getFormItemInt(F("pi2caddr"));
      P109_CONFIG_ROTATION       = getFormItemInt(F("rotate"));
      P109_CONFIG_DISPLAYTYPE    = getFormItemInt(F("controller"));
      P109_CONFIG_CONTRAST       = getFormItemInt(F("contrast"));
      P109_CONFIG_RELAYPIN       = getFormItemInt(F("heatrelay"));
      P109_CONFIG_HYSTERESIS     = (getFormItemInt(F("hyst")) / 10.0f);
      P109_CONFIG_SETPOINT_DELAY = getFormItemInt(F("setpdelay")) + P109_SETPOINT_OFFSET;
      uint32_t lSettings = 0u;
      bitWrite(lSettings, P109_FLAG_TASKNAME_IN_TITLE, isFormItemChecked(F("ptask")));
      bitWrite(lSettings, P109_FLAG_ALTERNATE_HEADER,  !isFormItemChecked(F("palt"))); // Inverted
      bitWrite(lSettings, P109_FLAG_RELAY_INVERT,      isFormItemChecked(F("invertrelay")));
      P109_FLAGS = lSettings;

      {
        P109_data_struct *P109_data = new (std::nothrow) P109_data_struct();

        if (nullptr != P109_data) {
          success = P109_data->plugin_webform_save(event); // Save CustomTaskSettings
          delete P109_data;
        }
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P109_data_struct());
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        if (P109_CONFIG_SETPOINT_DELAY == 0) { P109_CONFIG_SETPOINT_DELAY = P109_DEFAULT_SETPOINT_DELAY + P109_SETPOINT_OFFSET; }
        success = P109_data->plugin_init(event); // Start plugin
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        success = P109_data->plugin_exit(event); // Stop plugin
      }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        success = P109_data->plugin_ten_per_second(event); // Check buttons
      }

      break;
    }

    // Switch off display after displayTimer seconds
    case PLUGIN_ONCE_A_SECOND:
    {
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        success = P109_data->plugin_once_a_second(event); // Update display
      }

      break;
    }

    case PLUGIN_READ:
    {
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        success = P109_data->plugin_read(event); // Read operation, get data
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P109_data_struct *P109_data = static_cast<P109_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P109_data) {
        success = P109_data->plugin_write(event, string); // Write operation, handle commands
      }

      break;
    }
  }

  return success;
}

#endif // ifdef USES_P109

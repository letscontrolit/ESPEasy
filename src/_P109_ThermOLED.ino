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

   Command Examples :
   -  /control?cmd=thermo,setpoint,23          Set target setpoint to 23 Celsius
   -  /control?cmd=thermo,mode,1               Set mode to AUTOMATIC so it starts to maintain setpoint temperature
   -  /control?cmd=thermo,mode,2,5             Starts pre-heat for 5 minute, does not care about TEMP, then go to AUTO mode after timeout
   -  /control?cmd=thermo,mode,0               Switch heating off, absolutely do nothing until further notice

   ------------------------------------------------------------------------------------------
   Copyleft Nagy Sándor 2018 - https://bitekmindenhol.blog.hu/
   ------------------------------------------------------------------------------------------
   2022-06-17 tonhuisman: Optimizations
   2022-06-18 tonhuisman: More optimizations, use #defines where appropriate
   2022-06-18 tonhuisman: Enable multi-instance use, implement OLed_helper functions,
                          remove P109/Plugin_109 prefixes on variables and methods where appropriate
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
      Device[++deviceCount].Number           = PLUGIN_ID_109;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = false;
      Device[deviceCount].GlobalSyncOption   = true;
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

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x3c, 0x3d };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("pi2caddr"), 2, i2cAddressValues, P109_CONFIG_I2CADDRESS);
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

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

    case PLUGIN_WEBFORM_LOAD:
    {
      OLedFormController(F("controller"), nullptr, P109_CONFIG_DISPLAYTYPE);

      OLedFormRotation(F("rotate"), P109_CONFIG_ROTATION);

      {
        P109_data_struct *P109_data = new (std::nothrow) P109_data_struct();

        if (nullptr != P109_data) {
          success = P109_data->plugin_webform_load(event); // Load CustomTaskSettings
          delete P109_data;
        }
      }

      addFormPinSelect(F("Button left"),  F("taskdevicepin1"), CONFIG_PIN1);
      addFormPinSelect(F("Button right"), F("taskdevicepin2"), CONFIG_PIN2);
      addFormPinSelect(F("Button mode"),  F("taskdevicepin3"), CONFIG_PIN3);

      addFormPinSelect(F("Relay"),        F("heatrelay"),      P109_CONFIG_RELAYPIN);

      OLedFormContrast(F("contrast"), P109_CONFIG_CONTRAST);

      {
        const __FlashStringHelper *options4[] = { F("0.2"), F("0.5"), F("1") };
        const int optionValues4[]             = { 2, 5, 10 };
        addFormSelector(F("Hysteresis"), F("hyst"), 3, options4, optionValues4, static_cast<int>(P109_CONFIG_HYSTERESIS * 10.0f));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P109_CONFIG_I2CADDRESS  = getFormItemInt(F("pi2caddr"));
      P109_CONFIG_ROTATION    = getFormItemInt(F("rotate"));
      P109_CONFIG_DISPLAYTYPE = getFormItemInt(F("controller"));
      P109_CONFIG_CONTRAST    = getFormItemInt(F("contrast"));
      P109_CONFIG_RELAYPIN    = getFormItemInt(F("heatrelay"));
      P109_CONFIG_HYSTERESIS  = (getFormItemInt(F("hyst")) / 10.0f);

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

//#######################################################################################################
//################################## Plugin 240: Dust Sensor Ikea Vindriktning ##########################
//#######################################################################################################
/*
  Plugin is based upon various open sources on the internet. Plugin uses the serial lib to access a serial port
  This plugin was written by Mark Flesch

  This plugin reads the particle concentration from the PM1006 sensor in the Ikea Vindriktning
  The original Ikea processor is controlling the PM1006 sensor, the plugin is eavesdropping the responses
  DevicePin1 - RX on ESP, TX on PM1006
  DevicePin2 - TX on ESP, RX on PM1006 (optional, currently not accessed by the plugin)
*/


// #include section
#include "_Plugin_Helper.h"
#ifdef USES_P240
# include <ESPeasySerial.h>

// Standard plugin defines
#define PLUGIN_240
#define PLUGIN_ID_240     240                   // plugin id
#define PLUGIN_NAME_240   "Dust - Vindriktning" // "Plugin Name" is what will be dislpayed in the selection list
#define PLUGIN_VALUENAME1_240 "PM2.5"           // variable output of the plugin. The label is in quotation marks
#define PLUGIN_240_DEBUG  true                  // set to true for extra log info in the debug


//   PIN/port configuration is stored in the following:
//   CONFIG_PIN1 - Used by plugin_Helper_serial (RX pin) 
//   CONFIG_PIN2 - Used by plugin_Helper_serial (TX pin)
//   CONFIG_PIN3 - Not used
//   CONFIG_PORT - Used by the plugin_Helper_serial (serialType)
//
//   Custom configuration is stored in the following:
//   PCONFIG(x)
//   x can be between 1 - 8 and can store values between -32767 - 32768 (16 bit)
//
//   N.B. these are aliases for a longer less readable amount of code. See _Plugin_Helper.h
//
//
//   PCONFIG_LABEL(x) is a function to generate a unique label used as HTML id to be able to match 
//                    returned values when saving a configuration.

// Make accessing specific parameters more readable in the code
// #define P240_BAUDRATE           PCONFIG_LONG(0)
// #define P240_BAUDRATE_LABEL     PCONFIG_LABEL(0)

typedef enum {
    PM1006_HEADER,
    PM1006_LENGTH,
    PM1006_DATA,
    PM1006_CHECK
} pm1006_state_t;

// Global variables TODO: attach them to the task to allow multiple instances
  ESPeasySerial *easySerial = nullptr;
  const int bufferSize = 20;
  char serialRxBuffer[bufferSize];
  char debugBuffer[3*bufferSize];
  int rxBufferPtr = 0;
  int rxChecksum = 0;
  int _index = 0;
  int _rxlen = 0;
  int pm25 = 0;
  pm1006_state_t rxState = PM1006_HEADER;


// A plugin has to implement the following function

boolean Plugin_240(uint8_t function, struct EventStruct *event, String& string)
{
  // function: reason the plugin was called
  // event: ??add description here??
  // string: ??add description here??

  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      // This case defines the device characteristics, edit appropriately

      Device[++deviceCount].Number           = PLUGIN_ID_240;                    // Plugin ID number.   (PLUGIN_ID_xxx)
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;               // How the device is connected. e.g. DEVICE_TYPE_SINGLE => connected through 1 datapin
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE; // Type of value the plugin will return. e.g. SENSOR_TYPE_STRING
      Device[deviceCount].Ports              = 0;                                // Port to use when device has multiple I/O pins  (N.B. not used much)
      Device[deviceCount].ValueCount         = 1;                                // The number of output values of a plugin. The value should match the number of keys PLUGIN_VALUENAME1_xxx
      Device[deviceCount].OutputDataType     = Output_Data_type_t::Default;      // Subset of selectable output data types  (Default = no selection)
      Device[deviceCount].PullUpOption       = false;                            // Allow to set internal pull-up resistors.
      Device[deviceCount].InverseLogicOption = false;                            // Allow to invert the boolean state (e.g. a switch)
      Device[deviceCount].FormulaOption      = true;                             // Allow to enter a formula to convert values during read. (not possible with Custom enabled)
      Device[deviceCount].Custom             = false;
      Device[deviceCount].SendDataOption     = true;                             // Allow to send data to a controller.
      Device[deviceCount].GlobalSyncOption   = true;                             // No longer used. Was used for ESPeasy values sync between nodes
      Device[deviceCount].TimerOption        = true;                            // Allow to set the "Interval" timer for the plugin.
      Device[deviceCount].TimerOptional      = false;                            // When taskdevice timer is not set and not optional, use default "Interval" delay (Settings.Delay)
      Device[deviceCount].DecimalsOnly       = false;                             // Allow to set the number of decimals (otherwise treated a 0 decimals)
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      // return the device name
      string = F(PLUGIN_NAME_240);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      // called when the user opens the module configuration page
      // it allows to add a new row for each output variable of the plugin
      // For plugins able to choose output types, see P026_Sysinfo.ino.
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_240));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      // Called to show optinal extra UART parameters in the web interface (only called for SERIAL devices)
      //addFormNumericBox(F("Baudrate"), P240_BAUDRATE_LABEL, P240_BAUDRATE, 2400, 115200);
      //addUnit(F("baud"));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event, false, true); // TX optional
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      // Called to show non default pin assignment or addresses like for plugins using serial or 1-Wire
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      // This is only called when Device[deviceCount].OutputDataType is not Output_Data_type_t::Default
      // The position in the config parameters used in this example is PCONFIG(Pxxx_OUTPUT_TYPE_INDEX)
      // Must match the one used in case PLUGIN_GET_DEVICEVTYPE  (best to use a define for it)
      // see P026_Sysinfo.ino for more examples.
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      // This is only called when Device[deviceCount].OutputDataType is not Output_Data_type_t::Default
      // The position in the config parameters used in this example is PCONFIG(Pxxx_OUTPUT_TYPE_INDEX)
      // Must match the one used in case PLUGIN_GET_DEVICEVALUECOUNT  (best to use a define for it)
      // IDX is used here to mark the PCONFIG position used to store the Device VType.
      // see P026_Sysinfo.ino for more examples.
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      // Set a default config here, which will be called when a plugin is assigned to a task.
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // this case defines what should be displayed on the web form, when this plugin is selected
      // The user's selection will be stored in
      // PCONFIG(x) (custom configuration)

      // Make sure not to append data to the string variable in this PLUGIN_WEBFORM_LOAD call.
      // This has changed, so now use the appropriate functions to write directly to the Streaming
      // web_server. This takes much less memory and is faster.
      // There will be an error in the web interface if something is added to the "string" variable.

      // Use any of the following (defined at web_server.ino):
      // addFormNote(F("not editable text added here"));
      // To add some html, which cannot be done in the existing functions, add it in the following way:
      /// addRowLabel(F("Analog Pin"));

      // For strings, always use the F() macro, which stores the string in flash, not in memory.

      // String dropdown[5] = { F("option1"), F("option2"), F("option3"), F("option4")};
      // addFormSelector(string, F("drop-down menu"), F("plugin_xxx_displtype"), 4, dropdown, nullptr, PCONFIG(0));

      // number selection (min-value - max-value)
      /// addFormNumericBox(string, F("description"), F("plugin_240_description"), PCONFIG(1), min - value, max - value);

      // after the form has been loaded, set success and break
      //serialHelper_serialconfig_webformLoad(event, true);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // this case defines the code to be executed when the form is submitted
      // the plugin settings should be saved to PCONFIG(x)
      // ping configuration should be read from CONFIG_PIN1 and stored
      //serialHelper_webformSave(event);

      // after the form has been saved successfuly, set success and break
      success = true;
      break;
    }
    case PLUGIN_INIT:
    {
      // this case defines code to be executed when the plugin is initialised
      int8_t rxPin = serialHelper_getRxPin(event);
      int8_t txPin = serialHelper_getTxPin(event);
      easySerial = new (std::nothrow) ESPeasySerial(serialHelper_getSerialType(event), rxPin, txPin);
      if (easySerial != nullptr) {
        easySerial->begin(9600);
        success = true;
      }
      String log = F("P240 : Init OK  ESP GPIO-pin RX:");
      log += rxPin;
      log += F(" TX:");
      log += txPin;
      addLogMove(LOG_LEVEL_INFO, log);
      break;
    }

    case PLUGIN_READ:
    {
      // code to be executed to read data
      // It is executed according to the delay configured on the device configuration page, only once
      UserVar[event->BaseVarIndex] = pm25;
      String log = F("P240 : READ ");
      log += pm25;
      addLogMove(LOG_LEVEL_INFO, log);

      // after the plugin has read data successfuly, set success and break
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      // this case defines code to be executed when the plugin executes an action (command).
      // Commands can be accessed via rules or via http.
      // As an example, http://192.168.1.12//control?cmd=dothis
      // implies that there exists the comamnd "dothis"

      break;
    }

    case PLUGIN_EXIT:
    {
      // perform cleanup tasks here. For example, free memory
      if (easySerial != nullptr) {
        delete(easySerial);
        easySerial = nullptr;
      }
      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      // code to be executed once a second. Tasks which do not require fast response can be added here
      success = true;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      // code to be executed 10 times per second. Tasks which require fast response can be added here
      // be careful on what is added here. Heavy processing will result in slowing the module down!
      bool new_data = false;
      while ((easySerial->available() > 0) && !new_data) 
      {
        new_data = P240_process_rx(easySerial->read());
        if (new_data) 
        {
          pm25 = float((serialRxBuffer[3] << 8) + serialRxBuffer[4]);
          String log = F("P240 : Data ");
          log += pm25;
          addLogMove(LOG_LEVEL_INFO, log);
        }
      }
      success = true;
    }

  } // switch
  return success;
}   // function

// Function P240_process_rx
// Handles a single character received from the PM1006 in the Vindrikning
// It expects a response to the poll message sent by the Vindriknings own CPU
// 0x16 <len> <data> <chksum>
// <len>  = size of <data> in char [1 char]
// <data> = message payload, array of <len> chars
// <csum> = checksum, 256-(sum of all received characters) [1 char]
bool P240_process_rx(char c)
{
    switch (rxState) {
    case PM1006_HEADER:
        rxChecksum = c;
        if (c == 0x16) {
            rxState = PM1006_LENGTH;
        }
        break;

    case PM1006_LENGTH:
        rxChecksum += c;
        if (c <= bufferSize) {
            _rxlen = c;
            _index = 0;
            rxState = (_rxlen > 0) ? PM1006_DATA : PM1006_CHECK;
        } else {
            rxState = PM1006_HEADER;
        }
        break;

    case PM1006_DATA:
        rxChecksum += c;
        serialRxBuffer[_index++] = c;
        if (_index == _rxlen) {
            rxState = PM1006_CHECK;
        }
        break;

    case PM1006_CHECK:
        rxChecksum += c;
        rxState = PM1006_HEADER;
        P240_dump();
        return ((rxChecksum&0xFF) == 0);

    default:
        rxState = PM1006_HEADER;
        break;
    }
    return false;
}

// Function P240_to_hex
// Helper to convert uint8/char to HEX representation
char * P240_to_hex(char c, char * ptr)
{
  static const char hex[] = "0123456789ABCDEF";
  *ptr++ = hex[c>>4];
  *ptr++ = hex[c&0x7];
  return ptr;
}

// Function P240_dump
// Dump the received buffer
// Note: Contents may be inconsistent unless alingned with the decoder
void P240_dump()
{
  String log = F("P240 : Dump ");
  char *ptr = debugBuffer;
  for (int n=0; n< _rxlen; n++)
  {
    ptr = P240_to_hex(serialRxBuffer[n], ptr);
    *ptr++ = ' ';
  }
  *ptr++ = '\0';
  log += debugBuffer;
  log += " size ";
  log += _rxlen;
  log += " csum ";
  log += (rxChecksum & 0xFF);
  addLogMove(LOG_LEVEL_INFO, log);
}

#endif
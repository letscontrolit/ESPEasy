//#######################################################################################################
//################################## Plugin 144: Dust Sensor Ikea Vindriktning ##########################
//#######################################################################################################
/*
  Plugin is based upon various open sources on the internet. Plugin uses the serial lib to access a serial port
  This plugin was written by flashmark

  This plugin reads the particle concentration from the PM1006 sensor in the Ikea Vindriktning
  The original Ikea processor is controlling the PM1006 sensor, the plugin is eavesdropping the responses
  DevicePin1 - RX on ESP, TX on PM1006
  DevicePin2 - TX on ESP, RX on PM1006 (optional, currently not accessed by the plugin)
*/


// #include section
#include "_Plugin_Helper.h"
#ifdef USES_P144
# include <ESPeasySerial.h>

// Standard plugin defines
#define PLUGIN_144
#define PLUGIN_ID_144     144                   // plugin id
#define PLUGIN_NAME_144   "Dust - PM1006(K) (Vindriktning)" // "Plugin Name" is what will be dislpayed in the selection list
#define PLUGIN_VALUENAME1_144 "PM2.5"           // variable output of the plugin. The label is in quotation marks
#define PLUGIN_144_DEBUG  true                  // set to true for extra log info


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
// #define P144_BAUDRATE           PCONFIG_LONG(0)
// #define P144_BAUDRATE_LABEL     PCONFIG_LABEL(0)

// States for statemachine used to decode received message
typedef enum {
    PM1006_HEADER,
    PM1006_LENGTH,
    PM1006_DATA,
    PM1006_CHECK
} pm1006_state_t;

// Global variables TODO: attach them to the task to allow multiple instances
  ESPeasySerial *P144_easySerial = nullptr;
  const int P144_bufferSize = 20;
  char P144_serialRxBuffer[P144_bufferSize];
  #ifdef PLUGIN_144_DEBUG
  char P144_debugBuffer[3*P144_bufferSize];
  #endif
  //int P144_rxBufferPtr = 0;
  int P144_rxChecksum = 0;
  int P144_index = 0;
  int P144_rxlen = 0;
  int P144_pm25 = 0;
  pm1006_state_t P144_rxState = PM1006_HEADER;


// A plugin has to implement the following function

boolean Plugin_144(uint8_t function, struct EventStruct *event, String& string)
{
  // function: reason the plugin was called
  // event: ??add description here??
  // string: ??add description here??

  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      // This case defines the device characteristics

      Device[++deviceCount].Number           = PLUGIN_ID_144;                    // Plugin ID number.   (PLUGIN_ID_xxx)
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
      string = F(PLUGIN_NAME_144);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      // called when the user opens the module configuration page
      // it allows to add a new row for each output variable of the plugin
      // For plugins able to choose output types, see P026_Sysinfo.ino.
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_144));
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
      /// addFormNumericBox(string, F("description"), F("plugin_144_description"), PCONFIG(1), min - value, max - value);

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
      P144_easySerial = new (std::nothrow) ESPeasySerial(serialHelper_getSerialType(event), rxPin, txPin);
      if (P144_easySerial != nullptr) 
      {
        P144_easySerial->begin(9600);
        success = true;
      }
      #ifdef PLUGIN_144_DEBUG
      String log = F("P144 : Init OK  ESP GPIO-pin RX:");
      log += rxPin;
      log += F(" TX:");
      log += txPin;
      addLogMove(LOG_LEVEL_INFO, log);
      #endif
      break;
    }

    case PLUGIN_READ:
    {
      // code to be executed to read data
      // It is executed according to the delay configured on the device configuration page, only once
      UserVar[event->BaseVarIndex] = P144_pm25;
      #ifdef PLUGIN_144_DEBUG
      String log = F("P144 : READ ");
      log += P144_pm25;
      addLogMove(LOG_LEVEL_INFO, log);
      #endif
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
      if (P144_easySerial != nullptr) {
        delete(P144_easySerial);
        P144_easySerial = nullptr;
      }
      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      // code to be executed once a second. Tasks which do not require fast response can be added here
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      // code to be executed 10 times per second. Tasks which require fast response can be added here
      // be careful on what is added here. Heavy processing will result in slowing the module down!
      bool new_data = false;
      while ((P144_easySerial->available() > 0) && !new_data) 
      {
        new_data = P144_process_rx(P144_easySerial->read());
        if (new_data) 
        {
          P144_pm25 = float((P144_serialRxBuffer[3] << 8) + P144_serialRxBuffer[4]);
          #ifdef PLUGIN_144_DEBUG
          String log = F("P144 : Data ");
          log += P144_pm25;
          addLogMove(LOG_LEVEL_INFO, log);
          #endif
        }
      }
      success = true;
      break;
    }

  } // switch
  return success;
}   // function

// Function P144_process_rx
// Handles a single character received from the PM1006 in the Vindrikning
// It expects a response to the poll message sent by the Vindriknings own CPU
// 0x16 <len> <data> <chksum>
// <len>  = size of <data> in char [1 char]
// <data> = message payload, array of <len> chars
// <csum> = checksum, 256-(sum of all received characters) [1 char]
bool P144_process_rx(char c)
{
    switch (P144_rxState) {
    case PM1006_HEADER:
        P144_rxChecksum = c;
        if (c == 0x16) {
            P144_rxState = PM1006_LENGTH;
        }
        break;

    case PM1006_LENGTH:
        P144_rxChecksum += c;
        if (c <= P144_bufferSize) {
            P144_rxlen = c;
            P144_index = 0;
            P144_rxState = (P144_rxlen > 0) ? PM1006_DATA : PM1006_CHECK;
        } else {
            P144_rxState = PM1006_HEADER;
        }
        break;

    case PM1006_DATA:
        P144_rxChecksum += c;
        P144_serialRxBuffer[P144_index++] = c;
        if (P144_index == P144_rxlen) {
            P144_rxState = PM1006_CHECK;
        }
        break;

    case PM1006_CHECK:
        P144_rxChecksum += c;
        P144_rxState = PM1006_HEADER;
#ifdef PLUGIN_144_DEBUG
        P144_dump();
#endif
        return ((P144_rxChecksum&0xFF) == 0);

    default:
        P144_rxState = PM1006_HEADER;
        break;
    }
    return false;
}

#ifdef PLUGIN_144_DEBUG
// Function P144_to_hex
// Helper to convert uint8/char to HEX representation
char * P144_to_hex(char c, char * ptr)
{
  static const char hex[] = "0123456789ABCDEF";
  *ptr++ = hex[c>>4];
  *ptr++ = hex[c&0x7];
  return ptr;
}

// Function P144_dump
// Dump the received buffer
// Note: Contents may be inconsistent unless alingned with the decoder
void P144_dump()
{
  String log = F("P144 : Dump ");
  char *ptr = P144_debugBuffer;
  for (int n=0; n< P144_rxlen; n++)
  {
    ptr = P144_to_hex(P144_serialRxBuffer[n], ptr);
    *ptr++ = ' ';
  }
  *ptr++ = '\0';
  log += P144_debugBuffer;
  log += " size ";
  log += P144_rxlen;
  log += " csum ";
  log += (P144_rxChecksum & 0xFF);
  addLogMove(LOG_LEVEL_INFO, log);
}
#endif // PLUGIN_144_DEBUG
#endif
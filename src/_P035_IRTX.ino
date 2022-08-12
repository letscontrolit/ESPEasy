#include "_Plugin_Helper.h"
#ifdef USES_P035

// #######################################################################################################
// #################################### Plugin 035: Output IR ############################################
// #######################################################################################################
//
// Changelog:
// 2022-08-08, tonhuisman:  Fix listProtocols()/listACProtocols() to ignore 1-character type names 
// 2022-01-11, tonhuisman:  Move all code and globals to PluginStructs/P035_data_struct to enable multi-instance use
// No previous changelog recorded.

// Usage: Connect an IR led to ESP8266 GPIO14 (D5) preferably. (various schematics can be found online)
// On the device tab add a new device and select "Communication - IR Transmit"
// Enable the device and select the GPIO led pin
// Power on the ESP and connect to it
// Commands can be send to this plug in and it will translate them to IR signals.
// Possible commands are IRSEND and IRSENDAC
// ---IRSEND: That commands format is: IRSEND,<protocol>,<data>,<bits>,<repeat>
// OR JSON formated:                   IRSEND,'{"protocol":"<protocol>","data":"<data>","bits":<bits>,"repeats":<repeat>}'
// bits and repeat default to 0 if not used and they are optional
// For protocols RAW and RAW2 there is no bits and repeat part, they are supposed to be replayed as they are calculated by a Google docs
// sheet or by plugin P016
// ---IRSENDAC: That commands format is:
// IRSENDAC,'{"protocol":"COOLIX","power":"on","mode":"dry","fanspeed":"auto","temp":22,"swingv":"max","swingh":"off"}'
// The possible values
// Protocols: Argo Coolix Daikin Fujitsu Haier Hitachi Kelvinator Midea Mitsubishi MitsubishiHeavy Panasonic Samsung Sharp Tcl Teco Toshiba
// Trotec Vestel Whirlpool
// ---opmodes:      ---fanspeed:   --swingv:       --swingh:
// - "off"          - "auto"       - "off"         - "off"
// - "auto"         - "min"        - "auto"        - "auto"
// - "cool"         - "low"        - "highest"     - "leftmax"
// - "heat"         - "medium"     - "high"        - "left"
// - "dry"          - "high"       - "middle"      - "middle"
// - "fan_only"     - "max"        - "low"         - "right"
//                                 - "lowest"      - "rightmax"
//                                                 - "wide"
// "on" - "off" parameters are:
// - "power" - "celsius" - "quiet" - "turbo" - "econo" - "light" - "filter" - "clean" - "light" - "beep"
// If Celsius is set to "off" then farenheit will be used
// - "sleep" Nr. of mins of sleep mode, or use sleep mode. (<= 0 means off.)
// - "clock" Nr. of mins past midnight to set the clock to. (< 0 means off.)
// - "model" . Nr or string representation of the model. Better to find it throught P016 - IR RX (0 means default.)

# include "./src/PluginStructs/P035_data_struct.h"

# define PLUGIN_035
# define PLUGIN_ID_035    35
# define PLUGIN_NAME_035  "Communication - IR Transmit"

boolean Plugin_035(uint8_t function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number       = PLUGIN_ID_035;
      Device[deviceCount].Type           = DEVICE_TYPE_SINGLE;
      Device[deviceCount].SendDataOption = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_035);
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("LED"));
      break;
    }
    case PLUGIN_WEBFORM_LOAD:
    {
      addRowLabel(F("Command"));
      addHtml(F("IRSEND,[PROTOCOL],[DATA],[BITS optional],[REPEATS optional]<BR>BITS and REPEATS are optional and default to 0<BR/>"));
      addHtml(F("IRSENDAC,{JSON formated AC command}"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P035_data_struct(CONFIG_PIN1));
      P035_data_struct *P035_data = static_cast<P035_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P035_data) {
        return success;
      }
      success = P035_data->plugin_init(event);
      break;
    }

    case PLUGIN_EXIT:
    {
      P035_data_struct *P035_data = static_cast<P035_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P035_data) {
        return success;
      }
      success = P035_data->plugin_exit(event);

      break;
    }

    case PLUGIN_WRITE:
    {
      P035_data_struct *P035_data = static_cast<P035_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P035_data) {
        return success;
      }
      success = P035_data->plugin_write(event, string);

      break;
    } // PLUGIN_WRITE END
  }   // SWITCH END
  return success;
}     // Plugin_035 END

#endif // USES_P035

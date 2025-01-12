#include "_Plugin_Helper.h"
#ifdef USES_P101

// #######################################################################################################
// ####################################### Plugin 101: Wake On LAN #######################################
// #######################################################################################################

// Wake On LAN (WOL): Send Magic Packet to MAC address; Wake-up sleeping computer if configured for WOL.
// Plugin Written by ThomasB (ThomasTech), Oct-12-2020.
//   CHANGE LOG
//   Oct-12-2020: Creation
//   Oct-16-2020: Beta Test Release to ESPEasy Forum.
//   Oct-18-2020: Re-assigned as plugin number P101 (was P248).
//   Oct-20-2020: Github PR #3328, Submitted as plugin.
//
// This ESPEasy plugin requires the WakeOnLan library found here:
//   https://github.com/a7md0/WakeOnLan
//
// Command Keyword:
//   Command Keyword is WAKEONLAN or Device's Task name. Keyword is case insensitive.
//
// Command Syntax:
//   WakeOnLan,<Optional MAC Address>,<Optional IP Address>,<Optional Port Number>
//
// Command Examples (Using Task Name = WakeKitchenPC):
//   WakeOnLan
//   WakeKitchenPC
//   WakeOnLan,01:23:45:67:89:AB
//   WakeOnLan,01:23:45:67:89:AB,192.168.1.255
//   WakeOnLan,01:23:45:67:89:AB,255.255.255.255,9
//
// Multiple Task Instances:
//   Multiple Wake-On-Lan device tasks can be installed. Each task MUST have a unique name for use
//   as the command keyword.
//
// Configuration Tips:
//   The standard Magic Packet IP address is 255.255.255.255. If the router does not accept this
//   then use the router's local subnet IP address with 255 at the end. For example, 192.168.1.255.
//   Some WOL devices will accept magic packets sent to their specific IP too.
//   Common UDP Port values are 0, 7, and 9.
//   PC's with integrated WiFi and Ethernet ports will likely have two MAC addresses, with specific
//   WOL settings for each one.
//   I don't believe USB connected network adapters are capable of WOL; Check the adapter's manual.
//
// Useful WOL tools:
// WakeMeOnLan WOL Scanner/Sender: https://www.nirsoft.net/utils/wake_on_lan.html
// wolsniffer WOL Packet Analyzer: https://apreltech.com/Downloads/wolsniffer.zip
// Online MAC Address Generator  : https://www.browserling.com/tools/random-mac
//
// ************************************************************************************************


# include <WakeOnLan.h>

// Plugin defines
# define PLUGIN_101
# define PLUGIN_ID_101      101
# define PLUGIN_NAME_101    "Communication - Wake On LAN"

// Config Setting defines
# define CUSTOMTASK_STR_SIZE_P101 20
# define DEF_TASK_NAME_P101 "WAKE_ON_LAN"
# define SET_UDP_PORT_P101  ExtraTaskSettings.TaskDevicePluginConfigLong[1]
# define GET_UDP_PORT_P101  Cache.getTaskDevicePluginConfigLong(event->TaskIndex, 1)
# define FORM_PORT_P101     "pport"

// Command keyword defines, checked in lowercase
# define CMD_NAME_P101      "wakeonlan"

// MAC Defines.
# define MAC_ADDR_SIZE_P101 17  // MAC Addr String size (fixed length). e.g. FA:39:09:67:89:AB
# define MAC_BUFF_SIZE_P101 18  // MAC Addr Buffer size, including NULL terminator.
# define MAC_SEP_CHAR_P101  ':' // MAC Addr segments are separated by a colon.
# define MAC_SEP_CNT_P101   5   // MAC Addr colon separator count for valid address.
# define MAC_STR_DEF_P101   "00:00:00:00:00:00"
# define MAC_STR_EXP_P101   "5d:89:22:56:9c:60"

// IP Defines.
# define IP_ADDR_SIZE_P101  15  // IPv4 Addr String size (max length). e.g. 192.168.001.255
# define IP_BUFF_SIZE_P101  16  // IPv4 Addr Buffer size, including NULL terminator.
# define IP_MIN_SIZE_P101   7   // IPv4 Addr Minimum size, allows IP strings as short as 0.0.0.0
# define IP_SEP_CHAR_P101   '.' // IPv4 Addr segments are separated by a dot.
# define IP_SEP_CNT_P101    3   // IPv4 Addr dot separator count for valid address.
# define IP_STR_DEF_P101    "255.255.255.255"

// Port Defines.
# define PORT_DEF_P101      9
# define PORT_MAX_P101      65535

// Misc Defines
# define LOG_NAME_P101      "WAKE ON LAN: "
# define NAME_MISSING       0
# define NAME_SAFE          1
# define NAME_UNSAFE        2

// ************************************************************************************************
// WOL Objects
WiFiUDP   udp;
WakeOnLan WOL(udp);

// ************************************************************************************************
// Local Functions
uint8_t safeName(taskIndex_t index);
bool    validateIp(const String& ipStr);
bool    validateMac(const String& macStr);
bool    validatePort(const String& portStr);

// ************************************************************************************************

boolean Plugin_101(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      auto& dev = Device[++deviceCount];
      dev.Number   = PLUGIN_ID_101;
      dev.Type     = DEVICE_TYPE_DUMMY;
      dev.VType    = Sensor_VType::SENSOR_TYPE_NONE;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_101);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      SET_UDP_PORT_P101 = PORT_DEF_P101;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      addFormSubHeader(EMPTY_STRING); // Blank line, vertical space.
      addFormHeader(F("Default Settings"));

      String strings[2];
      LoadCustomTaskSettings(event->TaskIndex, strings, 2, CUSTOMTASK_STR_SIZE_P101);

      addFormTextBox(F("MAC Address"), getPluginCustomArgName(1), strings[1], MAC_ADDR_SIZE_P101);
      addFormNote(F("Format Example, " MAC_STR_EXP_P101));
      addFormSubHeader(EMPTY_STRING); // Blank line, vertical space.

      addFormTextBox(F("IPv4 Address"), getPluginCustomArgName(0), strings[0], IP_ADDR_SIZE_P101);
      addFormNumericBox(F("UDP Port"), F(FORM_PORT_P101), GET_UDP_PORT_P101, 0, PORT_MAX_P101);
      addFormNote(
        concat(F("Typical Installations use IP Address " IP_STR_DEF_P101
                 ", Port "),  PORT_DEF_P101));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      char   deviceTemplate[2][CUSTOMTASK_STR_SIZE_P101] {};
      String errorStr;

      // Check Task Name.
      const uint8_t nameCode = safeName(event->TaskIndex);

      if ((nameCode == NAME_MISSING) || (nameCode == NAME_UNSAFE)) {        // Check to see if user submitted safe device name.
        strcpy(ExtraTaskSettings.TaskDeviceName, PSTR(DEF_TASK_NAME_P101)); // Use default name.

        if (nameCode == NAME_UNSAFE) {
          errorStr = F("ALERT, Renamed Unsafe Task Name. ");
        }
      }

      // Check IP Address.
      if (!safe_strncpy(deviceTemplate[0], webArg(getPluginCustomArgName(0)), IP_BUFF_SIZE_P101)) {
        // msgStr = getCustomTaskSettingsError(0); // Report string too long.
        // errorStr += msgStr;
        // msgStr    = wolStr + msgStr;
        // addLog(LOG_LEVEL_INFO, msgStr);
      }

      if (strlen(deviceTemplate[0]) == 0) { // IP Address missing, use default value (without webform warning).
        strcpy_P(deviceTemplate[0], PSTR(IP_STR_DEF_P101));

        addLogMove(LOG_LEVEL_INFO, F(LOG_NAME_P101 "Loaded Default IP = " IP_STR_DEF_P101));
      }
      else if (strlen(deviceTemplate[0]) < IP_MIN_SIZE_P101) { // IP Address too short, load default value. Warn User.
        strcpy_P(deviceTemplate[0], PSTR(IP_STR_DEF_P101));

        errorStr += F("Provided IP Invalid (Using Default). ");
        addLogMove(LOG_LEVEL_INFO, F(LOG_NAME_P101 "Provided IP Invalid (Using Default). [" IP_STR_DEF_P101 "]"));
      }
      else if (!validateIp(deviceTemplate[0])) { // Unexpected IP Address value. Leave as-is, but Warn User.
        errorStr += F("WARNING, Please Review IP Address. ");
        addLogMove(LOG_LEVEL_INFO, strformat(F(LOG_NAME_P101 "WARNING, Please Review IP Address. [%s]"), deviceTemplate[0]));
      }

      // Check MAC Address.
      if (!safe_strncpy(deviceTemplate[1], webArg(getPluginCustomArgName(1)), MAC_BUFF_SIZE_P101)) {
        // msgStr += getCustomTaskSettingsError(1); // Report string too long.
        // errorStr += msgStr;
        // msgStr    = wolStr + msgStr;
        // addLog(LOG_LEVEL_INFO, msgStr);
      }

      if (strlen(deviceTemplate[1]) == 0) { // MAC Address missing, use default value.
        strcpy_P(deviceTemplate[1], PSTR(MAC_STR_DEF_P101));

        errorStr += F("MAC Address Not Provided, Populated with Zero Values. ");
        addLogMove(LOG_LEVEL_INFO, F(LOG_NAME_P101 "MAC Address Not Provided, Populated with Zero Values. "));
      }
      else if (!validateMac(deviceTemplate[1])) { // Suspicious MAC Address. Leave as-is, but warn User.
        errorStr += F("ERROR, MAC Address Invalid. ");
        addLogMove(LOG_LEVEL_INFO, strformat(F(LOG_NAME_P101 "ERROR, MAC Address Invalid. [%s]"), deviceTemplate[1]));
      }

      if (!errorStr.isEmpty()) { // Send error messages (if any) to webform.
        addHtmlError(errorStr);
      }

      // Save all the Task parameters.
      SaveCustomTaskSettings(event->TaskIndex, reinterpret_cast<const uint8_t *>(&deviceTemplate), sizeof(deviceTemplate));
      SET_UDP_PORT_P101 = getFormItemInt(F(FORM_PORT_P101));
      success           = true;
      break;
    }

    case PLUGIN_INIT: {
      success = true;
      break;
    }

    case PLUGIN_READ: {
      // Do nothing on purpose
      break;
    }

    case PLUGIN_WRITE: {
      String strings[2];

      //  addLog(LOG_LEVEL_INFO, concat(F("--> WOL taskIndex= "), event->TaskIndex)); // Debug

      const String cmd = parseString(string, 1);

      //  Warning, event->TaskIndex is invalid in PLUGIN_WRITE during controller ack calls.
      //  So checking the Device Name needs special attention.
      //  See https://github.com/letscontrolit/ESPEasy/issues/3317
      if (validTaskIndex(event->TaskIndex) &&
          (equals(cmd, F(CMD_NAME_P101)) ||
           cmd.equalsIgnoreCase(getTaskDeviceName(event->TaskIndex)))) {
        success = true;

        LoadCustomTaskSettings(event->TaskIndex, strings, 2, CUSTOMTASK_STR_SIZE_P101);

        String paramMac  = parseString(string, 2); // MAC Address (optional)
        String paramIp   = parseString(string, 3); // IP Address (optional)
        String paramPort = parseString(string, 4); // UDP Port (optional)

        // Populate Parameters with default settings when missing from command line.
        if (paramMac.isEmpty()) {                  // Missing from command line, use default setting.
          paramMac = strings[1];
        }

        if (paramIp.isEmpty()) { // Missing from command line, use default setting.
          paramIp = strings[0];
        }

        if (paramPort.isEmpty()) {
          LoadTaskSettings(event->TaskIndex); // FIXME Not sure if this is still needed...
          paramPort = GET_UDP_PORT_P101;      // Get default Port from user settings.
        }

        // Validate the MAC Address.
        if (!validateMac(paramMac)) {
          success = false;
          addLogMove(LOG_LEVEL_INFO, strformat(F(LOG_NAME_P101 "Error, MAC Addr Invalid [%s]"), paramMac.c_str()));
        }

        // Validate IP Address.
        if (!validateIp(paramIp)) {
          success = false;
          addLogMove(LOG_LEVEL_INFO, strformat(F(LOG_NAME_P101 "Error, IP Addr Invalid [%s]"), paramIp.c_str()));
        }

        // Validate UDP Port.
        if (!validatePort(paramPort)) {
          success = false;
          addLogMove(LOG_LEVEL_INFO, strformat(F(LOG_NAME_P101 "Error, Port Invalid [%s]"), paramPort.c_str()));
        }

        // If no errors we can send Magic Packet.
        if (!success) {
          String msgStr = F("CMD Syntax Error");
          addLogMove(LOG_LEVEL_INFO, concat(F(LOG_NAME_P101), msgStr));
          msgStr += F(" <br>");
          SendStatus(event, msgStr); // Reply (echo) to sender. This will print message on browser.
        }
        else {                       // No parsing errors, Send Magic Packet (Wake Up the MAC).
          addLogMove(LOG_LEVEL_INFO, strformat(
                       F(LOG_NAME_P101 "MAC= %s, IP= %s, Port= %s"),
                       paramMac.c_str(),
                       paramIp.c_str(),
                       paramPort.c_str()));

          // Send Magic Packet.
          if (WiFi.status() == WL_CONNECTED) {
            IPAddress local_IP;
            local_IP.fromString(paramIp);
            WOL.setBroadcastAddress(local_IP);

            // WOL.setRepeat(1, 0); // One Magic Packet, No Repeats. (Library default)

            if (!WOL.sendMagicPacket(paramMac, paramPort.toInt())) {
              addLogMove(LOG_LEVEL_INFO, F(LOG_NAME_P101 "Error, Magic Packet Failed (check parameters)"));
            }
          }
          else {
            addLogMove(LOG_LEVEL_INFO, F(LOG_NAME_P101 "Error, WiFi Off-Line"));
          }
        }
      }
      break;
    }
  }
  return success;
}

// ************************************************************************************************
// safeName(): Check task name and confirm it is present and safe to use.
// Arg: task index
// Returns: NAME_SAFE, NAME_MISSING, or NAME_USAFE.
uint8_t safeName(taskIndex_t index) {
  String devName = getTaskDeviceName(index);

  if (devName.isEmpty()) {
    return NAME_MISSING;
  }
  devName.toLowerCase();

  if (equals(devName, F("reboot")) ||
      equals(devName, F("reset"))) {
    return NAME_UNSAFE;
  }

  return NAME_SAFE;
}

// ************************************************************************************************
// validateIp(): IPv4 Address validity checker.
// Arg: IP Address String in dot separated format (192.168.1.255).
// Return true if IP string appears legit.
bool validateIp(const String& ipStr) {
  IPAddress ip;
  unsigned int length = ipStr.length();

  if ((length < IP_MIN_SIZE_P101) || (length > IP_ADDR_SIZE_P101)) {
    return false;
  }

  // else if (ip.fromString(ipStr) == false) { // ThomasTech's Trick to Check IP for valid formatting.
  //   return false;
  // }

  // return true;
  return ip.fromString(ipStr); // Trick is nice, code not so readable :-)
}

// ************************************************************************************************
// validateMac(): MAC Address validity checker.
// Arg: MAC Address String in colon separated format (00:00:00:00:00:00).
// Return true if MAC string appears legit.
bool validateMac(const String& macStr) {
  uint8_t pos = 0;

  if (macStr.length() != MAC_ADDR_SIZE_P101) {
    return false;
  }

  for (uint8_t strPos = 0; strPos < MAC_ADDR_SIZE_P101; ++strPos) {
    const uint8_t mod = strPos % 3;

    if (mod == 2) {
      if (macStr[strPos] == MAC_SEP_CHAR_P101) { // Must be a colon in the third position.
        pos++;
      }
      else {
        return false;
      }
    }
    else {
      if (!isHexadecimalDigit(macStr.charAt(strPos))) {
        return false;
      }
    }
  }
  return true;
}

// ************************************************************************************************
// validatePort(): Simple UDP Port Number validity checker.
// Arg: Port Number String (decimal).
// Return true if Port string appears legit.
bool validatePort(const String& portStr) {
  uint32_t portNumber{};

  return validUIntFromString(portStr, portNumber) && portNumber <= PORT_MAX_P101;
}

// ************************************************************************************************

#endif // USES_P101

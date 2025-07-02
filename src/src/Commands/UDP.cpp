#include "../Commands/UPD.h"


#include "../../ESPEasy_common.h"

#include "../Commands/Common.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Settings.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Network.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"

String Command_UDP_Port(struct EventStruct *event, const char *Line)
{
  return Command_GetORSetBool(event, F("UDPPort:"),
                              Line,
                              (bool *)&Settings.UDPPort,
                              1);
}

#if FEATURE_ESPEASY_P2P

const __FlashStringHelper* Command_UDP_Test(struct EventStruct *event, const char *Line)
{
  for (uint8_t x = 0; x < event->Par2; x++)
  {
    String eventName = "Test ";
    eventName += x;
    SendUDPCommand(event->Par1, eventName.c_str(), eventName.length());
  }
  return return_command_success_flashstr();
}

const __FlashStringHelper* Command_UPD_SendTo(struct EventStruct *event, const char *Line)
{
  int destUnit = parseCommandArgumentInt(Line, 1);

  if ((destUnit > 0) && (destUnit < 255))
  {
    String eventName = tolerantParseStringKeepCase(Line, 3);
    stripEscapeCharacters(eventName);
    SendUDPCommand(destUnit, eventName.c_str(), eventName.length());
  }
  return return_command_success_flashstr();
}

#endif // FEATURE_ESPEASY_P2P

const __FlashStringHelper* Command_UDP_SendToUPD(struct EventStruct *event, const char *Line)
{
  return Command_UDP_SendToUPD(event, Line, false);
}

const __FlashStringHelper* Command_UDP_SendToUPDMix(struct EventStruct *event, const char *Line)
{
  return Command_UDP_SendToUPD(event, Line, true);
}

const __FlashStringHelper* Command_UDP_SendToUPD(struct EventStruct *event, const char *Line, const bool handleMix)
{
  if (NetworkConnected()) {
    const String ip   = parseString(Line, 2);
    const int    port = parseCommandArgumentInt(Line, 2, -1);

    if ((port < 0) || (port > 65535)) {
      return return_command_failed_flashstr();
    }

    // FIXME TD-er: This command is not using the tolerance setting
    // tolerantParseStringKeepCase(Line, 4);
    String message;
    std::vector<uint8_t> argument;

    if (handleMix) {
      argument = parseHexTextData(Line, 4);
    } else {
      message = parseStringToEndKeepCase(Line, 4);
    }
    IPAddress UDP_IP;

    if (UDP_IP.fromString(ip)) {
      FeedSW_watchdog();
      portUDP.beginPacket(UDP_IP, port);

      if (handleMix) {
        portUDP.write(&argument[0], argument.size());
      } else {
        #if defined(ESP8266)
        portUDP.write(message.c_str(),                                    message.length());
        #endif // if defined(ESP8266)
        #if defined(ESP32)
        portUDP.write(reinterpret_cast<const uint8_t *>(message.c_str()), message.length());
        #endif // if defined(ESP32)
      }
      portUDP.endPacket();
      FeedSW_watchdog();
      delay(0);
    }
    return return_command_success_flashstr();
  }
  return return_not_connected();
}

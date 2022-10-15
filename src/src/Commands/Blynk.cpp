#include "../Commands/Blynk.h"

#ifdef USES_C012

#include "../Commands/Common.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../ESPEasyCore/ESPEasy_backgroundtasks.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Protocol.h"
#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/_CPlugin_Helper.h"

#include "../../ESPEasy_fdwdecl.h"


controllerIndex_t firstEnabledBlynk_ControllerIndex() {
  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(i);

    if (validProtocolIndex(ProtocolIndex)) {
      if ((Protocol[ProtocolIndex].Number == 12) && Settings.ControllerEnabled[i]) {
        return i;
      }
    }
  }
  return INVALID_CONTROLLER_INDEX;
}

const __FlashStringHelper * Command_Blynk_Get(struct EventStruct *event, const char *Line)
{
  controllerIndex_t first_enabled_blynk_controller = firstEnabledBlynk_ControllerIndex();

  if (!validControllerIndex(first_enabled_blynk_controller)) {
    return F("Controller not enabled");
  } else {
    // FIXME TD-er: This one is not using parseString* function
    String strLine = Line;
    strLine = strLine.substring(9);
    int index = strLine.indexOf(',');

    if (index > 0)
    {
      int index           = strLine.lastIndexOf(',');
      String blynkcommand = strLine.substring(index + 1);
      float  value        = 0;

      if (Blynk_get(blynkcommand, first_enabled_blynk_controller, &value))
      {
        UserVar[(VARS_PER_TASK * (event->Par1 - 1)) + event->Par2 - 1] = value;
      }
      else {
        return F("Error getting data");
      }
    }
    else
    {
      if (!Blynk_get(strLine, first_enabled_blynk_controller, nullptr))
      {
        return F("Error getting data");
      }
    }
  }
  return return_command_success();
}

bool Blynk_get(const String& command, controllerIndex_t controllerIndex, float *data)
{
  bool MustCheckReply = false;
  String hostname, pass;
  unsigned int ClientTimeout = 0;
  WiFiClient client;

  {
    // Place ControllerSettings in its own scope, as it is quite big.
    MakeControllerSettings(ControllerSettings); //-V522
    if (!AllocatedControllerSettings()) {
      addLog(LOG_LEVEL_ERROR, F("Blynk : Cannot run GET, out of RAM"));
      return false;
    }

    LoadControllerSettings(controllerIndex, ControllerSettings);
    MustCheckReply = ControllerSettings.MustCheckReply;
    hostname = ControllerSettings.getHost();
    pass = getControllerPass(controllerIndex, ControllerSettings);
    ClientTimeout = ControllerSettings.ClientTimeout;

    if (pass.isEmpty()) {
      addLog(LOG_LEVEL_ERROR, F("Blynk : No password set"));
      return false;
    }

    if (!try_connect_host(/* CPLUGIN_ID_012 */ 12, client, ControllerSettings)) {
      return false;
    }
  }

  // We now create a URI for the request
  {
    // Place this stack allocated array in its own scope, as it is quite big.
    char request[300] = { 0 };
    sprintf_P(request,
              PSTR("GET /%s/%s HTTP/1.1\r\n Host: %s \r\n Connection: close\r\n\r\n"),
              pass.c_str(),
              command.c_str(),
              hostname.c_str());
#ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, request);
#endif
    client.print(request);
  }
  bool success = !MustCheckReply;

  if (MustCheckReply || data) {
    unsigned long timer = millis() + ClientTimeout;

    while (!client_available(client) && !timeOutReached(timer)) {
      delay(1);
    }

    #ifndef BUILD_NO_DEBUG
    char log[80] = { 0 };
    #endif
    timer = millis() + 1500;

    // Read all the lines of the reply from server and log them
    while (client_available(client) && !success && !timeOutReached(timer)) {
      String line;
      safeReadStringUntil(client, line, '\n');
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG_MORE, line);
      #endif

      // success ?
      if (line.substring(0, 15).equals(F("HTTP/1.1 200 OK"))) {
        #ifndef BUILD_NO_DEBUG
        strcpy_P(log, PSTR("HTTP : Success"));
        #endif

        if (!data) { success = true; }
      }
      #ifndef BUILD_NO_DEBUG
      else if (line.substring(0, 24).equals(F("HTTP/1.1 400 Bad Request"))) {
        strcpy_P(log, PSTR("HTTP : Unauthorized"));
      }
      else if (line.substring(0, 25).equals(F("HTTP/1.1 401 Unauthorized"))) {
        strcpy_P(log, PSTR("HTTP : Unauthorized"));
      }
      addLog(LOG_LEVEL_DEBUG, log);
      #endif

      // data only
      if (data && line.startsWith("["))
      {
        String strValue = line;
        uint8_t   pos      = strValue.indexOf('"', 2);
        strValue = strValue.substring(2, pos);
        strValue.trim();
        *data   = 0.0f;
        validFloatFromString(strValue, *data);
        success = true;

        char value_char[5] = { 0 };
        strValue.toCharArray(value_char, 5);
        #ifndef BUILD_NO_DEBUG
        sprintf_P(log, PSTR("Blynk get - %s => %s"), command.c_str(), value_char);
        addLog(LOG_LEVEL_DEBUG, log);
        #endif
      }
      delay(0);
    }
  }
  #ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG, F("HTTP : closing connection (012)"));
  #endif

  client.flush();
  client.stop();

  // important - backgroundtasks - free mem
  unsigned long timer = millis() + 10;

  while (!timeOutReached(timer)) {
    backgroundtasks();
  }

  return success;
}

#endif // ifdef USES_C012

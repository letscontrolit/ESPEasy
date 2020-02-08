#include "src/Globals/CPlugins.h"
#include "src/Globals/Protocol.h"
#include "src/Globals/Settings.h"

#include "src/DataStructs/ESPEasy_EventStruct.h"
#include "src/DataStructs/TimingStats.h"

#include "ESPEasy_common.h"
#include "ESPEasy_plugindefs.h"


// ********************************************************************************
// Initialize all Controller CPlugins that where defined earlier
// and initialize the function call pointer into the CCPlugin array
// ********************************************************************************

static const char ADDCPLUGIN_ERROR[] PROGMEM = "System: Error - Too many C-Plugins";

// Because of compiler-bug (multiline defines gives an error if file ending is CRLF) the define is striped to a single line

/*
 #define ADDCPLUGIN(NNN) \
   if (x < CPLUGIN_MAX) \
   { \
    ProtocolIndex_to_CPlugin_id[x] = CPLUGIN_ID_##NNN; \
    CPlugin_id_to_ProtocolIndex[CPLUGIN_ID_##NNN] = x; \
    CPlugin_ptr[x++] = &CPlugin_##NNN; \
   } \
  else \
    addLog(LOG_LEVEL_ERROR, FPSTR(ADDCPLUGIN_ERROR));
*/
#define ADDCPLUGIN(NNN) if (x < CPLUGIN_MAX) {     ProtocolIndex_to_CPlugin_id[x] = CPLUGIN_ID_##NNN; CPlugin_id_to_ProtocolIndex[CPLUGIN_ID_##NNN] = x; CPlugin_ptr[x++] = &CPlugin_##NNN; } else addLog(LOG_LEVEL_ERROR, FPSTR(ADDCPLUGIN_ERROR));

void CPluginInit(void)
{
  ProtocolIndex_to_CPlugin_id.resize(CPLUGIN_MAX + 1); // INVALID_CONTROLLER_INDEX may be used as index for this array.
  ProtocolIndex_to_CPlugin_id[CPLUGIN_MAX] = INVALID_C_PLUGIN_ID;
  byte x;

  // Clear pointer table for all plugins
  for (x = 0; x < CPLUGIN_MAX; x++)
  {
    CPlugin_ptr[x]                 = nullptr;
    ProtocolIndex_to_CPlugin_id[x] = INVALID_C_PLUGIN_ID;
    // Do not initialize CPlugin_id_to_ProtocolIndex[x] to an invalid value. (it is map)
  }

  x = 0;

#ifdef CPLUGIN_001
  ADDCPLUGIN(001)
#endif

#ifdef CPLUGIN_002
  ADDCPLUGIN(002)
#endif

#ifdef CPLUGIN_003
  ADDCPLUGIN(003)
#endif

#ifdef CPLUGIN_004
  ADDCPLUGIN(004)
#endif

#ifdef CPLUGIN_005
  ADDCPLUGIN(005)
#endif

#ifdef CPLUGIN_006
  ADDCPLUGIN(006)
#endif

#ifdef CPLUGIN_007
  ADDCPLUGIN(007)
#endif

#ifdef CPLUGIN_008
  ADDCPLUGIN(008)
#endif

#ifdef CPLUGIN_009
  ADDCPLUGIN(009)
#endif

#ifdef CPLUGIN_010
  ADDCPLUGIN(010)
#endif

#ifdef CPLUGIN_011
  ADDCPLUGIN(011)
#endif

#ifdef CPLUGIN_012
  ADDCPLUGIN(012)
#endif

#ifdef CPLUGIN_013
  ADDCPLUGIN(013)
#endif

#ifdef CPLUGIN_014
  ADDCPLUGIN(014)
#endif

#ifdef CPLUGIN_015
  ADDCPLUGIN(015)
#endif

#ifdef CPLUGIN_016
  ADDCPLUGIN(016)
#endif

#ifdef CPLUGIN_017
  ADDCPLUGIN(017)
#endif

#ifdef CPLUGIN_018
  ADDCPLUGIN(018)
#endif

#ifdef CPLUGIN_019
  ADDCPLUGIN(019)
#endif

#ifdef CPLUGIN_020
  ADDCPLUGIN(020)
#endif

#ifdef CPLUGIN_021
  ADDCPLUGIN(021)
#endif

#ifdef CPLUGIN_022
  ADDCPLUGIN(022)
#endif

#ifdef CPLUGIN_023
  ADDCPLUGIN(023)
#endif

#ifdef CPLUGIN_024
  ADDCPLUGIN(024)
#endif

#ifdef CPLUGIN_025
  ADDCPLUGIN(025)
#endif

  CPluginCall(CPLUGIN_PROTOCOL_ADD, 0);

  // Set all not supported cplugins to disabled.
  for (controllerIndex_t controller = 0; controller < CONTROLLER_MAX; ++controller) {
    if (!supportedCPluginID(Settings.Protocol[controller])) {
      Settings.ControllerEnabled[controller] = false;
    }
  }
  CPluginCall(CPLUGIN_INIT_ALL, 0);
}

bool CPluginCall(protocolIndex_t protocolIndex, byte Function, struct EventStruct *event, String& str) {
  if (validProtocolIndex(protocolIndex)) {
    START_TIMER;
    bool ret = CPlugin_ptr[protocolIndex](Function, event, str);
    STOP_TIMER_CONTROLLER(protocolIndex, Function);
    return ret;
  }
  return false;
}

bool CPluginCall(byte Function, struct EventStruct *event, String& str)
{
  struct EventStruct TempEvent;

  if (event == 0) {
    event = &TempEvent;
  }

  switch (Function)
  {
    // Unconditional calls to all included controller in the build
    case CPLUGIN_PROTOCOL_ADD:

      for (protocolIndex_t x = 0; x < CPLUGIN_MAX; x++) {
        if (validCPluginID(ProtocolIndex_to_CPlugin_id[x])) {
          const unsigned int next_ProtocolIndex = protocolCount + 2;

          if (next_ProtocolIndex > Protocol.size()) {
            // Increase with 8 to get some compromise between number of resizes and wasted space
            unsigned int newSize = Protocol.size();
            newSize = newSize + 8 - (newSize % 8);
            Protocol.resize(newSize);
          }
          checkRAM(F("CPluginCallADD"), x);
          String dummy;
          CPluginCall(x, Function, event, dummy);
        }
      }
      return true;


    // calls to all active controllers
    case CPLUGIN_INIT_ALL:
    case CPLUGIN_UDP_IN:
    case CPLUGIN_INTERVAL:      // calls to send stats information
    case CPLUGIN_GOT_CONNECTED: // calls to send autodetect information
    case CPLUGIN_GOT_INVALID:   // calls to mark unit as invalid
    case CPLUGIN_FLUSH:
    case CPLUGIN_TEN_PER_SECOND:

      if (Function == CPLUGIN_INIT_ALL) {
        Function = CPLUGIN_INIT;
      }

      for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
        if ((Settings.Protocol[x] != 0) && Settings.ControllerEnabled[x]) {
          event->ProtocolIndex = getProtocolIndex(Settings.Protocol[x]);
          String dummy;
          CPluginCall(event->ProtocolIndex, Function, event, dummy);
        }
      }
      return true;

    // calls to specific controller
    case CPLUGIN_INIT:
    case CPLUGIN_EXIT:
    {
      controllerIndex_t controllerindex = event->ControllerIndex;

      if (Settings.ControllerEnabled[controllerindex] && supportedCPluginID(Settings.Protocol[controllerindex]))
      {
        if (!validProtocolIndex(event->ProtocolIndex))
        {
          event->ProtocolIndex = getProtocolIndex(Settings.Protocol[controllerindex]);
        }
        CPluginCall(event->ProtocolIndex, Function, event, str);
      }
      break;
    }

    case CPLUGIN_ACKNOWLEDGE: // calls to send acknowledge back to controller

      for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
        if (Settings.ControllerEnabled[x] && supportedCPluginID(Settings.Protocol[x])) {
          event->ProtocolIndex = getProtocolIndex(Settings.Protocol[x]);
          CPluginCall(event->ProtocolIndex, Function, event, str);
        }
      }
      return true;
  }

  return false;
}

// Check if there is any controller enabled.
bool anyControllerEnabled() {
  for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
    if (Settings.ControllerEnabled[x] && supportedCPluginID(Settings.Protocol[x])) {
      return true;
    }
  }
  return false;
}

// Find first enabled controller index with this protocol
controllerIndex_t findFirstEnabledControllerWithId(cpluginID_t cpluginid) {
  if (!supportedCPluginID(cpluginid)) {
    return INVALID_CONTROLLER_INDEX;
  }

  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; i++) {
    if ((Settings.Protocol[i] == cpluginid) && Settings.ControllerEnabled[i]) {
      return i;
    }
  }
  return INVALID_CONTROLLER_INDEX;
}

bool CPluginCall(byte Function, struct EventStruct *event) {
  String dummy;

  return CPluginCall(Function, event, dummy);
}

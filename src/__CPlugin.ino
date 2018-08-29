//********************************************************************************
// Initialize all Controller CPlugins that where defined earlier
// and initialize the function call pointer into the CCPlugin array
//********************************************************************************

static const char ADDCPLUGIN_ERROR[] PROGMEM = "System: Error - To much C-Plugins";

// Because of compiler-bug (multiline defines gives an error if file ending is CRLF) the define is striped to a single line
/*
#define ADDCPLUGIN(NNN) \
  if (x < CPLUGIN_MAX) \
  { \
    CPlugin_id[x] = CPLUGIN_ID_##NNN; \
    CPlugin_ptr[x++] = &CPlugin_##NNN; \
  } \
  else \
    addLog(LOG_LEVEL_ERROR, FPSTR(ADDCPLUGIN_ERROR));
*/
#define ADDCPLUGIN(NNN) if (x < CPLUGIN_MAX) { CPlugin_id[x] = CPLUGIN_ID_##NNN; CPlugin_ptr[x++] = &CPlugin_##NNN; } else addLog(LOG_LEVEL_ERROR, FPSTR(ADDCPLUGIN_ERROR));


void CPluginInit(void)
{
  byte x;

  // Clear pointer table for all plugins
  for (x = 0; x < CPLUGIN_MAX; x++)
  {
    CPlugin_ptr[x] = 0;
    CPlugin_id[x] = 0;
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
  CPluginCall(CPLUGIN_INIT, 0);
}

byte CPluginCall(byte Function, struct EventStruct *event)
{
  int x;
  struct EventStruct TempEvent;

 if (event == 0)
    event=&TempEvent;

  switch (Function)
  {
    // Unconditional calls to all plugins
    case CPLUGIN_PROTOCOL_ADD:
      for (x = 0; x < CPLUGIN_MAX; x++) {
        if (CPlugin_id[x] != 0) {
          const unsigned int next_ProtocolIndex = protocolCount + 2;
          if (next_ProtocolIndex > Protocol.size()) {
            // Increase with 8 to get some compromise between number of resizes and wasted space
            unsigned int newSize = Protocol.size();
            newSize = newSize + 8 - (newSize % 8);
            Protocol.resize(newSize);
          }
          checkRAM(F("CPluginCallADD"),x);
          CPlugin_ptr[x](Function, event, dummyString);
        }
      }
      return true;
      break;

    // calls to active plugins
    case CPLUGIN_INIT:
    case CPLUGIN_UDP_IN:
      for (byte x=0; x < CONTROLLER_MAX; x++)
        if (Settings.Protocol[x] != 0 && Settings.ControllerEnabled[x]) {
          event->ProtocolIndex = getProtocolIndex(Settings.Protocol[x]);
          CPlugin_ptr[event->ProtocolIndex](Function, event, dummyString);
        }
      return true;
      break;
  }

  return false;
}

// Check if there is any controller enabled.
bool anyControllerEnabled() {
  for (byte i=0; i < CONTROLLER_MAX; i++) {
    if (Settings.Protocol[i] != 0 && Settings.ControllerEnabled[i]) {
      return true;
    }
  }
  return false;
}

// Find first enabled controller index with this protocol
byte findFirstEnabledControllerWithId(byte cpluginid) {
  for (byte i=0; i < CONTROLLER_MAX; i++) {
    if (Settings.Protocol[i] == cpluginid && Settings.ControllerEnabled[i]) {
      return i;
    }
  }
  return CONTROLLER_MAX;
}

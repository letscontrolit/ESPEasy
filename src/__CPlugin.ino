//********************************************************************************
// Initialize all Controller CPlugins that where defined earlier
// and initialize the function call pointer into the CCPlugin array
//********************************************************************************
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
  CPlugin_id[x] = 1; CPlugin_ptr[x++] = &CPlugin_001;
#endif

#ifdef CPLUGIN_002
  CPlugin_id[x] = 2; CPlugin_ptr[x++] = &CPlugin_002;
#endif

#ifdef CPLUGIN_003
  CPlugin_id[x] = 3; CPlugin_ptr[x++] = &CPlugin_003;
#endif

#ifdef CPLUGIN_004
  CPlugin_id[x] = 4; CPlugin_ptr[x++] = &CPlugin_004;
#endif

#ifdef CPLUGIN_005
  CPlugin_id[x] = 5; CPlugin_ptr[x++] = &CPlugin_005;
#endif

#ifdef CPLUGIN_006
  CPlugin_id[x] = 6; CPlugin_ptr[x++] = &CPlugin_006;
#endif

#ifdef CPLUGIN_007
  CPlugin_id[x] = 7; CPlugin_ptr[x++] = &CPlugin_007;
#endif

#ifdef CPLUGIN_008
  CPlugin_id[x] = 8; CPlugin_ptr[x++] = &CPlugin_008;
#endif

#ifdef CPLUGIN_009
  CPlugin_id[x] = 9; CPlugin_ptr[x++] = &CPlugin_009;
#endif

#ifdef CPLUGIN_010
  CPlugin_id[x] = 10; CPlugin_ptr[x++] = &CPlugin_010;
#endif

#ifdef CPLUGIN_011
  CPlugin_id[x] = 11; CPlugin_ptr[x++] = &CPlugin_011;
#endif

#ifdef CPLUGIN_012
  CPlugin_id[x] = 12; CPlugin_ptr[x++] = &CPlugin_012;
#endif

#ifdef CPLUGIN_013
  CPlugin_id[x] = 13; CPlugin_ptr[x++] = &CPlugin_013;
#endif

#ifdef CPLUGIN_014
  CPlugin_id[x] = 14; CPlugin_ptr[x++] = &CPlugin_014;
#endif

#ifdef CPLUGIN_015
  CPlugin_id[x] = 15; CPlugin_ptr[x++] = &CPlugin_015;
#endif

#ifdef CPLUGIN_016
  CPlugin_id[x] = 16; CPlugin_ptr[x++] = &CPlugin_016;
#endif

#ifdef CPLUGIN_017
  CPlugin_id[x] = 17; CPlugin_ptr[x++] = &CPlugin_017;
#endif

#ifdef CPLUGIN_018
  CPlugin_id[x] = 18; CPlugin_ptr[x++] = &CPlugin_018;
#endif

#ifdef CPLUGIN_019
  CPlugin_id[x] = 19; CPlugin_ptr[x++] = &CPlugin_019;
#endif

#ifdef CPLUGIN_020
  CPlugin_id[x] = 20; CPlugin_ptr[x++] = &CPlugin_020;
#endif

#ifdef CPLUGIN_021
  CPlugin_id[x] = 21; CPlugin_ptr[x++] = &CPlugin_021;
#endif

#ifdef CPLUGIN_022
  CPlugin_id[x] = 22; CPlugin_ptr[x++] = &CPlugin_022;
#endif

#ifdef CPLUGIN_023
  CPlugin_id[x] = 23; CPlugin_ptr[x++] = &CPlugin_023;
#endif

#ifdef CPLUGIN_024
  CPlugin_id[x] = 24; CPlugin_ptr[x++] = &CPlugin_024;
#endif

#ifdef CPLUGIN_025
  CPlugin_id[x] = 25; CPlugin_ptr[x++] = &CPlugin_025;
#endif

  CPluginCall(CPLUGIN_PROTOCOL_ADD, 0);
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
      for (x = 0; x < CPLUGIN_MAX; x++)
        if (CPlugin_id[x] != 0)
          CPlugin_ptr[x](Function, event, dummyString);
      return true;
      break;
  }

  return false;
}


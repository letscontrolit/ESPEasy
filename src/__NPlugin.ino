//********************************************************************************
// Initialize all Controller NPlugins that where defined earlier
// and initialize the function call pointer into the CNPlugin array
//********************************************************************************
void NPluginInit(void)
{
  byte x;

  // Clear pointer table for all plugins
  for (x = 0; x < NPLUGIN_MAX; x++)
  {
    NPlugin_ptr[x] = 0;
    NPlugin_id[x] = 0;
  }

  x = 0;

#ifdef NPLUGIN_001
  NPlugin_id[x] = 1; NPlugin_ptr[x++] = &NPlugin_001;
#endif

#ifdef NPLUGIN_002
  NPlugin_id[x] = 2; NPlugin_ptr[x++] = &NPlugin_002;
#endif

#ifdef NPLUGIN_003
  NPlugin_id[x] = 3; NPlugin_ptr[x++] = &NPlugin_003;
#endif

#ifdef NPLUGIN_004
  NPlugin_id[x] = 4; NPlugin_ptr[x++] = &NPlugin_004;
#endif

#ifdef NPLUGIN_005
  NPlugin_id[x] = 5; NPlugin_ptr[x++] = &NPlugin_005;
#endif

#ifdef NPLUGIN_006
  NPlugin_id[x] = 6; NPlugin_ptr[x++] = &NPlugin_006;
#endif

#ifdef NPLUGIN_007
  NPlugin_id[x] = 7; NPlugin_ptr[x++] = &NPlugin_007;
#endif

#ifdef NPLUGIN_008
  NPlugin_id[x] = 8; NPlugin_ptr[x++] = &NPlugin_008;
#endif

#ifdef NPLUGIN_009
  NPlugin_id[x] = 9; NPlugin_ptr[x++] = &NPlugin_009;
#endif

#ifdef NPLUGIN_010
  NPlugin_id[x] = 10; NPlugin_ptr[x++] = &NPlugin_010;
#endif

#ifdef NPLUGIN_011
  NPlugin_id[x] = 11; NPlugin_ptr[x++] = &NPlugin_011;
#endif

#ifdef NPLUGIN_012
  NPlugin_id[x] = 12; NPlugin_ptr[x++] = &NPlugin_012;
#endif

#ifdef NPLUGIN_013
  NPlugin_id[x] = 13; NPlugin_ptr[x++] = &NPlugin_013;
#endif

#ifdef NPLUGIN_014
  NPlugin_id[x] = 14; NPlugin_ptr[x++] = &NPlugin_014;
#endif

#ifdef NPLUGIN_015
  NPlugin_id[x] = 15; NPlugin_ptr[x++] = &NPlugin_015;
#endif

#ifdef NPLUGIN_016
  NPlugin_id[x] = 16; NPlugin_ptr[x++] = &NPlugin_016;
#endif

#ifdef NPLUGIN_017
  NPlugin_id[x] = 17; NPlugin_ptr[x++] = &NPlugin_017;
#endif

#ifdef NPLUGIN_018
  NPlugin_id[x] = 18; NPlugin_ptr[x++] = &NPlugin_018;
#endif

#ifdef NPLUGIN_019
  NPlugin_id[x] = 19; NPlugin_ptr[x++] = &NPlugin_019;
#endif

#ifdef NPLUGIN_020
  NPlugin_id[x] = 20; NPlugin_ptr[x++] = &NPlugin_020;
#endif

#ifdef NPLUGIN_021
  NPlugin_id[x] = 21; NPlugin_ptr[x++] = &NPlugin_021;
#endif

#ifdef NPLUGIN_022
  NPlugin_id[x] = 22; NPlugin_ptr[x++] = &NPlugin_022;
#endif

#ifdef NPLUGIN_023
  NPlugin_id[x] = 23; NPlugin_ptr[x++] = &NPlugin_023;
#endif

#ifdef NPLUGIN_024
  NPlugin_id[x] = 24; NPlugin_ptr[x++] = &NPlugin_024;
#endif

#ifdef NPLUGIN_025
  NPlugin_id[x] = 25; NPlugin_ptr[x++] = &NPlugin_025;
#endif

  NPluginCall(NPLUGIN_PROTOCOL_ADD, 0);
}

byte NPluginCall(byte Function, struct EventStruct *event)
{
  int x;
  struct EventStruct TempEvent;

 if (event == 0)
    event=&TempEvent;
    
  switch (Function)
  {
    // Unconditional calls to all plugins
    case NPLUGIN_PROTOCOL_ADD:
      for (x = 0; x < NPLUGIN_MAX; x++)
        if (NPlugin_id[x] != 0)
          NPlugin_ptr[x](Function, event, dummyString);
      return true;
      break;
  }

  return false;
}


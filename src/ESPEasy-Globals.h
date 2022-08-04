#ifndef ESPEASY_GLOBALS_H_
#define ESPEASY_GLOBALS_H_



#include "ESPEasy_common.h"


//#include <FS.h>



//enable reporting status to ESPEasy developers.
//this informs us of crashes and stability issues.
// not finished yet!
// #define FEATURE_REPORTING  1

//Select which plugin sets you want to build.
//These are normally automaticly set via the Platformio build environment.
//If you use ArduinoIDE you might need to uncomment some of them, depending on your needs
//If you dont select any, a version with a minimal number of plugins will be biult for 512k versions.
//(512k is NOT finsihed or tested yet as of v2.0.0-dev6)


//build all plugins that still are being developed and are broken or incomplete
//#define PLUGIN_BUILD_DEV

//add this if you want SD support (add 10k flash)
//#define FEATURE_SD 1







/*
// TODO TD-er: Declare global variables as extern and construct them in the .cpp.
// Move all other defines in this file to separate .h files
// This file should only have the "extern" declared global variables so it can be included where they are needed.
//
// For a very good tutorial on how C++ handles global variables, see:
//    https://www.fluentcpp.com/2019/07/23/how-to-define-a-global-constant-in-cpp/
// For more information about the discussion which lead to this big change:
//    https://github.com/letscontrolit/ESPEasy/issues/2621#issuecomment-533673956
*/





/*********************************************************************************************\
 * pinStatesStruct
\*********************************************************************************************/
/*
struct pinStatesStruct
{
  pinStatesStruct() : value(0), plugin(0), index(0), mode(0) {}
  uint16_t value;
  uint8_t plugin;
  uint8_t index;
  uint8_t mode;
} pinStates[PINSTATE_TABLE_MAX];
*/




extern boolean printToWeb;
extern String printWebString;
extern boolean printToWebJSON;


//struct RTC_cache_handler_struct;


// FIXME TD-er: Must move this to some proper class (ESPEasy_Scheduler ?)
extern unsigned long timermqtt_interval;


extern unsigned long lastSend;
extern unsigned long lastWeb;

extern unsigned long wdcounter;
extern unsigned long timerAwakeFromDeepSleep;


#if FEATURE_ADC_VCC
extern float vcc;
#endif



extern bool shouldReboot;
extern bool firstLoop;

// This is read from the settings at boot.
// Even if this setting is changed, you need to reboot to activate the changes.
extern boolean UseRTOSMultitasking;

#endif /* ESPEASY_GLOBALS_H_ */

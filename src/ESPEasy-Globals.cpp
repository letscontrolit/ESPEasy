#include <Arduino.h>

#include "ESPEasy-Globals.h"






boolean printToWeb = false;
String printWebString;
boolean printToWebJSON = false;



unsigned long timermqtt_interval = 250;
unsigned long lastSend = 0;
unsigned long lastWeb = 0;

unsigned long wdcounter = 0;
unsigned long timerAwakeFromDeepSleep = 0;


#if FEATURE_ADC_VCC
float vcc = -1.0f;
#endif

boolean WebLoggedIn = false;
int WebLoggedInTimer = 300;


String dummyString;  // FIXME @TD-er  This may take a lot of memory over time, since long-lived Strings only tend to grow.






bool shouldReboot(false);
bool firstLoop(true);


boolean UseRTOSMultitasking(false);

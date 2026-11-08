#include "ESPEasy-Globals.h"






boolean printToWeb = false;
String printWebString;
boolean printToWebJSON = false;



uint32_t timermqtt_interval = 100;
uint32_t lastSend = 0;
uint32_t lastWeb = 0;

uint32_t wdcounter = 0;
uint32_t timerAwakeFromDeepSleep = 0;


#if FEATURE_ADC_VCC
float vcc = -1.0f;
#endif





bool shouldReboot(false);
bool firstLoop(true);


boolean UseRTOSMultitasking(false);

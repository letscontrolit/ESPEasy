#include "../ControllerQueue/DelayQueueElements.h"

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/TimingStats.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Helpers/PeriodicalActions.h"

#if FEATURE_MQTT
ControllerDelayHandlerStruct *MQTTDelayHandler = nullptr;

bool init_mqtt_delay_queue(controllerIndex_t ControllerIndex, String& pubname, bool& retainFlag) {
  MakeControllerSettings(ControllerSettings); // -V522

  if (!AllocatedControllerSettings()) {
    return false;
  }
  LoadControllerSettings(ControllerIndex, ControllerSettings);

  if (MQTTDelayHandler == nullptr) {
    # ifdef USE_SECOND_HEAP
    HeapSelectIram ephemeral;
    # endif // ifdef USE_SECOND_HEAP

    MQTTDelayHandler = new (std::nothrow) ControllerDelayHandlerStruct;
  }

  if (MQTTDelayHandler == nullptr) {
    return false;
  }
  MQTTDelayHandler->configureControllerSettings(ControllerSettings);
  pubname    = ControllerSettings.Publish;
  retainFlag = ControllerSettings.mqtt_retainFlag();
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT, 10); // Make sure the MQTT is being processed as soon
                                                                                          // as possible.
  scheduleNextMQTTdelayQueue();
  return true;
}

void exit_mqtt_delay_queue() {
  if (MQTTDelayHandler != nullptr) {
    delete MQTTDelayHandler;
    MQTTDelayHandler = nullptr;
  }
}

#endif // if FEATURE_MQTT


/*********************************************************************************************\
* C001_queue_element for queueing requests for C001.
\*********************************************************************************************/
#ifdef USES_C001
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00, 1) // -V522
#endif // ifdef USES_C001

/*********************************************************************************************\
* C003_queue_element for queueing requests for C003 Nodo Telnet.
\*********************************************************************************************/
#ifdef USES_C003
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00, 3) // -V522
#endif // ifdef USES_C003

#ifdef USES_C004
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00, 4) // -V522
#endif // ifdef USES_C004

#ifdef USES_C007
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00, 7) // -V522
#endif // ifdef USES_C007


/*********************************************************************************************\
* C008_queue_element for queueing requests for 008: Generic HTTP
* Using SimpleQueueElement_formatted_Strings
\*********************************************************************************************/
#ifdef USES_C008
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00, 8) // -V522
#endif // ifdef USES_C008

#ifdef USES_C009
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(00, 9) // -V522
#endif // ifdef USES_C009


/*********************************************************************************************\
* C010_queue_element for queueing requests for 010: Generic UDP
* Using SimpleQueueElement_formatted_Strings
\*********************************************************************************************/
#ifdef USES_C010
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 10) // -V522
#endif // ifdef USES_C010


/*********************************************************************************************\
* C011_queue_element for queueing requests for 011: Generic HTTP Advanced
\*********************************************************************************************/
#ifdef USES_C011
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 11) // -V522
#endif // ifdef USES_C011


/*********************************************************************************************\
* C012_queue_element for queueing requests for 012: Blynk
* Using SimpleQueueElement_formatted_Strings
\*********************************************************************************************/
#ifdef USES_C012
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 12) // -V522
#endif // ifdef USES_C012

/*
 #ifdef USES_C013
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 13)  // -V522
 #endif
 */

/*
 #ifdef USES_C014
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 14)  // -V522
 #endif
 */


#ifdef USES_C015
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 15) // -V522
#endif // ifdef USES_C015


#ifdef USES_C016
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 16) // -V522
#endif // ifdef USES_C016


#ifdef USES_C017
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 17) // -V522
#endif // ifdef USES_C017

#ifdef USES_C018
DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 18) // -V522
#endif // ifdef USES_C018


/*
 #ifdef USES_C019
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 19)  // -V522
 #endif
 */

/*
 #ifdef USES_C020
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 20)  // -V522
 #endif
 */

/*
 #ifdef USES_C021
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 21)  // -V522
 #endif
 */

/*
 #ifdef USES_C022
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 22)  // -V522
 #endif
 */

/*
 #ifdef USES_C023
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 23)  // -V522
 #endif
 */

/*
 #ifdef USES_C024
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 24)  // -V522
 #endif
 */

/*
 #ifdef USES_C025
   DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(0, 25)  // -V522
 #endif
 */


// When extending this, search for EXTEND_CONTROLLER_IDS
// in the code to find all places that need to be updated too.

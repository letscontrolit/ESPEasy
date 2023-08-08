#ifndef DELAY_QUEUE_ELEMENTS_H
#define DELAY_QUEUE_ELEMENTS_H


#include "../../ESPEasy_common.h"


#include "../ControllerQueue/ControllerDelayHandlerStruct.h"
#include "../ControllerQueue/Queue_element_base.h"
#include "../DataStructs/ControllerSettingsStruct.h"


// The most logical place to have these queue element handlers defined would be in their
// respective _Cxxx.ino file.
// But the PlatformIO/Arduino build process may then run into issues when compiling.
// Either some of the functions may not be (forward) declared yet when being called from the scheduler code.
// Or the forward declaration of a function may be generated by the pre-processor when expanding the macro to generate them.
// The #ifdef USES_Cxxx check is then no longer present in the generated ESPEasy.ino.cpp file which will lead to build errors
// when not all controllers are included in the build.
//
// To overcome build errors, one MUST forward declare the do_process_cXXX_delay_queue function in the .ino file of the controller itself.
// If someone finds a better way, please let me know.
// See: https://github.com/platformio/platformio-core/issues/2972
//
// N.B. These queue element classes should be defined as class (not a struct), to be used as template.
//




// Uncrustify must not be used on macros, so turn it off.
// Also make sure to wrap the forward declaration of this function in the same wrappers 
// as it may not split the forward declaration into multiple lines.
//
// *INDENT-OFF*



// Define the function wrappers to handle the calling to Cxxx_DelayHandler etc.
// If someone knows how to add leading zeros in macros, please be my guest :)


// This macro defines the code needed to create the 'process_c##NNN####M##_delay_queue()'
// function and all needed objects and forward declarations.
// It is a macro to prevent common typo errors.
// This function will perform the (re)scheduling and mark if it is processed (and can be removed)
// The controller itself must implement the 'do_process_c004_delay_queue' function to actually
// send the data.
// Its return value must state whether it can be marked 'Processed'.
// N.B. some controllers only can send one value per iteration, so a returned "false" can mean it
//      was still successful. The controller should keep track of the last value sent
//      in the element stored in the queue.
#define DEFINE_Cxxx_DELAY_QUEUE_MACRO(NNN, M)                                                                          \
  extern struct ControllerDelayHandlerStruct *C##NNN####M##_DelayHandler;                                              \
  bool do_process_c##NNN####M##_delay_queue(int controller_number, const Queue_element_base & element, ControllerSettingsStruct & ControllerSettings); \
  void process_c##NNN####M##_delay_queue();                                                                            \
  bool init_c##NNN####M##_delay_queue(controllerIndex_t ControllerIndex);                                              \
  void exit_c##NNN####M##_delay_queue();                                                                               \

#define DEFINE_Cxxx_DELAY_QUEUE_MACRO_CPP(NNN, M)                                                                    \
  ControllerDelayHandlerStruct *C##NNN####M##_DelayHandler = nullptr;                                                \
  void process_c##NNN####M##_delay_queue() {                                                                         \
    if (C##NNN####M##_DelayHandler == nullptr) return;                                                               \
    C##NNN####M##_DelayHandler->process(                                                                             \
      M, do_process_c##NNN####M##_delay_queue, TimingStatsElements::C##NNN####M##_DELAY_QUEUE,                                            \
      ESPEasy_Scheduler::IntervalTimer_e::TIMER_C##NNN####M##_DELAY_QUEUE);                                          \
  }                                                                                                                  \
  bool init_c##NNN####M##_delay_queue(controllerIndex_t ControllerIndex) {                                           \
    if (C##NNN####M##_DelayHandler == nullptr) {                                                                     \
      C##NNN####M##_DelayHandler = new (std::nothrow) (ControllerDelayHandlerStruct);                                \
    }                                                                                                                \
    if (C##NNN####M##_DelayHandler == nullptr) { return false; }                                                     \
    return C##NNN####M##_DelayHandler->cacheControllerSettings(ControllerIndex);                                 \
  }                                                                                                                  \
  void exit_c##NNN####M##_delay_queue() {                                                                            \
    if (C##NNN####M##_DelayHandler != nullptr) {                                                                     \
      delete C##NNN####M##_DelayHandler;                                                                             \
      C##NNN####M##_DelayHandler = nullptr;                                                                          \
    }                                                                                                                \
  }                                                                                                                  \




// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*




#if FEATURE_MQTT
# include "../ControllerQueue/MQTT_queue_element.h"
extern struct ControllerDelayHandlerStruct *MQTTDelayHandler;

bool init_mqtt_delay_queue(controllerIndex_t ControllerIndex,
                           String          & pubname,
                           bool            & retainFlag);
void exit_mqtt_delay_queue();
#endif // if FEATURE_MQTT


/*********************************************************************************************\
* C001_queue_element for queueing requests for C001.
\*********************************************************************************************/
#ifdef USES_C001
# include "../ControllerQueue/SimpleQueueElement_string_only.h"
typedef simple_queue_element_string_only C001_queue_element;
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00,  1)
#endif // ifdef USES_C001

/*********************************************************************************************\
* C003_queue_element for queueing requests for C003 Nodo Telnet.
\*********************************************************************************************/
#ifdef USES_C003
# include "../ControllerQueue/SimpleQueueElement_string_only.h"
typedef simple_queue_element_string_only C003_queue_element; 
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00, 3)
#endif // ifdef USES_C003

#ifdef USES_C004
# include "../ControllerQueue/SimpleQueueElement_formatted_Strings.h"
typedef SimpleQueueElement_formatted_Strings C004_queue_element;
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00, 4)
#endif // ifdef USES_C004

#ifdef USES_C007
# include "../ControllerQueue/SimpleQueueElement_formatted_Strings.h"
typedef SimpleQueueElement_formatted_Strings C007_queue_element;
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00, 7)
#endif // ifdef USES_C007


/*********************************************************************************************\
* C008_queue_element for queueing requests for 008: Generic HTTP
* Using SimpleQueueElement_formatted_Strings
\*********************************************************************************************/
#ifdef USES_C008
# include "../ControllerQueue/SimpleQueueElement_formatted_Strings.h"
typedef SimpleQueueElement_formatted_Strings C008_queue_element;
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00, 8)
#endif // ifdef USES_C008

#ifdef USES_C009
# include "../ControllerQueue/SimpleQueueElement_formatted_Strings.h"
typedef SimpleQueueElement_formatted_Strings C009_queue_element;
DEFINE_Cxxx_DELAY_QUEUE_MACRO(00, 9)
#endif // ifdef USES_C009


/*********************************************************************************************\
* C010_queue_element for queueing requests for 010: Generic UDP
* Using SimpleQueueElement_formatted_Strings
\*********************************************************************************************/
#ifdef USES_C010
# include "../ControllerQueue/SimpleQueueElement_formatted_Strings.h"
typedef SimpleQueueElement_formatted_Strings C010_queue_element;
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 10)
#endif // ifdef USES_C010


/*********************************************************************************************\
* C011_queue_element for queueing requests for 011: Generic HTTP Advanced
\*********************************************************************************************/
#ifdef USES_C011
# include "../ControllerQueue/C011_queue_element.h"
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 11)
#endif // ifdef USES_C011


/*********************************************************************************************\
* C012_queue_element for queueing requests for 012: Blynk
* Using SimpleQueueElement_formatted_Strings
\*********************************************************************************************/
#ifdef USES_C012
# include "../ControllerQueue/SimpleQueueElement_formatted_Strings.h"
typedef SimpleQueueElement_formatted_Strings C012_queue_element;
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 12)
#endif // ifdef USES_C012

/*
 #ifdef USES_C013
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 13)
 #endif
 */

/*
 #ifdef USES_C014
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 14)
 #endif
 */


#ifdef USES_C015
# include "../ControllerQueue/C015_queue_element.h"
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 15)
#endif // ifdef USES_C015


#ifdef USES_C016
# include "../ControllerQueue/C016_queue_element.h"
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 16)
#endif // ifdef USES_C016


#ifdef USES_C017
# include "../ControllerQueue/SimpleQueueElement_formatted_Strings.h"
typedef SimpleQueueElement_formatted_Strings C017_queue_element;
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 17)
#endif // ifdef USES_C017

#ifdef USES_C018
# include "../ControllerQueue/C018_queue_element.h"
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 18)
#endif // ifdef USES_C018


#ifdef USES_C019
# include "../ControllerQueue/C019_queue_element.h"
DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 19)
#endif // ifdef USES_C019


/*
 #ifdef USES_C020
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 20)
 #endif
 */

/*
 #ifdef USES_C021
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 21)
 #endif
 */

/*
 #ifdef USES_C022
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 22)
 #endif
 */

/*
 #ifdef USES_C023
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 23)
 #endif
 */

/*
 #ifdef USES_C024
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 24)
 #endif
 */

/*
 #ifdef USES_C025
   DEFINE_Cxxx_DELAY_QUEUE_MACRO(0, 25)
 #endif
 */


// When extending this, search for EXTEND_CONTROLLER_IDS
// in the code to find all places that need to be updated too.


#endif // ifndef DELAY_QUEUE_ELEMENTS_H

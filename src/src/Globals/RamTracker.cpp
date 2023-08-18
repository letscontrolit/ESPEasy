#include "../Globals/RamTracker.h"

#ifndef BUILD_NO_RAM_TRACKER

#if FEATURE_TIMING_STATS
#include "../DataStructs/TimingStats.h"
#endif

#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"

#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/StringConverter.h"

RamTracker myRamTracker;

/********************************************************************************************\
   Global convenience functions calling RamTracker
 \*********************************************************************************************/

void checkRAMtoLog(void){
  myRamTracker.getTraceBuffer();
}

void checkRAM(const String &flashString, int a ) {
  checkRAM(flashString, String(a));
}

void checkRAM(const __FlashStringHelper * flashString, int a) {
  checkRAM(String(flashString), String(a));
}


void checkRAM(const __FlashStringHelper * flashString, const String& a) {
  checkRAM(String(flashString), a);
}

void checkRAM(const __FlashStringHelper * flashString, const __FlashStringHelper * a) {
  checkRAM_values values;
  if (!values.mustContinue()) return;
  String s = flashString;
  s += F(" (");
  s += a;
  s += ')';
  checkRAM(values, s);
}

void checkRAM(const String& flashString, const String &a ) {
  checkRAM_values values;
  if (!values.mustContinue()) return;
  String s = flashString;
  s += F(" (");
  s += a;
  s += ')';
  checkRAM(values, s);
}

void checkRAM(const __FlashStringHelper * descr ) {
  checkRAM_values values;
  if (values.mustContinue()) 
    checkRAM(values, String(descr));
}

void checkRAM_PluginCall_task(uint8_t taskIndex, uint8_t Function) {
  checkRAM_values values;
  if (!values.mustContinue()) return;
  String s = concat(F("PluginCall_task_"), taskIndex + 1);

  s += F(" (");
  #if FEATURE_TIMING_STATS
  s += getPluginFunctionName(Function);
  #else // if FEATURE_TIMING_STATS
  s += String(Function);
  #endif // if FEATURE_TIMING_STATS
  s += ')';
  checkRAM(values, s);
}

void checkRAM(const String& descr ) {
  checkRAM_values values;
  checkRAM(values, descr);
}

checkRAM_values::checkRAM_values() {
  freeStack = getFreeStackWatermark();
#ifdef ESP32
  freeRAM = ESP.getMinFreeHeap();
#else
  freeRAM = FreeMem();
#endif
}

bool checkRAM_values::mustContinue() const {
  return Settings.EnableRAMTracking() || 
         freeStack <= lowestFreeStack ||
         freeRAM <= lowestRAM;
}

void checkRAM(const checkRAM_values & values, const String& descr)
{
  if (Settings.EnableRAMTracking())
    myRamTracker.registerRamState(descr);

  if (values.freeStack <= lowestFreeStack) {
    lowestFreeStack = values.freeStack;
    lowestFreeStackfunction = descr;
  }

  if (values.freeRAM <= lowestRAM)
  {
    lowestRAM = values.freeRAM;
    lowestRAMfunction = std::move(descr);
  }
}

/********************************************************************************************\
   RamTracker class
 \*********************************************************************************************/



// find highest the trace with the largest minimum memory (gets replaced by worse one)
unsigned int RamTracker::bestCaseTrace(void) {
  unsigned int lowestMemoryInTrace      = 0;
  unsigned int lowestMemoryInTraceIndex = 0;

  for (int i = 0; i < TRACES; i++) {
    if (tracesMemory[i] > lowestMemoryInTrace) {
      lowestMemoryInTrace      = tracesMemory[i];
      lowestMemoryInTraceIndex = i;
    }
  }

  // serialPrintln(lowestMemoryInTraceIndex);
  return lowestMemoryInTraceIndex;
}

RamTracker::RamTracker(void) {
  readPtr  = 0;
  writePtr = 0;

  for (int i = 0; i < TRACES; i++) {
    traces[i] = String();
    tracesMemory[i] = 0xffffffff; // init with best case memory values, so they get replaced if memory goes lower
  }

  for (int i = 0; i < TRACEENTRIES; i++) {
    nextAction[i]            = "startup";
    nextActionStartMemory[i] = ESP.getFreeHeap(); // init with best case memory values, so they get replaced if memory goes lower
  }
}

void RamTracker::registerRamState(const String& s) {   // store function
  nextAction[writePtr]            = s;                 // name and mem
  nextActionStartMemory[writePtr] = ESP.getFreeHeap(); // in cyclic buffer.
  int bestCase = bestCaseTrace();                      // find best case memory trace

  if (ESP.getFreeHeap() < tracesMemory[bestCase]) {    // compare to current memory value
    traces[bestCase] = String();
    readPtr          = writePtr + 1;                   // read out buffer, oldest value first

    if (readPtr >= TRACEENTRIES) { 
      readPtr = 0;        // read pointer wrap around
    }
    tracesMemory[bestCase] = ESP.getFreeHeap();        // store new lowest value of that trace

    for (int i = 0; i < TRACEENTRIES; i++) {           // tranfer cyclic buffer strings and mem values to this trace
      traces[bestCase] += nextAction[readPtr];
      traces[bestCase] += F("-> ");
      traces[bestCase] += String(nextActionStartMemory[readPtr]);
      traces[bestCase] += ' ';
      readPtr++;

      if (readPtr >= TRACEENTRIES) { readPtr = 0; // wrap around read pointer
      }
    }
  }
  writePtr++;

  if (writePtr >= TRACEENTRIES) { writePtr = 0; // inc write pointer and wrap around too.
  }
}

 // return giant strings, one line per trace. Add stremToWeb method to avoid large strings.
void RamTracker::getTraceBuffer() {
#ifndef BUILD_NO_DEBUG
  if (Settings.EnableRAMTracking() && loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String retval = F("Memtrace\n");

    for (int i = 0; i < TRACES; i++) {
      retval += String(i);
      retval += F(": lowest: ");
      retval += String(tracesMemory[i]);
      retval += ' ';
      retval += traces[i];
      addLogMove(LOG_LEVEL_DEBUG_DEV, retval);
      retval = String();
    }
  }
#endif // ifndef BUILD_NO_DEBUG
}

#else // BUILD_NO_RAM_TRACKER
/*

void checkRAMtoLog(void) {}

void checkRAM(const String& flashString,
              int           a) {}

void checkRAM(const String& flashString,
              const String& a) {}

void checkRAM(const String& descr) {}
*/

#endif // BUILD_NO_RAM_TRACKER
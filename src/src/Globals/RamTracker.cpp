#include "../Globals/RamTracker.h"

#ifndef BUILD_NO_RAM_TRACKER


#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"

#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"

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

void checkRAM(const __FlashStringHelper * flashString, const String& a) {
  checkRAM(String(flashString), a);
}

void checkRAM(const __FlashStringHelper * flashString, const __FlashStringHelper * a) {
  String s = flashString;
  s += F(" (");
  s += a;
  s += ')';
  checkRAM(s);
}

void checkRAM(const String& flashString, const String &a ) {
  String s = flashString;
  s += F(" (");
  s += a;
  s += ')';
  checkRAM(s);
}

void checkRAM(const __FlashStringHelper * descr ) {
  checkRAM(String(descr));
}

void checkRAM(const String& descr ) {
  if (Settings.EnableRAMTracking())
    myRamTracker.registerRamState(descr);

  const uint32_t freeStack = getFreeStackWatermark();
  if (freeStack <= lowestFreeStack) {
    lowestFreeStack = freeStack;
    lowestFreeStackfunction = descr;
  }

#ifdef ESP32
  const uint32_t freeRAM = ESP.getMinFreeHeap();
#else
  const uint32_t freeRAM = FreeMem();
#endif
  if (freeRAM <= lowestRAM)
  {
    lowestRAM = freeRAM;
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
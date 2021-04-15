#ifndef GLOBALS_RAMTRACKER_H
#define GLOBALS_RAMTRACKER_H


#define TRACES 3        // number of memory traces
#define TRACEENTRIES 15 // entries per trace

#include <Arduino.h>
#include "../../ESPEasy_common.h"

/********************************************************************************************\
   RamTracker class
 \*********************************************************************************************/

 #ifndef BUILD_NO_RAM_TRACKER
class RamTracker {
private:

  String traces[TRACES];                            // trace of latest memory checks
  unsigned int tracesMemory[TRACES];                // lowest memory for that  trace
  unsigned int readPtr, writePtr;                   // pointer to cyclic buffer
  String nextAction[TRACEENTRIES];                  // buffer to record the names of functions before they are transfered to a trace
  unsigned int nextActionStartMemory[TRACEENTRIES]; // memory levels for the functions.

  unsigned int bestCaseTrace(void);

public:

  RamTracker(void);

  void registerRamState(const String& s);

  // return giant strings, one line per trace. Add stremToWeb method to avoid large strings.
  void getTraceBuffer();
};

extern RamTracker myRamTracker; // instantiate class. (is global now)


/********************************************************************************************\
   Global convenience functions calling RamTracker
 \*********************************************************************************************/

#endif // BUILD_NO_RAM_TRACKER


#ifndef BUILD_NO_RAM_TRACKER
void checkRAMtoLog(void);

void checkRAM(const String& flashString,
              int           a);

void checkRAM(const String& flashString,
              const String& a);

void checkRAM(const String& descr);
#endif


#endif // GLOBALS_RAMTRACKER_H

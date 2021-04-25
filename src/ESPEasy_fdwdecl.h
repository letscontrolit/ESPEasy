#ifndef ESPEASY_FWD_DECL_H
#define ESPEASY_FWD_DECL_H

#include "ESPEasy_common.h"

// FIXME TD-er: This header file should only be included from .ino or .cpp files
// This is only needed until the classes that need these can include the appropriate .h files to have these forward declared.


void     backgroundtasks();


void flushAndDisconnectAllClients();


float getCPUload();
int getLoopCountPerSec();
int getUptimeMinutes();




void Blynk_Run_c015();





#endif // ESPEASY_FWD_DECL_H

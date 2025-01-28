#ifndef HELPERS_MEMORY_H
#define HELPERS_MEMORY_H


#include "../../ESPEasy_common.h"

/*********************************************************************************************\
   Memory management
\*********************************************************************************************/


// For keeping track of 'cont' stack
// See: https://github.com/esp8266/Arduino/issues/2557
//      https://github.com/esp8266/Arduino/issues/5148#issuecomment-424329183
//      https://github.com/letscontrolit/ESPEasy/issues/1824
#ifdef ESP32

// FIXME TD-er: For ESP32 you need to provide the task number, or nullptr to get from the calling task.
uint32_t getCurrentFreeStack();

uint32_t getFreeStackWatermark();

#else // ifdef ESP32

extern "C" {
# include <cont.h>
extern cont_t *g_pcont;
}

uint32_t getCurrentFreeStack();

uint32_t getFreeStackWatermark();

bool     allocatedOnStack(const void *address);

#endif // ESP32

/********************************************************************************************\
   Get free system mem
 \*********************************************************************************************/
unsigned long FreeMem();

#ifdef USE_SECOND_HEAP
unsigned long FreeMem2ndHeap();
#endif

unsigned long getMaxFreeBlock();


#endif

/********************************************************************************************\
   Special alloc functions to allocate in PSRAM if available
 \*********************************************************************************************/

void *special_malloc(uint32_t size);
void *special_realloc(void *ptr, size_t size);
void *special_calloc(size_t num, size_t size);


// Perform a special memory allocate for the buffer pointer in the String object
// Allocation is tried either on 2nd heap (ESP8266) or PSRAM (ESP32)
// when possible.
bool String_reserve_special(String& str, size_t size);


#include "../Helpers/Memory.h"


#ifdef ESP8266
extern "C" {
#include <user_interface.h>
}
#endif

#include "../../ESPEasy_common.h"


#ifdef ESP32
#if ESP_IDF_VERSION_MAJOR < 5
#include <soc/cpu.h>
#endif
#endif

#include "../Helpers/Hardware_device_info.h"

/*********************************************************************************************\
   Memory management
\*********************************************************************************************/


// For keeping track of 'cont' stack
// See: https://github.com/esp8266/Arduino/issues/2557
//      https://github.com/esp8266/Arduino/issues/5148#issuecomment-424329183
//      https://github.com/letscontrolit/ESPEasy/issues/1824
#ifdef ESP32

// FIXME TD-er: For ESP32 you need to provide the task number, or nullptr to get from the calling task.
uint32_t getCurrentFreeStack() {
  return ((uint8_t*)esp_cpu_get_sp()) - pxTaskGetStackStart(nullptr);
}

uint32_t getFreeStackWatermark() {
  return uxTaskGetStackHighWaterMark(nullptr);
}

#else // ifdef ESP32

uint32_t getCurrentFreeStack() {
  // https://github.com/esp8266/Arduino/issues/2557
  register uint32_t *sp asm ("a1");

  return 4 * (sp - g_pcont->stack);
}

uint32_t getFreeStackWatermark() {
  return cont_get_free_stack(g_pcont);
}

bool allocatedOnStack(const void *address) {
  register uint32_t *sp asm ("a1");

  if (sp < address) { return false; }
  return g_pcont->stack < address;
}

#endif // ESP32


/********************************************************************************************\
   Get free system mem
 \*********************************************************************************************/
unsigned long FreeMem()
{
  #if defined(ESP8266)
  return system_get_free_heap_size();
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  return ESP.getFreeHeap();
  #endif // if defined(ESP32)
}

#ifdef USE_SECOND_HEAP
unsigned long FreeMem2ndHeap()
{
  HeapSelectIram ephemeral;
  return ESP.getFreeHeap();
}
#endif


unsigned long getMaxFreeBlock()
{
  const unsigned long freemem = FreeMem();
  // computing max free block is a rather extensive operation, so only perform when free memory is already low.
  if (freemem < 6144) {
  #if  defined(ESP32)
    return ESP.getMaxAllocHeap();
  #endif // if  defined(ESP32)
  #ifdef CORE_POST_2_5_0
    return ESP.getMaxFreeBlockSize();
  #endif // ifdef CORE_POST_2_5_0
  }
  return freemem;
}

/********************************************************************************************\
   Special alloc functions to allocate in PSRAM if available
 \*********************************************************************************************/

void *special_malloc(uint32_t size) {
  #ifdef ESP32
  if (UsePSRAM()) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  } else {
    return malloc(size);
  }
  #else
  return malloc(size);
  #endif
}

void *special_realloc(void *ptr, size_t size) {
  #ifdef ESP32
  if (UsePSRAM()) {
    return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  } else {
    return realloc(ptr, size);
  }
  #else
  return realloc(ptr, size);
  #endif
}
void *special_calloc(size_t num, size_t size) {
  #ifdef ESP32
  if (UsePSRAM()) {
    return heap_caps_calloc(num, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  } else {
    return calloc(num, size);
  }
  #else
  return calloc(num, size);
  #endif
}
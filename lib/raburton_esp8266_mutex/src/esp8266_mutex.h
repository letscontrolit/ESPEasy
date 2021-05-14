//////////////////////////////////////////////////
// Mutex support for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// https://github.com/raburton/esp8266/tree/master/mutex
// See license.txt for license terms.
//////////////////////////////////////////////////

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <c_types.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef int32 mutex_t;

void ICACHE_FLASH_ATTR CreateMutux(mutex_t *mutex);
bool ICACHE_FLASH_ATTR GetMutex(mutex_t *mutex);
void ICACHE_FLASH_ATTR ReleaseMutex(mutex_t *mutex);

#ifdef __cplusplus
}
#endif

#endif

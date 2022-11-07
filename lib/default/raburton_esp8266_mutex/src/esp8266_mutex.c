//////////////////////////////////////////////////
// Mutex support for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// https://github.com/raburton/esp8266/tree/master/mutex
// See license.txt for license terms.
//////////////////////////////////////////////////

#include "esp8266_mutex.h"

// setup a new mutex
void ICACHE_FLASH_ATTR CreateMutux(mutex_t *mutex) {
	*mutex = 1;
}

// try to get a mutex
// returns true if successful, false if mutex not free
// as the esp8266 doesn't support the atomic S32C1I instruction
// we have to make the code uninterruptable to produce the
// same overall effect
bool ICACHE_FLASH_ATTR GetMutex(mutex_t *mutex) {

	int iOld = 1, iNew = 0;

	asm volatile (
		"rsil a15, 1\n"    // read and set interrupt level to 1
		"l32i %0, %1, 0\n" // load value of mutex
		"bne %0, %2, 1f\n" // compare with iOld, branch if not equal
		"s32i %3, %1, 0\n" // store iNew in mutex
		"1:\n"             // branch target
		"wsr.ps a15\n"     // restore program state
		"rsync\n"
		: "=&r" (iOld)
		: "r" (mutex), "r" (iOld), "r" (iNew)
		: "a15", "memory"
	);

	return (bool)iOld;
}

// release a mutex
void ICACHE_FLASH_ATTR ReleaseMutex(mutex_t *mutex) {
	*mutex = 1;
}

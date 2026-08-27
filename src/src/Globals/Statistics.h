#ifndef GLOBALS_STATISTICS_H
#define GLOBALS_STATISTICS_H

#include <stdint.h>

#include "../../ESPEasy_common.h"

#include "../DataStructs/I2CTypes.h"

#include "../DataStructs/SchedulerTimerID.h"


class String;


#define BOOT_CAUSE_MANUAL_REBOOT            0
#define BOOT_CAUSE_COLD_BOOT                1
#define BOOT_CAUSE_DEEP_SLEEP               2
#define BOOT_CAUSE_SOFT_RESTART             3
#define BOOT_CAUSE_EXT_WD                  10
#define BOOT_CAUSE_SW_WATCHDOG             11
#define BOOT_CAUSE_EXCEPTION               12
#define BOOT_CAUSE_POWER_UNSTABLE          20

#ifndef BUILD_NO_RAM_TRACKER
extern uint32_t lowestRAM;
extern String   lowestRAMfunction;
extern uint32_t lowestFreeStack;
extern String   lowestFreeStackfunction;
#endif

extern uint8_t lastBootCause;
extern SchedulerTimerID lastMixedSchedulerId_beforereboot;

extern uint32_t loopCounter;
extern uint32_t loopCounterLast;
extern uint32_t loopCounterMax;
extern uint32_t lastLoopStart;
extern uint32_t shortestLoop;
extern uint32_t longestLoop;
extern uint32_t loopCounter_full;
extern float loop_usec_duration_total;

extern uint32_t dailyResetCounter;
extern ESPEASY_VOLATILE(uint32_t) sw_watchdog_callback_count;

#if FEATURE_CLEAR_I2C_STUCK
extern I2C_bus_state I2C_state;
extern uint32_t I2C_bus_cleared_count;
#endif

#endif // GLOBALS_STATISTICS_H

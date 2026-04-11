#include "../Globals/Statistics.h"


#ifndef BUILD_NO_RAM_TRACKER
uint32_t lowestRAM = 0;
String   lowestRAMfunction;
uint32_t lowestFreeStack = 0;
String   lowestFreeStackfunction;
#endif

uint8_t lastBootCause                           = BOOT_CAUSE_MANUAL_REBOOT;
SchedulerTimerID lastMixedSchedulerId_beforereboot(0);

uint32_t idle_msec_per_sec = 0;
uint32_t elapsed10ps       = 0;
uint32_t elapsed10psU      = 0;
uint32_t elapsed50ps       = 0;
uint32_t loopCounter       = 0;
uint32_t loopCounterLast   = 0;
uint32_t loopCounterMax    = 1;
uint32_t lastLoopStart     = 0;
uint32_t shortestLoop      = 10000000;
uint32_t longestLoop       = 0;
uint32_t loopCounter_full  = 1;
float loop_usec_duration_total  = 0.0f;


uint32_t dailyResetCounter                   = 0;

ESPEASY_VOLATILE(uint32_t) sw_watchdog_callback_count{};

#if FEATURE_CLEAR_I2C_STUCK
I2C_bus_state I2C_state = I2C_bus_state::OK;
uint32_t I2C_bus_cleared_count = 0;
#endif
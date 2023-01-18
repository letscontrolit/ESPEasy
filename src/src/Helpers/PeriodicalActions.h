#ifndef HELPERS_PERIODICALACTIONS_H
#define HELPERS_PERIODICALACTIONS_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/MessageRouteInfo.h"
#include "../Globals/CPlugins.h"
#include "../Helpers/Scheduler.h"

/*********************************************************************************************\
 * Tasks that run 50 times per second
\*********************************************************************************************/

void run50TimesPerSecond();

/*********************************************************************************************\
 * Tasks that run 10 times per second
\*********************************************************************************************/
void run10TimesPerSecond();


/*********************************************************************************************\
 * Tasks each second
\*********************************************************************************************/
void runOncePerSecond();

/*********************************************************************************************\
 * Tasks each 30 seconds
\*********************************************************************************************/
void runEach30Seconds();

#if FEATURE_MQTT

void scheduleNextMQTTdelayQueue();
void schedule_all_MQTTimport_tasks();

void processMQTTdelayQueue();

bool processMQTT_message(controllerIndex_t controllerIndex,
                        const String    & topic,
                        const String    & payload,
                        bool retained
#ifdef USES_ESPEASY_NOW
                        , const MessageRouteInfo_t* messageRouteInfo
#endif
                        );

void updateMQTTclient_connected();

void runPeriodicalMQTT();

#endif //if FEATURE_MQTT


void logTimerStatistics();

void updateLoopStats_30sec(uint8_t loglevel);

/********************************************************************************************\
   Clean up all before going to sleep or reboot.
 \*********************************************************************************************/
void prepareShutdown(ESPEasy_Scheduler::IntendedRebootReason_e reason);



#endif // HELPERS_PERIODICALACTIONS_H
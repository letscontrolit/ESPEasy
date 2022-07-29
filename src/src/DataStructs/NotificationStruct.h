#ifndef DATASTRUCTS_NOTIFICATIONSTRUCT_H
#define DATASTRUCTS_NOTIFICATIONSTRUCT_H

#include "../../ESPEasy_common.h"

#if FEATURE_NOTIFIER

/*********************************************************************************************\
* NotificationStruct
\*********************************************************************************************/
struct NotificationStruct
{
  NotificationStruct() :
    Number(0), usesGPIO(0), usesMessaging(false) {}

  uint8_t Number;
  uint8_t usesGPIO;
  bool usesMessaging;
};

#endif


#endif // DATASTRUCTS_NOTIFICATIONSTRUCT_H

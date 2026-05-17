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
    Number(0), usesGPIO(0) {}

  uint8_t Number;
  uint8_t usesGPIO;
  union {
    struct {
      uint8_t usesMessaging : 1;
      uint8_t usesTLS       : 1;
      uint8_t unusedN02     : 1; // unused
      uint8_t unusedN03     : 1; // unused
      uint8_t unusedN04     : 1; // unused
      uint8_t unusedN05     : 1; // unused
      uint8_t unusedN06     : 1; // unused
      uint8_t unusedN07     : 1; // unused

    };

    uint8_t _notificationBits0{};

  };

};

#endif // if FEATURE_NOTIFIER


#endif // DATASTRUCTS_NOTIFICATIONSTRUCT_H

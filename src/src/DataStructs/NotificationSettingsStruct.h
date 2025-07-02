#ifndef DATASTRUCTS_NOTIFICATIONSETTINGSSTRUCT_H
#define DATASTRUCTS_NOTIFICATIONSETTINGSSTRUCT_H

#include "../../ESPEasy_common.h"

#if FEATURE_NOTIFIER

#include <memory> // For std::shared_ptr

# define NPLUGIN_001_DEF_TM     8000  // Email Server Default Response Time, in mS.
# define NPLUGIN_001_MIN_TM     5000
# define NPLUGIN_001_MAX_TM     20000

/*********************************************************************************************\
 * NotificationSettingsStruct
\*********************************************************************************************/
struct NotificationSettingsStruct
{
  NotificationSettingsStruct();

  void validate();

  char          Server[65];
  unsigned int  Port;
  char          Domain[65];
  char          Sender[65];
  char          Receiver[65];
  char          Subject[129];
  char          Body[513];
  int8_t        Pin1;
  int8_t        Pin2;
  char          User[49];
  char          Pass[33];
  unsigned int  Timeout_ms;
  //its safe to extend this struct, up to 4096 bytes, default values in config are 0
};

typedef std::shared_ptr<NotificationSettingsStruct> NotificationSettingsStruct_ptr_type;

#define MakeNotificationSettings(T) void * calloc_ptr = special_calloc(1,sizeof(NotificationSettingsStruct)); NotificationSettingsStruct_ptr_type T(new (calloc_ptr)  NotificationSettingsStruct());

// Check to see if MakeNotificationSettings was successful
#define AllocatedNotificationSettings() (NotificationSettings.get() != nullptr)


// Need to make sure every byte between the members is also zero
// Otherwise the checksum will fail and settings will be saved too often.
// The memset above is just for this.


#endif
#endif // DATASTRUCTS_NOTIFICATIONSETTINGSSTRUCT_H
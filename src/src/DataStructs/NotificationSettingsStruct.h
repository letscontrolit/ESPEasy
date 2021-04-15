#ifndef DATASTRUCTS_NOTIFICATIONSETTINGSSTRUCT_H
#define DATASTRUCTS_NOTIFICATIONSETTINGSSTRUCT_H

#include <Arduino.h>
#include <memory> // For std::shared_ptr

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
  byte          Pin1;
  byte          Pin2;
  char          User[49];
  char          Pass[33];
  //its safe to extend this struct, up to 4096 bytes, default values in config are 0
};

typedef std::shared_ptr<NotificationSettingsStruct> NotificationSettingsStruct_ptr_type;
#define MakeNotificationSettings(T) NotificationSettingsStruct_ptr_type NotificationSettingsStruct_ptr(new (std::nothrow)  NotificationSettingsStruct());\
                                    NotificationSettingsStruct& T = *NotificationSettingsStruct_ptr;

#endif // DATASTRUCTS_NOTIFICATIONSETTINGSSTRUCT_H
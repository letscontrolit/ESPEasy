#ifndef DATASTRUCTS_NOTIFICATIONSTRUCT_H
#define DATASTRUCTS_NOTIFICATIONSTRUCT_H


/*********************************************************************************************\
* NotificationStruct
\*********************************************************************************************/
struct NotificationStruct
{
  NotificationStruct() :
    Number(0), usesMessaging(false), usesGPIO(0) {}

  byte    Number;
  boolean usesMessaging;
  byte    usesGPIO;
};


#endif // DATASTRUCTS_NOTIFICATIONSTRUCT_H

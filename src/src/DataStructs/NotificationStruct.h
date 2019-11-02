#ifndef DATASTRUCTS_NOTIFICATIONSTRUCT_H
#define DATASTRUCTS_NOTIFICATIONSTRUCT_H


/*********************************************************************************************\
* NotificationStruct
\*********************************************************************************************/
struct NotificationStruct
{
  NotificationStruct() :
    Number(0), usesGPIO(0), usesMessaging(false) {}

  byte Number;
  byte usesGPIO;
  bool usesMessaging;
};


#endif // DATASTRUCTS_NOTIFICATIONSTRUCT_H

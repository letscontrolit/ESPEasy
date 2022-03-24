#include "ESPEasy_common.h"

#ifdef USES_N002
//#######################################################################################################
//########################### Notification Plugin 002: Buzzer ###########################################
//#######################################################################################################

#define NPLUGIN_002
#define NPLUGIN_ID_002         2
#define NPLUGIN_NAME_002       "Buzzer"

#include "src/DataStructs/ESPEasy_EventStruct.h"
#include "src/DataStructs/NotificationSettingsStruct.h"
#include "src/Globals/NPlugins.h"
#include "src/Helpers/Audio.h"
#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/_NPlugin_init.h"


bool NPlugin_002(NPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case NPlugin::Function::NPLUGIN_PROTOCOL_ADD:
      {
        Notification[++notificationCount].Number = NPLUGIN_ID_002;
        Notification[notificationCount].usesMessaging = false;
        Notification[notificationCount].usesGPIO=1;
        break;
      }

    case NPlugin::Function::NPLUGIN_GET_DEVICENAME:
      {
        string = F(NPLUGIN_NAME_002);
        break;
      }

    // Edwin: Not used/not implemented, so disabled for now.
    // case NPlugin::Function::NPLUGIN_WRITE:
    //   {
    //     String log = "";
    //     String command = parseString(string, 1);
    //
    //     if (command == F("buzzer"))
    //     {
    //       MakeNotificationSettings(NotificationSettings);
    //       LoadNotificationSettings(event->NotificationIndex, (uint8_t*)&NotificationSettings, sizeof(NotificationSettingsStruct));
    //       success = true;
    //     }
    //     break;
    //   }

    case NPlugin::Function::NPLUGIN_NOTIFY:
      {
        MakeNotificationSettings(NotificationSettings);
        LoadNotificationSettings(event->NotificationIndex, (uint8_t*)&NotificationSettings, sizeof(NotificationSettingsStruct));
        NotificationSettings.validate();
        //this reserves IRAM and uninitialized RAM
        tone_espEasy(NotificationSettings.Pin1, 440, 500);
        success = true;
      }

    default:
      break;

  }
  return success;
}
#endif

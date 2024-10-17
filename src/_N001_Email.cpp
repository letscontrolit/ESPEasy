#include "ESPEasy_common.h"

#ifdef USES_N001

// #######################################################################################################
// ########################### Notification Plugin 001: Email ############################################
// #######################################################################################################

/** Changelog:
 * 2024-07-01 ThomasB   : Modified to support new email server protocol used by some ISP hosting providers
 *                        Add support for (also) supplying an alternate email address via the notify command
 *                        Now uses Plugin's new Timeout Setting for SMTP server response.
 *                        Can now use substitute email address(s), provided within Notify command rule.
 * 2024-04-06 tonhuisman: Add support for (also) supplying a custom subject when sending an email notification via the notify command
 *                        Log reply from mailserver at DEBUG level
 *                        Code improvements
 * 2022-12-29 tonhuisman: Add Date: field to email header to reduce spam score, see https://github.com/letscontrolit/ESPEasy/issues/3865
 * 2022-12-29 tonhuisman: Start changelog
 */

# define NPLUGIN_001
# define NPLUGIN_ID_001         1
# define NPLUGIN_NAME_001       "Email (SMTP)"

# define NPLUGIN_001_PKT_SZ     256

# include "src/NotifierStructs/N001_data_struct.h"


// The message body is included in event->String1
bool NPlugin_001(NPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function) {
    case NPlugin::Function::NPLUGIN_PROTOCOL_ADD:
    {
      Notification[++notificationCount].Number      = NPLUGIN_ID_001;
      Notification[notificationCount].usesMessaging = true;
      Notification[notificationCount].usesGPIO      = 0;
      break;
    }

    case NPlugin::Function::NPLUGIN_GET_DEVICENAME:
    {
      string = F(NPLUGIN_NAME_001);
      break;
    }

    // Edwin: NPlugin::Function::NPLUGIN_WRITE seems to be not implemented/not used yet? Disabled because its confusing now.
    // case NPlugin::Function::NPLUGIN_WRITE:
    //   {
    //     String log;
    //     String command = parseString(string, 1);
    //
    //     if (command == F("email"))
    //     {
    //       MakeNotificationSettings(NotificationSettings);
    //       LoadNotificationSettings(event->NotificationIndex, (uint8_t*)&NotificationSettings, sizeof(NotificationSettingsStruct));
    //       NPlugin_001_send(NotificationSettings.Domain, NotificationSettings.Receiver, NotificationSettings.Sender,
    // NotificationSettings.Subject, NotificationSettings.Body, NotificationSettings.Server, NotificationSettings.Port);
    //       success = true;
    //     }
    //     break;
    //   }

    case NPlugin::Function::NPLUGIN_NOTIFY:
    {
      MakeNotificationSettings(NotificationSettings);
      LoadNotificationSettings(event->NotificationIndex, (uint8_t *)&NotificationSettings, sizeof(NotificationSettingsStruct));
      NotificationSettings.validate();

      String subject = NotificationSettings.Subject;
      String body    = NotificationSettings.Body;

      if (!event->String1.isEmpty()) {
        body = event->String1;
      }

      if (!event->String2.isEmpty()) {
        subject = event->String2;
      }
      NPlugin_001_send(NotificationSettings, subject, body);
      success = true;
      break;
    }

    default:
      break;
  }
  return success;
}

#endif // ifdef USES_N001

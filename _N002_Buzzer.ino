//#######################################################################################################
//########################### Notification Plugin 002: Buzzer ###########################################
//#######################################################################################################

#define NPLUGIN_002
#define NPLUGIN_ID_002         2
#define NPLUGIN_NAME_002       "Buzzer"

boolean NPlugin_002(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case NPLUGIN_PROTOCOL_ADD:
      {
        Notification[++notificationCount].Number = NPLUGIN_ID_002;
        Notification[notificationCount].usesMessaging = false;
        Notification[notificationCount].usesGPIO=1;
        break;
      }

    case NPLUGIN_GET_DEVICENAME:
      {
        string = F(NPLUGIN_NAME_002);
        break;
      }

    case NPLUGIN_WRITE:
      {
        String log = "";
        String command = parseString(string, 1);

        if (command == F("buzzer"))
        {
          NotificationSettingsStruct NotificationSettings;
          LoadNotificationSettings(event->NotificationIndex, (byte*)&NotificationSettings, sizeof(NotificationSettings));
          success = true;
        }
        break;
      }

    case NPLUGIN_NOTIFY:
      {
        NotificationSettingsStruct NotificationSettings;
        LoadNotificationSettings(event->NotificationIndex, (byte*)&NotificationSettings, sizeof(NotificationSettings));
        NPlugin_002_tone(NotificationSettings.Pin1, 500, 500);
        success = true;
      }

  }
  return success;
}

void NPlugin_002_tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {

analogWriteFreq(frequency);
analogWrite(_pin,100);
delay(duration);
analogWrite(_pin,0);
} 


#ifdef USES_C018
//#######################################################################################################
//########################### Controller Plugin 018: TTN - RN2483/RN2903 ################################
//#######################################################################################################

#define CPLUGIN_018
#define CPLUGIN_ID_018         18
#define CPLUGIN_NAME_018       "TTN - RN2483/RN2903"


#include <rn2xx3.h>
#include <ESPeasySerial.h>

       ESPeasySerial myeasySerial(12, 14);
       rn2xx3 myLora(myeasySerial);


bool CPlugin_018(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_018;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_018);
        break;
      }

    case CPLUGIN_INIT:
      {
        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        C018_DelayHandler.configureControllerSettings(ControllerSettings);

        myeasySerial.begin(9600);
        myLora.autobaud();

        String AppEUI = SecuritySettings.ControllerUser[event->ControllerIndex];
        String AppKey = SecuritySettings.ControllerPassword[event->ControllerIndex];

        myLora.initOTAA(AppEUI, AppKey);

        myLora.txUncnf("ESPeasy (TTN)");


        break;
      }

    case CPLUGIN_WEBFORM_LOAD:
    {
        myeasySerial.begin(9600);
        myLora.autobaud();

        addRowLabel(F("OTAA DevEUI"));
        addHtml(String(myLora.hweui()));
        addRowLabel(F("Version number"));
        addHtml(String(myLora.sysver()));

    }
    break;
    case CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
      {
        success = true;
        switch (event->idx) {
          case CONTROLLER_USER:
            string = F("AppEUI");
            break;
          case CONTROLLER_PASS:
            string = F("AppKey");
            break;
          default:
            success = false;
            break;
        }
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        success = C018_DelayHandler.addToQueue(C018_queue_element(event, VARS_PER_TASK));
        scheduleNextDelayQueue(TIMER_C018_DELAY_QUEUE, C018_DelayHandler.getNextScheduleTime());

        break;
      }

    case CPLUGIN_FLUSH:
      {
        process_c018_delay_queue();
        delay(0);
        break;
      }

  }
  return success;
}

bool do_process_c018_delay_queue(int controller_number, const C018_queue_element& element, ControllerSettingsStruct& ControllerSettings) {

  return true;
}
#endif

//#######################################################################################################
//#################################### Plugin 144: RC-Switch TX #########################################
//#######################################################################################################
// written by Jochen Krapf (jk@nerd2nerd.org)

// List of commands:
// (1) RC,<param>
// (2) RC,<param>,<param>,<param>

// List of RC params:
// (a) SEND=<binary_code>
//     Send binary code with any length ("RC,SEND=000000000001010100010001")
// (b) SEND=<tristate_code>
//     Send tristate code with any length ("RC,SEND=00000FFF0F0F")
// (c) SENDDEC=<decimal_code>
//     Send 24 bit decimal code ("RC,SENDDEC=5393")
// (d) ON=<binary_code>
//     Send binary code for simple 10 DIP switch devices ("RC,ON=1010100010")
// (e) ON=<1..4><1..4>
//     Send switch position for simple 2 rotary switch devices ("RC,ON=42")
// (f) ON=<a..f><1..4><1..4>
//     Send switch position for Intertechno devices ("RC,ON=a42")
// (f) OFF=   as ON...
// (g) PROTOCOL=<number>
//     Set protocoln for devices ("RC,PROTOCOL=2") default=1
// (h) PULSE=<number>
//     Set pulse length ("RC,PULSE=320") default=320
// (i) REPEAT=<number>
//     Set number of transmission repeats ("RC,REPEAT=15") default=?
//
// Combinations:
// e.g. "RC,PROTOCOL=2,PULSE=320,REPEAT=15,SEND=000000000001010100010001"


#include <RCSwitch.h>   //https://github.com/sui77/rc-switch.git

static RCSwitch Plugin_144_RC = RCSwitch();

#define PLUGIN_144
#define PLUGIN_ID_144         144
#define PLUGIN_NAME_144       "RC-Switch TX"


boolean Plugin_144(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_144;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_144);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        int pin = Settings.TaskDevicePin1[event->TaskIndex];
        if (pin >= 0)
        {
          String log = F("RC-Sw: Pin ");
          log += pin;
          log += F(" ");

          Plugin_144_RC.enableTransmit(pin);

          addLog(LOG_LEVEL_INFO, log);
        }


        if (Settings.TaskDevicePin1[event->TaskIndex] >= 0)
          pinMode(Settings.TaskDevicePin1[event->TaskIndex], OUTPUT);
        if (Settings.TaskDevicePin2[event->TaskIndex] >= 0)
          pinMode(Settings.TaskDevicePin2[event->TaskIndex], OUTPUT);
        if (Settings.TaskDevicePin3[event->TaskIndex] >= 0)
          pinMode(Settings.TaskDevicePin3[event->TaskIndex], OUTPUT);

        for (byte i=0; i<3; i++)
          if (Settings.TaskDevicePin[i][event->TaskIndex] >= 0)
            pinMode(Settings.TaskDevicePin[i][event->TaskIndex], OUTPUT);

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);

        if (command == F("rc"))
        {
          String param;
          byte paramIdx = 2;

          string.replace("  ", " ");
          string.replace(" =", "=");
          string.replace("= ", "=");

          param = parseString(string, paramIdx++);
          while (param.length())
          {
            addLog(LOG_LEVEL_DEBUG_MORE, param);

            int index = param.indexOf('=');
            if (index > 0)
            {
              String paramKey = param.substring(0, index);
              String paramVal = param.substring(index+1);
              paramKey.toUpperCase();

              addLog(LOG_LEVEL_DEBUG_MORE, paramKey);
              addLog(LOG_LEVEL_DEBUG_MORE, paramVal);

              if (paramKey == F("SEND"))
              {
                if (paramVal.indexOf("F") >= 0)
                  Plugin_144_RC.sendTriState(&(paramVal[0]));
                else
                  Plugin_144_RC.send(&(paramVal[0]));
              }
              if (paramKey == F("SENDDEC"))
              {
                Plugin_144_RC.send(paramVal.toInt(), 24);
              }
              if (paramKey == F("ON"))
              {
                if (paramVal.length()==10)   //simple 10 DIP switch
                  Plugin_144_RC.switchOn(&(paramVal.substring(0, 5)[0]), &(paramVal.substring(5)[0]));
                else if (paramVal.length()==2)   //2x rotary switch 1..4
                  Plugin_144_RC.switchOn(paramVal[0]-'0', paramVal[1]-'0');
                else if (paramVal.length()==3)   //Intertechno outlets
                  Plugin_144_RC.switchOn(paramVal[0], paramVal[1]-'0', paramVal[2]-'0');
              }
              if (paramKey == F("OFF"))
              {
                if (paramVal.length()==10)   //simple 10 DIP switch
                  Plugin_144_RC.switchOff(&(paramVal.substring(0, 5)[0]), &(paramVal.substring(5)[0]));
                else if (paramVal.length()==2)   //2x rotary switch 1..4
                  Plugin_144_RC.switchOff(paramVal[0]-'0', paramVal[1]-'0');
                else if (paramVal.length()==3)   //Intertechno outlets
                  Plugin_144_RC.switchOn(paramVal[0], paramVal[1]-'0', paramVal[2]-'0');
              }
              if (paramKey == F("PROTOCOL"))
              {
                  Plugin_144_RC.setProtocol(paramVal.toInt());
              }
              if (paramKey == F("PULSE"))
              {
                  Plugin_144_RC.setPulseLength(paramVal.toInt());
              }
              if (paramKey == F("REPEAT"))
              {
                  Plugin_144_RC.setRepeatTransmit(paramVal.toInt());
              }
            }

            param = parseString(string, paramIdx++);
          }

          success = true;
        }

        break;
      }

    case PLUGIN_READ:
      {
        //no values
        success = true;
        break;
      }

  }
  return success;
}

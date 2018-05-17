//#######################################################################################################
//################################### Plugin 075: Nextion <info@sensorio.cz>  ###########################
//################################### Created on the work of  majklovec       ###########################
//#######################################################################################################
#ifdef USES_P075

#include <ESPeasySoftwareSerial.h>

#define PLUGIN_075
#define PLUGIN_ID_075 75
#define PLUGIN_NAME_075 "Display - Nextion [TESTING]"
#define PLUGIN_VALUENAME1_075 "code"

unsigned long Plugin_075_code = 0;
int8_t Plugin_075_RXpin = -1;
int8_t Plugin_075_TXpin = -1;

#define Nlines 12        // The number of different lines which can be displayed - each line is 32 chars max

ESPeasySoftwareSerial *nextion = NULL;

boolean Plugin_075(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number = PLUGIN_ID_075;

      Device[deviceCount].Type = DEVICE_TYPE_DUAL;
      Device[deviceCount].VType = SENSOR_TYPE_DUAL;
      Device[deviceCount].Ports = 0;
      Device[deviceCount].PullUpOption = true;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption = false;
      Device[deviceCount].ValueCount = 2;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption = true;
      Device[deviceCount].GlobalSyncOption = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_075);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0],PSTR(PLUGIN_VALUENAME1_075));
      break;
    }

    case PLUGIN_TEN_PER_SECOND: {
      char __buffer[80];

      uint16_t i;
      uint8_t c;
      String Vidx;
      String Nvalue;
      String Svalue;
      String Nswitch;

      while (nextion->available() > 0) {
        delay(10);
        c = nextion->read();

        if (0x65 == c) {
          if (nextion->available() >= 6) {
            __buffer[0] = c;
            for (i = 1; i < 7; i++) {
              __buffer[i] = nextion->read();
            }

            __buffer[i] = 0x00;

            if (0xFF == __buffer[4] && 0xFF == __buffer[5] && 0xFF == __buffer[6]) {
              Plugin_075_code = ((__buffer[1] >> 8) & __buffer[2] >> 8) & __buffer[3];
              UserVar[event->BaseVarIndex] = __buffer[1] * 256 + __buffer[2];
              UserVar[event->BaseVarIndex + 1] = __buffer[3];
              String log = F("Nextion : code: ");
              log += __buffer[1];
              log += ",";
              log += __buffer[2];
              log += ",";
              log += __buffer[3];

              addLog(LOG_LEVEL_INFO, log);
              sendData(event);
            }
          }
        } else {

          if (c == '|') {
            i = 1;
            __buffer[0] = c;
            c=0;
            while (nextion->available() > 0) {
              __buffer[i] = nextion->read();
              if (__buffer[i]==0x0d) break;
              i++;
            }
            __buffer[i] = 0x00;

            String tmpString = __buffer;

            int argIndex = tmpString.indexOf(F(",i"));
            int argEnd = tmpString.indexOf(',', argIndex + 1);
            if (argIndex)
            Vidx = tmpString.substring(argIndex + 2,argEnd);

            switch (__buffer[1]){

              case 'u':

                argIndex = argEnd;
                argEnd = tmpString.indexOf(',',argIndex + 1);
                if (argIndex)
                  Nvalue = tmpString.substring(argIndex + 2,argEnd);

                argIndex = argEnd;
                argEnd = tmpString.indexOf(0x0a);
                if (argIndex)
                  Svalue = tmpString.substring(argIndex + 2,argEnd);

                break;

              case 's':

                argIndex = argEnd;
                argEnd = tmpString.indexOf(0x0a);
                if (argIndex)
                  Nvalue = tmpString.substring(argIndex + 2,argEnd);
                if (Nvalue == F("On"))
                  Svalue='1';
                if (Nvalue == F("Off"))
                  Svalue='0';

                break;

            }

            UserVar[event->BaseVarIndex] = Vidx.toFloat();
            UserVar[event->BaseVarIndex+1] = Svalue.toFloat();

            String log = F("Nextion : send command: ");
            log += __buffer;
            log += UserVar[event->BaseVarIndex];
            addLog(LOG_LEVEL_INFO, log);
            sendData(event);

            ExecuteCommand(VALUE_SOURCE_SYSTEM, __buffer);
          }
        }
      }

      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND: {
        success = true;
        break;
      }


    case PLUGIN_WEBFORM_SAVE: {

        String argName;

        char deviceTemplate[Nlines][64];
        for (byte varNr = 0; varNr < Nlines; varNr++)
        {
          String arg = F("Plugin_075_template");
          arg += varNr + 1;
          String tmpString = WebServer.arg(arg);
          strncpy(deviceTemplate[varNr], tmpString.c_str(), sizeof(deviceTemplate[varNr])-1);
          deviceTemplate[varNr][63]=0;

//          strncpy(deviceTemplate[varNr], WebServer.arg(argName).c_str(), sizeof(deviceTemplate[varNr]));
        }

        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      char deviceTemplate[Nlines][64];
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

      for (byte varNr = 0; varNr < Nlines; varNr++)
      {
        addFormTextBox(String(F("Line ")) + (varNr + 1), String(F("Plugin_075_template")) + (varNr + 1), deviceTemplate[varNr], 64);
      }

      success = true;
      break;
    }

    case PLUGIN_INIT: {

      LoadTaskSettings(event->TaskIndex);

      if (Settings.TaskDevicePin1[event->TaskIndex] != -1)
        {
          Plugin_075_RXpin = Settings.TaskDevicePin1[event->TaskIndex];
        }

      if (Settings.TaskDevicePin2[event->TaskIndex] != -1)
        {
          Plugin_075_TXpin = Settings.TaskDevicePin2[event->TaskIndex];
        }

      if (!nextion) {
        nextion = new ESPeasySoftwareSerial(Plugin_075_RXpin , Plugin_075_TXpin);
      }

      nextion->begin(9600);

      success = true;
      break;
    }

    case PLUGIN_EXIT:
      {
          if (nextion)
          {
            delete nextion;
            nextion=NULL;
          }
          break;
      }

    case PLUGIN_READ: {
        char deviceTemplate[Nlines][64];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        String newString;

        for (byte x = 0; x < Nlines; x++) {
          String tmpString = deviceTemplate[x];
          if (tmpString.length())
          {
            int rssiIndex = tmpString.indexOf(F("rssi"));
            if(rssiIndex >= 0)
            {
              int barVal;
              newString = tmpString.substring(0, rssiIndex);
              int nbars = WiFi.RSSI();
              if (nbars < -100)
                 barVal=0;
              else if (nbars >= -100 and nbars < -90)
                 barVal=20;
              else if (nbars >= -90 and nbars < -80)
                 barVal=40;
              else if (nbars >= -80 and nbars < -70)
                 barVal=60;
              else if (nbars >= -70 and nbars < -60)
                 barVal=80;
              else if (nbars >= -60)
                 barVal=100;

              newString += String(barVal,DEC);
            }
            else
            {
              newString = parseTemplate(tmpString, 0);
            }

            sendCommand(newString.c_str());
          }
        }

        success = false;
        break;
      }


    case PLUGIN_WRITE: {
      String tmpString = string;
      int argIndex = tmpString.indexOf(',');
      if (argIndex)
        tmpString = tmpString.substring(0, argIndex);
      if (tmpString.equalsIgnoreCase(F("NEXTION"))) {
        argIndex = string.indexOf(',');
        tmpString = string.substring(argIndex + 1);

        sendCommand(tmpString.c_str());

        Serial.println(tmpString);
        success = true;
      }
      break;
    }
  }
  return success;
}


void sendCommand(const char *cmd) {
  nextion->print(cmd);
  nextion->write(0xff);
  nextion->write(0xff);
  nextion->write(0xff);
}

#endif // USES_P075

#include "_Plugin_Helper.h"
#ifdef USES_P071
//#######################################################################################################
//############################# Plugin 071: Kamstrup Multical 401 #######################################
//#######################################################################################################

//IR RX/TX sensor based on http://wiki.hal9k.dk/projects/kamstrup
//schematic http://wiki.hal9k.dk/_media/projects/kamstrup/schematic.pdf
//software based on http://elektronikforumet.com/forum/viewtopic.php?f=2&t=79853

//Device pin 1 = RX
//Device pin 2 = TX


#include <ESPeasySerial.h>


#define PLUGIN_071
#define PLUGIN_ID_071 71
#define PLUGIN_NAME_071 "Communication - Kamstrup Multical 401 [TESTING]"
#define PLUGIN_VALUENAME1_071 "Heat"
#define PLUGIN_VALUENAME2_071 "Volume"

boolean Plugin_071_init = false;
byte PIN_KAMSER_RX = 0;
byte PIN_KAMSER_TX = 0;

boolean Plugin_071(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_071;
        Device[deviceCount].Type = DEVICE_TYPE_SERIAL;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_071);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_071));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_071));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        serialHelper_getGpioNames(event);
        break;
      }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
      {
        string += serialHelper_getSerialTypeLabel(event);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_071_init = true;

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE: {
      success = true;
      break;
    }

    case PLUGIN_READ:
      {
        PIN_KAMSER_RX = CONFIG_PIN1;
        PIN_KAMSER_TX = CONFIG_PIN2;
        const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

        ESPeasySerial kamSer(port, PIN_KAMSER_RX, PIN_KAMSER_TX, false);  // Initialize serial

        pinMode(PIN_KAMSER_RX,INPUT);
        pinMode(PIN_KAMSER_TX,OUTPUT);

        //read Kamstrup
        byte sendmsg1[] = { 175,163,177 };            //   /#1 with even parity

        byte r  = 0;
        byte to = 0;
        byte i;
        char message[255];
        int parityerrors;


        kamSer.begin(300);
        for (int x = 0; x < 3; x++) {
          kamSer.write(sendmsg1[x]);
        }

        kamSer.flush();
        //kamSer.end();
        kamSer.begin(1200);

        to = 0;
        r = 0;
        i = 0;
        parityerrors = 0;
        char *tmpstr;
        double m_energy, m_volume;
        float m_tempin, m_tempout, m_tempdiff, m_power;
        long m_hours, m_flow;

        while(r != 0x0A)
        {
          if (kamSer.available())
          {
            // receive byte
            r = kamSer.read();
            //serialPrintln(r);
            if (parity_check(r))
            {
               parityerrors += 1;
            }
            r = r & 127; // Mask MSB to remove parity

            message[i++] = char(r);
          }
          else
          {
            to++;
            delay(25);
          }

          if (i>=79)
          {
            if ( parityerrors == 0 )
            {
//              serialPrint("OK: " );
//              serialPrintln(message);
              message[i] = 0;

              tmpstr = strtok(message, " ");
              if (tmpstr){
               m_energy = atol(tmpstr)/3.6*1000;
              }
              else
               m_energy = 0;

              tmpstr = strtok(NULL, " ");
              if (tmpstr)
               m_volume = atol(tmpstr);
              else
               m_volume = 0;

              tmpstr = strtok(NULL, " ");
              if (tmpstr)
               m_hours = atol(tmpstr);
              else
               m_hours = 0;

              tmpstr = strtok(NULL, " ");
              if (tmpstr)
               m_tempin = atol(tmpstr)/100.0f;
              else
               m_tempin = 0;

              tmpstr = strtok(NULL, " ");
              if (tmpstr)
               m_tempout = atol(tmpstr)/100.0f;
              else
               m_tempout = 0;

              tmpstr = strtok(NULL, " ");
              if (tmpstr)
               m_tempdiff = atol(tmpstr)/100.0f;
              else
               m_tempdiff = 0;

              tmpstr = strtok(NULL, " ");
              if (tmpstr)
               m_power = atol(tmpstr)/10.0f;
              else
               m_power = 0;

              tmpstr = strtok(NULL, " ");
              if (tmpstr)
               m_flow = atol(tmpstr);
              else
               m_flow = 0;

               String log = F("Kamstrup output: ");
               log += m_energy;
               log += F(" MJ;  ");
               log += m_volume;
               log += F(" L; ");
               log += m_hours;
               log += F(" h; ");
               log += m_tempin;
               log += F(" C; ");
               log += m_tempout;
               log += F(" C; ");
               log += m_tempdiff;
               log += F(" C; ");
               log += m_power;
               log += ' ';
               log += m_flow;
               log += F(" L/H");
//              addLog(LOG_LEVEL_INFO, log);

              UserVar[event->BaseVarIndex] = m_energy; //gives energy in Wh
              UserVar[event->BaseVarIndex+1] = m_volume;  //gives volume in liters

              log = F("Kamstrup  : Heat value: ");
              log += m_energy/1000;
              log += F(" kWh");
              addLog(LOG_LEVEL_INFO, log);
              log = F("Kamstrup  : Volume value: ");
              log += m_volume;
              log += F(" Liter");
              addLog(LOG_LEVEL_INFO, log);
            }
            else
            {
              message[i] = 0;
              String log = F("ERR(PARITY):" );
              serialPrint("par");
              log += message;
              addLog(LOG_LEVEL_INFO, log);
              //UserVar[event->BaseVarIndex] = NAN;
              //UserVar[event->BaseVarIndex + 1] = NAN;
            }
            break;
          }
          if (to>100)
          {
            message[i] = 0;
            String log = F("ERR(TIMEOUT):" );
            log += message;
            addLog(LOG_LEVEL_INFO, log);

            //UserVar[event->BaseVarIndex] = NAN;
            //UserVar[event->BaseVarIndex + 1] = NAN;
            break;
          }
        }

        //end read Kamstrup


        success = true;
        break;
      }
  }
  return success;
}

bool parity_check(unsigned input) {
    bool inputparity = input & 128;
    int x = input & 127;

    int parity = 0;
    while(x != 0) {
        parity ^= x;
        x >>= 1;
    }

    if ( (parity & 0x1) != inputparity )
      return(1);
    else
      return(0);
}

#endif // USES_P071

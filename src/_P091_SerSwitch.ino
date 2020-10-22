#include "_Plugin_Helper.h"

/*##########################################################################################
  ######################### Plugin 091: Serial MCU controlled switch #######################
  ##########################################################################################

  Features :
   - Control serial linked devices through ESP8266
   - exactly only ONE P091 plugin can be used one time on one device!
   - serial have to be ENABLED, and serial logging level set to 0 at ESPEasy settings!
   - TUYA 4th button handling fixed by Mravko

  Compatible device list:
   1/ Tuya Wifi Touch wall switch (originally controlled by Tuya Smart/Smart Life app)
   2/ Tuya Wifi Dimmer Switch (originally controlled by Tuya Smart/Smart Life app)
   3/ Sonoff Dual - v1 only! (R2 has no serial MCU!)
   4/ LCTECH compatible 5V WiFi relay 1,2 and 4 relay versions also supported.
   5/ MOES Wifi Dimmer

   Relay states can be read from plugin values, the LCTech communication is only 1way, so the last stored state seen.
   Tuya can report states and can be queried the actual state, Sonoff Dual may report its state, when it's hardware buttons pushed.

   Support forum  thread: https://www.letscontrolit.com/forum/viewtopic.php?f=6&t=3245

   !!! For some reasons the serial 2way communication only works with Arduino ESP8266 core 2.4.0 !!!

  List of commands :
	- relay,[relay_number],[status]                  Set specific relay (0-3) to status (0/1)
	- relaypulse,[relay_number],[status],[delay]     Pulse specific relay for DELAY millisec with STATUS state,
                                                         than return to inverse state (blocking)
	- relaylongpulse,[relay_number],[status],[delay] Pulse specific relay for DELAY seconds with STATUS state,
                                                         than return to inverse state (non-blocking)
	- ydim,[DIM_VALUE]                               Set DIM_VALUE to Tuya dimmer switch (value can be 0-255, no range check!)
                                                         Of course, only the Tuya dimmer can do it... dim value can be read from plugin values.
                                                         There are no checks for is it state on or off.

  Command Examples :
	-  /control?cmd=relay,0,1             Switch on first relay
	-  /control?cmd=relay,0,0             Switch off first relay
	-  /control?cmd=relay,1,1             Switch on second relay
	-  /control?cmd=relay,1,0             Switch off second relay
	-  /control?cmd=relaypulse,0,1,500    Set first relay to ON for 500ms, than stay OFF
	-  /control?cmd=relaypulse,0,0,1000   Set first relay to OFF for 1s, than stay ON
	-  /control?cmd=ydim,255              Set dimmer to MAX value
	-  /control?cmd=ydim,25               Set dimmer to ~10%

  ------------------------------------------------------------------------------------------
	Copyleft Nagy Sándor 2019 - https://bitekmindenhol.blog.hu/
  ------------------------------------------------------------------------------------------
*/

#ifdef USES_P091


#define PLUGIN_091
#define PLUGIN_ID_091         91
#define PLUGIN_NAME_091       "Serial MCU controlled switch"
#define PLUGIN_VALUENAME1_091 "Relay0"
#define PLUGIN_VALUENAME2_091 "Relay1"
#define PLUGIN_VALUENAME3_091 "Relay2"
#define PLUGIN_VALUENAME4_091 "Relay3"

#define BUFFER_SIZE   168 // increased for 4 button Tuya

#define SER_SWITCH_YEWE 1
#define SER_SWITCH_SONOFFDUAL 2
#define SER_SWITCH_LCTECH 3
#define SER_SWITCH_WIFIDIMMER 4

static byte Plugin_091_switchstate[4];
static byte Plugin_091_ostate[4];
byte Plugin_091_commandstate = 0; // 0:no,1:inprogress,2:finished
Sensor_VType Plugin_091_type = Sensor_VType::SENSOR_TYPE_NONE;
byte Plugin_091_numrelay = 1;
byte Plugin_091_ownindex;
byte Plugin_091_globalpar0;
byte Plugin_091_globalpar1;
byte Plugin_091_cmddbl = false;
byte Plugin_091_ipd = false;
boolean Plugin_091_init = false;

boolean Plugin_091(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_091;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_QUAD;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 4;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }
    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_091);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_091));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_091));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_091));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_091));
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        {
          byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
          String options[4];
          options[0] = F("Yewelink/TUYA");
          options[1] = F("Sonoff Dual");
          options[2] = F("LC TECH");
          options[3] = F("Moes Wifi Dimmer");
          int optionValues[4] = { SER_SWITCH_YEWE, SER_SWITCH_SONOFFDUAL, SER_SWITCH_LCTECH, SER_SWITCH_WIFIDIMMER };
          addFormSelector(F("Switch Type"), F("plugin_091_type"), 4, options, optionValues, choice);
        }

        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_YEWE)
        {
          byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
          String buttonOptions[4];
          buttonOptions[0] = F("1");
          buttonOptions[1] = F("2/Dimmer#2");
          buttonOptions[2] = F("3/Dimmer#3");
          buttonOptions[3] = F("4");
          int buttonoptionValues[4] = { 1, 2, 3, 4 };
          addFormSelector(F("Number of relays"), F("plugin_091_button"), 4, buttonOptions, buttonoptionValues, choice);
        }

        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_SONOFFDUAL)
        {
          byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
          String modeoptions[3];
          modeoptions[0] = F("Normal");
          modeoptions[1] = F("Exclude/Blinds mode");
          modeoptions[2] = F("Simultaneous mode");
          int modeoptionValues[3] = { 0, 1, 2 };
          addFormSelector(F("Relay working mode"), F("plugin_091_mode"), 3, modeoptions, modeoptionValues, choice);
        }

        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_LCTECH)
        {
          {
            byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
            String buttonOptions[4];
            buttonOptions[0] = F("1");
            buttonOptions[1] = F("2");
            buttonOptions[2] = F("3");
            buttonOptions[3] = F("4");
            int buttonoptionValues[4] = { 1, 2, 3, 4 };
            addFormSelector(F("Number of relays"), F("plugin_091_button"), 4, buttonOptions, buttonoptionValues, choice);
          }

          {
            byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
            String speedOptions[8];
            speedOptions[0] = F("9600");
            speedOptions[1] = F("19200");
            speedOptions[2] = F("115200");
            speedOptions[3] = F("1200");
            speedOptions[4] = F("2400");
            speedOptions[5] = F("4800");
            speedOptions[6] = F("38400");
            speedOptions[7] = F("57600");
            addFormSelector(F("Serial speed"), F("plugin_091_speed"), 8, speedOptions, NULL, choice);
          }

          addFormCheckBox(F("Use command doubling"), F("plugin_091_dbl"), Settings.TaskDevicePluginConfig[event->TaskIndex][3]);
          addFormCheckBox(F("Use IPD preamble"), F("plugin_091_ipd"), Settings.TaskDevicePluginConfig[event->TaskIndex][4]);
        }

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {

        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_091_type"));
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_YEWE)
        {
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_091_button"));
        }
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_SONOFFDUAL)
        {
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_091_mode"));
        }
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_LCTECH)
        {
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_091_button"));
          Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_091_speed"));
          Settings.TaskDevicePluginConfig[event->TaskIndex][3] = isFormItemChecked(F("plugin_091_dbl"));
          Settings.TaskDevicePluginConfig[event->TaskIndex][4] = isFormItemChecked(F("plugin_091_ipd"));
          Plugin_091_cmddbl = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
          Plugin_091_ipd    = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
        }

        Plugin_091_globalpar0 = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        Plugin_091_globalpar1 = Settings.TaskDevicePluginConfig[event->TaskIndex][1];

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        String log = "";
        LoadTaskSettings(event->TaskIndex);
        Plugin_091_ownindex = event->TaskIndex;
        Settings.UseSerial = true;         // make sure that serial enabled
        Settings.SerialLogLevel = 0;       // and logging disabled
        Serial.setDebugOutput(false);      // really, disable it!
        log = F("SerSW : Init ");
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_YEWE)
        {
          Plugin_091_numrelay = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
          Serial.begin(9600, SERIAL_8N1);
          Serial.setRxBufferSize(BUFFER_SIZE); // Arduino core for ESP8266 WiFi chip 2.4.0
          delay(1);
          getmcustate(); // request status on startup
          log += F(" Yewe ");
          log += Plugin_091_numrelay;
          log += F(" btn");
        }
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_SONOFFDUAL)
        {
          Plugin_091_numrelay = 3; // 3rd button is the "wifi" button
          Serial.begin(19230, SERIAL_8N1);
          log += F(" Sonoff Dual");
        }
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_LCTECH)
        {
          Plugin_091_numrelay = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
          Plugin_091_cmddbl = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
          Plugin_091_ipd    = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
          unsigned long Plugin_091_speed = 9600;
          switch (Settings.TaskDevicePluginConfig[event->TaskIndex][2]) {
            case 1: {
                Plugin_091_speed = 19200;
                break;
              }
            case 2: {
                Plugin_091_speed = 115200;
                break;
              }
            case 3: {
                Plugin_091_speed = 1200;
                break;
              }
            case 4: {
                Plugin_091_speed = 2400;
                break;
              }
            case 5: {
                Plugin_091_speed = 4800;
                break;
              }
            case 6: {
                Plugin_091_speed = 38400;
                break;
              }
            case 7: {
                Plugin_091_speed = 57600;
                break;
              }
          }
          Serial.begin(Plugin_091_speed, SERIAL_8N1);
          log += F(" LCTech ");
          log += Plugin_091_speed;
          log += F(" baud ");
          log += Plugin_091_numrelay;
          log += F(" btn");
        }
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_WIFIDIMMER)
        {
          Plugin_091_numrelay = 2; // 2nd button is the dimvalue
          Plugin_091_switchstate[1] = 255;
          Plugin_091_ostate[1] = 255;
          Serial.begin(9600, SERIAL_8N1);
          log += F(" Wifi Dimmer");
        }

        Plugin_091_globalpar0 = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        Plugin_091_globalpar1 = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        switch (Plugin_091_numrelay)
        {
          case 1:
            Plugin_091_type = Sensor_VType::SENSOR_TYPE_SWITCH;
            break;
          case 2:
            Plugin_091_type = Sensor_VType::SENSOR_TYPE_DUAL;
            break;
          case 3:
            Plugin_091_type = Sensor_VType::SENSOR_TYPE_TRIPLE;
            break;
          case 4:
            Plugin_091_type = Sensor_VType::SENSOR_TYPE_QUAD;
            break;
        }
        addLog(LOG_LEVEL_INFO, log);

        success = true;
        Plugin_091_init = true;
        break;
      }


    case PLUGIN_SERIAL_IN:
      {
        int bytes_read = 0;
        byte serial_buf[BUFFER_SIZE];
        String log;

        if (Plugin_091_init)
        {
          while (Serial.available() > 0) {
            yield();
            if (bytes_read < BUFFER_SIZE) {
              serial_buf[bytes_read] = Serial.read();

              if (bytes_read == 0) { // packet start

                Plugin_091_commandstate = 0;
                switch (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
                {
                  case SER_SWITCH_YEWE: //decode first byte of package
                    {
                      if (serial_buf[bytes_read] == 0x55) {
                        Plugin_091_commandstate = 1;
                      }
                      break;
                    }
                  case SER_SWITCH_SONOFFDUAL: //decode first byte of package
                    {
                      if (serial_buf[bytes_read] == 0xA0) {
                        Plugin_091_commandstate = 1;
                      }
                      break;
                    }
                }
              } else {

                if (Plugin_091_commandstate == 1) {

                  if (bytes_read == 1) { // check if packet is valid
                    switch (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
                    {
                      case SER_SWITCH_YEWE:
                        {
                          if (serial_buf[bytes_read] != 0xAA) {
                            Plugin_091_commandstate = 0;
                            bytes_read = 0;
                          }
                          break;
                        }
                      case SER_SWITCH_SONOFFDUAL:
                        {
                          if ((serial_buf[bytes_read] != 0x04) && (serial_buf[bytes_read] != 0x00)) {
                            Plugin_091_commandstate = 0;
                            bytes_read = 0;
                          }
                          break;
                        }
                    }
                  }

                  if ( (bytes_read == 2) && (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_SONOFFDUAL)) { // decode Sonoff Dual status changes
                    Plugin_091_ostate[0] = Plugin_091_switchstate[0]; Plugin_091_ostate[1] = Plugin_091_switchstate[1]; Plugin_091_ostate[2] = Plugin_091_switchstate[2];
                    Plugin_091_switchstate[0] = 0; Plugin_091_switchstate[1] = 0; Plugin_091_switchstate[2] = 0;
                    if ((serial_buf[bytes_read] & 1) == 1) {
                      Plugin_091_switchstate[0] = 1;
                    }
                    if ((serial_buf[bytes_read] & 2) == 2) {
                      Plugin_091_switchstate[1] = 1;
                    }
                    if ((serial_buf[bytes_read] & 4) == 4) {
                      Plugin_091_switchstate[2] = 1;
                    }
                    Plugin_091_commandstate = 2; bytes_read = 0;

                    if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 1)
                    { // exclusive on mode
                      if ((Plugin_091_ostate[0] == 1) && (Plugin_091_switchstate[1] == 1)) {
                        sendmcucommand(0, 0, Settings.TaskDevicePluginConfig[event->TaskIndex][0], Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
                        Plugin_091_switchstate[0] = 0;
                      }
                      if ((Plugin_091_ostate[1] == 1) && (Plugin_091_switchstate[0] == 1)) {
                        sendmcucommand(1, 0, Settings.TaskDevicePluginConfig[event->TaskIndex][0], Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
                        Plugin_091_switchstate[1] = 0;
                      }
                    }
                    if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 2)
                    { // simultaneous mode
                      if ((Plugin_091_ostate[0] + Plugin_091_switchstate[0]) == 1) {
                        sendmcucommand(1, Plugin_091_switchstate[0], Settings.TaskDevicePluginConfig[event->TaskIndex][0], Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
                        Plugin_091_switchstate[1] = Plugin_091_switchstate[0];
                      } else {
                        if ((Plugin_091_ostate[1] + Plugin_091_switchstate[1]) == 1) {
                          sendmcucommand(0, Plugin_091_switchstate[1], Settings.TaskDevicePluginConfig[event->TaskIndex][0], Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
                          Plugin_091_switchstate[0] = Plugin_091_switchstate[1];
                        }
                      }
                    }

                    log = F("SerSW   : State ");
                    if (Plugin_091_ostate[0] != Plugin_091_switchstate[0]) {
                      UserVar[event->BaseVarIndex] = Plugin_091_switchstate[0];
                      log += F(" r0:");
                      log += Plugin_091_switchstate[0];
                    }
                    if (Plugin_091_ostate[1] != Plugin_091_switchstate[1]) {
                      UserVar[event->BaseVarIndex + 1] = Plugin_091_switchstate[1];
                      log += F(" r1:");
                      log += Plugin_091_switchstate[1];
                    }
                    if (Plugin_091_ostate[2] != Plugin_091_switchstate[2]) {
                      UserVar[event->BaseVarIndex + 2] = Plugin_091_switchstate[2];
                      log += F(" r2:");
                      log += Plugin_091_switchstate[2];
                    }
                    if (Plugin_091_ostate[3] != Plugin_091_switchstate[3]) {
                      UserVar[event->BaseVarIndex + 3] = Plugin_091_switchstate[3];
                      log += F(" r3:");
                      log += Plugin_091_switchstate[3];
                    }
                    addLog(LOG_LEVEL_INFO, log);
                    if ( (Plugin_091_ostate[0] != Plugin_091_switchstate[0]) || (Plugin_091_ostate[1] != Plugin_091_switchstate[1]) || (Plugin_091_ostate[2] != Plugin_091_switchstate[2]) || (Plugin_091_ostate[3] != Plugin_091_switchstate[3]) ) {
                      event->sensorType = Plugin_091_type;
                      sendData(event);
                    }
                  }
                  if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_YEWE) { // decode Tuya/Yewelink status report package
                    if ((bytes_read == 3) && (serial_buf[bytes_read] != 7))
                    {
                      Plugin_091_commandstate = 0;  // command code 7 means status reporting, we do not need other packets
                      bytes_read = 0;
                    }
                    if (bytes_read == 10) {
                      if (serial_buf[5] == 5) {
                        byte btnnum = (serial_buf[6] - 1);
                        Plugin_091_ostate[btnnum] = Plugin_091_switchstate[btnnum];
                        Plugin_091_switchstate[btnnum] = serial_buf[10];
                        Plugin_091_commandstate = 2; bytes_read = 0;

                        if (Plugin_091_ostate[btnnum] != Plugin_091_switchstate[btnnum]) {
                          log = F("SerSW   : State");
                          switch (btnnum) {
                            case 0: {
                                if (Plugin_091_numrelay > 0) {
                                  UserVar[event->BaseVarIndex + btnnum] = Plugin_091_switchstate[btnnum];
                                  log += F(" r0:");
                                  log += Plugin_091_switchstate[btnnum];
                                }
                                break;
                              }
                            case 1: {
                                if (Plugin_091_numrelay > 1) {
                                  UserVar[event->BaseVarIndex + btnnum] = Plugin_091_switchstate[btnnum];
                                  log += F(" r1:");
                                  log += Plugin_091_switchstate[btnnum];
                                }
                                break;
                              }
                            case 2: {
                                if (Plugin_091_numrelay > 2) {
                                  UserVar[event->BaseVarIndex + btnnum] = Plugin_091_switchstate[btnnum];
                                  log += F(" r2:");
                                  log += Plugin_091_switchstate[btnnum];
                                }
                                break;
                              }
                            case 3: {
                                if (Plugin_091_numrelay > 3) {
                                  UserVar[event->BaseVarIndex + btnnum] = Plugin_091_switchstate[btnnum];
                                  log += F(" r3:");
                                  log += Plugin_091_switchstate[btnnum];
                                }
                                break;
                              }
                          }
                          event->sensorType = Plugin_091_type;
                          addLog(LOG_LEVEL_INFO, log);
                          sendData(event);
                        }
                      }
                    } //10th byte end (Tuya switch)

                    if (bytes_read == 13) {
                      if (serial_buf[5] == 8) {
                        byte btnnum = (serial_buf[6] - 1);
                        Plugin_091_ostate[btnnum] = Plugin_091_switchstate[btnnum];
                        Plugin_091_switchstate[btnnum] = serial_buf[13];
                        Plugin_091_commandstate = 2; bytes_read = 0;

                        if (Plugin_091_ostate[btnnum] != Plugin_091_switchstate[btnnum]) {
                          log = F("SerSW   : Dimmer");
                          switch (btnnum) {
                            case 1: {
                                if (Plugin_091_numrelay > 1) {
                                  UserVar[event->BaseVarIndex + btnnum] = Plugin_091_switchstate[btnnum];
                                  log += F(" d1:");
                                  log += Plugin_091_switchstate[btnnum];
                                }
                                break;
                              }
                            case 2: {
                                if (Plugin_091_numrelay > 2) {
                                  UserVar[event->BaseVarIndex + btnnum] = Plugin_091_switchstate[btnnum];
                                  log += F(" d2:");
                                  log += Plugin_091_switchstate[btnnum];
                                }
                                break;
                              }
                          }
                          event->sensorType = Plugin_091_type;
                          addLog(LOG_LEVEL_INFO, log);
                          sendData(event);
                        }
                      }
                    } //13th byte end (Tuya dimmer)

                  } // yewe decode end
                } // Plugin_091_commandstate 1 end
              } // end of status decoding

              if (Plugin_091_commandstate == 1) {
                bytes_read++;
              }
            } else
              Serial.read(); // if buffer full, dump incoming
          }
        } // plugin initialized end
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (Plugin_091_init)
        {
          if ((Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_YEWE) && (Plugin_091_commandstate != 1))
          { // check Tuya state if anybody ask for it
            String log = F("SerSW   : ReadState");
            addLog(LOG_LEVEL_INFO, log);
            getmcustate();
          }
          if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == SER_SWITCH_WIFIDIMMER) {
            if (Plugin_091_switchstate[1] < 1)
            {
              UserVar[event->BaseVarIndex] = 0;
            } else {
              UserVar[event->BaseVarIndex] = 1;
            }
            UserVar[event->BaseVarIndex + 1] = Plugin_091_switchstate[1];
          }
          success = true;
        }
        break;
      }

    case PLUGIN_WRITE:
      {
        String log = "";
        String command = parseString(string, 1);
        byte rnum = 0;
        byte rcmd = 0;
        byte par3 = 0;

        if (Plugin_091_init)
        {
          if ( command == F("relay") ) // deal with relay change command
          {
            success = true;

            if ((event->Par1 >= 0) && (event->Par1 < Plugin_091_numrelay)) {
              rnum = event->Par1;
            }
            if ((event->Par2 == 0) || (event->Par2 == 1)) {
              rcmd = event->Par2;
            }

            LoadTaskSettings(Plugin_091_ownindex); // get our own task values please
            event->setTaskIndex(Plugin_091_ownindex);

            if (event->Par2 == 2) { // toggle
              rcmd = 1 - UserVar[(event->BaseVarIndex + rnum)];
            }

            if ( Plugin_091_globalpar0 < SER_SWITCH_LCTECH) {
              par3 = Plugin_091_globalpar1;
            }
            sendmcucommand(rnum, rcmd, Plugin_091_globalpar0, par3);
            if ( Plugin_091_globalpar0 > SER_SWITCH_YEWE) { // report state only if not Yewe
              if (UserVar[(event->BaseVarIndex + rnum)] != Plugin_091_switchstate[rnum]) { // report only if state is really changed
                UserVar[(event->BaseVarIndex + rnum)] = Plugin_091_switchstate[rnum];
                if (( par3 == 1) && (rcmd == 1) && (rnum < 2))
                { // exclusive on mode for Dual
                  // FIXME TD-er: Is this a valid UserVar index?
                  UserVar[(event->BaseVarIndex + 1 - rnum)] = 0;
                }
                if (par3 == 2) { // simultaneous mode for Dual
                  // FIXME TD-er: Is this a valid UserVar index?
                  UserVar[(event->BaseVarIndex + 1 - rnum)] = Plugin_091_switchstate[1 - rnum];
                }
                if (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER) {
                  UserVar[event->BaseVarIndex + 1] = Plugin_091_switchstate[1];
                }
                event->sensorType = Plugin_091_type;
                sendData(event);
              }
            }
            String log = F("SerSW   : SetSwitch r");
            log += rnum;
            log += F(":");
            log += rcmd;
            addLog(LOG_LEVEL_INFO, log);
          }

          if ( command == F("relaypulse") )
          {
            success = true;

            if ((event->Par1 >= 0) && (event->Par1 < Plugin_091_numrelay)) {
              rnum = event->Par1;
            }
            if ((event->Par2 == 0) || (event->Par2 == 1)) {
              rcmd = event->Par2;
            }
            LoadTaskSettings(Plugin_091_ownindex); // get our own task values please
            event->setTaskIndex(Plugin_091_ownindex);

            if ( Plugin_091_globalpar0 < SER_SWITCH_LCTECH) {
              par3 = Plugin_091_globalpar1;
            }

            sendmcucommand(rnum, rcmd, Plugin_091_globalpar0, par3); // init state
            delay(event->Par3);
            sendmcucommand(rnum, !rcmd, Plugin_091_globalpar0, par3); // invert state
            if ( Plugin_091_globalpar0 > SER_SWITCH_YEWE) { // report state only if not Yewe
              if (UserVar[(event->BaseVarIndex + rnum)] != Plugin_091_switchstate[rnum]) { // report only if state is really changed
                UserVar[(event->BaseVarIndex + rnum)] = Plugin_091_switchstate[rnum];
                if (( par3 == 1) && (rcmd == 1) && (rnum < 2))
                { // exclusive on mode for Dual
                  // FIXME TD-er: Is this a valid UserVar index?
                  UserVar[(event->BaseVarIndex + 1 - rnum)] = 0;
                }
                if (par3 == 2) { // simultaneous mode for Dual
                  // FIXME TD-er: Is this a valid UserVar index?
                  UserVar[(event->BaseVarIndex + 1 - rnum)] = Plugin_091_switchstate[1 - rnum];
                }
                if (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER) {
                  UserVar[event->BaseVarIndex + 1] = Plugin_091_switchstate[1];
                }
                event->sensorType = Plugin_091_type;
                sendData(event);
              }
            }

            String log = F("SerSW   : SetSwitchPulse r");
            log += rnum;
            log += F(":");
            log += rcmd;
            log += F(" Pulsed for ");
            log += String(event->Par3);
            log += F(" mS");
            addLog(LOG_LEVEL_INFO, log);
          }

          if ( command == F("relaylongpulse") )
          {
            success = true;

            if ((event->Par1 >= 0) && (event->Par1 < Plugin_091_numrelay)) {
              rnum = event->Par1;
            }
            if ((event->Par2 == 0) || (event->Par2 == 1)) {
              rcmd = event->Par2;
            }
            LoadTaskSettings(Plugin_091_ownindex); // get our own task values please
            event->setTaskIndex(Plugin_091_ownindex);

            if ( Plugin_091_globalpar0 < SER_SWITCH_LCTECH) {
              par3 = Plugin_091_globalpar1;
            }
            unsigned long timer = event->Par3 * 1000;

            sendmcucommand(rnum, rcmd, Plugin_091_globalpar0, par3); // init state
            //Scheduler.setPluginTimer(timer, PLUGIN_ID_091, rnum, !rcmd);
            Scheduler.setPluginTaskTimer(timer, PLUGIN_ID_091, event->TaskIndex, rnum, !rcmd);
            if ( Plugin_091_globalpar0 > SER_SWITCH_YEWE) { // report state only if not Yewe
              if (UserVar[(event->BaseVarIndex + rnum)] != Plugin_091_switchstate[rnum]) { // report only if state is really changed
                UserVar[(event->BaseVarIndex + rnum)] = Plugin_091_switchstate[rnum];
                if (( par3 == 1) && (rcmd == 1) && (rnum < 2))
                { // exclusive on mode for Dual
                  // FIXME TD-er: Is this a valid UserVar index?
                  UserVar[(event->BaseVarIndex + 1 - rnum)] = 0;
                }
                if (par3 == 2) { // simultaneous mode for Dual
                  // FIXME TD-er: Is this a valid UserVar index?
                  UserVar[(event->BaseVarIndex + 1 - rnum)] = Plugin_091_switchstate[1 - rnum];
                }
                if (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER) {
                  UserVar[event->BaseVarIndex + 1] = Plugin_091_switchstate[1];
                }
                event->sensorType = Plugin_091_type;
                sendData(event);
              }
            }

            String log = F("SerSW   : SetSwitchPulse r");
            log += rnum;
            log += F(":");
            log += rcmd;
            log += F(" Pulse for ");
            log += String(event->Par3);
            log += F(" sec");
            addLog(LOG_LEVEL_INFO, log);
          }
          if ( command == F("ydim") ) // deal with dimmer command
          {
            String log = F("SerSW   : SetDim ");
            if (( (Plugin_091_globalpar0 == SER_SWITCH_YEWE) && (Plugin_091_numrelay > 1)) || (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER)) { // only on tuya dimmer
              success = true;

              LoadTaskSettings(Plugin_091_ownindex); // get our own task values please
              event->setTaskIndex(Plugin_091_ownindex);

              sendmcudim(event->Par1, Plugin_091_globalpar0);
              if (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER) {
                if (Plugin_091_switchstate[1] < 1) // follow state
                {
                  UserVar[event->BaseVarIndex] = 0;
                  UserVar[event->BaseVarIndex + 1] = Plugin_091_switchstate[1];
                } else {
                  UserVar[event->BaseVarIndex] = 1;
                  UserVar[event->BaseVarIndex + 1] = Plugin_091_switchstate[1];
                }
                event->sensorType = Plugin_091_type;
                sendData(event);
              }
              log += event->Par1;
              addLog(LOG_LEVEL_INFO, log);
            } else {
              log = F("\nYDim not supported");
              SendStatus(event->Source, log);
            }
          }

        }

        break;
      }

    case PLUGIN_TIMER_IN:
      {
        byte par3 = 0;

        LoadTaskSettings(Plugin_091_ownindex); // get our own task values please
        event->setTaskIndex(Plugin_091_ownindex);

        if ( Plugin_091_globalpar0 < SER_SWITCH_LCTECH) {
          par3 = Plugin_091_globalpar1;
        }

        byte rnum = event->Par1;
        byte rcmd = event->Par2;

        sendmcucommand(rnum, rcmd, Plugin_091_globalpar0, par3); // invert state
        if ( Plugin_091_globalpar0 > SER_SWITCH_YEWE) { // report state only if not Yewe
          if (UserVar[(event->BaseVarIndex + rnum)] != Plugin_091_switchstate[rnum]) { // report only if state is really changed
            UserVar[(event->BaseVarIndex + rnum)] = Plugin_091_switchstate[rnum];
            if (( par3 == 1) && (rcmd == 1) && (rnum < 2))
            { // exclusive on mode for Dual
              // FIXME TD-er: Is this a valid UserVar index?
              UserVar[(event->BaseVarIndex + 1 - rnum)] = 0;
            }
            if (par3 == 2) { // simultaneous mode for Dual
              // FIXME TD-er: Is this a valid UserVar index?
              UserVar[(event->BaseVarIndex + 1 - rnum)] = Plugin_091_switchstate[1 - rnum];
            }
            if (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER) {
              UserVar[event->BaseVarIndex + 1] = Plugin_091_switchstate[1];
            }
            event->sensorType = Plugin_091_type;
            sendData(event);
          }
        }

        String log = F("SerSW   : SetSwitchPulse r");
        log += rnum;
        log += F(":");
        log += rcmd;
        log += F(" Pulse ended");
        addLog(LOG_LEVEL_INFO, log);

        break;
      }


  }
  return success;
}

void getmcustate() {
  Serial.write(0x55); // Tuya header 55AA
  Serial.write(0xAA);
  Serial.write(0x00); // version 00
  Serial.write(0x08); // Tuya command 08 - request status
  Serial.write(0x00);
  Serial.write(0x00);
  Serial.write(0x07);
  Serial.flush();
}

void sendmcucommand(byte btnnum, byte state, byte swtype, byte btnum_mode) // btnnum=0,1,2, state=0/1
{
  byte sstate;

  switch (swtype)
  {
    case SER_SWITCH_YEWE:
      {
        Serial.write(0x55); // Tuya header 55AA
        Serial.write(0xAA);
        Serial.write(0x00); // version 00
        Serial.write(0x06); // Tuya command 06 - send order
        Serial.write(0x00);
        Serial.write(0x05); // following data length 0x05
        Serial.write( (btnnum + 1) ); // relay number 1,2,3
        Serial.write(0x01); // ?
        Serial.write(0x00); // ?
        Serial.write(0x01); // ?
        Serial.write( state ); // status
        Serial.write((13 + btnnum + state)); // checksum:sum of all bytes in packet mod 256
        Serial.flush();
        break;
      }
    case SER_SWITCH_SONOFFDUAL:
      {
        Plugin_091_switchstate[btnnum] = state;
        if (( btnum_mode == 1) && (state == 1) && (btnnum < 2))
        { // exclusive on mode
          Plugin_091_switchstate[(1 - btnnum)] = 0;
        }
        if (btnum_mode == 2)
        { // simultaneous mode
          Plugin_091_switchstate[0] = state;
          Plugin_091_switchstate[1] = state;
        }
        sstate = Plugin_091_switchstate[0] + (Plugin_091_switchstate[1] << 1) + (Plugin_091_switchstate[2] << 2);
        Serial.write(0xA0);
        Serial.write(0x04);
        Serial.write( sstate );
        Serial.write(0xA1);
        Serial.flush();
        break;
      }
    case SER_SWITCH_LCTECH:
      {
        byte c_d = 1;
        if (Plugin_091_cmddbl) {
          c_d = 2;
        }
        Plugin_091_switchstate[btnnum] = state;
        for (byte x = 0; x < c_d; x++) // try twice to be sure
        {
          if (x > 0) {
            delay(1);
          }
          if (Plugin_091_ipd) {
            Serial.write(0x0D);
            Serial.write(0x0A);
            Serial.write(0x2B);
            Serial.write(0x49);
            Serial.write(0x50);
            Serial.write(0x44);
            Serial.write(0x2C);
            Serial.write(0x30);
            Serial.write(0x2C);
            Serial.write(0x34);
            Serial.write(0x3A);
          }
          Serial.write(0xA0);
          Serial.write((0x01 + btnnum));
          Serial.write((0x00 + state));
          Serial.write((0xA1 + state + btnnum));
          Serial.flush();
        }

        break;

      }
    case SER_SWITCH_WIFIDIMMER:
      {
        if (btnnum == 0) {
          if (state == 0) { // off
            Plugin_091_switchstate[0] = 0;
            if (Plugin_091_switchstate[1] < 1) {
              if (Plugin_091_ostate[1] < 1) {
                Plugin_091_ostate[1] = 255;
              }
            } else {
              Plugin_091_ostate[1] = Plugin_091_switchstate[1];
            }
            sendmcudim(0, SER_SWITCH_WIFIDIMMER);
          } else { // on
            Plugin_091_switchstate[0] = 1;
            if (Plugin_091_ostate[1] < 1) {
              if (Plugin_091_switchstate[1] > 0) {
                sstate = Plugin_091_switchstate[1];
              } else  {
                sstate = 255;
              }
            } else {
              sstate = Plugin_091_ostate[1];
            }
            sendmcudim(sstate, SER_SWITCH_WIFIDIMMER);
          }
        }
        break;
      }
  }
}

void sendmcudim(byte dimvalue, byte swtype)
{
  switch (swtype)
  {
    case SER_SWITCH_YEWE:
      {
        Serial.write(0x55); // Tuya header 55AA
        Serial.write(0xAA);
        Serial.write(0x00); // version 00
        Serial.write(0x06); // Tuya command 06 - send order
        Serial.write(0x00);
        Serial.write(0x08); // following data length 0x08
        Serial.write(Plugin_091_numrelay); // dimmer order-id? select it at plugin settings 2/3!!!
        Serial.write(0x02); // type=value
        Serial.write(0x00); // length hi
        Serial.write(0x04); // length low
        Serial.write(0x00); // ?
        Serial.write(0x00); // ?
        Serial.write(0x00); // ?
        Serial.write( dimvalue ); // dim value (0-255)
        Serial.write( byte(19 + Plugin_091_numrelay + dimvalue) ); // checksum:sum of all bytes in packet mod 256
        Serial.flush();
        break;
      }
    case SER_SWITCH_WIFIDIMMER:
      {
        Serial.write(0xFF); // Wifidimmer header FF55
        Serial.write(0x55);
        Serial.write( dimvalue ); // dim value (0-255)
        Serial.write(0x05);
        Serial.write(0xDC);
        Serial.write(0x0A);
        Serial.flush();
        Plugin_091_switchstate[1] = dimvalue;
        break;
      }
  }
}

#endif // USES_P091
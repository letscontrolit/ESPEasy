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
                                                         Of course, only the Tuya dimmer can do it... dim value can be read from plugin
 #values.
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


# define PLUGIN_091
# define PLUGIN_ID_091         91
# define PLUGIN_NAME_091       "Serial MCU controlled switch"
# define PLUGIN_VALUENAME1_091 "Relay0"
# define PLUGIN_VALUENAME2_091 "Relay1"
# define PLUGIN_VALUENAME3_091 "Relay2"
# define PLUGIN_VALUENAME4_091 "Relay3"

# define BUFFER_SIZE   168 // increased for 4 button Tuya

# define SER_SWITCH_YEWE 1
# define SER_SWITCH_SONOFFDUAL 2
# define SER_SWITCH_LCTECH 3
# define SER_SWITCH_WIFIDIMMER 4

static uint8_t Plugin_091_switchstate[4];
static uint8_t Plugin_091_ostate[4];
uint8_t Plugin_091_commandstate = 0; // 0:no,1:inprogress,2:finished
Sensor_VType Plugin_091_type    = Sensor_VType::SENSOR_TYPE_NONE;
uint8_t Plugin_091_numrelay     = 1;
uint8_t Plugin_091_ownindex;
uint8_t Plugin_091_globalpar0;
uint8_t Plugin_091_globalpar1;
uint8_t Plugin_091_cmddbl = false;
uint8_t Plugin_091_ipd    = false;
boolean Plugin_091_init   = false;

boolean Plugin_091(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_091;
      dev.Type           = DEVICE_TYPE_DUMMY;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
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
        const __FlashStringHelper *options[] = {
          F("Yewelink/TUYA"),
          F("Sonoff Dual"),
          F("LC TECH"),
          F("Moes Wifi Dimmer")
        };
        const int optionValues[]     = { SER_SWITCH_YEWE, SER_SWITCH_SONOFFDUAL, SER_SWITCH_LCTECH, SER_SWITCH_WIFIDIMMER };
        constexpr size_t optionCount = NR_ELEMENTS(optionValues);
        addFormSelector(F("Switch Type"), F("type"), optionCount, options, optionValues, PCONFIG(0));
      }

      if (PCONFIG(0) == SER_SWITCH_YEWE)
      {
        const __FlashStringHelper *buttonOptions[] = {
          F("1"),
          F("2/Dimmer#2"),
          F("3/Dimmer#3"),
          F("4"),
        };
        const int buttonoptionValues[] = { 1, 2, 3, 4 };
        constexpr size_t optionCount   = NR_ELEMENTS(buttonoptionValues);
        addFormSelector(F("Number of relays"), F("button"), optionCount, buttonOptions, buttonoptionValues, PCONFIG(1));
      }

      if (PCONFIG(0) == SER_SWITCH_SONOFFDUAL)
      {
        const __FlashStringHelper *modeoptions[] = {
          F("Normal"),
          F("Exclude/Blinds mode"),
          F("Simultaneous mode"),
        };
        constexpr size_t optionCount = NR_ELEMENTS(modeoptions);
        addFormSelector(F("Relay working mode"), F("mode"), optionCount, modeoptions, nullptr, PCONFIG(1));
      }

      if (PCONFIG(0) == SER_SWITCH_LCTECH)
      {
        {
          const __FlashStringHelper *buttonOptions[] = {
            F("1"),
            F("2"),
            F("3"),
            F("4"),
          };
          const int buttonoptionValues[] = { 1, 2, 3, 4 };
          constexpr size_t optionCount   = NR_ELEMENTS(buttonoptionValues);
          addFormSelector(F("Number of relays"), F("button"), optionCount, buttonOptions, buttonoptionValues, PCONFIG(1));
        }

        {
          const __FlashStringHelper *speedOptions[] = {
            F("9600"),
            F("19200"),
            F("115200"),
            F("1200"),
            F("2400"),
            F("4800"),
            F("38400"),
            F("57600"),
          };
          constexpr size_t optionCount = NR_ELEMENTS(speedOptions);
          addFormSelector(F("Serial speed"), F("speed"), optionCount, speedOptions, nullptr, PCONFIG(2));
        }

        addFormCheckBox(F("Use command doubling"), F("dbl"), PCONFIG(3));
        addFormCheckBox(F("Use IPD preamble"),     F("ipd"), PCONFIG(4));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("type"));

      if (PCONFIG(0) == SER_SWITCH_YEWE)
      {
        PCONFIG(1) = getFormItemInt(F("button"));
      }

      if (PCONFIG(0) == SER_SWITCH_SONOFFDUAL)
      {
        PCONFIG(1) = getFormItemInt(F("mode"));
      }

      if (PCONFIG(0) == SER_SWITCH_LCTECH)
      {
        PCONFIG(1)        = getFormItemInt(F("button"));
        PCONFIG(2)        = getFormItemInt(F("speed"));
        PCONFIG(3)        = isFormItemChecked(F("dbl"));
        PCONFIG(4)        = isFormItemChecked(F("ipd"));
        Plugin_091_cmddbl = PCONFIG(3);
        Plugin_091_ipd    = PCONFIG(4);
      }

      Plugin_091_globalpar0 = PCONFIG(0);
      Plugin_091_globalpar1 = PCONFIG(1);

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      String log;
      Plugin_091_ownindex     = event->TaskIndex;
      Settings.UseSerial      = true;         // FIXME This is most likely very wrong... make sure that serial enabled
      Settings.SerialLogLevel = 0;            // and logging disabled
      ESPEASY_SERIAL_0.setDebugOutput(false); // really, disable it!
      log = F("SerSW : Init ");

      if (PCONFIG(0) == SER_SWITCH_YEWE)
      {
        Plugin_091_numrelay = PCONFIG(1);
        ESPEASY_SERIAL_0.begin(9600, SERIAL_8N1);
        ESPEASY_SERIAL_0.setRxBufferSize(BUFFER_SIZE); // Arduino core for ESP8266 WiFi chip 2.4.0
        delay(1);
        getmcustate();                                 // request status on startup
        log += strformat(F(" Yewe %d btn"), Plugin_091_numrelay);
      } else
      if (PCONFIG(0) == SER_SWITCH_SONOFFDUAL)
      {
        Plugin_091_numrelay = 3; // 3rd button is the "wifi" button
        ESPEASY_SERIAL_0.begin(19230, SERIAL_8N1);
        log += F(" Sonoff Dual");
      } else
      if (PCONFIG(0) == SER_SWITCH_LCTECH)
      {
        Plugin_091_numrelay = PCONFIG(1);
        Plugin_091_cmddbl   = PCONFIG(3);
        Plugin_091_ipd      = PCONFIG(4);
        const int bauds[]              = { 9600, 19200, 115200, 1200, 2400, 4800, 38400, 57600 };
        unsigned long Plugin_091_speed = bauds[PCONFIG(2)];
        ESPEASY_SERIAL_0.begin(Plugin_091_speed, SERIAL_8N1);
        log += strformat(F(" LCTech %d baud %d btn"), Plugin_091_speed, Plugin_091_numrelay);
      } else
      if (PCONFIG(0) == SER_SWITCH_WIFIDIMMER)
      {
        Plugin_091_numrelay       = 2; // 2nd button is the dimvalue
        Plugin_091_switchstate[1] = 255;
        Plugin_091_ostate[1]      = 255;
        ESPEASY_SERIAL_0.begin(9600, SERIAL_8N1);
        log += F(" Wifi Dimmer");
      }

      Plugin_091_globalpar0 = PCONFIG(0);
      Plugin_091_globalpar1 = PCONFIG(1);

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
      addLogMove(LOG_LEVEL_INFO, log);

      success         = true;
      Plugin_091_init = true;
      break;
    }


    case PLUGIN_SERIAL_IN:
    {
      int bytes_read = 0;
      uint8_t serial_buf[BUFFER_SIZE];
      String  log;

      if (Plugin_091_init)
      {
        while (ESPEASY_SERIAL_0.available() > 0) {
          yield();

          if (bytes_read < BUFFER_SIZE) {
            serial_buf[bytes_read] = ESPEASY_SERIAL_0.read();

            if (bytes_read == 0) { // packet start
              Plugin_091_commandstate = 0;

              switch (PCONFIG(0))
              {
                case SER_SWITCH_YEWE: // decode first uint8_t of package
                {
                  if (serial_buf[bytes_read] == 0x55) {
                    Plugin_091_commandstate = 1;
                  }
                  break;
                }
                case SER_SWITCH_SONOFFDUAL: // decode first uint8_t of package
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
                  switch (PCONFIG(0))
                  {
                    case SER_SWITCH_YEWE:
                    {
                      if (serial_buf[bytes_read] != 0xAA) {
                        Plugin_091_commandstate = 0;
                        bytes_read              = 0;
                      }
                      break;
                    }
                    case SER_SWITCH_SONOFFDUAL:
                    {
                      if ((serial_buf[bytes_read] != 0x04) && (serial_buf[bytes_read] != 0x00)) {
                        Plugin_091_commandstate = 0;
                        bytes_read              = 0;
                      }
                      break;
                    }
                  }
                }

                if ((bytes_read == 2) && (PCONFIG(0) == SER_SWITCH_SONOFFDUAL)) { // decode Sonoff Dual status changes
                  Plugin_091_ostate[0]      = Plugin_091_switchstate[0]; Plugin_091_ostate[1] = Plugin_091_switchstate[1];
                  Plugin_091_ostate[2]      = Plugin_091_switchstate[2];
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

                  if (PCONFIG(1) == 1)
                  { // exclusive on mode
                    if ((Plugin_091_ostate[0] == 1) && (Plugin_091_switchstate[1] == 1)) {
                      sendmcucommand(0, 0, PCONFIG(0), PCONFIG(1));
                      Plugin_091_switchstate[0] = 0;
                    }

                    if ((Plugin_091_ostate[1] == 1) && (Plugin_091_switchstate[0] == 1)) {
                      sendmcucommand(1, 0, PCONFIG(0), PCONFIG(1));
                      Plugin_091_switchstate[1] = 0;
                    }
                  }

                  if (PCONFIG(1) == 2)
                  { // simultaneous mode
                    if ((Plugin_091_ostate[0] + Plugin_091_switchstate[0]) == 1) {
                      sendmcucommand(1, Plugin_091_switchstate[0], PCONFIG(0), PCONFIG(1));
                      Plugin_091_switchstate[1] = Plugin_091_switchstate[0];
                    } else {
                      if ((Plugin_091_ostate[1] + Plugin_091_switchstate[1]) == 1) {
                        sendmcucommand(0, Plugin_091_switchstate[1], PCONFIG(0), PCONFIG(1));
                        Plugin_091_switchstate[0] = Plugin_091_switchstate[1];
                      }
                    }
                  }

                  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                    log = F("SerSW   : State ");
                  }

                  for (int i = 0; i < 3; ++i) {
                    if (Plugin_091_ostate[i] != Plugin_091_switchstate[i]) {
                      UserVar.setFloat(event->TaskIndex, i, Plugin_091_switchstate[i]);

                      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                        log += strformat(F(" r%d:%d"), i, Plugin_091_switchstate[i]);
                      }
                    }
                  }
                  addLogMove(LOG_LEVEL_INFO, log);

                  if ((Plugin_091_ostate[0] != Plugin_091_switchstate[0]) || (Plugin_091_ostate[1] != Plugin_091_switchstate[1]) ||
                      (Plugin_091_ostate[2] != Plugin_091_switchstate[2]) || (Plugin_091_ostate[3] != Plugin_091_switchstate[3])) {
                    event->sensorType = Plugin_091_type;
                    sendData(event);
                  }
                }

                if (PCONFIG(0) == SER_SWITCH_YEWE) { // decode Tuya/Yewelink status report package
                  if ((bytes_read == 3) && (serial_buf[bytes_read] != 7))
                  {
                    Plugin_091_commandstate = 0;     // command code 7 means status reporting, we do not need other packets
                    bytes_read              = 0;
                  }

                  if (bytes_read == 10) {
                    if (serial_buf[5] == 5) {
                      uint8_t btnnum = (serial_buf[6] - 1);
                      Plugin_091_ostate[btnnum]      = Plugin_091_switchstate[btnnum];
                      Plugin_091_switchstate[btnnum] = serial_buf[10];
                      Plugin_091_commandstate        = 2; bytes_read = 0;

                      if (Plugin_091_ostate[btnnum] != Plugin_091_switchstate[btnnum]) {
                        log = F("SerSW   : State");

                        switch (btnnum) {
                          case 0: {
                            if (Plugin_091_numrelay > 0) {
                              UserVar.setFloat(event->TaskIndex, btnnum, Plugin_091_switchstate[btnnum]);
                              log += concat(F(" r0:"), Plugin_091_switchstate[btnnum]);
                            }
                            break;
                          }
                          case 1: {
                            if (Plugin_091_numrelay > 1) {
                              UserVar.setFloat(event->TaskIndex, btnnum, Plugin_091_switchstate[btnnum]);
                              log += concat(F(" r1:"), Plugin_091_switchstate[btnnum]);
                            }
                            break;
                          }
                          case 2: {
                            if (Plugin_091_numrelay > 2) {
                              UserVar.setFloat(event->TaskIndex, btnnum, Plugin_091_switchstate[btnnum]);
                              log += concat(F(" r2:"), Plugin_091_switchstate[btnnum]);
                            }
                            break;
                          }
                          case 3: {
                            if (Plugin_091_numrelay > 3) {
                              UserVar.setFloat(event->TaskIndex, btnnum, Plugin_091_switchstate[btnnum]);
                              log += concat(F(" r3:"), Plugin_091_switchstate[btnnum]);
                            }
                            break;
                          }
                        }
                        event->sensorType = Plugin_091_type;
                        addLogMove(LOG_LEVEL_INFO, log);
                        sendData(event);
                      }
                    }
                  } // 10th uint8_t end (Tuya switch)

                  if (bytes_read == 13) {
                    if (serial_buf[5] == 8) {
                      uint8_t btnnum = (serial_buf[6] - 1);
                      Plugin_091_ostate[btnnum]      = Plugin_091_switchstate[btnnum];
                      Plugin_091_switchstate[btnnum] = serial_buf[13];
                      Plugin_091_commandstate        = 2; bytes_read = 0;

                      if (Plugin_091_ostate[btnnum] != Plugin_091_switchstate[btnnum]) {
                        log = F("SerSW   : Dimmer");

                        switch (btnnum) {
                          case 1: {
                            if (Plugin_091_numrelay > 1) {
                              UserVar.setFloat(event->TaskIndex, btnnum, Plugin_091_switchstate[btnnum]);
                              log += concat(F(" d1:"), Plugin_091_switchstate[btnnum]);
                            }
                            break;
                          }
                          case 2: {
                            if (Plugin_091_numrelay > 2) {
                              UserVar.setFloat(event->TaskIndex, btnnum, Plugin_091_switchstate[btnnum]);
                              log += concat(F(" d2:"), Plugin_091_switchstate[btnnum]);
                            }
                            break;
                          }
                        }
                        event->sensorType = Plugin_091_type;
                        addLogMove(LOG_LEVEL_INFO, log);
                        sendData(event);
                      }
                    }
                  } // 13th uint8_t end (Tuya dimmer)
                }   // yewe decode end
              }     // Plugin_091_commandstate 1 end
            }       // end of status decoding

            if (Plugin_091_commandstate == 1) {
              bytes_read++;
            }
          } else {
            ESPEASY_SERIAL_0.read(); // if buffer full, dump incoming
          }
        }
      }                              // plugin initialized end
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (Plugin_091_init)
      {
        if ((PCONFIG(0) == SER_SWITCH_YEWE) && (Plugin_091_commandstate != 1))
        { // check Tuya state if anybody ask for it
          addLog(LOG_LEVEL_INFO, F("SerSW   : ReadState"));
          getmcustate();
        }

        if (PCONFIG(0) == SER_SWITCH_WIFIDIMMER) {
          if (Plugin_091_switchstate[1] < 1)
          {
            UserVar.setFloat(event->TaskIndex, 0, 0);
          } else {
            UserVar.setFloat(event->TaskIndex, 0, 1);
          }
          UserVar.setFloat(event->TaskIndex, 1, Plugin_091_switchstate[1]);
        }
        success = true;
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      const String command = parseString(string, 1);
      String  log;
      uint8_t rnum{};
      uint8_t rcmd{};
      uint8_t par3{};

      if (Plugin_091_init)
      {
        if (equals(command, F("relay"))) // deal with relay change command
        {
          success = true;

          if ((event->Par1 >= 0) && (event->Par1 < Plugin_091_numrelay)) {
            rnum = event->Par1;
          }

          if ((event->Par2 == 0) || (event->Par2 == 1)) {
            rcmd = event->Par2;
          }

          // LoadTaskSettings(Plugin_091_ownindex); // get our own task values please
          event->setTaskIndex(Plugin_091_ownindex);

          if (event->Par2 == 2) { // toggle
            rcmd = 1 - UserVar[(event->BaseVarIndex + rnum)];
          }

          if (Plugin_091_globalpar0 < SER_SWITCH_LCTECH) {
            par3 = Plugin_091_globalpar1;
          }
          sendmcucommand(rnum, rcmd, Plugin_091_globalpar0, par3);

          if (Plugin_091_globalpar0 > SER_SWITCH_YEWE) {                                 // report state only if not Yewe
            if (UserVar[(event->BaseVarIndex + rnum)] != Plugin_091_switchstate[rnum]) { // report only if state is really changed
              UserVar.setFloat(event->TaskIndex, rnum, Plugin_091_switchstate[rnum]);

              if ((par3 == 1) && (rcmd == 1) && (rnum < 2))
              { // exclusive on mode for Dual
                // FIXME TD-er: Is this a valid UserVar index?
                UserVar.setFloat(event->TaskIndex, 1 - rnum, 0);
              }

              if (par3 == 2) { // simultaneous mode for Dual
                // FIXME TD-er: Is this a valid UserVar index?
                UserVar.setFloat(event->TaskIndex, 1 - rnum, Plugin_091_switchstate[1 - rnum]);
              }

              if (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER) {
                UserVar.setFloat(event->TaskIndex, 1, Plugin_091_switchstate[1]);
              }
              event->sensorType = Plugin_091_type;
              sendData(event);
            }
          }

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, strformat(F("SerSW   : SetSwitch r%d:%d"), rnum, rcmd));
          }
        } else

        if (equals(command, F("relaypulse")))
        {
          success = true;

          if ((event->Par1 >= 0) && (event->Par1 < Plugin_091_numrelay)) {
            rnum = event->Par1;
          }

          if ((event->Par2 == 0) || (event->Par2 == 1)) {
            rcmd = event->Par2;
          }

          // LoadTaskSettings(Plugin_091_ownindex); // get our own task values please
          event->setTaskIndex(Plugin_091_ownindex);

          if (Plugin_091_globalpar0 < SER_SWITCH_LCTECH) {
            par3 = Plugin_091_globalpar1;
          }

          sendmcucommand(rnum, rcmd, Plugin_091_globalpar0, par3);                       // init state
          delay(event->Par3);
          sendmcucommand(rnum, !rcmd, Plugin_091_globalpar0, par3);                      // invert state

          if (Plugin_091_globalpar0 > SER_SWITCH_YEWE) {                                 // report state only if not Yewe
            if (UserVar[(event->BaseVarIndex + rnum)] != Plugin_091_switchstate[rnum]) { // report only if state is really changed
              UserVar.setFloat(event->TaskIndex, rnum, Plugin_091_switchstate[rnum]);

              if ((par3 == 1) && (rcmd == 1) && (rnum < 2))
              { // exclusive on mode for Dual
                // FIXME TD-er: Is this a valid UserVar index?
                UserVar.setFloat(event->TaskIndex, 1 - rnum, 0);
              }

              if (par3 == 2) { // simultaneous mode for Dual
                // FIXME TD-er: Is this a valid UserVar index?
                UserVar.setFloat(event->TaskIndex, 1 - rnum, Plugin_091_switchstate[1 - rnum]);
              }

              if (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER) {
                UserVar.setFloat(event->TaskIndex, 1, Plugin_091_switchstate[1]);
              }
              event->sensorType = Plugin_091_type;
              sendData(event);
            }
          }

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, strformat(F("SerSW   : SetSwitchPulse r%d:%d Pulsed for %d mS"), rnum, rcmd, event->Par3));
          }
        } else

        if (equals(command, F("relaylongpulse")))
        {
          success = true;

          if ((event->Par1 >= 0) && (event->Par1 < Plugin_091_numrelay)) {
            rnum = event->Par1;
          }

          if ((event->Par2 == 0) || (event->Par2 == 1)) {
            rcmd = event->Par2;
          }

          // LoadTaskSettings(Plugin_091_ownindex); // get our own task values please
          event->setTaskIndex(Plugin_091_ownindex);

          if (Plugin_091_globalpar0 < SER_SWITCH_LCTECH) {
            par3 = Plugin_091_globalpar1;
          }
          unsigned long timer = event->Par3 * 1000;

          sendmcucommand(rnum, rcmd, Plugin_091_globalpar0, par3); // init state
          // Scheduler.setPluginTimer(timer, PLUGIN_ID_091, rnum, !rcmd);
          Scheduler.setPluginTaskTimer(timer, event->TaskIndex, rnum, !rcmd);

          if (Plugin_091_globalpar0 > SER_SWITCH_YEWE) {                                 // report state only if not Yewe
            if (UserVar[(event->BaseVarIndex + rnum)] != Plugin_091_switchstate[rnum]) { // report only if state is really changed
              UserVar.setFloat(event->TaskIndex, rnum, Plugin_091_switchstate[rnum]);

              if ((par3 == 1) && (rcmd == 1) && (rnum < 2))
              { // exclusive on mode for Dual
                // FIXME TD-er: Is this a valid UserVar index?
                UserVar.setFloat(event->TaskIndex, 1 - rnum, 0);
              }

              if (par3 == 2) { // simultaneous mode for Dual
                // FIXME TD-er: Is this a valid UserVar index?
                UserVar.setFloat(event->TaskIndex, 1 - rnum, Plugin_091_switchstate[1 - rnum]);
              }

              if (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER) {
                UserVar.setFloat(event->TaskIndex, 1, Plugin_091_switchstate[1]);
              }
              event->sensorType = Plugin_091_type;
              sendData(event);
            }
          }

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, strformat(F("SerSW   : SetSwitchPulse r%d:%d Pulse for %d sec"), rnum, rcmd, event->Par3));
          }
        } else

        // deal with dimmer command
        if (equals(command, F("ydim")))
        {
          // only on tuya dimmer
          if (((Plugin_091_globalpar0 == SER_SWITCH_YEWE) && (Plugin_091_numrelay > 1)) || (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER)) {
            success = true;

            // LoadTaskSettings(Plugin_091_ownindex); // get our own task values please
            event->setTaskIndex(Plugin_091_ownindex);

            sendmcudim(event->Par1, Plugin_091_globalpar0);

            if (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER) {
              if (Plugin_091_switchstate[1] < 1) // follow state
              {
                UserVar.setFloat(event->TaskIndex, 0, 0);
                UserVar.setFloat(event->TaskIndex, 1, Plugin_091_switchstate[1]);
              } else {
                UserVar.setFloat(event->TaskIndex, 0, 1);
                UserVar.setFloat(event->TaskIndex, 1, Plugin_091_switchstate[1]);
              }
              event->sensorType = Plugin_091_type;
              sendData(event);
            }

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              addLogMove(LOG_LEVEL_INFO, concat(F("SerSW   : SetDim "), event->Par1));
            }
          } else {
            SendStatus(event, F("\nYDim not supported"));
          }
        }
      }

      break;
    }

    case PLUGIN_TASKTIMER_IN:
    {
      uint8_t par3 = 0;

      // LoadTaskSettings(Plugin_091_ownindex); // get our own task values please
      event->setTaskIndex(Plugin_091_ownindex);

      if (Plugin_091_globalpar0 < SER_SWITCH_LCTECH) {
        par3 = Plugin_091_globalpar1;
      }

      uint8_t rnum = event->Par1;
      uint8_t rcmd = event->Par2;

      sendmcucommand(rnum, rcmd, Plugin_091_globalpar0, par3);                       // invert state

      if (Plugin_091_globalpar0 > SER_SWITCH_YEWE) {                                 // report state only if not Yewe
        if (UserVar[(event->BaseVarIndex + rnum)] != Plugin_091_switchstate[rnum]) { // report only if state is really changed
          UserVar.setFloat(event->TaskIndex, rnum, Plugin_091_switchstate[rnum]);

          if ((par3 == 1) && (rcmd == 1) && (rnum < 2))
          { // exclusive on mode for Dual
            // FIXME TD-er: Is this a valid UserVar index?
            UserVar.setFloat(event->TaskIndex, 1 - rnum, 0);
          }

          if (par3 == 2) { // simultaneous mode for Dual
            // FIXME TD-er: Is this a valid UserVar index?
            UserVar.setFloat(event->TaskIndex, 1 - rnum, Plugin_091_switchstate[1 - rnum]);
          }

          if (Plugin_091_globalpar0 == SER_SWITCH_WIFIDIMMER) {
            UserVar.setFloat(event->TaskIndex, 1, Plugin_091_switchstate[1]);
          }
          event->sensorType = Plugin_091_type;
          sendData(event);
        }
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, strformat(F("SerSW   : SetSwitchPulse r%d:%d Pulse ended"), rnum, rcmd));
      }

      break;
    }
  }
  return success;
}

void getmcustate() {
  const uint8_t msg[] = {
    0x55, // Tuya header 55AA
    0xAA,
    0x00, // version 00
    0x08, // Tuya command 08 - request status
    0x00,
    0x00,
    0x07 };

  ESPEASY_SERIAL_0.write(msg, NR_ELEMENTS(msg));
  ESPEASY_SERIAL_0.flush();
}

void sendmcucommand(uint8_t btnnum, uint8_t state, uint8_t swtype, uint8_t btnum_mode) // btnnum=0,1,2, state=0/1
{
  uint8_t sstate;

  switch (swtype)
  {
    case SER_SWITCH_YEWE:
    {
      const uint8_t msg[] = {
        0x55,                                        // Tuya header 55AA
        0xAA,
        0x00,                                        // version 00
        0x06,                                        // Tuya command 06 - send order
        0x00,
        0x05,                                        // following data length 0x05
        static_cast<uint8_t>(btnnum + 1),            // relay number 1,2,3
        0x01,                                        // ?
        0x00,                                        // ?
        0x01,                                        // ?
        state,                                       // status
        static_cast<uint8_t>(13 + btnnum + state) }; // checksum:sum of all bytes in packet mod 256

      ESPEASY_SERIAL_0.write(msg, NR_ELEMENTS(msg));
      ESPEASY_SERIAL_0.flush();
      break;
    }
    case SER_SWITCH_SONOFFDUAL:
    {
      Plugin_091_switchstate[btnnum] = state;

      if ((btnum_mode == 1) && (state == 1) && (btnnum < 2))
      { // exclusive on mode
        Plugin_091_switchstate[(1 - btnnum)] = 0;
      }

      if (btnum_mode == 2)
      { // simultaneous mode
        Plugin_091_switchstate[0] = state;
        Plugin_091_switchstate[1] = state;
      }
      sstate = Plugin_091_switchstate[0] + (Plugin_091_switchstate[1] << 1) + (Plugin_091_switchstate[2] << 2);
      const uint8_t msg[] = {
        0xA0,
        0x04,
        sstate,
        0xA1 };
      ESPEASY_SERIAL_0.write(msg, NR_ELEMENTS(msg));
      ESPEASY_SERIAL_0.flush();
      break;
    }
    case SER_SWITCH_LCTECH:
    {
      uint8_t c_d = 1;

      if (Plugin_091_cmddbl) {
        c_d = 2;
      }
      Plugin_091_switchstate[btnnum] = state;

      for (uint8_t x = 0; x < c_d; ++x) // try twice to be sure
      {
        if (x > 0) {
          delay(1);
        }

        if (Plugin_091_ipd) {
          const uint8_t msg[] = {
            0x0D,
            0x0A,
            0x2B,
            0x49,
            0x50,
            0x44,
            0x2C,
            0x30,
            0x2C,
            0x34,
            0x3A };
          ESPEASY_SERIAL_0.write(msg, NR_ELEMENTS(msg));
        }
        const uint8_t msg[] = {
          0xA0,
          static_cast<uint8_t>(0x01 + btnnum),
          static_cast<uint8_t>(0x00 + state),
          static_cast<uint8_t>(0xA1 + state + btnnum) };
        ESPEASY_SERIAL_0.write(msg, NR_ELEMENTS(msg));
        ESPEASY_SERIAL_0.flush();
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

void sendmcudim(uint8_t dimvalue, uint8_t swtype)
{
  switch (swtype)
  {
    case SER_SWITCH_YEWE:
    {
      const uint8_t msg[] = {
        0x55,                                                        // Tuya header 55AA
        0xAA,
        0x00,                                                        // version 00
        0x06,                                                        // Tuya command 06 - send order
        0x00,
        0x08,                                                        // following data length 0x08
        Plugin_091_numrelay,                                         // dimmer order-id? select it at plugin settings 2/3!!!
        0x02,                                                        // type=value
        0x00,                                                        // length hi
        0x04,                                                        // length low
        0x00,                                                        // ?
        0x00,                                                        // ?
        0x00,                                                        // ?
        dimvalue,                                                    // dim value (0-255)
        static_cast<uint8_t>(19 + Plugin_091_numrelay + dimvalue) }; // checksum:sum of all bytes in packet mod 256
      ESPEASY_SERIAL_0.write(msg, NR_ELEMENTS(msg));
      ESPEASY_SERIAL_0.flush();
      break;
    }
    case SER_SWITCH_WIFIDIMMER:
    {
      const uint8_t msg[] = {
        0xFF,     // Wifidimmer header FF55
        0x55,
        dimvalue, // dim value (0-255)
        0x05,
        0xDC,
        0x0A };
      ESPEASY_SERIAL_0.write(msg, NR_ELEMENTS(msg));
      ESPEASY_SERIAL_0.flush();
      Plugin_091_switchstate[1] = dimvalue;
      break;
    }
  }
}

#endif // USES_P091

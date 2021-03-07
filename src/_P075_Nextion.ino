#include "_Plugin_Helper.h"
#ifdef USES_P075


//#######################################################################################################
//#######################################################################################################
//################################### Plugin 075: Nextion <info@sensorio.cz>  ###########################
//###################################   Created on the work of  majklovec     ###########################
//###################################    Revisions by BertB and ThomasB       ###########################
//###################################    Last Revision: Oct-03-2018 (TB)      ###########################
//#######################################################################################################
//
// Updated: Oct-03-2018, ThomasB.
// Added DEBUG_LOG define to reduce info log messages and prevent serial log flooding.
// Added SendStatus() to post log message on browser to acknowledge HTTP write.
// Added reserve() to minimize string memory allocations.
//

#include <ESPeasySerial.h>


// *****************************************************************************************************
// Defines start here
// *****************************************************************************************************

//#define DEBUG_LOG             // Enable this to include additional info messages in log output.


// Plug-In defines
#define PLUGIN_075
#define PLUGIN_ID_075 75
#define PLUGIN_NAME_075 "Display - Nextion [TESTING]"
#define PLUGIN_DEFAULT_NAME "NEXTION"
#define PLUGIN_VALUENAME1_075 "idx"
#define PLUGIN_VALUENAME2_075 "value"

// Configuration Settings. Custom Configuration Memory must be less than 1024 Bytes (per TD'er findings).
//#define P75_Nlines 12             // Custom Config, Number of user entered Command Statment Lines. DO NOT USE!
//#define P75_Nchars 64           // Custom Config, Length of user entered Command Statment Lines. DO NOT USE!
#define P75_Nlines 10             // Custom Config, Number of user entered Command Statments.
#define P75_Nchars 51             // Custom Config, Length of user entered Command Statments.

// Nextion defines
#define RXBUFFSZ  64              // Serial RxD buffer (Local staging buffer and ESPeasySerial).
#define RXBUFFWARN (RXBUFFSZ-16)  // Warning, Rx buffer close to being full.
#define TOUCH_BASE 500            // Base offset for 0X65 Touch Event Send Component ID.

// Serial defines
#define P075_B9600    0
#define P075_B38400   1
#define P075_B57600   2
#define P075_B115200  3

#define P075_BaudRate         PCONFIG(1)
#define P075_IncludeValues    PCONFIG(2)   // Web GUI checkbox flag; false = don't send idx & value data at interval.


struct P075_data_struct : public PluginTaskData_base {

  P075_data_struct(ESPEasySerialPort port, int rx, int tx, uint32_t baud) : rxPin(rx), txPin(tx), baudrate(baud) {
    if (baudrate < 9600 || baudrate > 115200) {
      baudrate = 9600;
    }
    easySerial = new (std::nothrow) ESPeasySerial(port, rx, tx, false, RXBUFFSZ);
    if (easySerial != nullptr) {
      easySerial->begin(baudrate);
      easySerial->flush();
    }
  }

  ~P075_data_struct() {
    if (easySerial != nullptr) {
      easySerial->flush();
      delete easySerial;
      easySerial = nullptr;
    }
  }

  void loadDisplayLines(taskIndex_t taskIndex) {
    LoadCustomTaskSettings(taskIndex, displayLines, P75_Nlines, P75_Nchars);
  }

  String getLogString() const {
    String result;
    if (easySerial != nullptr) {
      result = easySerial->getLogString();
    }
    return result;
  }

  ESPeasySerial *easySerial = nullptr;
  int rxPin = -1;
  int txPin = -1;
  uint32_t baudrate = 9600UL;

  String displayLines[P75_Nlines];
};


// *****************************************************************************************************
// PlugIn starts here
// *****************************************************************************************************

boolean Plugin_075(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
//  static boolean AdvHwSerial = false;                   // Web GUI checkbox flag; false = softserial mode, true = hardware UART serial.

  switch (function) {

    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number = PLUGIN_ID_075;
      Device[deviceCount].Type = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_DUAL;
      Device[deviceCount].Ports = 0;
      Device[deviceCount].PullUpOption = false;         // Pullup is not used.
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption = false;
      Device[deviceCount].ValueCount = 2;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption = true;
      Device[deviceCount].TimerOptional = true;         // Allow user to disable interval function.
      Device[deviceCount].GlobalSyncOption = true;
      break;
    }


    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_075);
      break;
    }


    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0],PSTR(PLUGIN_VALUENAME1_075));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1],PSTR(PLUGIN_VALUENAME2_075));
      break;
    }


    case PLUGIN_GET_DEVICEGPIONAMES: {
//      AdvHwSerial = PCONFIG(0);
      serialHelper_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
      {
        string += serialHelper_getSerialTypeLabel(event);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      String options[4];
      options[0] = F("9600");
      options[1] = F("38400");
      options[2] = F("57600");
      options[3] = F("115200");

      addFormSelector(F("Baud Rate"), F("p075_baud"), 4, options, nullptr, P075_BaudRate);
      addUnit(F("baud"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {

//    ** DEVELOPER DEBUG MESSAGE AREA **
//    int datax = (int)(Settings.TaskDeviceEnabled[event->TaskIndex]); // Debug value.
//    String Data = "Debug. Plugin Enable State: ";
//    Data += String(datax);
//    addFormNote(Data);

      addFormSubHeader("");                          // Blank line, vertical space.
      addFormHeader(F("Nextion Command Statements (Optional)"));
      P075_data_struct* P075_data = static_cast<P075_data_struct*>(getPluginTaskData(event->TaskIndex));
      if (nullptr != P075_data) {
        P075_data->loadDisplayLines(event->TaskIndex);
        for (byte varNr = 0; varNr < P75_Nlines; varNr++) {
          addFormTextBox(String(F("Line ")) + (varNr + 1), getPluginCustomArgName(varNr), P075_data->displayLines[varNr], P75_Nchars-1);
        }
      }
      if( Settings.TaskDeviceTimer[event->TaskIndex]==0) { // Is interval timer disabled?
        if(P075_IncludeValues) {
            addFormNote(F("Interval Timer OFF, Nextion Lines (above) and Values (below) <b>NOT</b> scheduled for updates"));
        }
        else {
            addFormNote(F("Interval Timer OFF, Nextion Lines (above) <b>NOT</b> scheduled for updates"));
        }
      }

      addFormSeparator(2);
      addFormSubHeader(F("Interval Options"));
      addFormCheckBox(F("Resend <b>Values</b> (below) at Interval"), F("IncludeValues"), P075_IncludeValues);

      success = true;
      break;
    }


    case PLUGIN_WEBFORM_SAVE: {

        {
          // FIXME TD-er: This is a huge object allocated on the Stack.
          char deviceTemplate[P75_Nlines][P75_Nchars];
          String error;
          for (byte varNr = 0; varNr < P75_Nlines; varNr++)
          {
            if (!safe_strncpy(deviceTemplate[varNr], web_server.arg(getPluginCustomArgName(varNr)), P75_Nchars)) {
              error += getCustomTaskSettingsError(varNr);
            }
          }
          if (error.length() > 0) {
            addHtmlError(error);
          }
          SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        }

        if(getTaskDeviceName(event->TaskIndex) == "") {         // Check to see if user entered device name.
            strcpy(ExtraTaskSettings.TaskDeviceName,PLUGIN_DEFAULT_NAME); // Name missing, populate default name.
        }
//        PCONFIG(0) = isFormItemChecked(F("AdvHwSerial"));
        P075_BaudRate = getFormItemInt(F("p075_baud"));
        P075_IncludeValues = isFormItemChecked(F("IncludeValues"));
        P075_data_struct* P075_data = static_cast<P075_data_struct*>(getPluginTaskData(event->TaskIndex));
        if (nullptr != P075_data) {
          P075_data->loadDisplayLines(event->TaskIndex);
        }
        success = true;
        break;
    }


    case PLUGIN_INIT: {
//      AdvHwSerial   = PCONFIG(0);
      uint8_t BaudCode = P075_BaudRate;

      if(BaudCode > P075_B115200) BaudCode = P075_B9600;
      const uint32_t BaudArray[4] = {9600UL, 38400UL, 57600UL, 115200UL};
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P075_data_struct(port, CONFIG_PIN1, CONFIG_PIN2, BaudArray[BaudCode]));
      P075_data_struct* P075_data = static_cast<P075_data_struct*>(getPluginTaskData(event->TaskIndex));
      if (nullptr != P075_data) {
        P075_data->loadDisplayLines(event->TaskIndex);
        addLog(LOG_LEVEL_INFO, P075_data->getLogString());
        success = true;
      }
      break;
    }


    case PLUGIN_READ: {
      P075_data_struct* P075_data = static_cast<P075_data_struct*>(getPluginTaskData(event->TaskIndex));
      if (nullptr != P075_data) {
        int RssiIndex;
        String newString;
        String UcTmpString;

        // Get optional LINE command statements. Special RSSIBAR bargraph keyword is supported.
        for (byte x = 0; x < P75_Nlines; x++) {
          if (P075_data->displayLines[x].length()) {
            String tmpString = P075_data->displayLines[x];
            UcTmpString = P075_data->displayLines[x];
            UcTmpString.toUpperCase();
            RssiIndex = UcTmpString.indexOf(F("RSSIBAR"));  // RSSI bargraph Keyword found, wifi value in dBm.
            if(RssiIndex >= 0) {
              int barVal=0;
              newString.reserve(P75_Nchars+10);               // Prevent re-allocation
              newString = P075_data->displayLines[x].substring(0, RssiIndex);
              int nbars = WiFi.RSSI();
              if (nbars < -100 || nbars >= 0)
                 barVal=0;
              else if (nbars >= -100 && nbars < -95)
                 barVal=5;
              else if (nbars >= -95 && nbars < -90)
                 barVal=10;
              else if (nbars >= -90 && nbars < -85)
                 barVal=20;
              else if (nbars >= -85 && nbars < -80)
                 barVal=30;
              else if (nbars >= -80 && nbars < -75)
                 barVal=45;
              else if (nbars >= -75 && nbars < -70)
                 barVal=60;
              else if (nbars >= -70 && nbars < -65)
                 barVal=70;
              else if (nbars >= -65 && nbars < -55)
                 barVal=80;
              else if (nbars >= -55 && nbars < -50)
                 barVal=90;
              else if (nbars >= -50)
                 barVal=100;

              newString += String(barVal,DEC);
            }
            else {
              newString = parseTemplate(tmpString);
            }

            P075_sendCommand(event->TaskIndex, newString.c_str());
            #ifdef DEBUG_LOG
              String log;
              log.reserve(P75_Nchars+50);                 // Prevent re-allocation
              log = F("NEXTION075 : Cmd Statement Line-");
              log += String(x+1);
              log += F(" Sent: ");
              log += newString;
              addLog(LOG_LEVEL_INFO, log);
            #endif
          }
        }

        // At Interval timer, send idx & value data only if user enabled "values" interval mode.
        if(P075_IncludeValues) {
            #ifdef DEBUG_LOG
             String log;
             log.reserve(120);                          // Prevent re-allocation
             log = F("NEXTION075: Interval values data enabled, resending idx=");
             log += formatUserVarNoCheck(event->TaskIndex, 0);
             log += F(", value=");
             log += formatUserVarNoCheck(event->TaskIndex, 1);
             addLog(LOG_LEVEL_INFO, log);
            #endif

            success = true;
        }
        else {
            #ifdef DEBUG_LOG
             String log = F("NEXTION075: Interval values data disabled, idx & value not resent");
             addLog(LOG_LEVEL_INFO, log);
            #endif

            success = false;
        }

      }
      break;
    }

// Nextion commands received from events (including http) get processed here. PLUGIN_WRITE
// does NOT process publish commands that are sent.
    case PLUGIN_WRITE: {
      String command = parseString(string, 1);
      // If device names match we have a command to write.
      if (command.equalsIgnoreCase(getTaskDeviceName(event->TaskIndex))) {
        success = true; // Set true only if plugin found a command to execute.
        String nextionArguments = parseStringToEndKeepCase(string, 2);
        P075_sendCommand(event->TaskIndex, nextionArguments.c_str());
        {
          String log;
          log.reserve(24 + nextionArguments.length()); // Prevent re-allocation
          log = F("NEXTION075 : WRITE = ");
          log += nextionArguments;
          addLog(LOG_LEVEL_DEBUG, log);
          SendStatus(event, log);              // Reply (echo) to sender. This will print message on browser.
        }

// Enable addLog() code below to help debug plugin write problems.
/*
        String log;
        log.reserve(140);                               // Prevent re-allocation
        String log = F("Nextion arg0: ");
        log += command;
        log += F(", TaskDeviceName: ");
        log += getTaskDeviceName(event->TaskIndex);
        log += F(", event->TaskIndex: ");
        log += String(event->TaskIndex);
        log += F(", nextionArguments: ");
        log += nextionArguments;
        addLog(LOG_LEVEL_INFO, log);
*/
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND: {
        success = true;
        break;
    }


    case PLUGIN_TEN_PER_SECOND: {
      P075_data_struct* P075_data = static_cast<P075_data_struct*>(getPluginTaskData(event->TaskIndex));
      if (nullptr == P075_data) {
        break;
      }
      if(P075_data->rxPin < 0) {
        String log = F("NEXTION075 : Missing RxD Pin, aborted serial receive");
        addLog(LOG_LEVEL_INFO, log);
        break;
      }
      if(P075_data->easySerial == nullptr) break;                   // P075_data->easySerial missing, exit.

      {
        uint16_t i;
        uint8_t c;
        String Vidx;
        String Nvalue;
        String Svalue;
        String Nswitch;
        char __buffer[RXBUFFSZ+1];                        // Staging buffer.
        uint8_t charCount = P075_data->easySerial->available();            // Prime the Soft Serial engine.
        if(charCount >= RXBUFFWARN) {
            String log;
            log.reserve(70);                           // Prevent re-allocation
            log = F("NEXTION075 : RxD P075_data->easySerial Buffer capacity warning, ");
            log += String(charCount);
            log += F(" bytes");
            addLog(LOG_LEVEL_INFO, log);
        }
        uint32_t baudrate_delay_unit = P075_data->baudrate / 9600;
        if (baudrate_delay_unit == 0) {
          baudrate_delay_unit = 1;
        }

        while (charCount) {                               // This is the serial engine. It processes the serial Rx stream.
          c = P075_data->easySerial->read();

          if (c == 0x65) {
            if (charCount < 6) delay((5/(baudrate_delay_unit))+1); // Let's wait for a few more chars to arrive.

            charCount = P075_data->easySerial->available();
            if (charCount >= 6) {
              __buffer[0] = c;                            // Store in staging buffer.
              for (i = 1; i < 7; i++) {
                  __buffer[i] = P075_data->easySerial->read();
              }

              __buffer[i] = 0x00;

              if (0xFF == __buffer[4] && 0xFF == __buffer[5] && 0xFF == __buffer[6]) {
                UserVar[event->BaseVarIndex] = (__buffer[1] * 256) + __buffer[2] + TOUCH_BASE;
                UserVar[event->BaseVarIndex + 1] = __buffer[3];
                sendData(event);

                #ifdef DEBUG_LOG
                  String log;
                  log.reserve(70);                        // Prevent re-allocation
                  log = F("NEXTION075 : code: ");
                  log += __buffer[1];
                  log += ",";
                  log += __buffer[2];
                  log += ",";
                  log += __buffer[3];
                  addLog(LOG_LEVEL_INFO, log);
                #endif
              }
            }
          }
          else {
            if (c == '|') {
              __buffer[0] = c;                                  // Store in staging buffer.

              if (charCount < 8) delay((9/(baudrate_delay_unit))+1); // Let's wait for more chars to arrive.
              else delay((3/(baudrate_delay_unit))+1);               // Short wait for tardy chars.
              charCount = P075_data->easySerial->available();

              i = 1;
              while (P075_data->easySerial->available() > 0 && i<RXBUFFSZ) { // Copy global serial buffer to local buffer.
                __buffer[i] = P075_data->easySerial->read();
                if (__buffer[i]==0x0a || __buffer[i]==0x0d) break;
                i++;
              }

              __buffer[i] = 0x00;

              String tmpString = __buffer;

              #ifdef DEBUG_LOG
                String log;
                log.reserve(50);                          // Prevent re-allocation
                log = F("NEXTION075 : Code = ");
                log += tmpString;
                addLog(LOG_LEVEL_INFO, log);
              #endif

              int argIndex = tmpString.indexOf(F(",i"));
              int argEnd = tmpString.indexOf(',', argIndex + 1);
              if (argIndex) Vidx = tmpString.substring(argIndex + 2,argEnd);

              boolean GotPipeCmd = false;
              switch (__buffer[1]){
                case 'u':
                  GotPipeCmd = true;
                  argIndex = argEnd;
                  argEnd = tmpString.indexOf(',',argIndex + 1);
                  if (argIndex) Nvalue = tmpString.substring(argIndex + 2,argEnd);
                  argIndex = argEnd;
                  argEnd = tmpString.indexOf(0x0a);
                  if (argIndex) Svalue = tmpString.substring(argIndex + 2,argEnd);
                  break;
                case 's':
                  GotPipeCmd = true;
                  argIndex = argEnd;
                  argEnd = tmpString.indexOf(0x0a);
                  if (argIndex) Nvalue = tmpString.substring(argIndex + 2,argEnd);
                  if (Nvalue == F("On")) Svalue='1';
                  if (Nvalue == F("Off")) Svalue='0';
                  break;
              }

              if (GotPipeCmd) {
                  UserVar[event->BaseVarIndex] = 0.0f;
                  UserVar[event->BaseVarIndex+1] = 0.0f;
                  validFloatFromString(Vidx, UserVar[event->BaseVarIndex]);
                  validFloatFromString(Svalue, UserVar[event->BaseVarIndex+1]);
                  sendData(event);

                  #ifdef DEBUG_LOG
                  String log;
                  log.reserve(80);                       // Prevent re-allocation
                  log = F("NEXTION075 : Pipe Command Sent: ");
                  log += __buffer;
                  log += formatUserVarNoCheck(event->TaskIndex, 0);
                  addLog(LOG_LEVEL_INFO, log);
                  #endif
              }
              else {
                  #ifdef DEBUG_LOG
                  String log = F("NEXTION075 : Unknown Pipe Command, skipped");
                  addLog(LOG_LEVEL_INFO, log);
                  #endif
              }
            }
          }
          charCount = P075_data->easySerial->available();
        }
      }

      success = true;
      break;
    }
  }
  return success;
}


void P075_sendCommand(taskIndex_t taskIndex, const char *cmd)
{
  P075_data_struct* P075_data = static_cast<P075_data_struct*>(getPluginTaskData(taskIndex));
  if (!P075_data) return;
  if (P075_data->txPin < 0) {
      String log = F("NEXTION075 : Missing TxD Pin Number, aborted sendCommand");
      addLog(LOG_LEVEL_INFO, log);
  }
  else
  {
      if (P075_data->easySerial != nullptr){
          P075_data->easySerial->print(cmd);
          P075_data->easySerial->write(0xff);
          P075_data->easySerial->write(0xff);
          P075_data->easySerial->write(0xff);
      }
      else {
          String log = F("NEXTION075 : P075_data->easySerial error, aborted sendCommand");
          addLog(LOG_LEVEL_INFO, log);
      }
  }
}

#endif // USES_P075

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
// Aug-03-2018 change log summary:
// This updated Nextion Plugin introduces hardware (UART) serial as an alternative to SoftSerial.
// Hardware serial offers more reliable communication with the Nextion display.
// Created on the experimental Serial.swap() work of BertB.
//

#ifdef USES_P075
#include <ESPeasySoftwareSerial.h>

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
#define P75_Nlines 10               // Custom Config, Number of user entered Command Statments.
#define P75_Nchars 51             // Custom Config, Length of user entered Command Statments.

// Nextion defines
#define RXBUFFSZ  64            // Serial RxD buffer (Local staging buffer and ESPeasySoftwareSerial).
#define RXBUFFWARN RXBUFFSZ-16  // Warning, Rx buffer close to being full.
#define TOUCH_BASE 500          // Base offset for 0X65 Touch Event Send Component ID.

// Serial defines
#define B9600    0
#define B38400   1
#define B57600   2
#define B115200  3
#define DEFAULT_BAUD B9600
#define SOFTSERIAL 0
#define UARTSERIAL 1


// Global vars
ESPeasySoftwareSerial *SoftSerial = NULL;
int rxPin = -1;
int txPin = -1;

String P075_displayLines[P75_Nlines];

void Plugin_075_loadDisplayLines(byte taskIndex) {
  char P075_deviceTemplate[P75_Nlines][P75_Nchars];
  LoadCustomTaskSettings(taskIndex, (byte*)&P075_deviceTemplate, sizeof(P075_deviceTemplate));
  for (byte varNr = 0; varNr < P75_Nlines; varNr++) {
    P075_displayLines[varNr] = P075_deviceTemplate[varNr];
  }
}

// *****************************************************************************************************
// PlugIn starts here
// *****************************************************************************************************

boolean Plugin_075(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static uint8_t BaudCode = 0;                          // Web GUI baud rate drop down. 9600 if 0, See array for other rates.
  static boolean HwSerial = SOFTSERIAL;                 // Serial mode, hardware uart or softserial.
  static boolean AdvHwSerial = false;                   // Web GUI checkbox flag; false = softserial mode, true = hardware UART serial.
  static boolean IncludeValues = false;                 // Web GUI checkbox flag; false = don't send idx & value data at interval.
  uint32_t AdvHwBaud = 9600UL;

  switch (function) {

    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number = PLUGIN_ID_075;
      Device[deviceCount].Type = DEVICE_TYPE_DUAL;
      Device[deviceCount].VType = SENSOR_TYPE_DUAL;
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

      AdvHwSerial = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
      rxPin = Settings.TaskDevicePin1[event->TaskIndex];
      txPin = Settings.TaskDevicePin2[event->TaskIndex];

      event->String1 = formatGpioName_RX(false);
      event->String2 = formatGpioName_TX(false);

      if(AdvHwSerial == true) {
        if ((rxPin == 3 && txPin == 1) || (rxPin == 13 && txPin == 15)) {
          event->String1 = formatGpioName_RX_HW(false);
          event->String2 = formatGpioName_TX_HW(false);
        }
      }
      break;
    }


    case PLUGIN_WEBFORM_LOAD: {
      rxPin = Settings.TaskDevicePin1[event->TaskIndex];
      txPin = Settings.TaskDevicePin2[event->TaskIndex];

      if (!((rxPin == 3 && txPin == 1) || (rxPin == 13 && txPin == 15))) { // Hardware Serial Compatible?
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = false;      // Not HW serial compatible, Reset Check Box.
      }

      if (rxPin == 3 && txPin == 1) {                                      // UART USB Port?
        if(Settings.TaskDevicePluginConfig[event->TaskIndex][0]==false &&  // Hardware serial currently disabled.
         Settings.TaskDeviceEnabled[event->TaskIndex] == true) {           // Plugin is enabled.
            Settings.TaskDevicePluginConfig[event->TaskIndex][0]=true;     // USB port access uses HW serial, Force set Check Box.
        }
      }

      if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == false) { // SoftSerial mode.
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = B9600;      // Reset to 9600 baud.
      }

      if(rxPin <0 || txPin <0) {                                            // Missing serial I/O pins!
        addFormNote(F("Please configure the RX and TX sensor pins before enabling this plugin."));
        addFormSubHeader(""); // Blank line, vertical space.
      }

      addFormSeparator(2);
      addFormSubHeader(F("Enhanced Serial Communication"));
      addFormCheckBox(F("Use Hardware Serial"), F("AdvHwSerial"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

      byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
      String options[4];
      options[0] = F("9600");
      options[1] = F("38400");
      options[2] = F("57600");
      options[3] = F("115200");

      addFormSelector(F("Baud Rate"), F("p075_baud"), 4, options, NULL, choice);
      addFormNote(F("Un-check box for Soft Serial communication (low performance mode, 9600 Baud)."));
      addFormNote(F("Hardware Serial is available when the GPIO pins are RX=D7 and TX=D8."));
      addFormNote(F("D8 (GPIO-15) requires a Buffer Circuit (PNP transistor) or ESP boot may fail."));
      addFormNote(F("Do <b>NOT</b> enable the Serial Log file on Tools->Advanced->Serial Port."));

//    ** DEVELOPER DEBUG MESSAGE AREA **
//    int datax = (int)(Settings.TaskDeviceEnabled[event->TaskIndex]); // Debug value.
//    String Data = "Debug. Plugin Enable State: ";
//    Data += String(datax);
//    addFormNote(Data);

      addFormSubHeader("");                          // Blank line, vertical space.
      addFormHeader(F("Nextion Command Statements (Optional)"));

      Plugin_075_loadDisplayLines(event->TaskIndex);
      for (byte varNr = 0; varNr < P75_Nlines; varNr++) {
        addFormTextBox(String(F("Line ")) + (varNr + 1), String(F("p075_template")) + (varNr + 1), P075_displayLines[varNr], P75_Nchars-1);
      }
      if( Settings.TaskDeviceTimer[event->TaskIndex]==0) { // Is interval timer disabled?
        if(IncludeValues) {
            addFormNote(F("Interval Timer OFF, Nextion Lines (above) and Values (below) <b>NOT</b> scheduled for updates"));
        }
        else {
            addFormNote(F("Interval Timer OFF, Nextion Lines (above) <b>NOT</b> scheduled for updates"));
        }
      }

      addFormSeparator(2);
      addFormSubHeader(F("Interval Options"));
      addFormCheckBox(F("Resend <b>Values</b> (below) at Interval"), F("IncludeValues"), Settings.TaskDevicePluginConfig[event->TaskIndex][2]);

      success = true;
      break;
    }


    case PLUGIN_WEBFORM_SAVE: {
        char deviceTemplate[P75_Nlines][P75_Nchars];
        String error;
        for (byte varNr = 0; varNr < P75_Nlines; varNr++)
        {
          String argName = F("p075_template");
          argName += varNr + 1;
          if (!safe_strncpy(deviceTemplate[varNr], WebServer.arg(argName), P75_Nchars)) {
            error += getCustomTaskSettingsError(varNr);
          }
        }
        if (error.length() > 0) {
          addHtmlError(error);
        }
        if(getTaskDeviceName(event->TaskIndex) == "") {         // Check to see if user entered device name.
            strcpy(ExtraTaskSettings.TaskDeviceName,PLUGIN_DEFAULT_NAME); // Name missing, populate default name.
        }
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = isFormItemChecked(F("AdvHwSerial"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("p075_baud"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = isFormItemChecked(F("IncludeValues"));
        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        Plugin_075_loadDisplayLines(event->TaskIndex);
        success = true;
        break;
    }


    case PLUGIN_INIT: {

      AdvHwSerial   = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
      BaudCode      = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
      IncludeValues = Settings.TaskDevicePluginConfig[event->TaskIndex][2];

      if(BaudCode > B115200) BaudCode = B9600;
      const uint32_t BaudArray[4] = {9600UL, 38400UL, 57600UL, 115200UL};
      AdvHwBaud = BaudArray[BaudCode];

      if (Settings.TaskDevicePin1[event->TaskIndex] != -1) {
        rxPin = Settings.TaskDevicePin1[event->TaskIndex];
      }
      if (Settings.TaskDevicePin2[event->TaskIndex] != -1) {
        txPin = Settings.TaskDevicePin2[event->TaskIndex];
      }

      if (SoftSerial != NULL) {
        delete SoftSerial;
        SoftSerial = NULL;
      }

      String log;
      log.reserve(80);                                 // Prevent re-allocation
      log = F("NEXTION075 : serial pin config RX:");
      log += rxPin;
      log += F(", TX:");
      log += txPin;
      if(Settings.TaskDeviceEnabled[event->TaskIndex]==true) {
        log += F(", Plugin Enabled");                   // Plugin is enabled.
      }
      else {
        log += F(", Plugin Disabled");
      }
      addLog(LOG_LEVEL_INFO, log);

      if(Settings.TaskDeviceEnabled[event->TaskIndex] == true) { // Plugin is enabled.
      // Hardware serial is RX on 13 and TX on 15 (swapped hw serial)
        if (AdvHwSerial &&  rxPin == 13 && txPin == 15) {
            log = F("NEXTION075 : Using swap hardware serial");
            addLog(LOG_LEVEL_INFO, log);
            HwSerial = UARTSERIAL;
            Settings.UseSerial = false;                 // Disable global Serial port.
            Settings.SerialLogLevel = 0;                // Disable logging on serial port.
            Settings.BaudRate = AdvHwBaud;              // Set BaudRate for Nextion.
            Serial.flush();
            Serial.begin(AdvHwBaud);
            Serial.swap();
        }
        // Hardware serial is RX on 3 and TX on 1. USB serial for Nextion IDE (User MCU Input function).
        else if(AdvHwSerial && rxPin == 3 && txPin == 1) {
            log = F("NEXTION075 : Using USB hardware serial");
            addLog(LOG_LEVEL_INFO, log);
            HwSerial = UARTSERIAL;
            Settings.UseSerial = false;                 // Disable global Serial port.
            Settings.SerialLogLevel = 0;                // Disable logging on serial port.
            Settings.BaudRate = AdvHwBaud;              // Set BaudRate for Nextion.
            Serial.flush();
            Serial.begin(AdvHwBaud);
        }
        else {
            log = F("NEXTION075 : Using software serial");
            addLog(LOG_LEVEL_INFO, log);
            HwSerial = SOFTSERIAL;
            if (SoftSerial == NULL) {
                SoftSerial = new ESPeasySoftwareSerial(rxPin, txPin, false, RXBUFFSZ);
            }
            SoftSerial->begin(9600);
            SoftSerial->flush();
        }
        Plugin_075_loadDisplayLines(event->TaskIndex);
    }
    else {
    }
      success = true;
      break;
    }


    case PLUGIN_READ: {
        int RssiIndex;
        String newString;
        String UcTmpString;

        // Get optional LINE command statements. Special RSSIBAR bargraph keyword is supported.
        for (byte x = 0; x < P75_Nlines; x++) {
          if (P075_displayLines[x].length()) {
            String tmpString = P075_displayLines[x];
            UcTmpString = P075_displayLines[x];
            UcTmpString.toUpperCase();
            RssiIndex = UcTmpString.indexOf(F("RSSIBAR"));  // RSSI bargraph Keyword found, wifi value in dBm.
            if(RssiIndex >= 0) {
              int barVal;
              newString.reserve(P75_Nchars+10);               // Prevent re-allocation
              newString = P075_displayLines[x].substring(0, RssiIndex);
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
              newString = parseTemplate(tmpString, 0);
            }

            sendCommand(newString.c_str(), HwSerial);
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
        if(IncludeValues) {
            #ifdef DEBUG_LOG
             String log;
             log.reserve(120);                          // Prevent re-allocation
             log = F("NEXTION075: Interval values data enabled, resending idx=");
             log += String(UserVar[event->BaseVarIndex]);
             log += F(", value=");
             log += String(UserVar[event->BaseVarIndex+1]);
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

        break;
    }

// Nextion commands received from events (including http) get processed here. PLUGIN_WRITE
// does NOT process publish commands that are sent.
    case PLUGIN_WRITE: {

        String tmpString = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex) tmpString = tmpString.substring(0, argIndex);

// Enable addLog() code below to help debug plugin write problems.
/*
        String log;
        log.reserve(140);                               // Prevent re-allocation
        String log = F("Nextion arg0: ");
        log += tmpString;
        log += F(", TaskDeviceName: ");
        log += getTaskDeviceName(event->TaskIndex);
        log += F(", event->TaskIndex: ");
        log += String(event->TaskIndex);
        log += F(", cmd str: ");
        log += string;
        addLog(LOG_LEVEL_INFO, log);
*/
        if (tmpString.equalsIgnoreCase(getTaskDeviceName(event->TaskIndex)) == true) { // If device names match we have a command to write.
            argIndex = string.indexOf(',');
            tmpString = string.substring(argIndex + 1);
            sendCommand(tmpString.c_str(), HwSerial);

            String log;
            log.reserve(110);                           // Prevent re-allocation
            log = F("NEXTION075 : WRITE = ");
            log += tmpString;
            #ifdef DEBUG_LOG
              addLog(LOG_LEVEL_INFO, log);
            #endif
            SendStatus(event->Source, log);             // Reply (echo) to sender. This will print message on browser.
            success = true;                             // Set true only if plugin found a command to execute.
        }
        break;
    }


    case PLUGIN_EXIT: {
        if (SoftSerial) {
            delete SoftSerial;
            SoftSerial=NULL;
        }

        if(HwSerial == UARTSERIAL) {
            HwSerial = SOFTSERIAL;
            Settings.UseSerial		= DEFAULT_USE_SERIAL;
            Settings.BaudRate		= DEFAULT_SERIAL_BAUD;
            Serial.flush();
            Serial.begin(DEFAULT_SERIAL_BAUD);          // Restart System Serial Port (Serial Log) with default baud.
        }
        for (byte varNr = 0; varNr < P75_Nlines; varNr++) {
          P075_displayLines[varNr] = "";
        }
        break;
    }


    case PLUGIN_ONCE_A_SECOND: {
        success = true;
        break;
    }


    case PLUGIN_TEN_PER_SECOND: {
      uint16_t i;
      uint8_t c;
      uint8_t charCount;
      String Vidx;
      String Nvalue;
      String Svalue;
      String Nswitch;
      char __buffer[RXBUFFSZ+1];                        // Staging buffer.

      if(rxPin < 0) {
        String log = F("NEXTION075 : Missing RxD Pin, aborted serial receive");
        addLog(LOG_LEVEL_INFO, log);
        break;
      }

      if(HwSerial == UARTSERIAL) {
        charCount = Serial.available();                 // Prime the Hardware Serial engine.
        if(charCount >= RXBUFFWARN) {                   // ESP8266 has 128 byte circular Rx buffer.
            String log;
            log.reserve(70);                           // Prevent re-allocation
            log = F("NEXTION075 : RxD UART Buffer capacity warning,");
            log += String(charCount);
            log += F(" bytes");
            addLog(LOG_LEVEL_INFO, log);
        }
      }
      else {
        if(SoftSerial == NULL) break;                   // SoftSerial missing, exit.
        charCount = SoftSerial->available();            // Prime the Soft Serial engine.
        if(charCount >= RXBUFFWARN) {
            String log;
            log.reserve(70);                           // Prevent re-allocation
            log = F("NEXTION075 : RxD SoftSerial Buffer capacity warning, ");
            log += String(charCount);
            log += F(" bytes");
            addLog(LOG_LEVEL_INFO, log);
        }
      }

      while (charCount) {                               // This is the serial engine. It processes the serial Rx stream.
        if(HwSerial == UARTSERIAL) c = Serial.read();
        else c = SoftSerial->read();

        if (c == 0x65) {
          if (charCount < 6) delay((5/(AdvHwBaud/9600))+1); // Let's wait for a few more chars to arrive.

          if (HwSerial == UARTSERIAL) charCount = Serial.available();
          else charCount = SoftSerial->available();
          if (charCount >= 6) {
            __buffer[0] = c;                            // Store in staging buffer.
            for (i = 1; i < 7; i++) {
                if(HwSerial == UARTSERIAL) __buffer[i] = Serial.read();
                else __buffer[i] = SoftSerial->read();
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

            if (charCount < 8) delay((9/(AdvHwBaud/9600))+1); // Let's wait for more chars to arrive.
            else delay((3/(AdvHwBaud/9600))+1);               // Short wait for tardy chars.
            if (HwSerial == UARTSERIAL) charCount = Serial.available();
            else charCount = SoftSerial->available();

            if(HwSerial == UARTSERIAL) {
                i = 1;
                while (Serial.available() > 0 && i<RXBUFFSZ) { // Copy global serial buffer to local buffer.
                  __buffer[i] = Serial.read();
                  if (__buffer[i]==0x0a || __buffer[i]==0x0d) break;
                  i++;
                }
            }
            else {
                i = 1;
                while (SoftSerial->available() > 0 && i<RXBUFFSZ) { // Copy global serial buffer to local buffer.
                  __buffer[i] = SoftSerial->read();
                  if (__buffer[i]==0x0a || __buffer[i]==0x0d) break;
                  i++;
                }
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
                UserVar[event->BaseVarIndex] = Vidx.toFloat();
                UserVar[event->BaseVarIndex+1] = Svalue.toFloat();
                sendData(event);

                #ifdef DEBUG_LOG
                 String log;
                 log.reserve(80);                       // Prevent re-allocation
                 log = F("NEXTION075 : Pipe Command Sent: ");
                 log += __buffer;
                 log += UserVar[event->BaseVarIndex];
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
        if(HwSerial == UARTSERIAL) charCount = Serial.available();
        else charCount = SoftSerial->available();
      }

      success = true;
      break;
    }
  }
  return success;
}


void sendCommand(const char *cmd, boolean SerialMode)
{
    if(txPin < 0) {
        String log = F("NEXTION075 : Missing TxD Pin Number, aborted sendCommand");
        addLog(LOG_LEVEL_INFO, log);
    }
    else if(SerialMode == UARTSERIAL) {                 // Send command using Hardware UART serial.
        Serial.print(cmd);
        Serial.write(0xff);
        Serial.write(0xff);
        Serial.write(0xff);
    }
    else {                                              // Send command using bit-bang soft serial.
        if(SoftSerial != NULL){
            SoftSerial->print(cmd);
            SoftSerial->write(0xff);
            SoftSerial->write(0xff);
            SoftSerial->write(0xff);
        }
        else {
            String log = F("NEXTION075 : SoftSerial error, aborted sendCommand");
            addLog(LOG_LEVEL_INFO, log);
        }
    }
    return;
}

#endif // USES_P075

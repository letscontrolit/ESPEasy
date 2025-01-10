#include "_Plugin_Helper.h"
#ifdef USES_P075

# include "src/PluginStructs/P075_data_struct.h"

# include "src/ESPEasyCore/ESPEasyWifi.h"

// #######################################################################################################
// #######################################################################################################
// ################################### Plugin 075: Nextion <info@sensorio.cz>  ###########################
// ###################################   Created on the work of  majklovec     ###########################
// ###################################  Revisions by BertB, ThomasB and others ###########################
// ###################################    Last Revision: 2022-09-27            ###########################
// #######################################################################################################
//

/** Changelog:
 * 2022-09-27 tonhuisman: Use Changelog formatted updates
 *                        Extend nr. of lines available for text/commands to 20, minor code improvements
 * Updated: Oct-03-2018, ThomasB.
 * Added P075_DEBUG_LOG define to reduce info log messages and prevent serial log flooding.
 * Added SendStatus() to post log message on browser to acknowledge HTTP write.
 * Added reserve() to minimize string memory allocations.
 */

// *****************************************************************************************************
// Defines start here
// *****************************************************************************************************

// #define P075_DEBUG_LOG             // Enable this to include additional info messages in log output.


// Plug-In defines
# define PLUGIN_075
# define PLUGIN_ID_075          75
# define PLUGIN_NAME_075        "Display - Nextion"
# define PLUGIN_DEFAULT_NAME    "NEXTION"
# define PLUGIN_VALUENAME1_075  "idx"
# define PLUGIN_VALUENAME2_075  "value"


// *****************************************************************************************************
// PlugIn starts here
// *****************************************************************************************************

boolean Plugin_075(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_075;
      dev.Type           = DEVICE_TYPE_SERIAL;
      dev.VType          = Sensor_VType::SENSOR_TYPE_DUAL;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true; // Allow user to disable interval function.

      // FIXME TD-er: Not sure if access to any existing task data is needed when saving
      dev.ExitTaskBeforeSave = false;

      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_075);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_075));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_075));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
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
      const __FlashStringHelper *options[] = {
        F("9600"),
        F("38400"),
        F("57600"),
        F("115200")
      };

      constexpr size_t optionCount = NR_ELEMENTS(options);
      addFormSelector(F("Baud Rate"), F("baud"), optionCount, options, nullptr, P075_BaudRate);
      addUnit(F("baud"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      //    ** DEVELOPER DEBUG MESSAGE AREA **
      //    int datax = static_cast<int>(Settings.TaskDeviceEnabled[event->TaskIndex]); // Debug value.
      //    String Data = "Debug. Plugin Enable State: ";
      //    Data += String(datax);
      //    addFormNote(Data);

      addFormSubHeader(F("")); // Blank line, vertical space.
      addFormHeader(F("Nextion Command Statements (Optional)"));
      P075_data_struct *P075_data = static_cast<P075_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P075_data) {
        P075_data->loadDisplayLines(event->TaskIndex);

        for (int varNr = 0; varNr < P75_Nlines; varNr++) {
          addFormTextBox(concat(F("Line "), varNr + 1), getPluginCustomArgName(varNr), P075_data->displayLines[varNr], P75_Nchars - 1);
        }
      }

      if (Settings.TaskDeviceTimer[event->TaskIndex] == 0) { // Is interval timer disabled?
        addFormNote(concat(F("Interval Timer OFF, Nextion Lines (above)"), P075_IncludeValues
          ? F(" and Values (below) <b>NOT</b> scheduled for updates")
          : F(" <b>NOT</b> scheduled for updates")));
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
        char deviceTemplate[P75_Nlines][P75_Nchars] = {};
        String error;

        for (uint8_t varNr = 0; varNr < P75_Nlines; varNr++)
        {
          if (!safe_strncpy(deviceTemplate[varNr], webArg(getPluginCustomArgName(varNr)), P75_Nchars)) {
            error += getCustomTaskSettingsError(varNr);
          }
        }

        if (error.length() > 0) {
          addHtmlError(error);
        }
        SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&deviceTemplate, sizeof(deviceTemplate));
      }

      if (getTaskDeviceName(event->TaskIndex).isEmpty()) {             // Check to see if user entered device name.
        strcpy(ExtraTaskSettings.TaskDeviceName, PLUGIN_DEFAULT_NAME); // Name missing, populate default name.
      }

      P075_BaudRate      = getFormItemInt(F("baud"));
      P075_IncludeValues = isFormItemChecked(F("IncludeValues"));

      success = true;
      break;
    }


    case PLUGIN_INIT: {
      uint8_t BaudCode = P075_BaudRate;

      if (BaudCode > P075_B115200) { BaudCode = P075_B9600; }
      constexpr uint32_t BaudArray[] = { 9600UL, 38400UL, 57600UL, 115200UL };
      const ESPEasySerialPort port   = static_cast<ESPEasySerialPort>(CONFIG_PORT);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P075_data_struct(port, CONFIG_PIN1, CONFIG_PIN2, BaudArray[BaudCode]));
      P075_data_struct *P075_data = static_cast<P075_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P075_data) {
        P075_data->loadDisplayLines(event->TaskIndex);
        addLog(LOG_LEVEL_INFO, P075_data->getLogString());
        success = true;
      }
      break;
    }


    case PLUGIN_READ: {
      P075_data_struct *P075_data = static_cast<P075_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P075_data) {
        String newString;

        // Get optional LINE command statements. Special RSSIBAR bargraph keyword is supported.
        for (uint8_t x = 0; x < P75_Nlines; x++) {
          if (P075_data->displayLines[x].length()) {
            int RssiIndex;
            {
              String UcTmpString(P075_data->displayLines[x]);
              UcTmpString.toUpperCase();
              RssiIndex = UcTmpString.indexOf(F("RSSIBAR")); // RSSI bargraph Keyword found, wifi value in dBm.
            }

            if (RssiIndex >= 0) {
              newString = concat(
                P075_data->displayLines[x].substring(0, RssiIndex),
                GetRSSI_quality() * 10);
            }
            else {
              String tmpString(P075_data->displayLines[x]);
              newString = parseTemplate(tmpString);
            }

            P075_sendCommand(event->TaskIndex, newString.c_str());
            # ifdef P075_DEBUG_LOG
            addLog(LOG_LEVEL_INFO, strformat(F("NEXTION075 : Cmd Statement Line-%d Sent: %s"), x + 1, newString.c_str()));
            # endif // ifdef P075_DEBUG_LOG
          }
        }

        // At Interval timer, send idx & value data only if user enabled "values" interval mode.
        if (P075_IncludeValues) {
          # ifdef P075_DEBUG_LOG
          addLogMove(LOG_LEVEL_INFO,
                     strformat(F("NEXTION075: Interval values data enabled, resending idx=%s, value=%s"),
                               formatUserVarNoCheck(event, 0).c_str(),
                               formatUserVarNoCheck(event, 1).c_str()));
          # endif // ifdef P075_DEBUG_LOG

          success = true;
        }
        else {
          # ifdef P075_DEBUG_LOG
          addLog(LOG_LEVEL_INFO, F("NEXTION075: Interval values data disabled, idx & value not resent"));
          # endif // ifdef P075_DEBUG_LOG

          success = false;
        }
      }
      break;
    }

    // Nextion commands received from events (including http) get processed here. PLUGIN_WRITE
    // does NOT process publish commands that are sent.
    case PLUGIN_WRITE: {
      const String command = parseString(string, 1);

      // If device names match we have a command to write.
      if (command.equalsIgnoreCase(getTaskDeviceName(event->TaskIndex))) {
        success = true; // Set true only if plugin found a command to execute.
        const String nextionArguments = parseStringToEndKeepCase(string, 2);
        P075_sendCommand(event->TaskIndex, nextionArguments.c_str());
        {
          const String log = concat(F("NEXTION075 : WRITE = "), nextionArguments);
          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, log);
          # endif // ifndef BUILD_NO_DEBUG
          SendStatus(event, log); // Reply (echo) to sender. This will print message on browser.
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

    case PLUGIN_TEN_PER_SECOND: {
      P075_data_struct *P075_data = static_cast<P075_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P075_data) {
        break;
      }

      if (validGpio(P075_data->rxPin)) {
        addLog(LOG_LEVEL_INFO, F("NEXTION075 : Missing RxD Pin, aborted serial receive"));
        break;
      }

      if (P075_data->easySerial == nullptr) {
        break; // P075_data->easySerial missing, exit.
      }
      {
        uint16_t i;
        uint8_t  c;
        String   Vidx;
        String   Nvalue;
        String   Svalue;
        String   Nswitch;
        char     __buffer[RXBUFFSZ + 1];                         // Staging buffer.
        uint8_t  charCount = P075_data->easySerial->available(); // Prime the Soft Serial engine.

        if (charCount >= RXBUFFWARN) {
          addLog(LOG_LEVEL_INFO, strformat(F("NEXTION075 : RxD P075_data->easySerial Buffer capacity warning, %d bytes"), charCount));
        }
        uint32_t baudrate_delay_unit = P075_data->baudrate / 9600;

        if (baudrate_delay_unit == 0) {
          baudrate_delay_unit = 1;
        }

        while (charCount) { // This is the serial engine. It processes the serial Rx stream.
          c = P075_data->easySerial->read();

          if (c == 0x65) {
            if (charCount < 6) { delay((5 / (baudrate_delay_unit)) + 1); // Let's wait for a few more chars to arrive.
            }
            charCount = P075_data->easySerial->available();

            if (charCount >= 6) {
              __buffer[0] = c; // Store in staging buffer.

              for (i = 1; i < 7; i++) {
                __buffer[i] = P075_data->easySerial->read();
              }

              __buffer[i] = 0x00;

              // FIXME TD-er: (PVS Studio) A part of conditional expression is always false: (0xFF == __buffer[4]). The value range of char
              // type: [-128, 127].
              if ((0xFF == __buffer[4]) && (0xFF == __buffer[5]) && (0xFF == __buffer[6])) {
                UserVar.setFloat(event->TaskIndex, 0, (__buffer[1] * 256) + __buffer[2] + TOUCH_BASE);
                UserVar.setFloat(event->TaskIndex, 1, __buffer[3]);
                sendData(event);

                # ifdef P075_DEBUG_LOG
                addLogMove(LOG_LEVEL_INFO,
                           strformat(F("NEXTION075 : code: %c,%c,%c"),
                                     __buffer[1],
                                     __buffer[2],
                                     __buffer[3]));
                # endif // ifdef P075_DEBUG_LOG
              }
            }
          }
          else {
            if (c == '|') {
              __buffer[0] = c;                                             // Store in staging buffer.

              if (charCount < 8) { delay((9 / (baudrate_delay_unit)) + 1); // Let's wait for more chars to arrive.
              }
              else { delay((3 / (baudrate_delay_unit)) + 1);               // Short wait for tardy chars.
              }
              charCount = P075_data->easySerial->available();

              i = 1;

              while (P075_data->easySerial->available() > 0 && i < RXBUFFSZ) { // Copy global serial buffer to local buffer.
                __buffer[i] = P075_data->easySerial->read();

                if ((__buffer[i] == 0x0a) || (__buffer[i] == 0x0d)) { break; }
                i++;
              }

              __buffer[i] = 0x00;

              String tmpString = __buffer;

              # ifdef P075_DEBUG_LOG
              addLogMove(LOG_LEVEL_INFO, concat(F("NEXTION075 : Code = "), tmpString));
              # endif // ifdef P075_DEBUG_LOG

              int argIndex = tmpString.indexOf(F(",i"));
              int argEnd   = tmpString.indexOf(',', argIndex + 1);

              if (argIndex) { Vidx = tmpString.substring(argIndex + 2, argEnd); }

              bool GotPipeCmd = false;

              switch (__buffer[1]) {
                case 'u':
                  GotPipeCmd = true;
                  argIndex   = argEnd;
                  argEnd     = tmpString.indexOf(',', argIndex + 1);

                  if (argIndex) { Nvalue = tmpString.substring(argIndex + 2, argEnd); }
                  argIndex = argEnd;
                  argEnd   = tmpString.indexOf(0x0a);

                  if (argIndex) { Svalue = tmpString.substring(argIndex + 2, argEnd); }
                  break;
                case 's':
                  GotPipeCmd = true;
                  argIndex   = argEnd;
                  argEnd     = tmpString.indexOf(0x0a);

                  if (argIndex) { Nvalue = tmpString.substring(argIndex + 2, argEnd); }

                  if (equals(Nvalue, F("On"))) { Svalue = '1'; }

                  if (equals(Nvalue, F("Off"))) { Svalue = '0'; }
                  break;
              }

              if (GotPipeCmd) {
                float Vidx_f{};
                float Svalue_f{};

                validFloatFromString(Vidx,   Vidx_f);
                validFloatFromString(Svalue, Svalue_f);
                UserVar.setFloat(event->TaskIndex, 0, Vidx_f);
                UserVar.setFloat(event->TaskIndex, 1, Svalue_f);
                sendData(event);

                # ifdef P075_DEBUG_LOG
                String log;
                log.reserve(80); // Prevent re-allocation
                log += F("NEXTION075 : Pipe Command Sent: ");
                log += __buffer;
                log += formatUserVarNoCheck(event, 0);
                addLogMove(LOG_LEVEL_INFO, log);
                # endif // ifdef P075_DEBUG_LOG
              }
              else {
                # ifdef P075_DEBUG_LOG
                addLog(LOG_LEVEL_INFO, F("NEXTION075 : Unknown Pipe Command, skipped"));
                # endif // ifdef P075_DEBUG_LOG
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
  P075_data_struct *P075_data = static_cast<P075_data_struct *>(getPluginTaskData(taskIndex));

  if (!P075_data) { return; }

  if (!validGpio(P075_data->txPin)) {
    addLog(LOG_LEVEL_INFO, F("NEXTION075 : Missing TxD Pin Number, aborted sendCommand"));
  }
  else
  {
    if (P075_data->easySerial != nullptr) {
      P075_data->easySerial->print(cmd);
      P075_data->easySerial->write(0xff);
      P075_data->easySerial->write(0xff);
      P075_data->easySerial->write(0xff);
    }
    else {
      addLog(LOG_LEVEL_INFO, F("NEXTION075 : P075_data->easySerial error, aborted sendCommand"));
    }
  }
}

#endif // USES_P075

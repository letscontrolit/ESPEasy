#include "_Plugin_Helper.h"

#ifdef USES_P012

# include "src/PluginStructs/P012_data_struct.h"

// #######################################################################################################
// #################################### Plugin 012: LCD ##################################################
// #######################################################################################################


// Sample templates
//  Temp: [DHT11#Temperature]   Hum:[DHT11#humidity]
//  DS Temp:[Dallas1#Temperature#R]
//  Lux:[Lux#Lux#R]
//  Baro:[Baro#Pressure#R]
//  Pump:[Pump#on#O] -> ON/OFF


# define PLUGIN_012
# define PLUGIN_ID_012         12
# define PLUGIN_NAME_012       "Display - LCD2004"
# define PLUGIN_VALUENAME1_012 "LCD"

# define P12_Nlines 4 // The number of different lines which can be displayed
# define P12_Nchars 80

# define P012_I2C_ADDR    PCONFIG(0)
# define P012_SIZE        PCONFIG(1)
# define P012_TIMER       PCONFIG(2)
# define P012_MODE        PCONFIG(3)

boolean Plugin_012(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_012;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_012);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_012));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      byte choice = P012_I2C_ADDR;

      // String options[16];
      int optionValues[16];

      for (byte x = 0; x < 16; x++)
      {
        if (x < 8) {
          optionValues[x] = 0x20 + x;
        }
        else {
          optionValues[x] = 0x30 + x;
        }

        // options[x] = F("0x");
        // options[x] += String(optionValues[x], HEX);
      }
      addFormSelectorI2C(F("p012_adr"), 16, optionValues, choice);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      byte   choice2 = P012_SIZE;
      String options2[2];
      options2[0] = F("2 x 16");
      options2[1] = F("4 x 20");
      int optionValues2[2] = { 1, 2 };
      addFormSelector(F("Display Size"), F("p012_size"), 2, options2, optionValues2, choice2);

      {
        String strings[P12_Nlines];
        LoadCustomTaskSettings(event->TaskIndex, strings, P12_Nlines, P12_Nchars);

        for (byte varNr = 0; varNr < P12_Nlines; varNr++)
        {
          addFormTextBox(String(F("Line ")) + (varNr + 1), getPluginCustomArgName(varNr), strings[varNr], P12_Nchars);
        }
      }

      addRowLabel(F("Display button"));
      addPinSelect(false, F("taskdevicepin3"), CONFIG_PIN3);

      addFormNumericBox(F("Display Timeout"), F("p012_timer"), P012_TIMER);

      String options3[3];
      options3[0] = F("Continue to next line (as in v1.4)");
      options3[1] = F("Truncate exceeding message");
      options3[2] = F("Clear then truncate exceeding message");
      int optionValues3[3] = { 0, 1, 2 };
      addFormSelector(F("LCD command Mode"), F("p012_mode"), 3, options3, optionValues3, P012_MODE);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P012_I2C_ADDR = getFormItemInt(F("p012_adr"));
      P012_SIZE     = getFormItemInt(F("p012_size"));
      P012_TIMER    = getFormItemInt(F("p012_timer"));
      P012_MODE     = getFormItemInt(F("p012_mode"));

      // FIXME TD-er: This is a huge stack allocated object.
      char   deviceTemplate[P12_Nlines][P12_Nchars];
      String error;

      for (byte varNr = 0; varNr < P12_Nlines; varNr++)
      {
        if (!safe_strncpy(deviceTemplate[varNr], web_server.arg(getPluginCustomArgName(varNr)), P12_Nchars)) {
          error += getCustomTaskSettingsError(varNr);
        }
      }

      if (error.length() > 0) {
        addHtmlError(error);
      }
      SaveCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemplate, sizeof(deviceTemplate));
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new P012_data_struct(P012_I2C_ADDR, P012_SIZE, P012_MODE, P012_TIMER));
      P012_data_struct *P012_data =
        static_cast<P012_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P012_data) {
        break;
      }

      if (CONFIG_PIN3 != -1) {
        pinMode(CONFIG_PIN3, INPUT_PULLUP);
      }
      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      clearPluginTaskData(event->TaskIndex);
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      if (CONFIG_PIN3 != -1)
      {
        if (!digitalRead(CONFIG_PIN3))
        {
          P012_data_struct *P012_data =
            static_cast<P012_data_struct *>(getPluginTaskData(event->TaskIndex));

          if (nullptr != P012_data) {
            P012_data->setBacklightTimer(P012_TIMER);
          }
        }
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P012_data_struct *P012_data =
        static_cast<P012_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P012_data) {
        P012_data->checkTimer();
      }
      break;
    }

    case PLUGIN_READ:
    {
      P012_data_struct *P012_data =
        static_cast<P012_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P012_data) {
        // FIXME TD-er: This is a huge stack allocated object.
        char deviceTemplate[P12_Nlines][P12_Nchars];
        LoadCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemplate, sizeof(deviceTemplate));

        for (byte x = 0; x < P012_data->Plugin_012_rows; x++)
        {
          String tmpString = deviceTemplate[x];

          if (tmpString.length())
          {
            String newString = P012_parseTemplate(tmpString, P012_data->Plugin_012_cols);
            P012_data->lcdWrite(newString, 0, x);
          }
        }
        success = false;
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P012_data_struct *P012_data =
        static_cast<P012_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P012_data) {
        String cmd = parseString(string, 1);

        if (cmd.equalsIgnoreCase(F("LCDCMD")))
        {
          success = true;
          String arg1 = parseString(string, 2);

          if (arg1.equalsIgnoreCase(F("Off"))) {
            P012_data->lcd.noBacklight();
          }
          else if (arg1.equalsIgnoreCase(F("On"))) {
            P012_data->lcd.backlight();
          }
          else if (arg1.equalsIgnoreCase(F("Clear"))) {
            P012_data->lcd.clear();
          }
        }
        else if (cmd.equalsIgnoreCase(F("LCD")))
        {
          success = true;
          int colPos  = event->Par2 - 1;
          int rowPos  = event->Par1 - 1;
          String text = parseStringKeepCase(string, 4);
          text = P012_parseTemplate(text, P012_data->Plugin_012_cols);

          P012_data->lcdWrite(text, colPos, rowPos);
        }
        break;
      }
    }
  }
  return success;
}

// Perform some specific changes for LCD display
// https://www.letscontrolit.com/forum/viewtopic.php?t=2368
String P012_parseTemplate(String& tmpString, byte lineSize) {
  String result            = parseTemplate_padded(tmpString, lineSize);
  const char degree[3]     = { 0xc2, 0xb0, 0 }; // Unicode degree symbol
  const char degree_lcd[2] = { 0xdf, 0 };       // P012_LCD degree symbol

  result.replace(degree, degree_lcd);

  char unicodePrefix = 0xc3;

  if (result.indexOf(unicodePrefix) != -1) {
    // See: https://github.com/letscontrolit/ESPEasy/issues/2081

    const char umlautAE_uni[3] = { 0xc3, 0x84, 0 };  // Unicode Umlaute AE
    const char umlautAE_lcd[2] = { 0xe1, 0 };        // P012_LCD Umlaute
    result.replace(umlautAE_uni,  umlautAE_lcd);

    const char umlaut_ae_uni[3] = { 0xc3, 0xa4, 0 }; // Unicode Umlaute ae
    result.replace(umlaut_ae_uni, umlautAE_lcd);

    const char umlautOE_uni[3] = { 0xc3, 0x96, 0 };  // Unicode Umlaute OE
    const char umlautOE_lcd[2] = { 0xef, 0 };        // P012_LCD Umlaute
    result.replace(umlautOE_uni,  umlautOE_lcd);

    const char umlaut_oe_uni[3] = { 0xc3, 0xb6, 0 }; // Unicode Umlaute oe
    result.replace(umlaut_oe_uni, umlautOE_lcd);

    const char umlautUE_uni[3] = { 0xc3, 0x9c, 0 };  // Unicode Umlaute UE
    const char umlautUE_lcd[2] = { 0xf5, 0 };        // P012_LCD Umlaute
    result.replace(umlautUE_uni,  umlautUE_lcd);

    const char umlaut_ue_uni[3] = { 0xc3, 0xbc, 0 }; // Unicode Umlaute ue
    result.replace(umlaut_ue_uni, umlautUE_lcd);

    const char umlaut_sz_uni[3] = { 0xc3, 0x9f, 0 }; // Unicode Umlaute sz
    const char umlaut_sz_lcd[2] = { 0xe2, 0 };       // P012_LCD Umlaute
    result.replace(umlaut_sz_uni, umlaut_sz_lcd);
  }
  return result;
}

#endif // USES_P012

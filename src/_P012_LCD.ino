#ifdef USES_P012
//#######################################################################################################
//#################################### Plugin 012: LCD ##################################################
//#######################################################################################################

// Sample templates
//  Temp: [DHT11#Temperature]   Hum:[DHT11#humidity]
//  DS Temp:[Dallas1#Temperature#R]
//  Lux:[Lux#Lux#R]
//  Baro:[Baro#Pressure#R]
//  Pump:[Pump#on#O] -> ON/OFF
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C *lcd=NULL;
int Plugin_012_cols = 16;
int Plugin_012_rows = 2;
int Plugin_012_mode = 1;

#define PLUGIN_012
#define PLUGIN_ID_012         12
#define PLUGIN_NAME_012       "Display - LCD2004"
#define PLUGIN_VALUENAME1_012 "LCD"

#define P12_Nlines 4        // The number of different lines which can be displayed
#define P12_Nchars 80

boolean Plugin_012(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte displayTimer = 0;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_012;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_NONE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = true;
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

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        //String options[16];
        int optionValues[16];
        for (byte x = 0; x < 16; x++)
        {
          if (x < 8)
            optionValues[x] = 0x20 + x;
          else
            optionValues[x] = 0x30 + x;
          //options[x] = F("0x");
          //options[x] += String(optionValues[x], HEX);
        }
        addFormSelectorI2C(F("p012_adr"), 16, optionValues, choice);


        byte choice2 = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        String options2[2];
        options2[0] = F("2 x 16");
        options2[1] = F("4 x 20");
        int optionValues2[2] = { 1, 2 };
        addFormSelector(F("Display Size"), F("p012_size"), 2, options2, optionValues2, choice2);


        char deviceTemplate[P12_Nlines][P12_Nchars];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        for (byte varNr = 0; varNr < P12_Nlines; varNr++)
        {
          addFormTextBox(String(F("Line ")) + (varNr + 1), String(F("p012_template")) + (varNr + 1), deviceTemplate[varNr], P12_Nchars);
        }

        addRowLabel(F("Display button"));
        addPinSelect(false, F("taskdevicepin3"), Settings.TaskDevicePin3[event->TaskIndex]);

        addFormNumericBox(F("Display Timeout"), F("p012_timer"), Settings.TaskDevicePluginConfig[event->TaskIndex][2]);

        String options3[3];
        options3[0] = F("Continue to next line (as in v1.4)");
        options3[1] = F("Truncate exceeding message");
        options3[2] = F("Clear then truncate exceeding message");
        int optionValues3[3] = { 0,1,2 };
        addFormSelector(F("LCD command Mode"), F("p012_mode"), 3, options3, optionValues3, Settings.TaskDevicePluginConfig[event->TaskIndex][3]);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p012_adr"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("p012_size"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("p012_timer"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = getFormItemInt(F("p012_mode"));

        char deviceTemplate[P12_Nlines][P12_Nchars];
        String error;
        for (byte varNr = 0; varNr < P12_Nlines; varNr++)
        {
          String argName = F("p012_template");
          argName += varNr + 1;
          if (!safe_strncpy(deviceTemplate[varNr], WebServer.arg(argName), P12_Nchars)) {
            error += getCustomTaskSettingsError(varNr);
          }
        }
        if (error.length() > 0) {
          addHtmlError(error);
        }
        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 2) {
          Plugin_012_rows = 4;
          Plugin_012_cols = 20;
        } else if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 1) {
          Plugin_012_rows = 2;
          Plugin_012_cols = 16;
        }

        Plugin_012_mode = Settings.TaskDevicePluginConfig[event->TaskIndex][3];

        //TODO:LiquidCrystal_I2C class doesn't have destructor. So if LCD type (size) is changed better reboot for changes to take effect.
        // workaround is to fix the cols and rows at its maximum (20 and 4)
        if (!lcd)
          lcd = new LiquidCrystal_I2C(Settings.TaskDevicePluginConfig[event->TaskIndex][0], 20, 4); //Plugin_012_cols, Plugin_012_rows);

        // Setup LCD display
        lcd->init();                      // initialize the lcd
        lcd->backlight();
        lcd->print(F("ESP Easy"));
        displayTimer = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        if (Settings.TaskDevicePin3[event->TaskIndex] != -1)
          pinMode(Settings.TaskDevicePin3[event->TaskIndex], INPUT_PULLUP);
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Settings.TaskDevicePin3[event->TaskIndex] != -1)
        {
          if (!digitalRead(Settings.TaskDevicePin3[event->TaskIndex]))
          {
            if (lcd) {
              lcd->backlight();
            }
            displayTimer = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
          }
        }
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        if ( displayTimer > 0)
        {
          displayTimer--;
          if (lcd && displayTimer == 0)
            lcd->noBacklight();
        }
        break;
      }

    case PLUGIN_READ:
      {
        char deviceTemplate[P12_Nlines][P12_Nchars];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        for (byte x = 0; x < Plugin_012_rows; x++)
        {
          String tmpString = deviceTemplate[x];
          if (lcd && tmpString.length())
          {
            String newString = P012_parseTemplate(tmpString, Plugin_012_cols);
            lcd->setCursor(0, x);
            lcd->print(newString);
          }
        }
        success = false;
        break;
      }

    case PLUGIN_WRITE:
      {
        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);

        if (lcd && tmpString.equalsIgnoreCase(F("LCDCMD")))
        {
          success = true;
          argIndex = string.lastIndexOf(',');
          tmpString = string.substring(argIndex + 1);
          if (tmpString.equalsIgnoreCase(F("Off"))){
              lcd->noBacklight();
          }
          else if (tmpString.equalsIgnoreCase(F("On"))){
              lcd->backlight();
          }
          else if (tmpString.equalsIgnoreCase(F("Clear"))){
              lcd->clear();
          }
        }
        else if (lcd && tmpString.equalsIgnoreCase(F("LCD")))
        {
          success = true;
          tmpString = P012_parseTemplate(string, Plugin_012_cols);
          argIndex = tmpString.lastIndexOf(',');
          tmpString = tmpString.substring(argIndex + 1);

          int colPos = event->Par2 - 1;
          int rowPos = event->Par1 - 1;

          //clear line before writing new string
          if (Plugin_012_mode == 2){
              lcd->setCursor(colPos, rowPos);
              for (byte i = colPos; i < Plugin_012_cols; i++) {
                  lcd->print(" ");
              }
          }

          // truncate message exceeding cols
          lcd->setCursor(colPos, rowPos);
          if(Plugin_012_mode == 1 || Plugin_012_mode == 2){
              lcd->setCursor(colPos, rowPos);
              for (byte i = 0; i < Plugin_012_cols - colPos; i++) {
                  if(tmpString[i]){
                     lcd->print(tmpString[i]);
                  }
              }
          }

          // message exceeding cols will continue to next line
          else{
              // Fix Weird (native) lcd display behaviour that split long string into row 1,3,2,4, instead of 1,2,3,4
              boolean stillProcessing = 1;
              byte charCount = 1;
              while(stillProcessing) {
                   if (++colPos > Plugin_012_cols) {    // have we printed 20 characters yet (+1 for the logic)
                        rowPos += 1;
                        lcd->setCursor(0,rowPos);   // move cursor down
                        colPos = 1;
                   }

                   //dont print if "lower" than the lcd
                   if(rowPos < Plugin_012_rows  ){
                       lcd->print(tmpString[charCount - 1]);
                   }

                   if (!tmpString[charCount]) {   // no more chars to process?
                        stillProcessing = 0;
                   }
                   charCount += 1;
              }
              //lcd->print(tmpString.c_str());
              // end fix
          }

        }
        break;
      }

  }
  return success;
}

// Perform some specific changes for LCD display
// https://www.letscontrolit.com/forum/viewtopic.php?t=2368
String P012_parseTemplate(String &tmpString, byte lineSize) {
  String result = parseTemplate(tmpString, lineSize);
  const char degree[3] = {0xc2, 0xb0, 0};  // Unicode degree symbol
  const char degree_lcd[2] = {0xdf, 0};  // P012_LCD degree symbol
  result.replace(degree, degree_lcd);
  
  char unicodePrefix = 0xc3;
  if (result.indexOf(unicodePrefix) != -1) {
    // See: https://github.com/letscontrolit/ESPEasy/issues/2081

    const char umlautAE_uni[3] = {0xc3, 0x84, 0}; // Unicode Umlaute AE
    const char umlautAE_lcd[2] = {0xe1, 0}; // P012_LCD Umlaute
    result.replace(umlautAE_uni, umlautAE_lcd);

    const char umlaut_ae_uni[3] = {0xc3, 0xa4, 0}; // Unicode Umlaute ae
    result.replace(umlaut_ae_uni, umlautAE_lcd);

    const char umlautOE_uni[3] = {0xc3, 0x96, 0}; // Unicode Umlaute OE
    const char umlautOE_lcd[2] = {0xef, 0}; // P012_LCD Umlaute
    result.replace(umlautOE_uni, umlautOE_lcd);

    const char umlaut_oe_uni[3] = {0xc3, 0xb6, 0}; // Unicode Umlaute oe
    result.replace(umlaut_oe_uni, umlautOE_lcd);

    const char umlautUE_uni[3] = {0xc3, 0x9c, 0}; // Unicode Umlaute UE
    const char umlautUE_lcd[2] = {0xf5, 0}; // P012_LCD Umlaute
    result.replace(umlautUE_uni, umlautUE_lcd);

    const char umlaut_ue_uni[3] = {0xc3, 0xbc, 0}; // Unicode Umlaute ue
    result.replace(umlaut_ue_uni, umlautUE_lcd);

    const char umlaut_sz_uni[3] = {0xc3, 0x9f, 0}; // Unicode Umlaute sz
    const char umlaut_sz_lcd[2] = {0xe2, 0}; // P012_LCD Umlaute
    result.replace(umlaut_sz_uni, umlaut_sz_lcd);
  }
  return result;
}
#endif // USES_P012

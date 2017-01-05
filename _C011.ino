//#######################################################################################################
//########################## Controller Plugin 011: SD Card Writer ######################################
//#######################################################################################################

// Wiring
// https://de.wikipedia.org/wiki/Serial_Peripheral_Interface
// You need an ESP8266 device with accessable SPI Pins. These are:
// Name   Description     GPIO      NodeMCU   Notes
// MOSI   Master Output   GPIO13    D7        Not used (No Data sending to MAX)
// MISO   Master Input    GPIO12    D6        Hardware SPI
// SCK    Clock Output    GPIO14    D5        Hardware SPI
// CS     Chip Select     GPIO15    D8        Hardware SPI (CS is configurable through the web interface)

// If you like to send suggestions feel free to send me an email : dominik@logview.info
// Have fun ... Dominik

#include <SPI.h>
#include <SD.h>

#define CPLUGIN_011
#define CPLUGIN_ID_011         11
#define CPLUGIN_NAME_011       "SD Card Writer"

struct C011_ConfigStruct
{
  byte    ChipSelectPin;
  byte    Separator;
  boolean CreateNew;
  boolean UseLatest;
  int     NewAfterKB;
  boolean ClearSD;
  boolean IncludeTimeStamp;
  boolean IncludeDeviceIndex;
  boolean IncludeDeviceName;
  boolean IncludeValueName;
  byte    FileNr;
  byte    FilePartNr;
};

boolean InitOK = false;

// Config is here a global variable. Otherwise the flash must be read
// every time we write to the SD card.
C011_ConfigStruct Config;

boolean CPlugin_011(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  String logging = "";
  File dataFile;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        logging = F("SD   : CPLUGIN_PROTOCOL_ADD");
        addLog(LOG_LEVEL_DEBUG, logging);

        LoadCustomControllerSettings((byte*)&Config, sizeof(Config));

        // Increase the File Number on bootup if needed ...
        if (Config.CreateNew) {
          Config.FileNr++;
          Config.FilePartNr = 0;
          SaveCustomControllerSettings((byte*)&Config, sizeof(Config));
        }

        Protocol[++protocolCount].Number = CPLUGIN_ID_011;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesTemplate = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = false;
        Protocol[protocolCount].defaultPort = 0;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        logging = F("SD   : CPLUGIN_GET_DEVICENAME");
        addLog(LOG_LEVEL_DEBUG, logging);

        string = F(CPLUGIN_NAME_011);
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        if (!InitOK)
        {
          InitOK = SD.begin(Config.ChipSelectPin);

          if (!InitOK) {
            logging = F("SD   : Card failed, or not present");
            addLog(LOG_LEVEL_ERROR, logging);
          } else
          {
            logging = F("SD   : Card found");
            addLog(LOG_LEVEL_INFO, logging);

            logging = F("SD   : SIZE : ");
            logging += SD.size();
            addLog(LOG_LEVEL_INFO, logging);
          }
        }

        if (InitOK && Config.ClearSD)
        {
          logging = F("SD   : Starting SD Clean ... ");
          addLog(LOG_LEVEL_DEBUG, logging);

          // Delete all Stuff from SD card ...
          cleanPath("/");

          logging = F("SD   : ... Finished SD Clean");
          addLog(LOG_LEVEL_DEBUG, logging);

          logging = F("SD   : SIZE : ");
          logging += SD.size();
          addLog(LOG_LEVEL_INFO, logging);

          Config.FileNr = 0;
          Config.FilePartNr = 0;
          Config.ClearSD = false;
          SaveCustomControllerSettings((byte*)&Config, sizeof(Config));
        }

        if (InitOK)
        {
          statusLED(true);

          byte deviceIndex = event->idx;
          String deviceName = ExtraTaskSettings.TaskDeviceName;
          byte valueCount = getValueCountFromSensorType(event->sensorType);
          char separator = ';';
          switch (Config.Separator)
          {
            case 0:
              separator = ';';
              break;
            case 1:
              separator = ',';
              break;
            case 2:
              separator = ':';
              break;
            case 3:
              separator = '\t';
              break;
            case 4:
              separator = ' ';
              break;
          }

          String value = "";

          if (Config.IncludeTimeStamp) {
            if (Settings.UseNTP) {
              value += tm.Month; value += '.';
              value += tm.Day; value += ' ';
              value += tm.Hour; value += ':';
              value += tm.Minute; value += ':';
              value += tm.Second; value += separator;
            }
            else
            {
              value += millis(); value += separator;
            }
          }

          if (Config.IncludeDeviceIndex) {
            value += deviceIndex;
            value += separator;
          }
          if (Config.IncludeDeviceName) {
            value += deviceName;
            value += separator;
          }
          for (byte x = 0; x < valueCount; x++)
          {
            if (Config.IncludeValueName) {
              value += ExtraTaskSettings.TaskDeviceValueNames[x];
              value += separator;
            }
            if (event->sensorType == SENSOR_TYPE_LONG)
              value += (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
            else
              value += toString(UserVar[event->BaseVarIndex + x], ExtraTaskSettings.TaskDeviceValueDecimals[x]);
            if (x < valueCount - 1) {
              value += separator;
            }
          }

          // Create Filename DATXX_YY.csv
          String filename = "DAT";
          if (Config.FileNr < 16) {
            filename += "0";
          }
          filename += String(Config.FileNr, HEX);
          filename += "_";
          if (Config.FilePartNr < 16) {
            filename += "0";
          }
          filename += String(Config.FilePartNr, HEX);
          filename += ".csv";

          // There is a Bug in the ESP SD Lib
          // http://www.esp8266.com/viewtopic.php?f=32&t=10755&start=16
          // So we need a casting here ...
          if (SD.exists((char *)filename.c_str()))
          {
            dataFile = SD.open(filename, FILE_WRITE);
            if (dataFile) {

              logging = F("SD   : File : ");
              logging += filename;
              logging += F(" Size : ");
              logging += dataFile.size();
              addLog(LOG_LEVEL_DEBUG, logging);

              if (Config.NewAfterKB > 0 && Config.NewAfterKB && dataFile.size() > Config.NewAfterKB * 1024) {
                Config.FilePartNr++;
                SaveCustomControllerSettings((byte*)&Config, sizeof(Config));

                filename = "DAT";
                if (Config.FileNr < 16) {
                  filename += "0";
                }
                filename += String(Config.FileNr, HEX);
                filename += "_";
                if (Config.FilePartNr < 16) {
                  filename += "0";
                }
                filename += String(Config.FilePartNr, HEX);
                filename += ".csv";
              }
              dataFile.close();
            } else {
              logging = F("SD   : No File access : ");
              logging += filename;
              addLog(LOG_LEVEL_ERROR, logging);
            }
          }

          // Write Data to File ...
          if (!SD.exists((char *)filename.c_str())) {
            logging = F("SD   : New File Create : ");
            logging += filename;
            addLog(LOG_LEVEL_INFO, logging);
          }
          dataFile = SD.open(filename, FILE_WRITE);
          if (dataFile) {
            dataFile.println(value);
            dataFile.close();
            value = "SD W : " + value;
            addLog(LOG_LEVEL_INFO, value);
          } else {
            logging = F("SD   : No File access : ");
            logging += filename;
            addLog(LOG_LEVEL_ERROR, logging);
          }

          // open the file. note that only one file can be open at a time,
          // so you have to close this one before opening another.
        }

        break;
      }

    case CPLUGIN_WEBFORM_LOAD:
      {
        logging = F("SD   : CPLUGIN_WEBFORM_LOAD");
        addLog(LOG_LEVEL_DEBUG, logging);

        LoadCustomControllerSettings((byte*)&Config, sizeof(Config));

        // File Counter
        string += F("<TR><TD>File Counter:<TD>");
        string += F("File Number : <b>");
        string += Config.FileNr;
        string += F("</b>&nbsp;&nbsp;&nbsp;File Part : <b>");
        string += Config.FilePartNr;
        string += F("</b>");
                
        // CS Pin Selection
        string += F("<TR><TD>Chip Select SD Card:<TD>");
        addPinSelect(false, string, "CS", Config.ChipSelectPin);

        // Separator
        String options[5];
        options[0] = F("';'  Semicolon");
        options[1] = F("','  Comma");
        options[2] = F("':'  Colon");
        options[3] = F("'/t' Tabulator");
        options[4] = F("' '  Space");
        int optionValues[5] = {0, 1, 2, 3, 4};
        string += F("<TR><TD>Separator:<TD><select name='C011_Sep'>");
        for (byte x = 0; x < 5; x++)
        {
          string += F("<option value='");
          string += optionValues[x];
          string += "'";
          if (Config.Separator == optionValues[x])
            string += F(" selected");
          string += ">";
          string += options[x];
          string += F("</option>");
        }
        string += F("</select>");

        string += F("<TR><TD>File Handling:<TD>"); // checked
        string += F("<input type='radio' id='C011_New' name='C011_FileUse' value='new'");
        if ((!Config.CreateNew && !Config.UseLatest) || Config.CreateNew) {
          string += F(" checked>");
        } else {
          string += F(">");
        }
        string += F("<label for='C011_New'> Create new Filename on Startup</label><br>");
        string += F("<input type='radio' id='C011_Old' name='C011_FileUse' value='old'");
        if (Config.UseLatest) {
          string += F(" checked>");
        } else {
          string += F(">");
        }
        string += F("<label for='C011_Old'> Use latest Filename</label><br>");

        string += F("<TR><TD>New File after x KB:<TD><input type='number' name='C011_xKB' min='0' max='1048576' step='10' value='");
        string += Config.NewAfterKB;
        string += F("'></input>(0 = off)");

        string += F("<TR><TD>Include Time Stamp:<TD><input type='checkbox' name='C011_IncTime'");
        if (Config.IncludeTimeStamp) {
          string += F(" checked");
        };
        string += F("></input>");
        string += F("<TR><TD>Include Device Idx:<TD><input type='checkbox' name='C011_IncTaskId'");
        if (Config.IncludeDeviceIndex) {
          string += F(" checked");
        };
        string += F("></input>");
        string += F("<TR><TD>Include Device Name:<TD><input type='checkbox' name='C011_IncDevName'");
        if (Config.IncludeDeviceName) {
          string += F(" checked");
        };
        string += F("></input>");
        string += F("<TR><TD>Include Value Name:<TD><input type='checkbox' name='C011_IncValName'");
        if (Config.IncludeValueName) {
          string += F(" checked");
        };
        string += F("></input>");
        string += F("<TR><TD>Clear SD Card:<TD><input type='checkbox' name='C011_Del'");
        if (Config.ClearSD) {
          string += F(" checked");
        };
        string += F("></input><FONT COLOR='#FF0000'>&nbsp;<b>WARNING</b></FONT><br>");
        string += F("* All Files and Folders on the SD card get deleted<br>");
        string += F("* File Number Counter are resetted to 00_00<br>");

        success = true;
        break;
      }
    case CPLUGIN_WEBFORM_SAVE:
      {
        logging = F("SD   : CPLUGIN_WEBFORM_SAVE");
        addLog(LOG_LEVEL_DEBUG, logging);

        String argus;
        for (byte x = 0; x < WebServer.args(); x++) {
          argus = "SD  : ";
          argus += x;
          argus += " - ";
          argus += WebServer.argName(x);
          argus += " > ";
          argus += WebServer.arg(x);
          addLog(LOG_LEVEL_DEBUG, argus);
        }

        Config.ChipSelectPin = WebServer.arg("CS").toInt();
        Config.Separator = WebServer.arg("C011_Sep").toInt();
        Config.CreateNew = WebServer.arg("C011_FileUse") == "new";
        Config.UseLatest = WebServer.arg("C011_FileUse") == "old";
        Config.NewAfterKB = WebServer.arg("C011_xKB").toInt();
        Config.ClearSD = WebServer.arg("C011_Del") == "on";
        Config.IncludeTimeStamp = WebServer.arg("C011_IncTime") == "on";
        Config.IncludeDeviceIndex = WebServer.arg("C011_IncTaskId") == "on";
        Config.IncludeDeviceName = WebServer.arg("C011_IncDevName") == "on";
        Config.IncludeValueName = WebServer.arg("C011_IncValName") == "on";

        SaveCustomControllerSettings((byte*)&Config, sizeof(Config));
        break;
      }
      return success;
  }
}

// Based on this code
// https://gist.github.com/jenschr/5713c927c3fb8663d662
void cleanPath(String path) {
  String logging = F("SD     : OPEN > ");
  logging += path;
  addLog(LOG_LEVEL_DEBUG, logging);

  File root = SD.open(path);
  while (true) {
    File entry = root.openNextFile();
    String localPath;

    if (entry) {
      if ( entry.isDirectory() )
      {
        localPath = path + entry.name(); 
        cleanPath(localPath + "/"); 

        if ( SD.rmdir( (char *)localPath.c_str() ) )
        {
          logging = F("SD Del : DIR  > ");
          logging += localPath; //folderBuf;
          addLog(LOG_LEVEL_DEBUG, logging);
        }
        else
        {
          logging = F("SD Del : Folder Delete FAILED > ");
          logging += localPath; //folderBuf;
          addLog(LOG_LEVEL_ERROR, logging);
        }
      }
      else
      {
        localPath = path + entry.name(); 

        if ( SD.remove( (char *)localPath.c_str() ) )
        {
          logging = F("SD Del : FILE > ");
          logging += localPath; 
          addLog(LOG_LEVEL_DEBUG, logging);
        }
        else
        {
          logging = F("SD Del : File Delete FAILED > ");
          logging += localPath; 
          addLog(LOG_LEVEL_DEBUG, logging);
        }
      }
    }
    else {
      // break out of recursion
      break;
    }
  }
}

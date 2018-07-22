#ifdef USES_P065
//#######################################################################################################
//############################# Plugin 065: P065_DFR0299_MP3 ############################################
//#######################################################################################################

// ESPEasy Plugin to controls a MP3-player-module DFPlayer-Mini SKU:DFR0299
// written by Jochen Krapf (jk@nerd2nerd.org)

// Important! The module WTV020-SD look similar to the module DFPlayer-Mini but is NOT pin nor command compatible!

// Commands:
// play,<track>        Plays the n-th track 1...3000 on SD-card in root folder. The track number is the physical order - not the order displayed in file explorer!
// stop                Stops actual playing sound
// vol,<volume>        Set volume level 1...30
// eq,<type>           Set the equalizer type 0=Normal, 1=Pop, 2=Rock, 3=Jazz, 4=classic, 5=Base

// Circuit wiring
// 1st-GPIO -> ESP TX to module RX [Pin2]
// 5 V to module VCC [Pin1] (can be more than 100 mA) Note: Use a blocking capacitor to stabilise VCC
// GND to module GND [Pin7+Pin10]
// Speaker to module SPK_1 and SPK_2 [Pin6,Pin8] (not to GND!) Note: If speaker has to low impedance, use a resistor (like 33 Ohm) in line to speaker
// (optional) module BUSY [Pin16] to LED driver (3.3 V on idle, 0 V on playing)
// All other pins unconnected

// Note: Notification sounds with Creative Commons Attribution license: https://notificationsounds.com/

// Datasheet: https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299



#define PLUGIN_065
#define PLUGIN_ID_065         65
#define PLUGIN_NAME_065       "Notify - DFPlayer-Mini MP3 [TESTING]"
#define PLUGIN_VALUENAME1_065 ""

#include <ESPeasySoftwareSerial.h>

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif
#ifndef PIN
#define PIN(n) (Settings.TaskDevicePin[n][event->TaskIndex])
#endif

ESPeasySoftwareSerial* Plugin_065_SoftSerial = NULL;


boolean Plugin_065(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_065;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_NONE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_065);
        break;
      }

      case PLUGIN_GET_DEVICEGPIONAMES:
        {
          event->String1 = F("GPIO &rarr; RX");
          break;
        }

    case PLUGIN_WEBFORM_LOAD:
      {
          addFormNumericBox(F("Volume"), F("volume"), CONFIG(0), 1, 30);

          success = true;
          break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        CONFIG(0) = getFormItemInt(F("volume"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        #pragma GCC diagnostic push
        //note: we cant fix this, its a upstream bug.
        #pragma GCC diagnostic warning "-Wdelete-non-virtual-dtor"
        if (Plugin_065_SoftSerial)
          delete Plugin_065_SoftSerial;
        #pragma GCC diagnostic pop


        Plugin_065_SoftSerial = new ESPeasySoftwareSerial(-1, PIN(0));   // no RX, only TX

        Plugin_065_SoftSerial->begin(9600);

        Plugin_065_SetVol(CONFIG(0));   // set default volume

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (!Plugin_065_SoftSerial)
          break;

        String command = parseString(string, 1);
        String param = parseString(string, 2);

        if (command == F("play"))
        {
          int track;
          if (validIntFromString(param, track)) {
            Plugin_065_Play(track);
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log = F("MP3  : play=");
              log += track;
              addLog(LOG_LEVEL_INFO, log);
            }
          }
          success = true;
        }

        if (command == F("stop"))
        {
          String log = F("MP3  : stop");

          Plugin_065_SendCmd(0x0E, 0);

          addLog(LOG_LEVEL_INFO, log);
          success = true;
        }

        if (command == F("vol"))
        {
          String log = F("MP3  : vol=");

          int8_t vol = param.toInt();
          if (vol == 0) vol = 30;
          CONFIG(0) = vol;
          Plugin_065_SetVol(vol);
          log += vol;

          addLog(LOG_LEVEL_INFO, log);
          success = true;
        }

        if (command == F("eq"))
        {
          String log = F("MP3  : eq=");

          int8_t eq = param.toInt();
          Plugin_065_SetEQ(eq);
          log += eq;

          addLog(LOG_LEVEL_INFO, log);
          success = true;
        }

        break;
      }
  }
  return success;
}


void Plugin_065_Play(uint16_t track)
{
  Plugin_065_SendCmd(0x03, track);
}

void Plugin_065_SetVol(int8_t vol)
{
  if (vol < 1) vol = 1;
  if (vol > 30) vol = 30;
  Plugin_065_SendCmd(0x06, vol);
}

void Plugin_065_SetEQ(int8_t eq)
{
  if (eq < 0) eq = 0;
  if (eq > 5) eq = 5;
  Plugin_065_SendCmd(0x07, eq);
}

void Plugin_065_SendCmd(byte cmd, int16_t data)
{
  if (!Plugin_065_SoftSerial)
    return;

  byte buffer[10] = { 0x7E, 0xFF, 0x06, 0, 0x00, 0, 0, 0, 0, 0xEF };

  buffer[3] = cmd;
  buffer[5] = data >> 8;   // high byte
  buffer[6] = data & 0xFF;   // low byte

  int16_t checksum = -(buffer[1] + buffer[2] + buffer[3] + buffer[4] + buffer[5] + buffer[6]);
  buffer[7] = checksum >> 8;   // high byte
  buffer[8] = checksum & 0xFF;   // low byte

  Plugin_065_SoftSerial->write(buffer, 10);   //Send the byte array

  String log = F("MP3  : Send Cmd ");
  for (byte i=0; i<10; i++)
  {
    log += String(buffer[i], 16);
    log += F(" ");
  }
  addLog(LOG_LEVEL_DEBUG, log);
}

#endif // USES_P065

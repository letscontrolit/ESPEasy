#include "_Plugin_Helper.h"
#ifdef USES_P065

// #######################################################################################################
// ############################# Plugin 065: P065_DFR0299_MP3 ############################################
// #######################################################################################################

// ESPEasy Plugin to controls a MP3-player-module DFPlayer-Mini SKU:DFR0299
// written by Jochen Krapf (jk@nerd2nerd.org)

// Change history:
// 2023-03-12, tonhuisman: Set default serial port for ESP8266 too, and set a default volume value
// 2023-03-11, tonhuisman: Change plugin from DEVICE_TYPE_SINGLE to DEVICE_TYPE_SERIAL, so it can be used on ESP32 (default: Serial1)
//                         Initially switch CONFIG_PIN1 to CONFIG_PIN2 and always reset CONFIG_PIN1 to -1 on PLUGIN_WEBFORM_SAVE
//                         Handle initial volume after ca. 0.5 sec delay to allow proper initialization of the player
// 2021-05-01, tonhuisman: Add mode and repeat commands, optimize string and log usage to reduce size

// Important! The module WTV020-SD look similar to the module DFPlayer-Mini but is NOT pin nor command compatible!

// Commands:
// play,<track>        Plays the n-th track 1...3000 on SD-card in root folder. The track number is the physical order - not the order
// displayed in file explorer!
// stop                Stops actual playing sound
// vol,<volume>        Set volume level 1...30
// eq,<type>           Set the equalizer type 0=Normal, 1=Pop, 2=Rock, 3=Jazz, 4=classic, 5=Base
// mode,<mode>         Set the playback mode 0=Repeat, 1=Folder repeat, 2=Single repeat, 3=Random
// repeat,<0/1>        Set repeat mode 0=Off, 1=On

// Circuit wiring
// 1st-GPIO -> ESP TX to module RX [Pin2]
// 5 V to module VCC [Pin1] (can be more than 100 mA) Note: Use a blocking capacitor to stabilise VCC
// GND to module GND [Pin7+Pin10]
// Speaker to module SPK_1 and SPK_2 [Pin6,Pin8] (not to GND!) Note: If speaker has to low impedance, use a resistor (like 33 Ohm) in line
// to speaker
// (optional) module BUSY [Pin16] to LED driver (3.3 V on idle, 0 V on playing)
// All other pins unconnected

// Note: Notification sounds with Creative Commons Attribution license: https://notificationsounds.com/

// Datasheet: https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299


# define PLUGIN_065
# define PLUGIN_ID_065         65
# define PLUGIN_NAME_065       "Notify - DFPlayer-Mini MP3"
# define PLUGIN_VALUENAME1_065 ""

# include <ESPeasySerial.h>

# define P065_DEFAULT_VOLUME    15 // Set initial volume
# define P065_VOLUME_DELAY      6  // At least 500 milliseconds delay to allow proper initialization of the player before setting the
                                   // default volume
ESPeasySerial *P065_easySerial = nullptr;
uint8_t P065_initialVolumeSet  = P065_VOLUME_DELAY;


boolean Plugin_065(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_065;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].GlobalSyncOption   = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_065);
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event, true); // RX optional
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      # ifdef ESP8266
      CONFIG_PORT = static_cast<uint8_t>(ESPEasySerialPort::software); // Set default to SoftwareSerial for ESP8266
      # endif // ifdef ESP8266
      # ifdef ESP32
      CONFIG_PORT = static_cast<uint8_t>(ESPEasySerialPort::serial1);  // Set default to Serial1 for ESP32
      # endif // ifdef ESP32
      PCONFIG(0) = P065_DEFAULT_VOLUME;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      if ((CONFIG_PIN1 != -1) && (CONFIG_PIN2 == -1)) { // Use old plugin config
        CONFIG_PIN2 = CONFIG_PIN1;
        CONFIG_PIN1 = -1;
      }
      # ifdef ESP32
      addFormNote(F("MP3 TX pin is unused, and will be reset to '- None -' on save!"));
      # endif // ifdef ESP32
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Volume"), F("volume"), PCONFIG(0), 1, 30);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      CONFIG_PIN1 = -1; // Always reset TX pin to none
      PCONFIG(0)  = getFormItemInt(F("volume"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (P065_easySerial) {
        delete P065_easySerial;
      }

      if ((CONFIG_PIN1 != -1) && (CONFIG_PIN2 == -1)) { // Use old plugin config
        CONFIG_PIN2 = CONFIG_PIN1;
        CONFIG_PIN1 = -1;
      }

      P065_easySerial = new (std::nothrow) ESPeasySerial(static_cast<ESPEasySerialPort>(CONFIG_PORT), -1, CONFIG_PIN2); // no RX, only TX

      if (P065_easySerial != nullptr) {
        P065_easySerial->begin(9600);
        P065_initialVolumeSet = P065_VOLUME_DELAY;

        success = true;
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      if (P065_easySerial != nullptr) {
        delete P065_easySerial;
        P065_easySerial = nullptr;
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      if ((P065_easySerial != nullptr) && (P065_initialVolumeSet > 0)) {
        P065_initialVolumeSet--;

        if (P065_initialVolumeSet == 0) {
          Plugin_065_SetVol(PCONFIG(0)); // set default volume
        }
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      if (!P065_easySerial) {
        break;
      }

      String command = parseString(string, 1);
      String param   = parseString(string, 2);
      int    value;
      bool   valueValid = validIntFromString(param, value);

      if (valueValid && equals(command, F("play")))
      {
        Plugin_065_Play(value);
        success = true;
      }

      if (equals(command, F("stop")))
      {
        Plugin_065_SendCmd(0x0E, 0);
        success = true;
      }

      if (valueValid && equals(command, F("vol")))
      {
        if (value == 0) { value = 30; }
        PCONFIG(0) = value;
        Plugin_065_SetVol(value);
        success = true;
      }

      if (valueValid && equals(command, F("eq")))
      {
        Plugin_065_SetEQ(value);
        success = true;
      }

      if (valueValid && equals(command, F("mode")))
      {
        Plugin_065_SetMode(value);
        success = true;
      }

      if (valueValid && equals(command, F("repeat")))
      {
        Plugin_065_SetRepeat(value);
        success = true;
      }

      if (success && loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log;
        log.reserve(20);
        log  = F("MP3  : ");
        log += command;

        if (!equals(command, F("stop"))) {
          log += '=';
          log += value;
        }
        addLogMove(LOG_LEVEL_INFO, log);
      }
      break;
    }
  }
  return success;
}

void Plugin_065_Play(uint16_t track)
{
  if (track < 0) { track = 0; }

  if (track > 2999) { track = 2999; }

  Plugin_065_SendCmd(0x03, track);
}

void Plugin_065_SetVol(int8_t vol)
{
  if (vol < 1) { vol = 1; }

  if (vol > 30) { vol = 30; }

  Plugin_065_SendCmd(0x06, vol);
}

void Plugin_065_SetEQ(int8_t eq)
{
  if (eq < 0) { eq = 0; }

  if (eq > 5) { eq = 5; }

  Plugin_065_SendCmd(0x07, eq);
}

void Plugin_065_SetMode(int8_t mode)
{
  if (mode < 0) { mode = 0; }

  if (mode > 3) { mode = 3; }

  Plugin_065_SendCmd(0x08, mode);
}

void Plugin_065_SetRepeat(int8_t repeat)
{
  Plugin_065_SendCmd(0x11, (repeat <= 0) ? 0 : 1);
}

void Plugin_065_SendCmd(uint8_t cmd, int16_t data)
{
  if (!P065_easySerial) {
    return;
  }

  uint8_t buffer[10] = { 0x7E, 0xFF, 0x06, 0, 0x00, 0, 0, 0, 0, 0xEF };

  buffer[3] = cmd;
  buffer[5] = data >> 8;   // high byte
  buffer[6] = data & 0xFF; // low byte

  int16_t checksum = -(buffer[1] + buffer[2] + buffer[3] + buffer[4] + buffer[5] + buffer[6]);

  buffer[7] = checksum >> 8;          // high byte
  buffer[8] = checksum & 0xFF;        // low byte

  P065_easySerial->write(buffer, 10); // Send the byte array

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("MP3  : Send Cmd ");

    for (uint8_t i = 0; i < 10; i++) {
      log += String(buffer[i], 16);
      log += ' ';
    }
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifndef BUILD_NO_DEBUG
}

#endif // USES_P065

#include "_Plugin_Helper.h"
#ifdef USES_P054

// #######################################################################################################
// ######################################## Plugin 054: DMX512 TX ########################################
// #######################################################################################################

/** Changelog:
 * 2024-09-21 tonhuisman: Use Direct-GPIO for interacting with GPIO pins for speed & timing accuracy
 *                        Use ESPEasySerial for more flexible serial configuration
 * 2024-09-20 tonhuisman: Reformat source, allow GPIO selection on ESP32 by _not_ resetting to GPIO 2 when not ESP8266
 * 2024-09-20 Start changelog
 */

// ESPEasy Plugin to control DMX-512 Devices (DMX 512/1990; DIN 56930-2) like Dimmer-Packs, LED-Bars, Moving-Heads, Event-Lighting
// written by Jochen Krapf (jk@nerd2nerd.org)

// List of commands:
// (1) DMX,<param>
// (2) DMX,<param>,<param>,<param>, ...

// List of DMX params:
// (a) <value>
//     DMX-value (0...255) to write to the next channel address (1...512) starting with 1
// (b) <channel>=<value>
//     DMX-value (0...255) to write to the given channel address (1...512).
// (c) <channel>=<value>,<value>,<value>,...
//     List of DMX-values (0...255) to write beginning with the given channel address (1...512).
// (d) "OFF"
//     Set DMX-values of all channels to 0.
// (e) "ON"
//     Set DMX-values of all channels to 255.
// (f) "LOG"
//     Print DMX-values of all channels to log output.

// Examples:
// DMX,123"   Set channel 1 to value 123
// DMX,123,22,33,44"   Set channel 1 to value 123, channel 2 to 22, channel 3 to 33, channel 4 to 44
// DMX,5=123"   Set channel 5 to value 123
// DMX,5=123,22,33,44"   Set channel 5 to value 123, channel 6 to 22, channel 7 to 33, channel 8 to 44
// DMX,5=123,8=44"   Set channel 5 to value 123, channel 8 to 44
// DMX,OFF"   Pitch Black

// Transceiver:
// SN75176 or MAX485 or LT1785 or ...
// Pin 5: GND
// Pin 2, 3, 8: +5V
// Pin 4: to ESP D4
// Pin 6: DMX+ (hot)
// Pin 7: DMX- (cold)

// XLR Plug:
// Pin 1: GND, Shield
// Pin 2: DMX- (cold)
// Pin 3: DMX+ (hot)

// Note: The ESP serial FIFO has size of 128 uint8_t. Therefore it is rcommented to use DMX buffer sizes below 128


// #include <*.h>   //no lib needed

# include <ESPeasySerial.h>
# include <GPIO_Direct_Access.h>

# define PLUGIN_054
# define PLUGIN_ID_054         54
# define PLUGIN_NAME_054       "Communication - DMX512 TX"

uint8_t *Plugin_054_DMXBuffer    = 0;
int16_t  Plugin_054_DMXSize      = 32;
ESPeasySerial *Plugin_054_Serial = nullptr;

static inline void PLUGIN_054_Limit(int16_t& value, int16_t min, int16_t max)
{
  if (value < min) {
    value = min;
  }

  if (value > max) {
    value = max;
  }
}

boolean Plugin_054(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number   = PLUGIN_ID_054;
      Device[deviceCount].Type       = DEVICE_TYPE_SERIAL;
      Device[deviceCount].Ports      = 0;
      Device[deviceCount].VType      = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].ValueCount = 0;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_054);
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      # ifdef ESP8266
      CONFIG_PORT = static_cast<int>(ESPEasySerialPort::serial1); // Serial1 port
      CONFIG_PIN1 = -1;                                           // RX pin
      CONFIG_PIN2 = 2;                                            // TX pin
      # endif // ifdef ESP8266
      # ifdef ESP32
      CONFIG_PORT = static_cast<int>(ESPEasySerialPort::serial1); // Serial1 port
      int rxPin                    = 0;
      int txPin                    = 0;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

      ESPeasySerialType::getSerialTypePins(port, rxPin, txPin);
      CONFIG_PIN1 = rxPin;
      CONFIG_PIN2 = txPin;
      # endif // ifdef ESP32

      PCONFIG(0) = Plugin_054_DMXSize; // Holds default value
      break;
    }

    case PLUGIN_WEBFORM_PRE_SERIAL_PARAMS:
    {
      if ((-1 == CONFIG_PIN2) && (2 == CONFIG_PIN1)) {
        CONFIG_PIN2 = CONFIG_PIN1;
        CONFIG_PIN1 = -1;
      }
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      addFormNote(F("An on-chip ESP Serial port"
                    #if USES_USBCDC
                    " (<B>not</B> USB CDC)"
                    #endif // if USES_USBCDC
                    #if USES_HWCDC
                    " (<B>not</B> USB HWCDC)"
                    #endif // if USES_HWCDC
                    " must be selected!"));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event, true);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Channels"), F("channels"), PCONFIG(0), 1, 512);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      # ifdef ESP8266

      if (static_cast<int>(ESPEasySerialPort::serial1) == CONFIG_PORT) {
        CONFIG_PIN2 = 2;
      }
      # endif // ifdef ESP8266

      if (Settings.Pin_status_led == CONFIG_PIN2) { // Status LED assigned to TX1?
        Settings.Pin_status_led = -1;
      }
      Plugin_054_DMXSize = getFormItemInt(F("channels"));
      PLUGIN_054_Limit(Plugin_054_DMXSize, 1, 512);
      PCONFIG(0) = Plugin_054_DMXSize;
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      Plugin_054_DMXSize = PCONFIG(0);

      if (Plugin_054_DMXBuffer) {
        delete[] Plugin_054_DMXBuffer;
      }
      Plugin_054_DMXBuffer = new (std::nothrow) uint8_t[Plugin_054_DMXSize];

      if (Plugin_054_DMXBuffer != nullptr) {
        memset(Plugin_054_DMXBuffer, 0, Plugin_054_DMXSize);
      }

      if ((-1 == CONFIG_PIN2) && (2 == CONFIG_PIN1)) { // Convert previous GPIO settings
        CONFIG_PIN2 = CONFIG_PIN1;
        CONFIG_PIN1 = -1;
      }
      int rxPin                    = CONFIG_PIN1;
      int txPin                    = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

      if ((rxPin < 0) && (txPin < 0)) {
        ESPeasySerialType::getSerialTypePins(port, rxPin, txPin);
        CONFIG_PIN1 = rxPin;
        CONFIG_PIN2 = txPin;
      }
      delete Plugin_054_Serial;
      Plugin_054_Serial = new (std::nothrow) ESPeasySerial(port, rxPin, txPin);

      if (nullptr != Plugin_054_Serial) {
        # ifdef ESP8266
        Plugin_054_Serial->begin(250000, (SerialConfig)SERIAL_8N2);
        # endif // ifdef ESP8266
        # ifdef ESP32
        Plugin_054_Serial->begin(250000, SERIAL_8N2);
        # endif // ifdef ESP32
      }

      success = Plugin_054_DMXBuffer != nullptr && Plugin_054_Serial != nullptr && validGpio(CONFIG_PIN2);
      break;
    }

    case PLUGIN_EXIT:
    {
      if (Plugin_054_DMXBuffer) {
        delete[] Plugin_054_DMXBuffer;
        Plugin_054_DMXBuffer = nullptr;
      }
      delete Plugin_054_Serial;
      Plugin_054_Serial = nullptr;
      break;
    }

    case PLUGIN_WRITE:
    {
      const String command = parseString(string, 1);

      if (equals(command, F("dmx"))) {
        String  param;
        String  paramKey;
        String  paramVal;
        uint8_t paramIdx = 2;
        int16_t channel  = 1;
        int16_t value    = 0;

        param = parseString(string, paramIdx++);

        while (param.length()) {
            # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG_MORE, param);
            # endif // ifndef BUILD_NO_DEBUG

          if (equals(param, F("log"))) {
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log = F("DMX  : ");

              for (int16_t i = 0; i < Plugin_054_DMXSize; ++i) {
                log += concat(Plugin_054_DMXBuffer[i], F(", "));
              }
              addLogMove(LOG_LEVEL_INFO, log);
            }
            success = true;
          }

          else if (equals(param, F("test"))) {
            for (int16_t i = 0; i < Plugin_054_DMXSize; ++i) {
              Plugin_054_DMXBuffer[i] = rand() & 255;
            }
            success = true;
          }

          else if (equals(param, F("on"))) {
            memset(Plugin_054_DMXBuffer, 255, Plugin_054_DMXSize);
            success = true;
          }

          else if (equals(param, F("off"))) {
            memset(Plugin_054_DMXBuffer, 0, Plugin_054_DMXSize);
            success = true;
          }

          else {
            int16_t index = param.indexOf('=');

            if (index > 0) { // syntax: "<channel>=<value>"
              paramKey = param.substring(0, index);
              paramVal = param.substring(index + 1);
              channel  = paramKey.toInt();
            }
            else { // syntax: "<value>"
              paramVal = param;
            }

            value = paramVal.toInt();
            PLUGIN_054_Limit(value, 0, 255);

            if ((channel > 0) && (channel <= Plugin_054_DMXSize)) {
              Plugin_054_DMXBuffer[channel - 1] = value;
            }
            channel++;
            success = true;
          }

          param = parseString(string, paramIdx++);
        }
      }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      if (Plugin_054_DMXBuffer && Plugin_054_Serial) {
        int16_t sendPin = CONFIG_PIN2; // ESP8266: TX1 fixed to GPIO2 (D4) == onboard LED

        // empty serial from prev. transmit
        Plugin_054_Serial->flush();

        // send break
        Plugin_054_Serial->end();

        // DIRECT_PINMODE_OUTPUT(sendPin); // This shouldn't be needed
        DIRECT_pinWrite(sendPin, LOW);
        delayMicroseconds(120); // 88µs ... inf
        DIRECT_pinWrite(sendPin, HIGH);
        delayMicroseconds(12);  // 8µs ... 1s

        // send DMX data
        # ifdef ESP8266
        Plugin_054_Serial->begin(250000, (SerialConfig)SERIAL_8N2);
        # endif // ifdef ESP8266
        # ifdef ESP32
        Plugin_054_Serial->begin(250000, SERIAL_8N2);
        # endif // ifdef ESP32
        Plugin_054_Serial->write((uint8_t)0); // start uint8_t
        Plugin_054_Serial->write(Plugin_054_DMXBuffer, Plugin_054_DMXSize);
      }
      break;
    }
  }
  return success;
}

#endif // USES_P054

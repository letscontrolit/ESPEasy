/*
 * Example sketch to show using configuration commands on the LD2410.
 * 
 * This has been tested on the following platforms...
 * 
 * On ESP32, connect the LD2410 to GPIO pins 32&33
 * On ESP32S2, connect the LD2410 to GPIO pins 8&9
 * On ESP32C3, connect the LD2410 to GPIO pins 4&5
 * On Arduino Leonardo or other ATmega32u4 board connect the LD2410 to GPIO pins TX & RX hardware serial
 * 
 * The serial configuration for other boards will vary and you'll need to assign them yourself
 * 
 * There is no example for ESP8266 as it only has one usable UART and will not boot if the alternate UART pins are used for the radar.
 * 
 * For this sketch and other examples to be useful the board needs to have two usable UARTs.
 * 
 */

#if defined(ESP32)
  #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
    #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
      #define MONITOR_SERIAL Serial
      #define RADAR_SERIAL Serial1
      #define RADAR_RX_PIN 32
      #define RADAR_TX_PIN 33
    #elif CONFIG_IDF_TARGET_ESP32S2
      #define MONITOR_SERIAL Serial
      #define RADAR_SERIAL Serial1
      #define RADAR_RX_PIN 9
      #define RADAR_TX_PIN 8
    #elif CONFIG_IDF_TARGET_ESP32C3
      #define MONITOR_SERIAL Serial
      #define RADAR_SERIAL Serial1
      #define RADAR_RX_PIN 4
      #define RADAR_TX_PIN 5
    #else 
      #error Target CONFIG_IDF_TARGET is not supported
    #endif
  #else // ESP32 Before IDF 4.0
    #define MONITOR_SERIAL Serial
    #define RADAR_SERIAL Serial1
    #define RADAR_RX_PIN 32
    #define RADAR_TX_PIN 33
  #endif
#elif defined(__AVR_ATmega32U4__)
  #define MONITOR_SERIAL Serial
  #define RADAR_SERIAL Serial1
  #define RADAR_RX_PIN 0
  #define RADAR_TX_PIN 1
#endif

#include <ld2410.h>

ld2410 radar;
bool engineeringMode = false;
String command;

void setup(void)
{
  MONITOR_SERIAL.begin(115200); //Feedback over Serial Monitor
  delay(500); //Give a while for Serial Monitor to wake up
  //radar.debug(Serial); //Uncomment to show debug information from the library on the Serial Monitor. By default this does not show sensor reads as they are very frequent.
  #if defined(ESP32)
    RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN); //UART for monitoring the radar
  #elif defined(__AVR_ATmega32U4__)
    RADAR_SERIAL.begin(256000); //UART for monitoring the radar
  #endif
  delay(500);
  MONITOR_SERIAL.print(F("\nConnect LD2410 radar TX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_RX_PIN);
  MONITOR_SERIAL.print(F("Connect LD2410 radar RX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_TX_PIN);
  MONITOR_SERIAL.print(F("LD2410 radar sensor initialising: "));
  if(radar.begin(RADAR_SERIAL))
  {
    MONITOR_SERIAL.println(F("OK"));
    MONITOR_SERIAL.print(F("LD2410 firmware version: "));
    MONITOR_SERIAL.print(radar.firmware_major_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.print(radar.firmware_minor_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.println(radar.firmware_bugfix_version, HEX);
  }
  else
  {
    MONITOR_SERIAL.println(F("not connected"));
  }
  MONITOR_SERIAL.println(F("Supported commands\nread: read current values from the sensor\nreadconfig: read the configuration from the sensor\nsetmaxvalues <motion gate> <stationary gate> <inactivitytimer>\nsetsensitivity <gate> <motionsensitivity> <stationarysensitivity>\nenableengineeringmode: enable engineering mode\ndisableengineeringmode: disable engineering mode\nrestart: restart the sensor\nreadversion: read firmware version\nfactoryreset: factory reset the sensor\n"));
}

void loop()
{
  radar.read(); //Always read frames from the sensor
  if(MONITOR_SERIAL.available())
  {
    char typedCharacter = MONITOR_SERIAL.read();
    if(typedCharacter == '\r' || typedCharacter == '\n')
    {
      command.trim();
      if(command.equals("read"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Reading from sensor: "));
        if(radar.isConnected())
        {
          MONITOR_SERIAL.println(F("OK"));
          if(radar.presenceDetected())
          {
            if(radar.stationaryTargetDetected())
            {
              MONITOR_SERIAL.print(F("Stationary target: "));
              MONITOR_SERIAL.print(radar.stationaryTargetDistance());
              MONITOR_SERIAL.print(F("cm energy: "));
              MONITOR_SERIAL.println(radar.stationaryTargetEnergy());
            }
            if(radar.movingTargetDetected())
            {
              MONITOR_SERIAL.print(F("Moving target: "));
              MONITOR_SERIAL.print(radar.movingTargetDistance());
              MONITOR_SERIAL.print(F("cm energy: "));
              MONITOR_SERIAL.println(radar.movingTargetEnergy());
            }
          }
          else
          {
            MONITOR_SERIAL.println(F("nothing detected"));
          }
        }
        else
        {
          MONITOR_SERIAL.println(F("failed to read"));
        }
      }
      else if(command.equals("readconfig"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Reading configuration from sensor: "));
        if(radar.requestCurrentConfiguration())
        {
          MONITOR_SERIAL.println(F("OK"));
          MONITOR_SERIAL.print(F("Maximum gate ID: "));
          MONITOR_SERIAL.println(radar.max_gate);
          MONITOR_SERIAL.print(F("Maximum gate for moving targets: "));
          MONITOR_SERIAL.println(radar.max_moving_gate);
          MONITOR_SERIAL.print(F("Maximum gate for stationary targets: "));
          MONITOR_SERIAL.println(radar.max_stationary_gate);
          MONITOR_SERIAL.print(F("Idle time for targets: "));
          MONITOR_SERIAL.println(radar.sensor_idle_time);
          MONITOR_SERIAL.println(F("Gate sensitivity"));
          for(uint8_t gate = 0; gate <= radar.max_gate; gate++)
          {
            MONITOR_SERIAL.print(F("Gate "));
            MONITOR_SERIAL.print(gate);
            MONITOR_SERIAL.print(F(" moving targets: "));
            MONITOR_SERIAL.print(radar.motion_sensitivity[gate]);
            MONITOR_SERIAL.print(F(" stationary targets: "));
            MONITOR_SERIAL.println(radar.stationary_sensitivity[gate]);
          }
        }
        else
        {
          MONITOR_SERIAL.println(F("Failed"));
        }
      }
      else if(command.startsWith("setmaxvalues"))
      {
        uint8_t firstSpace = command.indexOf(' ');
        uint8_t secondSpace = command.indexOf(' ',firstSpace + 1);
        uint8_t thirdSpace = command.indexOf(' ',secondSpace + 1);
        uint8_t newMovingMaxDistance = (command.substring(firstSpace,secondSpace)).toInt();
        uint8_t newStationaryMaxDistance = (command.substring(secondSpace,thirdSpace)).toInt();
        uint16_t inactivityTimer = (command.substring(thirdSpace,command.length())).toInt();
        if(newMovingMaxDistance > 0 && newStationaryMaxDistance > 0 && newMovingMaxDistance <= 8 && newStationaryMaxDistance <= 8)
        {
          MONITOR_SERIAL.print(F("Setting max values to gate "));
          MONITOR_SERIAL.print(newMovingMaxDistance);
          MONITOR_SERIAL.print(F(" moving targets, gate "));
          MONITOR_SERIAL.print(newStationaryMaxDistance);
          MONITOR_SERIAL.print(F(" stationary targets, "));
          MONITOR_SERIAL.print(inactivityTimer);
          MONITOR_SERIAL.print(F("s inactivity timer: "));
          command = "";
          if(radar.setMaxValues(newMovingMaxDistance, newStationaryMaxDistance, inactivityTimer))
          {
            MONITOR_SERIAL.println(F("OK, now restart to apply settings"));
          }
          else
          {
            MONITOR_SERIAL.println(F("failed"));
          }
        }
        else
        {
          MONITOR_SERIAL.print(F("Can't set distances to "));
          MONITOR_SERIAL.print(newMovingMaxDistance);
          MONITOR_SERIAL.print(F(" moving "));
          MONITOR_SERIAL.print(newStationaryMaxDistance);
          MONITOR_SERIAL.println(F(" stationary, try again"));
          command = "";
        }
      }
      else if(command.startsWith("setsensitivity"))
      {
        uint8_t firstSpace = command.indexOf(' ');
        uint8_t secondSpace = command.indexOf(' ',firstSpace + 1);
        uint8_t thirdSpace = command.indexOf(' ',secondSpace + 1);
        uint8_t gate = (command.substring(firstSpace,secondSpace)).toInt();
        uint8_t motionSensitivity = (command.substring(secondSpace,thirdSpace)).toInt();
        uint8_t stationarySensitivity = (command.substring(thirdSpace,command.length())).toInt();
        if(motionSensitivity >= 0 && stationarySensitivity >= 0 && motionSensitivity <= 100 && stationarySensitivity <= 100)
        {
          MONITOR_SERIAL.print(F("Setting gate "));
          MONITOR_SERIAL.print(gate);
          MONITOR_SERIAL.print(F(" motion sensitivity to "));
          MONITOR_SERIAL.print(motionSensitivity);
          MONITOR_SERIAL.print(F(" & stationary sensitivity to "));
          MONITOR_SERIAL.print(stationarySensitivity);
          MONITOR_SERIAL.println(F(": "));
          command = "";
          if(radar.setGateSensitivityThreshold(gate, motionSensitivity, stationarySensitivity))
          {
            MONITOR_SERIAL.println(F("OK, now restart to apply settings"));
          }
          else
          {
            MONITOR_SERIAL.println(F("failed"));
          }
        }
        else
        {
          MONITOR_SERIAL.print(F("Can't set gate "));
          MONITOR_SERIAL.print(gate);
          MONITOR_SERIAL.print(F(" motion sensitivity to "));
          MONITOR_SERIAL.print(motionSensitivity);
          MONITOR_SERIAL.print(F(" & stationary sensitivity to "));
          MONITOR_SERIAL.print(stationarySensitivity);
          MONITOR_SERIAL.println(F(", try again"));
          command = "";
        }
      }
      else if(command.equals("enableengineeringmode"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Enabling engineering mode: "));
        if(radar.requestStartEngineeringMode())
        {
          MONITOR_SERIAL.println(F("OK"));
        }
        else
        {
          MONITOR_SERIAL.println(F("failed"));
        }
      }
      else if(command.equals("disableengineeringmode"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Disabling engineering mode: "));
        if(radar.requestEndEngineeringMode())
        {
          MONITOR_SERIAL.println(F("OK"));
        }
        else
        {
          MONITOR_SERIAL.println(F("failed"));
        }
      }
      else if(command.equals("restart"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Restarting sensor: "));
        if(radar.requestRestart())
        {
          MONITOR_SERIAL.println(F("OK"));
        }
        else
        {
          MONITOR_SERIAL.println(F("failed"));
        }
      }
      else if(command.equals("readversion"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Requesting firmware version: "));
        if(radar.requestFirmwareVersion())
        {
          Serial.print(radar.cmdFirmwareVersion());
        }
        else
        {
          MONITOR_SERIAL.println(F("Failed"));
        }
      }
      else if(command.equals("factoryreset"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Factory resetting sensor: "));
        if(radar.requestFactoryReset())
        {
          MONITOR_SERIAL.println(F("OK, now restart sensor to take effect"));
        }
        else
        {
          MONITOR_SERIAL.println(F("failed"));
        }
      }
      else
      {
        MONITOR_SERIAL.print(F("Unknown command: "));
        MONITOR_SERIAL.println(command);
        command = "";
      }
    }
    else
    {
      command += typedCharacter;
    }
  }
  /*
  if()  //Some data has been received from the radar
  {
    if(radar.presenceDetected())
    {
      MONITOR_SERIAL.print(F("Stationary target: "));
      MONITOR_SERIAL.println(radar.stationaryTargetDistance());
      MONITOR_SERIAL.print(F("Moving target: "));
      MONITOR_SERIAL.println(radar.movingTargetDistance());
    }
  }
  */
}

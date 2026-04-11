#include "_Plugin_Helper.h"
#ifdef USES_P088

// #######################################################################################################
// #################################### Plugin 088: Heatpump IR ##########################################
// #######################################################################################################

# define PLUGIN_088
# define PLUGIN_ID_088         88
# define PLUGIN_NAME_088       "Energy (Heat) - Heatpump IR transmitter"


/*
 * ESPEasy plugin to send air conditioner / heatpump IR signals
 * * Use the device type 'Heatpump IR transmitter' as the device type in Devices -> Edit
 * * Connect and IR LED + series resistor between the GPIO pin configured for this device and ground
 * * This is not a standalone plugin, also the _C002.ino modification is needed for Domoticz MQTT to work
 *
 * Send commands through http, like this example (assuming the IP address of the ESP node is 192.168.0.61):
 * * curl http://192.168.0.61/control?cmd=heatpumpir,panasonic_ckp,1,1,0,22,0,0
 *
 * Send commands through OpenHAB MQTT with Mosquitto, like this example,
 * assuming the 'Name' of the ESP node in ESPEasy Main Settings page is 'ESP_Easy')
 * * mosquitto_pub -t /ESP_Easy/cmd -m heatpumpir,panasonic_ckp,1,1,0,22,0,0
 *
 * Send commands through Domoticz MQTT, like this example,
 * assuming the IDX of the heatpump device in both Domoticz and ESP Easy is 13:
 * * Create a 'Dummy' hardware in Domoticz, create a 'Text' virtual sensor into the 'Dummy' device (assumed we got IDX 13)
 * * Define the Domoticz MQTT protocol in ESP Easy, define the 'Heatpump IR transmitter' device using the same IDX as in Domoticz
 * * Update the 'Text' sensor using the Domoticz API, like navigating to this URL (assuming Domoticz is at 192.168.0.5:8080):
 * *   http://192.168.0.5:8080/json.htm?type=command&param=udevice&idx=13&svalue=panasonic_ckp,1,1,0,22,0,0
 *
 * Take a look at https://github.com/ToniA/cabin-village-project/blob/eventscripts/script_device_hp.lua for Domoticz event examples
 *
 * The parameters are (in this order)
 * * The type of the heatpump as a string, see the implementations of different models, like
 * https://github.com/ToniA/arduino-heatpumpir/blob/master/MitsubishiHeatpumpIR.cpp
 * * power state (see https://github.com/ToniA/arduino-heatpumpir/blob/master/HeatpumpIR.h for modes)
 * * operating mode
 * * fan speed
 * * temperature
 * * vertical air direction
 * * horizontal air direction
 *
 * See the HeatpumpIR library for further information: https://github.com/ToniA/arduino-heatpumpir
 *
 */

# include <HeatpumpIRFactory.h>

# include "ESPEasy-Globals.h"

IRSenderIRremoteESP8266 *Plugin_088_irSender = nullptr;
int panasonicCKPTimer                        = 0;

boolean Plugin_088(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number   = PLUGIN_ID_088;
      dev.Type     = DEVICE_TYPE_SINGLE;
      dev.VType    = Sensor_VType::SENSOR_TYPE_NONE;
      dev.setPin1Direction(gpio_direction::gpio_output);

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_088);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // We need the index of the controller we are: 0-CONTROLLER_MAX
      // FIXME TD-er: Why looking for Domoticz MQTT? Other plugins also support IDX values.
      controllerIndex_t controllerNr = 0;

      for (controllerIndex_t i = 0; i < CONTROLLER_MAX; i++)
      {
        if (Settings.Protocol[i] == 2) { controllerNr = i; }
      }

      if (Settings.ControllerEnabled[controllerNr])
      {
        addRowLabel(F("IDX"));
        String id = F("TDID"); // ="taskdeviceid"
        id += controllerNr + 1;
        addNumericBox(id, Settings.TaskDeviceID[controllerNr][event->TaskIndex], 0, 9999);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      int irPin = CONFIG_PIN1;

      if (validGpio(irPin))
      {
        addLog(LOG_LEVEL_INFO, F("P088: Heatpump IR transmitter activated"));

        if (Plugin_088_irSender != nullptr)
        {
          delete Plugin_088_irSender;
        }
        Plugin_088_irSender = new (std::nothrow) IRSenderIRremoteESP8266(irPin);
      }

      if ((Plugin_088_irSender != nullptr) && (irPin == -1))
      {
        addLog(LOG_LEVEL_INFO, F("P088: Heatpump IR transmitter deactivated"));
        delete Plugin_088_irSender;
        Plugin_088_irSender = nullptr;
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      String heatpumpModel;
      unsigned int powerMode     = POWER_ON;
      unsigned int operatingMode = MODE_HEAT;
      unsigned int fanSpeed      = FAN_2;
      unsigned int temperature   = 22;
      unsigned int vDir          = VDIR_UP;
      unsigned int hDir          = HDIR_AUTO;

      String cmd = parseString(string, 1);

      if (cmd.equalsIgnoreCase(F("HEATPUMPIR")) && (Plugin_088_irSender != nullptr))
      {
        String TmpStr1;

        if (GetArgv(string.c_str(), TmpStr1, 2)) { heatpumpModel = TmpStr1; }

        if (GetArgv(string.c_str(), TmpStr1, 3)) { powerMode = str2int(TmpStr1.c_str()); }

        if (GetArgv(string.c_str(), TmpStr1, 4)) { operatingMode = str2int(TmpStr1.c_str()); }

        if (GetArgv(string.c_str(), TmpStr1, 5)) { fanSpeed = str2int(TmpStr1.c_str()); }

        if (GetArgv(string.c_str(), TmpStr1, 6)) { temperature = str2int(TmpStr1.c_str()); }

        if (GetArgv(string.c_str(), TmpStr1, 7)) { vDir = str2int(TmpStr1.c_str()); }

        if (GetArgv(string.c_str(), TmpStr1, 8)) { hDir = str2int(TmpStr1.c_str()); }
# ifdef IR_SEND_TIME
        sendHour    = node_time.hour();
        sendMinute  = node_time.minute();
        sendWeekday = node_time.weekday();
# endif // ifdef IR_SEND_TIME
        HeatpumpIR *heatpumpIR = HeatpumpIRFactory::create(heatpumpModel.c_str());

        if (heatpumpIR != nullptr) {
          enableIR_RX(false);
          heatpumpIR->send(*Plugin_088_irSender, powerMode, operatingMode, fanSpeed, temperature, vDir, hDir);
          enableIR_RX(true);

          delete heatpumpIR;
          heatpumpIR = nullptr;

          addLog(LOG_LEVEL_INFO,  F("P088: Heatpump IR code transmitted"));
# ifdef IR_DEBUG_PACKET
#  ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, IRPacket);
#  endif // ifndef BUILD_NO_DEBUG
# endif // ifdef IR_DEBUG_PACKET

          if (printToWeb)
          {
            printWebString += F("P088: Heatpump IR code transmitted");
# ifdef IR_DEBUG_PACKET
            printWebString += F(" <BR>\n"); // do both <BR> and \n to break line both in browser and curl -s
            printWebString += IRPacket;
            printWebString += F("\n");
# endif // ifdef IR_DEBUG_PACKET
          }

          // Panasonic CKP can only be turned ON/OFF by using the timer,
          // so cancel the timer in 2 minutes, after the heatpump has turned on or off
          if (strcmp_P(heatpumpModel.c_str(), PSTR("panasonic_ckp")) == 0)
          {
            panasonicCKPTimer = 120;
          }

          success = true;
        }
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      addLog(LOG_LEVEL_INFO, F("P088: Heatpump IR transmitter deactivated"));

      if (Plugin_088_irSender != nullptr)
      {
        delete Plugin_088_irSender;
        Plugin_088_irSender = nullptr;
      }

      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      if (panasonicCKPTimer > 0)
      {
        panasonicCKPTimer--;

        if (panasonicCKPTimer == 0)
        {
          PanasonicCKPHeatpumpIR *panasonicHeatpumpIR = new (std::nothrow) PanasonicCKPHeatpumpIR();

          if (panasonicHeatpumpIR != nullptr) {
            enableIR_RX(false);
            panasonicHeatpumpIR->sendPanasonicCKPCancelTimer(*Plugin_088_irSender);
            enableIR_RX(true);

            delete panasonicHeatpumpIR;
            addLog(LOG_LEVEL_INFO, F("P088: The TIMER led on Panasonic CKP should now be OFF"));
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      success = true;
      break;
    }
  }

  return success;
}

#endif // USES_P088

//#######################################################################################################
//#################################### Plugin 115: Heatpump IR ##########################################
//#######################################################################################################

#define PLUGIN_115
#define PLUGIN_ID_115         115
#define PLUGIN_NAME_115       "Heatpump IR transmitter"

/*
 * ESPEasy plugin to send air conditioner / heatpump IR signals
 * * Use the device type 'Heatpump IR transmitter' as the device type in Devices -> Edit
 * * Connect and IR LED + series resistor between the GPIO pin configured for this device and ground
 *
 * Send commands through http, like this example (assuming the IP address of the ESP node is 192.168.0.61):
 * * curl http://192.168.0.61/control?cmd=heatpumpir,panasonic_ckp,1,1,0,22,0,0
 *
 * Send commands through OpenHAB MQTT with Mosquitto, like this example,
 * assuming the 'Name' of the ESP node in ESPEasy Main Settings page is 'ESP_Easy')
 * * mosquitto_pub -t /ESP_Easy/cmd -m heatpumpir,panasonic_ckp,1,1,0,22,0,0
 *
 * ToDo: Domoticz MQTT support
 *
 * The parameters are (in this order)
 * * The type of the heatpump as a string, see the implementations of different models, like https://github.com/ToniA/arduino-heatpumpir/blob/master/MitsubishiHeatpumpIR.cpp
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

 #include <FujitsuHeatpumpIR.h>
 #include <PanasonicCKPHeatpumpIR.h>
 #include <PanasonicHeatpumpIR.h>
 #include <CarrierHeatpumpIR.h>
 #include <MideaHeatpumpIR.h>
 #include <MitsubishiHeatpumpIR.h>
 #include <SamsungHeatpumpIR.h>
 #include <SharpHeatpumpIR.h>
 #include <DaikinHeatpumpIR.h>
 #include <MitsubishiHeavyHeatpumpIR.h>
 #include <MitsubishiSEZKDXXHeatpumpIR.h>
 #include <HyundaiHeatpumpIR.h>
 #include <HisenseHeatpumpIR.h>
 #include <GreeHeatpumpIR.h>
 #include <FuegoHeatpumpIR.h>
 #include <ToshibaHeatpumpIR.h>
 #include <ToshibaDaiseikaiHeatpumpIR.h>
 #include <IVTHeatpumpIR.h>
 #include <HitachiHeatpumpIR.h>
 #include <BalluHeatpumpIR.h>
 #include <AUXHeatpumpIR.h>

// Array with all supported heatpumps
HeatpumpIR *heatpumpIR[] = {new PanasonicCKPHeatpumpIR(), new PanasonicDKEHeatpumpIR(), new PanasonicJKEHeatpumpIR(),
                            new PanasonicNKEHeatpumpIR(), new PanasonicLKEHeatpumpIR(),
                            new CarrierNQVHeatpumpIR(), new CarrierMCAHeatpumpIR(),
                            new MideaHeatpumpIR(), new FujitsuHeatpumpIR(),
                            new MitsubishiFDHeatpumpIR(), new MitsubishiFEHeatpumpIR(), new MitsubishiMSYHeatpumpIR(), new MitsubishiFAHeatpumpIR(),
                            new SamsungAQVHeatpumpIR(), new SamsungFJMHeatpumpIR(),new SharpHeatpumpIR(), new DaikinHeatpumpIR(),
                            new MitsubishiHeavyZJHeatpumpIR(), new MitsubishiHeavyZMHeatpumpIR(),
                            new MitsubishiSEZKDXXHeatpumpIR(),
                            new HyundaiHeatpumpIR(), new HisenseHeatpumpIR(),
                            new GreeGenericHeatpumpIR(), new GreeYANHeatpumpIR(), new GreeYAAHeatpumpIR(),
                            new FuegoHeatpumpIR(), new ToshibaHeatpumpIR(), new ToshibaDaiseikaiHeatpumpIR(),
                            new IVTHeatpumpIR(), new HitachiHeatpumpIR(),
                            new BalluHeatpumpIR(), new AUXHeatpumpIR(),
                            NULL};

IRSender *Plugin_115_irSender;

int panasonicCKPTimer = 0;

boolean Plugin_115(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_115;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].SendDataOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_115);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        break;
      }

    case PLUGIN_INIT:
      {
        int irPin = Settings.TaskDevicePin1[event->TaskIndex];
        if (irPin != -1)
        {
          addLog(LOG_LEVEL_INFO, "INIT: Heatpump IR transmitter activated");
          if (Plugin_115_irSender != NULL)
          {
            delete Plugin_115_irSender;
          }
          Plugin_115_irSender = new IRSenderIRremoteESP8266(irPin);
        }
        if (Plugin_115_irSender != 0 && irPin == -1)
        {
          addLog(LOG_LEVEL_INFO, "INIT: Heatpump IR transmitter deactivated");
          delete Plugin_115_irSender;
          Plugin_115_irSender = NULL;
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String heatpumpModel;
        unsigned int powerMode = POWER_ON;
        unsigned int operatingMode = MODE_HEAT;
        unsigned int fanSpeed = FAN_2;
        unsigned int temperature = 22;
        unsigned int vDir = VDIR_UP;
        unsigned int hDir = HDIR_AUTO;
        char command[80];
        command[0] = 0;
        char TmpStr1[80];
        TmpStr1[0] = 0;
        string.toCharArray(command, 80);

        String tmpString = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex) tmpString = tmpString.substring(0, argIndex);

        if (tmpString.equalsIgnoreCase("HEATPUMPIR") && Plugin_115_irSender != NULL)
        {
          if (GetArgv(command, TmpStr1, 2)) heatpumpModel = TmpStr1;
          if (GetArgv(command, TmpStr1, 3)) powerMode = str2int(TmpStr1);
          if (GetArgv(command, TmpStr1, 4)) operatingMode = str2int(TmpStr1);
          if (GetArgv(command, TmpStr1, 5)) fanSpeed = str2int(TmpStr1);
          if (GetArgv(command, TmpStr1, 6)) temperature = str2int(TmpStr1);
          if (GetArgv(command, TmpStr1, 7)) vDir = str2int(TmpStr1);
          if (GetArgv(command, TmpStr1, 8)) hDir = str2int(TmpStr1);

          int i = 0;
          do
          {
            const char* shortName = heatpumpIR[i]->model();
            const char* longName = heatpumpIR[i]->info();

            if (strcmp_P(heatpumpModel.c_str(), shortName) == 0)
            {
              Serial.print(F("Found: "));
              Serial.print(heatpumpModel);
              Serial.print(F(" as index: "));
              Serial.println(i);

              heatpumpIR[i]->send(*Plugin_115_irSender, powerMode, operatingMode, fanSpeed, temperature, vDir, hDir);
              success = true;

              addLog(LOG_LEVEL_INFO, "Heatpump IR code transmitted");
              if (printToWeb)
              {
                printWebString += F("Heatpump IR code transmitted");
              }

              // Panasonic CKP can only be turned ON/OFF by using the timer,
              // so cancel the timer in 2 minutes, after the heatpump has turned on or off
              if (strcmp(heatpumpModel.c_str(), "panasonic_ckp") == 0)
              {
                panasonicCKPTimer = 120;
              }

              break;
            }
          }
          while (heatpumpIR[++i] != NULL);
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
            PanasonicCKPHeatpumpIR *panasonicHeatpumpIR = new PanasonicCKPHeatpumpIR();
            panasonicHeatpumpIR->sendPanasonicCKPCancelTimer(*Plugin_115_irSender);
            Serial.println("The TIMER led on Panasonic CKP should now be OFF");
          }
        }
        break;
      }
  }
  return success;
}

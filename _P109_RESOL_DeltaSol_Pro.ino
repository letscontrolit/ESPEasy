#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//################################## Plugin 109 RESOL DeltaSol Pro ######################################
//#######################################################################################################

#include <SoftwareSerial.h>

#define PLUGIN_109
#define PLUGIN_ID_109         109
#define PLUGIN_NAME_109       "RESOL DeltaSol Pro [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_109 "register"

// uart rx-buffer size
#define RXBUF_SIZE    32

// RESOL register options
#define REG_TEMP_SENSOR_1   0
#define REG_TEMP_SENSOR_2   1
#define REG_TEMP_SENSOR_3   2
#define REG_RELAIS_1        3
#define REG_RELAIS_2        4

// total number of selectable registers
#define RESOL_REGISTER_OPTIONS 5

uint8_t Plugin_109_UART_Pin;
boolean Plugin_109_init = false;
boolean valuesValid = false;
uint8_t RXbuf[RXBUF_SIZE];
uint8_t RXbuf_IDX;
int16_t T1, T2 ,T3;
uint8_t R1, R2;

SoftwareSerial *Plugin_109_UART;


// RESOL CRC check
uint8_t VBus_CalcCrc(uint8_t *buf, uint8_t len)
{
  uint8_t crc;

  crc = 0x7F;

  for (uint8_t i = 0; i < len; i++)
    crc = (crc - buf [i]) & 0x7F;

  return crc;
}


boolean Plugin_109(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_109;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_109);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_109));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        int16_t choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        String options[RESOL_REGISTER_OPTIONS];
        uint optionValues[RESOL_REGISTER_OPTIONS];

        optionValues[0] = REG_TEMP_SENSOR_1;
        options[0] = F("Temperature sensor 1");
        optionValues[1] = REG_TEMP_SENSOR_2;
        options[1] = F("Temperature sensor 2");
        optionValues[2] = REG_TEMP_SENSOR_3;
        options[2] = F("Temperature sensor 3");
        optionValues[3] = REG_RELAIS_1;
        options[3] = F("Relais 1");
        optionValues[4] = REG_RELAIS_2;
        options[4] = F("Relais 2");

        string += F("<TR><TD>Register:<TD><select name='plugin_109_register'>");
        for (byte x = 0; x < RESOL_REGISTER_OPTIONS; x++)
        {
          string += F("<option value='");
          string += optionValues[x];
          string += "'";
          if (choice == optionValues[x])
            string += F(" selected");
          string += ">";
          string += options[x];
          string += F("</option>");
        }
        string += F("</select>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_109_register");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        Plugin_109_init = false; // Force device setup next time
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        addLog(LOG_LEVEL_INFO, (char*)"INIT : RESOL DeltaSol Pro");

        Plugin_109_UART = new SoftwareSerial(Settings.TaskDevicePin1[event->TaskIndex], Settings.TaskDevicePin2[event->TaskIndex], false, (2 * RXBUF_SIZE));		// set RX und Tx Pin number, no invert, buffer

        Plugin_109_init = true;

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_109_init == true)
        {
          // buffer filled with more than twize the frame size bytes? -> at least one complete frame should then be available
          if (Plugin_109_UART->available() > (2 * 30))
          {
            // search for first appearance of SOF (0xAA)
            while ((Plugin_109_UART->available()) && (Plugin_109_UART->read() != 0xAA));

            RXbuf_IDX = 0;
            RXbuf[RXbuf_IDX] = 0;

            while ((Plugin_109_UART->available()) && (RXbuf[RXbuf_IDX] != 0xAA) && (RXbuf_IDX < (RXBUF_SIZE - 1)))
              RXbuf[++RXbuf_IDX] = Plugin_109_UART->read();

            if (RXbuf[RXbuf_IDX] == 0xAA)
            {
              // ********* DeltaSol Pro registers *********

              // check CRC of first and second group
              if ( (VBus_CalcCrc(&RXbuf[10], 5) == RXbuf[15]) && (VBus_CalcCrc(&RXbuf[16], 5) == RXbuf[21]) )
              {
                // Temperature sensor 1
                T1 = RXbuf[10] + ((RXbuf[14] & (1 << 0)) << 7);
                T1 += (RXbuf[11] + ((RXbuf[14] & (1 << 1)) << 6)) << 8;

                // Temperature sensor 2
                T2 = RXbuf[12] + ((RXbuf[14] & (1 << 2)) << 5);
                T2 += (RXbuf[13] + ((RXbuf[14] & (1 << 3)) << 4)) << 8;


                // Temperature sensor 3
                T3 = RXbuf[16] + ((RXbuf[20] & (1 << 0)) << 7);
                T3 += (RXbuf[17] + ((RXbuf[20] & (1 << 1)) << 6)) << 8;

                // Relais/speed
                R1 = RXbuf[18] + ((RXbuf[20] & (1 << 2)) << 5);
                R2 = RXbuf[19] + ((RXbuf[20] & (1 << 3)) << 4);

                valuesValid = true;
              } else
                valuesValid = false;
            }

            Plugin_109_UART->flush();
          }
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        float regValue;
        String log = F("RESOL : ");

        if (valuesValid == true)
        {
          switch (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
            {
              case REG_TEMP_SENSOR_1:
                {
                  log += F("T1");
                  regValue = (float)T1 / 10;
                  break;
                }

              case REG_TEMP_SENSOR_2:
                {
                  log += F("T2");
                  regValue = (float)T2 / 10;
                  break;
                }

              case REG_TEMP_SENSOR_3:
                {
                  log += F("T3");
                  regValue = (float)T3 / 10;
                  break;
                }

              case REG_RELAIS_1:
                {
                  log += F("R1");
                  regValue = R1;
                  break;
                }

              case REG_RELAIS_2:
                {
                  log += F("R2");
                  regValue = R2;
                  break;
                }
              default:
                {
                  log += F("unknown");
                  regValue = 0;
                  break;
                }
            }

          log += F("=");
          log += regValue;
          addLog(LOG_LEVEL_INFO, log);

          UserVar[event->BaseVarIndex] = regValue;
          success = true;
          break;
        }
    }
  }
  return success;
}
#endif

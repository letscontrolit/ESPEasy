//#######################################################################################################
//#################################### Plugin 053: Plantower PMSx003 ####################################
//#######################################################################################################
//
// http://www.aqmd.gov/docs/default-source/aq-spec/resources-page/plantower-pms5003-manual_v2-3.pdf?sfvrsn=2
//
// The PMSx005 are particle sensors. Particles are measured by blowing air though the enclosue and,
// togther with a laser, count the amount of particles. These sensors have an integrated microcontroller
// that counts particles and transmits measurement data over the serial connection.

#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_053
#define PLUGIN_ID_053 53
#define PLUGIN_NAME_053 "Particle Sensor - PMSx003"
#define PLUGIN_VALUENAME1_053 "pm1.0"
#define PLUGIN_VALUENAME2_053 "pm2.5"
#define PLUGIN_VALUENAME3_053 "pm10"

boolean Plugin_053_init = false;
byte Plugin_PMSx003_UART = 0;
byte timer = 0;
boolean values_received = false;

void SerialRead16(uint16_t* value, uint16_t* checksum)
{
  uint8_t data_high, data_low;

  while(Serial.available() == 0) {};
  data_high = Serial.read();
  while(Serial.available() == 0) {};
  data_low = Serial.read();
  *value = data_low;
  *value |= (data_high << 8);

  if (checksum != NULL)
  {
    *checksum += data_high;
    *checksum += data_low;
  }
// Low-level logging to see data from sensor
#if 0
  String log = F("PMSx003 : byte high=0x");
  log += String(data_high,HEX);
  log += F(" byte low=0x");
  log += String(data_low,HEX);
  log += F(" result=0x");
  log += String(*value,HEX);
  addLog(LOG_LEVEL_INFO, log);
#endif
}

boolean Plugin_053(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_053;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_053);
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_053));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_053));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_053));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Serial.begin(9600);
        // Toggle 'reset' to assure we start reading header
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], OUTPUT);
        digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], LOW);
        delay(250);
        digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], HIGH);
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);

        // Put device in suspend by driving 'set'low
        pinMode(Settings.TaskDevicePin2[event->TaskIndex], OUTPUT);
        digitalWrite(Settings.TaskDevicePin2[event->TaskIndex], HIGH);

        Plugin_053_init = true;
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        // Do measurements only every 30 seconds
        #if 0
        if ((timer++) % 30 == 0)
          if (digitalRead(Settings.TaskDevicePin2[event->TaskIndex]))
            digitalWrite(Settings.TaskDevicePin2[event->TaskIndex], LOW);
          else
            digitalWrite(Settings.TaskDevicePin2[event->TaskIndex], HIGH);
        #endif
        success = true;
      }

    case PLUGIN_SERIAL_IN:
      {
        if (Plugin_053_init)
        {
          uint16_t checksum = 0, checksum2 = 0;
          uint16_t framelength = 0;
          uint16_t data[13];
          byte data_low, data_high;
          int i = 0;
          String log;

          // Check for 2-byte header. A total of 32 bytes is to be received.
          if (Serial.read() == 0x42 && Serial.read() == 0x4d)
          {
            checksum += 0x42 + 0x4d;
            SerialRead16(&framelength, &checksum);
            if (framelength != 28)
            {
              log = F("PMSx003 : invalid framelength - ");
              log+=framelength;
              addLog(LOG_LEVEL_INFO, log);
              break;
            }

            for (i = 0; i < 13; i++)
            {
              SerialRead16(&data[i], &checksum);
            }

            log = F("PMSx003 : pm1.0=");
            log += data[0];
            log += F(", pm2.5=");
            log += data[1];
            log += F(", pm10=");
            log += data[2];
            log += F(", pm1.0a=");
            log += data[3];
            log += F(", pm2.5a=");
            log += data[4];
            log += F(", pm10a=");
            log += data[5];
            addLog(LOG_LEVEL_INFO, log);

            log = F("PMSx003 : count/0.1L : 0.3um=");
            log += data[6];
            log += F(", 0.5um=");
            log += data[7];
            log += F(", 1.0um=");
            log += data[8];
            log += F(", 2.5um=");
            log += data[9];
            log += F(", 5.0um=");
            log += data[10];
            log += F(", 10um=");
            log += data[11];

            addLog(LOG_LEVEL_INFO, log);

            // Compare checksums
            SerialRead16(&checksum2, NULL);
            if (checksum == checksum2)
            {
              /* Data is checked and good, fill in output */
              UserVar[event->BaseVarIndex]     = data[3];
              UserVar[event->BaseVarIndex + 1] = data[4];
              UserVar[event->BaseVarIndex + 2] = data[5];
              values_received = true;
            }
          }
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // Prevent sending data to controller when nothing received yet
        if (values_received)
          success = true;
        break;
      }
  }
  return success;
}
#endif // PLUGIN_BUILD_TESTING

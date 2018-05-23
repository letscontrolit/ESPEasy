#ifdef USES_P005
//#######################################################################################################
//######################## Plugin 005: Temperature and Humidity sensor DHT 11/22 ########################
//#######################################################################################################

#define PLUGIN_005
#define PLUGIN_ID_005         5
#define PLUGIN_NAME_005       "Environment - DHT11/12/22  SONOFF2301/7021"
#define PLUGIN_VALUENAME1_005 "Temperature"
#define PLUGIN_VALUENAME2_005 "Humidity"

uint8_t Plugin_005_DHT_Pin;

boolean Plugin_005(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_005;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_TEMP_HUM;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_005);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_005));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_005));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        const String options[] = { F("DHT 11"), F("DHT 22"), F("DHT 12"), F("Sonoff am2301"), F("Sonoff si7021") };
        int indices[] = { 11, 22, 12, 23, 70 };

        addFormSelector(F("DHT Type"), F("plugin_005_dhttype"), 5, options, indices, Settings.TaskDevicePluginConfig[event->TaskIndex][0] );

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_005_dhttype"));

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        byte dht_dat[5];
        byte i;
        boolean error = false;

        byte Par3 = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        Plugin_005_DHT_Pin = Settings.TaskDevicePin1[event->TaskIndex];

        pinMode(Plugin_005_DHT_Pin, OUTPUT);
        digitalWrite(Plugin_005_DHT_Pin, LOW);              // Pull low
        if(Par3 == 11 || Par3 == 22 || Par3 == 12)  delay(18);
        else if (Par3 == 23 )         delayMicroseconds(900);
        else if (Par3 == 70 )         delayMicroseconds(500);
        pinMode(Plugin_005_DHT_Pin, INPUT);                 // change pin to input
        delayMicroseconds(50);

        error = waitState(0);
        if(error)
        {   logError(event, F("DHT  : no Reading !"));
            break;
        }
        error = waitState(1);
        if(error)
        {   logError(event, F("DHT  : no Reading !"));
            break;
        }
        noInterrupts();
        error = waitState(0);
        if(error)
        {   logError(event, F("DHT  : no Reading !"));
            break;
        }
        for (i = 0; i < 5; i++)
        {
            byte data = Plugin_005_read_dht_dat();
            if(data == -1)
            {   logError(event, F("DHT  : protocol timeout!"));
                break;
            }
            dht_dat[i] = data;
        }
        interrupts();

              // Checksum calculation is a Rollover Checksum by design!
        byte dht_check_sum = (dht_dat[0] + dht_dat[1] + dht_dat[2] + dht_dat[3]) & 0xFF; // check check_sum
        if (dht_dat[4] != dht_check_sum)
        {
            logError(event, F("DHT  : checksum error!"));
            break;
        }

        float temperature = NAN;
        float humidity = NAN;
        if (Par3 == 11)
        {
          temperature = float(dht_dat[2]); // Temperature
          humidity = float(dht_dat[0]); // Humidity
        }
        else if (Par3 == 12)
        {
            temperature = float(dht_dat[2]*10 + (dht_dat[3] & 0x7f)) / 10.0; // Temperature
            if (dht_dat[3] & 0x80) { temperature = -temperature; } // Negative temperature
            humidity = float(dht_dat[0]*10+dht_dat[1]) / 10.0; // Humidity
        }
        else if (Par3 == 22 || Par3 == 23 || Par3 == 70)
        {
          if (dht_dat[2] & 0x80) // negative temperature
            temperature = -0.1 * word(dht_dat[2] & 0x7F, dht_dat[3]);
          else
            temperature = 0.1 * word(dht_dat[2], dht_dat[3]);
          humidity = 0.1 * word(dht_dat[0], dht_dat[1]); // Humidity
        }

        if (temperature == NAN || humidity == NAN)
        {     logError(event, F("DHT  : invalid NAN reading !"));
              break;
        }

        UserVar[event->BaseVarIndex] = temperature;
        UserVar[event->BaseVarIndex + 1] = humidity;
        String log = F("DHT  : Temperature: ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        log = F("DHT  : Humidity: ");
        log += UserVar[event->BaseVarIndex + 1];
        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }
  }
  return success;
}


/*********************************************************************************************\
* DHT sub to log an error
\*********************************************************************************************/
void logError(struct EventStruct *event, String text)
{
  addLog(LOG_LEVEL_INFO, text);
  UserVar[event->BaseVarIndex] = NAN;
  UserVar[event->BaseVarIndex + 1] = NAN;
}

/*********************************************************************************************\
* DHT sub to wait until a pin is in a certiin state
\*********************************************************************************************/
boolean waitState(int state)
{
  byte counter = 0;
  while (( digitalRead(Plugin_005_DHT_Pin) != state) && (counter < 100))
  {
    delayMicroseconds(1);
    counter++;
  }
  if( counter < 100) return false;
  return true;
}

/*********************************************************************************************\
* DHT sub to get an 8 bit value from the receiving bitstream
\*********************************************************************************************/
int Plugin_005_read_dht_dat(void)
{
  byte i = 0;
  byte result = 0;
  byte counter = 0;
  for (i = 0; i < 8; i++)
  {
    while ((!digitalRead(Plugin_005_DHT_Pin)) && (counter < 100))
    {
      delayMicroseconds(1);
      counter++;
    }
    if (counter >= 100)
    {
      return -1;
    }
    delayMicroseconds(35); // was 30
    if (digitalRead(Plugin_005_DHT_Pin))
      result |= (1 << (7 - i));
    counter = 0;
    while ((digitalRead(Plugin_005_DHT_Pin)) && (counter < 100))
    {
      delayMicroseconds(1);
      counter++;
    }
    if (counter >= 100)
    {
      return -1;
    }
  }
  return result;
}
#endif // USES_P005

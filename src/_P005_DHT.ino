#ifdef USES_P005
//#######################################################################################################
//######################## Plugin 005: Temperature and Humidity sensor DHT 11/22 ########################
//#######################################################################################################

#include "_Plugin_Helper.h"

#define PLUGIN_005
#define PLUGIN_ID_005         5
#define PLUGIN_NAME_005       "Environment - DHT11/12/22  SONOFF2301/7021"
#define PLUGIN_VALUENAME1_005 "Temperature"
#define PLUGIN_VALUENAME2_005 "Humidity"

#define P005_DHT11    11
#define P005_DHT12    12
#define P005_DHT22    22
#define P005_AM2301   23
#define P005_SI7021   70

#define P005_error_no_reading          1
#define P005_error_protocol_timeout    2
#define P005_error_checksum_error      3
#define P005_error_invalid_NAN_reading 4
#define P005_info_temperature          5
#define P005_info_humidity             6

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
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
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

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_bidirectional(F("Data"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        const String options[] = { F("DHT 11"), F("DHT 22"), F("DHT 12"), F("Sonoff am2301"), F("Sonoff si7021") };
        int indices[] = { P005_DHT11, P005_DHT22, P005_DHT12, P005_AM2301, P005_SI7021 };

        addFormSelector(F("Sensor model"), F("p005_dhttype"), 5, options, indices, PCONFIG(0) );

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("p005_dhttype"));

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        success = P005_do_plugin_read(event);
        break;
      }
  }
  return success;
}


/*********************************************************************************************\
* DHT sub to log an error
\*********************************************************************************************/
void P005_log(struct EventStruct *event, int logNr)
{
  bool isError = true;
  String text = F("DHT  : ");
  switch (logNr) {
    case P005_error_no_reading:          text += F("No Reading"); break;
    case P005_error_protocol_timeout:    text += F("Protocol Timeout"); break;
    case P005_error_checksum_error:      text += F("Checksum Error"); break;
    case P005_error_invalid_NAN_reading: text += F("Invalid NAN reading"); break;
    case P005_info_temperature:
      text += F("Temperature: ");
      text += UserVar[event->BaseVarIndex];
      isError = false;
      break;
    case P005_info_humidity:
      text += F("Humidity: ");
      text += UserVar[event->BaseVarIndex + 1];
      isError = false;
      break;
  }
  addLog(LOG_LEVEL_INFO, text);
  if (isError) {
    UserVar[event->BaseVarIndex] = NAN;
    UserVar[event->BaseVarIndex + 1] = NAN;
  }
}

/*********************************************************************************************\
* DHT sub to wait until a pin is in a certain state
\*********************************************************************************************/
boolean P005_waitState(int state)
{
  unsigned long timeout = micros() + 100;
  while (digitalRead(Plugin_005_DHT_Pin) != state)
  {
    if (usecTimeOutReached(timeout)) return false;
    delayMicroseconds(1);
  }
  return true;
}

/*********************************************************************************************\
* Perform the actual reading + interpreting of data.
\*********************************************************************************************/
bool P005_do_plugin_read(struct EventStruct *event) {
  byte i;

  byte Par3 = PCONFIG(0);
  Plugin_005_DHT_Pin = CONFIG_PIN1;

  pinMode(Plugin_005_DHT_Pin, OUTPUT);
  digitalWrite(Plugin_005_DHT_Pin, LOW);              // Pull low
  
  switch (Par3) {
    case P005_DHT11:  delay(19); break;  // minimum 18ms
    case P005_DHT22:  delay(2);  break;  // minimum 1ms
    case P005_DHT12:  delay(200); break; // minimum 200ms
    case P005_AM2301: delayMicroseconds(900); break;
    case P005_SI7021: delayMicroseconds(500); break;
  }
  
  pinMode(Plugin_005_DHT_Pin, INPUT_PULLUP);
  
  switch (Par3) {
    case P005_DHT11:
    case P005_DHT22:
    case P005_DHT12:
    case P005_AM2301:
      delayMicroseconds(50);
      break;
    case P005_SI7021:
      // See: https://github.com/letscontrolit/ESPEasy/issues/1798
      delayMicroseconds(20);
      break;
  }

  noInterrupts();
  if(!P005_waitState(0)) {interrupts(); P005_log(event, P005_error_no_reading); return false; }
  if(!P005_waitState(1)) {interrupts(); P005_log(event, P005_error_no_reading); return false; }
  if(!P005_waitState(0)) {interrupts(); P005_log(event, P005_error_no_reading); return false; }

  bool readingAborted = false;
  byte dht_dat[5];
  for (i = 0; i < 5 && !readingAborted; i++)
  {
      int data = Plugin_005_read_dht_dat();
      if(data == -1)
      {   P005_log(event, P005_error_protocol_timeout);
          readingAborted = true;
      }
      dht_dat[i] = data;
  }
  interrupts();
  if (readingAborted)
    return false;

  // Checksum calculation is a Rollover Checksum by design!
  byte dht_check_sum = (dht_dat[0] + dht_dat[1] + dht_dat[2] + dht_dat[3]) & 0xFF; // check check_sum
  if (dht_dat[4] != dht_check_sum)
  {
      P005_log(event, P005_error_checksum_error);
      return false;
  }

  float temperature = NAN;
  float humidity = NAN;
  switch (Par3) {
    case P005_DHT11:
    case P005_DHT12:
      temperature = float(dht_dat[2]*10 + (dht_dat[3] & 0x7f)) / 10.0f; // Temperature
      if (dht_dat[3] & 0x80) { temperature = -temperature; } // Negative temperature
      humidity = float(dht_dat[0]*10+dht_dat[1]) / 10.0f; // Humidity
      break;
    case P005_DHT22:
    case P005_AM2301:
    case P005_SI7021:
      if (dht_dat[2] & 0x80) // negative temperature
        temperature = -0.1f * word(dht_dat[2] & 0x7F, dht_dat[3]);
      else
        temperature = 0.1f * word(dht_dat[2], dht_dat[3]);
      humidity = 0.1f * word(dht_dat[0], dht_dat[1]); // Humidity
      break;
  }

  if (isnan(temperature) || isnan(humidity))
  {     P005_log(event, P005_error_invalid_NAN_reading);
        return false;
  }

  UserVar[event->BaseVarIndex] = temperature;
  UserVar[event->BaseVarIndex + 1] = humidity;
  P005_log(event, P005_info_temperature);
  P005_log(event, P005_info_humidity);
  return true;
}



/*********************************************************************************************\
* DHT sub to get an 8 bit value from the receiving bitstream
\*********************************************************************************************/
int Plugin_005_read_dht_dat(void)
{
  byte i = 0;
  byte result = 0;
  for (i = 0; i < 8; i++)
  {
    if (!P005_waitState(1))  return -1;
    delayMicroseconds(35); // was 30
    if (digitalRead(Plugin_005_DHT_Pin))
      result |= (1 << (7 - i));
    if (!P005_waitState(0))  return -1;
  }
  return result;
}
#endif // USES_P005

#include "../PluginStructs/P005_data_struct.h"



// Macros to perform direct access on GPIOs
// Macros written by Paul Stoffregen
// See: https://github.com/PaulStoffregen/OneWire/blob/master/util/
#include <GPIO_Direct_Access.h>



P005_data_struct::P005_data_struct(struct EventStruct *event) {
  Par3 = PCONFIG(0);
  Plugin_005_DHT_Pin = CONFIG_PIN1;
}

/*********************************************************************************************\
* DHT sub to log an error
\*********************************************************************************************/
void P005_data_struct::P005_log(struct EventStruct *event, int logNr)
{
  bool isError = true;
  String text = F("DHT  : ");
  switch (logNr) {
    case P005_error_no_reading:          text += F("No Reading"); break;
    case P005_error_protocol_timeout:    text += F("Protocol Timeout"); break;
    case P005_error_checksum_error:      text += F("Checksum Error"); break;
    case P005_error_invalid_NAN_reading: text += F("Invalid NAN reading"); break;
    case P005_info_temperature:
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        text += F("Temperature: ");
        text += formatUserVarNoCheck(event->TaskIndex, 0);
      }
      isError = false;
      break;
    case P005_info_humidity:
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        text += F("Humidity: ");
        text += formatUserVarNoCheck(event->TaskIndex, 1);
      }
      isError = false;
      break;
  }
  addLogMove(LOG_LEVEL_INFO, text);
  if (isError) {
    UserVar[event->BaseVarIndex] = NAN;
    UserVar[event->BaseVarIndex + 1] = NAN;
  }
}

/*********************************************************************************************\
* DHT sub to wait until a pin is in a certain state
\*********************************************************************************************/
bool P005_data_struct::P005_waitState(int state)
{
  const uint64_t timeout = getMicros64() + 100;
  IO_REG_TYPE mask IO_REG_MASK_ATTR = PIN_TO_BITMASK(Plugin_005_DHT_Pin);

  while (DIRECT_READ(reg, mask) != state)
  {
    if (usecTimeOutReached(timeout)) return false;
    delayMicroseconds(1);
  }
  return true;
}

/*********************************************************************************************\
* Perform the actual reading + interpreting of data.
\*********************************************************************************************/
bool P005_data_struct::P005_do_plugin_read(struct EventStruct *event) {
  uint8_t i;

  // Need to call this first to make sure the pull-up is enabled
  // Direct access functions cannot set the pull-up, or at least I have no clue where to set it.
  pinMode(Plugin_005_DHT_Pin, INPUT_PULLUP);


  IO_REG_TYPE mask IO_REG_MASK_ATTR = PIN_TO_BITMASK(Plugin_005_DHT_Pin);

  // To begin asking the DHT22 for humidity and temperature data, 
  // Start sequence to get data from a DHTxx sensor:
  // Pin must be a logic 0 (low) for at least 500 microseconds (DHT22, others may need different timing)
  // followed by a logic 1 (high). 
  // 
  // Response from DHTxx: (N = 80 usec for DHT22)
  // Low for N usec  
  // Hight for N usec
  // Low for N usec

  DIRECT_MODE_OUTPUT(reg, mask);
  DIRECT_WRITE_LOW(reg, mask);           // Pull low
  
  switch (Par3) {
    case P005_DHT11:  delay(19); break;  // minimum 18ms
    case P005_DHT22:  delay(2);  break;  // minimum 1ms
    case P005_DHT12:  delay(200); break; // minimum 200ms
    case P005_AM2301: delayMicroseconds(900); break;
    case P005_SI7021: delayMicroseconds(500); break;
  }
  DIRECT_MODE_INPUT(reg, mask);  // FIXME TD-er: Not sure if pull-up is still enabled
  
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

  bool readingAborted = false;
  uint8_t dht_dat[5];

  bool receive_start;
  noInterrupts();
  receive_start = P005_waitState(0) && P005_waitState(1) && P005_waitState(0);
  if (receive_start) {
    for (i = 0; i < 5 && !readingAborted; i++)
    {
        int data = Plugin_005_read_dht_dat();
        if(data == -1)
        {   
            readingAborted = true;
        }
        dht_dat[i] = data;
    }
  }
  interrupts();
  if (!receive_start) {
    P005_log(event, P005_error_no_reading); 
    return false; 
  }

  if (readingAborted) {
    P005_log(event, P005_error_protocol_timeout);
    return false;
  }

  // Checksum calculation is a Rollover Checksum by design!
  uint8_t dht_check_sum = (dht_dat[0] + dht_dat[1] + dht_dat[2] + dht_dat[3]) & 0xFF; // check check_sum
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
int P005_data_struct::Plugin_005_read_dht_dat(void)
{
  // Timing for a single bit:
  // Logic "1":  50 usec low, 70 usec high
  // Logic "0":  50 usec low, 26 usec high
  IO_REG_TYPE mask IO_REG_MASK_ATTR = PIN_TO_BITMASK(Plugin_005_DHT_Pin);

  uint8_t i = 0;
  uint8_t result = 0;
  for (i = 0; i < 8; i++)
  {
    if (!P005_waitState(1))  return -1;
    delayMicroseconds(35); // was 30
    if (DIRECT_READ(reg, mask))
      result |= (1 << (7 - i));
    if (!P005_waitState(0))  return -1;
  }
  return result;
}
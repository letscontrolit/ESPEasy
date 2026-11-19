#include "_Plugin_Helper.h"
#ifdef USES_P046

// #######################################################################################################
// #################################### Plugin 046: Ventus W266 ################################
// #######################################################################################################

// Purpose: Sniff the data received by the Ventus W266 display unit and send it to Domoticz
// Status : "Initial release"

// This plugin can be used on a esp8266 connected to the receiver module inside a Ventus W266 or
// Renkforce W205GU display unit. The plugin then reads the data send from the remote sensor unit and
// can send the data to Domoticz or an other controller. The plugin does not read the data of the
// sensors in the display unit such as indoor temperature and air pressure.

// The display unit has a RFM31 receiver but the pinout of this receiver is identical to the pinout
// of a RFM01 receiver. We need to connect 4 wires to read the SPI exchange between the host and the
// receiver. Because we need to read both MOSI and MISO signals the hardware SPI is unusable and we
// use bit-banging to achieve the same result.

// Connect pins 5-8 from the "RFM31BJ-S1" to the pins you defined in the webgui.
// Pinout of the RFM31: 5-MOSI, 6-SCLK, 7-nSEL (active low CS), 8-MISO, 14-GND.
// In my original setup these were connected to 4, 12, 14 and 5.
// Try to avoid GPIO 15 as nSEL line because if the line is high during a reboot, and it is mostly high,
// the boot will fail!

// The Ventus W266 remote has the following sensor outputs:
// Humidity, Temperature, Wind direction, Wind average, Wind gust, Rainfall, UV and Lightning. That is
// more than te maximum of 4 values per device for espeasy. The plugins functionality is therefore
// divided per sensor group. To read all the sensor data you need to run 6 instances of the plugin.

// The plugin can (and should) be used more then one time, however only one plugin instance can be
// the main plugin. The plugin function can be selected by a dropdown and only the main plugin has
// the ability to set the I/O lines.

// The plugin uses two buffers, ine for the ISR routines and one for the other plugin instances.

// This plugin was originally released into the playground as Plugin 186, it is assimilated into the core as 046.
// Why plugin nr 186? Well 266 is not possible, 206 (id on the back of the unit) is nit within the
// playground range and the W186 is an additional external thermometer for the Ventus W266.
// The Ventus W266 is also known as the Renkforce W205GU.

// TaskDevicePluginConfig[x][0] = Instance function
// TaskDevicePluginConfig[x][1] = MOSI (e.g. GPOI pin 4)
// TaskDevicePluginConfig[x][2] = SCLK (e.g. GPIO pin 12)
// TaskDevicePluginConfig[x][3] = nSEL (e.g. GPIO pin 14)
// TaskDevicePluginConfig[x][4] = MISO (e.g. GPIO pin 5)
// If you use GPIO 4&5, please disable the I2C/SPI option in the hardware tab.

// The buffers contain the following data:
// hhIDhh 1A tlth ?b tlth wb alahglgh rlrh?? uv ld?? lllhcrc
// 7F9827 1A B100 00 B100 06 00000000 1E0000 00 3F8A 2A0017
//  0 1 2  3  4 5  6  7 8  9  0 1 2 3  4 5 6  7  8 9  0 1 2
// hh=header > This is actually not part of the real payload but a wanted artifact of the sniffing method.
// hh=humidity (bcd) > Humidity is bcd encoded
// tlth=temperature-low/temphigh (*10) > Temperature is stored as a 16bit integer holding the temperature in Celsius * 10 but low uint8_t
// first.
// b=battery (1=low) > This uint8_t is 00 but 01 when the battery of the transmitter runs low
// wb=bearing (cw0-15) > The windbearing in 16 clockwise steps (0 = north, 4 = east, 8 = south and C = west)
// alah=windaverage-low/high (m/s/2) > A 16 bit int holding the wind avarage in m/s * 2, low uint8_t first
// rlrh=rainfall-low/high (1/4mm) > A 16 bit int holding the wind gust in m/s * 2, low uint8_t first
// uv=uvindex (*10) > The UV value * 10
// ld=lightningstorm-distance (km, 3F is max) > The distance to the stormfront in km
// lllh=strikecount-low/high (#) > A 16 bit integer holding the number of detected lightning strikes, low uint8_t first
// crc > poly 0x31, init 0xff, revin&revout, xorout 0x00. Like Maxim 1-wire but with a 0xff initvalue. Crc is calculated over bytes 1-22

// Events:
//   None

// Commands:
//   None

// Current state / limitations:
//    1.0 Initial release. All values are always visible although sometimes only one is really used with domoticz.
//        Needs work on a sliding window for the lightning detection.
//        When porting be beware of the fact that the esp8266 uses 4 bytes as an int when reading the temperature.
//
// This plugin is based on the work of the Plugin 199: RF KaKu receiver/sender and Plugin 026: Analog.
// CRC calculation is based on the works by Paul Stoffregen from the 1-Wire arduino library. Special
// thanks to Greg Cook and the team behind reveng.sourceforge.net.

/** Changelog
 * 2026-11-05 TheItschi:  Enable Interval setting (TimerOption), was taken from 'Sleep time' (default 0 = never read)
 *                        Fix inverted 5 min interval check for rain rate and lightning strikes (were always 0)
 *                        Fix lightning reference count init (-1 on unsigned), use first valid frame as reference
 *                        Add raw lightning strike counter as value 2
 *                        Fix ISR buffer overflow check writing 1 byte past ISR buffer
 *                        Number of values, VType and default value names now depend on the selected function
 *                        UV has a 2nd value (always 0), as a Domoticz UV device expects "UV;temp"
 *                        Use the default interval for existing tasks that were saved with interval 0
 * 2025-01-12 tonhuisman: Add support for MQTT AutoDiscovery (not supported)
 *                        Uncrustify source
 * 2023-07-24 tonhuisman: Fix compilation issues
 * 2023-07-23 tonhuisman: Add Changelog
 *                        Only destruct plugin instance if the 'main' task is being stopped (PLUGIN_EXIT)
 */

// edwin: Disabled for now: hardware is not generic enough and  uses lots of ram and iram,
// #ifdef PLUGIN_BUILD_DISABLED

# define PLUGIN_046_DEBUG            true // Shows received frames and crc in log@INFO

# define PLUGIN_046                       // Mandatory framework constants
# define PLUGIN_ID_046               46
# define PLUGIN_NAME_046             "Hardware - Ventus W266"

# define Plugin_046_MagicByte        0x7F // When we read this uint8_t on MOSI, switch to MISO
# define Plugin_046_RAW_BUFFER_SIZE  24   // Payload is 23 bytes, added space for header
# define Plugin_046_Payload          23

struct P046_data_struct {
  int8_t Plugin_046_MOSIpin = -1; // GPIO pins
  int8_t Plugin_046_SCLKpin = -1;
  int8_t Plugin_046_nSELpin = -1;
  int8_t Plugin_046_MISOpin = -1;

  // Vars used in data collection:
  uint8_t Plugin_046_ISR_Buffer[Plugin_046_RAW_BUFFER_SIZE]; // Buffer used in ISR routine
  // Test data: volatile uint8_t Plugin_046_databuffer[] = {0x7F, 0x98, 0x33, 0x1A, 0xB0, 0x00, 0x00, 0xB0, 0x00, 0x0E, 0x00, 0x00, 0x00,
  // 0x00, 0x11, 0x00, 0x00, 0x00, 0x3F, 0x8A, 0x25, 0x00, 0x49, 0x00}; // Buffer used by other instances
  uint8_t Plugin_046_databuffer[Plugin_046_RAW_BUFFER_SIZE]; // Buffer used by other instances
  bool    Plugin_046_ReceiveActive = false;                  // Active session in progress
  bool    Plugin_046_MasterSlave   = false;                  // Which pin o read? false=MOSI, true=MISO
  bool    Plugin_046_newData       = false;                  // "Valid" data ready, please process
  uint8_t Plugin_046_bitpointer;                             // Pointer for received bit
  uint8_t Plugin_046_bytepointer;                            // Pointer for ISR receive buffer
  uint8_t Plugin_046_receivedData;                           // Byte to store received bits
};

# include "./ESPEasy-Globals.h"

P046_data_struct*P046_data = nullptr;

// TODO TD-er:  Not sure what to do with these, since interrupt driven data + callback functions should be fast and small
// Not sure if it is possible to share it with other instances.
// Vars used for interpreting the data:
volatile uint32_t Plugin_046_lastrainctr;     // Keep track of wdcounter (1/2 min tick)
volatile int   Plugin_046_lastraincount;           // Last rain count
volatile float Plugin_046_rainmmph = 0;
volatile uint32_t Plugin_046_laststrikectr;   // Keep track of wdcounter (1/2 min tick)
volatile unsigned int  Plugin_046_laststrikecount; // Last number of strikes
volatile int Plugin_046_strikesph = 0;


// TODO TD-er: These call-back functions are quite long and will take a lot of iRAM.
void    Plugin_046_ISR_nSEL() IRAM_ATTR; // Interrupt routines
void    Plugin_046_ISR_SCLK() IRAM_ATTR;

// Number of values and VType depend on the selected plugin function (PCONFIG(0))
uint8_t P046_getValueCount(int choice) {
  switch (choice) {
    case 0: // Main + Temp/Hygro
    case 2: // Rain
    case 3: // UV: 2nd value is always 0, a Domoticz UV device expects "UV;temp"
    case 4: // Lightning strikes
      return 2;
    case 1: // Wind
      return 3;
  }
  return 1;
}

Sensor_VType P046_getVType(int choice) {
  switch (choice) {
    case 0: return Sensor_VType::SENSOR_TYPE_TEMP_HUM;
    case 1: return Sensor_VType::SENSOR_TYPE_WIND;
    case 2: return Sensor_VType::SENSOR_TYPE_DUAL; // Rain rate + total, as expected by a Domoticz rain device
    case 3: return Sensor_VType::SENSOR_TYPE_DUAL; // UV: a Domoticz UV device needs "UV;temp", value 2 stays 0
  }

  // Lightning strikes (4): SINGLE, so controllers only get the strike count, the raw counter (value 2) is for rules/display
  return Sensor_VType::SENSOR_TYPE_SINGLE;
}

boolean Plugin_046(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_046;
      dev.Type           = DEVICE_TYPE_DUMMY;              // Nothing else really fit the bill ...
      dev.VType          = Sensor_VType::SENSOR_TYPE_DUAL; // New type, see ESPEasy.ino
      dev.FormulaOption  = true;
      dev.SendDataOption = true;
      dev.TimerOption    = true;                           // Allow setting the task interval
      dev.ValueCount     = 3;
      break;
    }

    # if FEATURE_MQTT_DISCOVER
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      event->Par1 = static_cast<int>(Sensor_VType::SENSOR_TYPE_NONE); // Not yet supported
      success     = true;
      break;
    }
    # endif // if FEATURE_MQTT_DISCOVER

    case PLUGIN_WEBFORM_LOAD:
    {
      uint8_t choice = PCONFIG(0);
      {
        const uint8_t nrchoices                       = 9;
        const __FlashStringHelper *options[nrchoices] = {
          F("Main + Temp/Hygro"),
          F("Wind"),
          F("Rain"),
          F("UV"),
          F("Lightning strikes"),
          F("Lightning distance"),

          F("Unknown 1, uint8_t 6"),
          F("Unknown 2, uint8_t 16"),
          F("Unknown 3, uint8_t 19"),
        };

        FormSelectorOptions selector(NR_ELEMENTS(options),  options);
        selector.reloadonchange = true;
        selector.addFormSelector(F("Plugin function"), F("p046"), choice);
        // addFormNote(F("Changing the function will reload this page."));
      }

      if (choice == 0) {
        addFormPinSelect(PinSelectPurpose::SPI, F("1st GPIO (5-MOSI)"), F("taskdevicepin1"), PCONFIG(1));
        addFormPinSelect(PinSelectPurpose::SPI, F("2nd GPIO (6-SCLK)"), F("taskdevicepin2"), PCONFIG(2));
        addFormPinSelect(PinSelectPurpose::SPI, F("3rd GPIO (7-nSEL)"), F("taskdevicepin3"), PCONFIG(3));
        addFormPinSelect(PinSelectPurpose::SPI, F("4th GPIO (8-MISO)"), F("taskdeviceport"), PCONFIG(4));
      }

      switch (choice)
      {
        case (0):
        {
          addHtml(F("<TR><TD><B>Be sure you only have 1 main plugin!</B></TD>"));
          addHtml(F("<TR><TD>Value 1: Temperature, 1 decimal<BR>Value 2: Humidity, 0 decimals</TD>"));
          break;
        }
        case (1):
        {
          addHtml(F("<TR><TD>Value 1: Direction, 0 decimals<BR>"));
          addHtml(F("Value 2: Average, 1 decimal<Br>Value 3: Gust, 1 decimal</TD>"));
          break;
        }
        case (2):
        {
          addHtml(F("<TR><TD>Value 1: Rain in mm per hour<BR>Value 2: Total rain in mm</TD>"));
          break;
        }
        case (3):
        {
          addHtml(F("<TR><TD>Value 1: UV, 1 decimal<BR>Value 2: always 0 (Domoticz UV device expects \"UV;temp\")</TD>"));
          break;
        }
        case (4):
        {
          addHtml(F("<TR><TD>Value 1: Strikes per 5 min interval, 0 decimals"));
          addHtml(F("<BR>Value 2: Raw strike counter</TD>"));
          break;
        }
        case (5):
        {
          addHtml(F("<TR><TD>Value 1: Distance in km, 0 decimals</TD>"));
          break;
        }
        case (6):
        {
          addHtml(F("<TR><TD>Value 1: Batterybyte, 0 decimals</TD>"));
          break;
        }
        case (7):
        {
          addHtml(F("<TR><TD>Value 1: Last rainbyte, 0 decimals</TD>"));
          break;
        }
        case (8):
        {
          addHtml(F("<TR><TD>Value 1: Last lightningbyte, 0 decimals</TD>"));
          break;
        }
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p046"));

      if (PCONFIG(0) == 0) {
        PCONFIG(1) = getFormItemInt(F("taskdevicepin1"));
        PCONFIG(2) = getFormItemInt(F("taskdevicepin2"));
        PCONFIG(3) = getFormItemInt(F("taskdevicepin3"));
        PCONFIG(4) = getFormItemInt(F("taskdeviceport"));
      }
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_046);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      // Default value names depend on the selected plugin function
      const __FlashStringHelper *names[3] = { F("Value"), F(""), F("") };

      switch (PCONFIG(0)) {
        case 0: names[0] = F("Temperature"); names[1] = F("Humidity"); break;
        case 1: names[0] = F("Direction");   names[1] = F("Average"); names[2] = F("Gust"); break;
        case 2: names[0] = F("Rain");        names[1] = F("RainTotal"); break;
        case 3: names[0] = F("UV");          names[1] = F("Unused"); break;
        case 4: names[0] = F("Strikes");     names[1] = F("StrikeCount"); break;
        case 5: names[0] = F("Distance");    break;
      }

      for (uint8_t i = 0; i < 3; ++i) {
        ExtraTaskSettings.setTaskDeviceValueName(i, names[i]);
      }
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P046_getValueCount(PCONFIG(0));
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = P046_getVType(PCONFIG(0));
      event->idx        = -1;
      success           = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      if (P046_data && (PCONFIG(0) == 0)) { // Only destruct instance when main unit is stopped
        delete P046_data;
        P046_data = nullptr;
      }
      break;
    }

    case PLUGIN_INIT:
    {
      if (!P046_data) {
        P046_data = new (std::nothrow) P046_data_struct();
      }

      // Tasks saved before the interval was configurable can have interval 0 and would only be read once
      // at boot, before the first frame arrived, leaving their values at 0 forever.
      if (Settings.TaskDeviceTimer[event->TaskIndex] == 0) {
        Settings.TaskDeviceTimer[event->TaskIndex] = (Settings.Delay > 0) ? Settings.Delay : 60;
      }

      uint8_t choice = PCONFIG(0);

      switch (choice)
      {
        case (0):
        {
          P046_data->Plugin_046_MOSIpin = PCONFIG(1);
          P046_data->Plugin_046_SCLKpin = PCONFIG(2);
          P046_data->Plugin_046_nSELpin = PCONFIG(3);
          P046_data->Plugin_046_MISOpin = PCONFIG(4);
          int8_t total = P046_data->Plugin_046_MOSIpin + P046_data->Plugin_046_SCLKpin + P046_data->Plugin_046_nSELpin +
                         P046_data->Plugin_046_MISOpin;

          if (total > 6) { // All pins configured?
            pinMode(P046_data->Plugin_046_MOSIpin, INPUT);
            pinMode(P046_data->Plugin_046_SCLKpin, INPUT);
            pinMode(P046_data->Plugin_046_nSELpin, INPUT);
            pinMode(P046_data->Plugin_046_MISOpin, INPUT);
            P046_data->Plugin_046_databuffer[0] = 0; // buffer is "empty"
            Plugin_046_lastrainctr              = 0;
            Plugin_046_lastraincount            = -1;
            Plugin_046_laststrikectr            = 0;
            Plugin_046_laststrikecount          = UINT_MAX;
            attachInterrupt(P046_data->Plugin_046_SCLKpin, Plugin_046_ISR_SCLK, RISING);
            attachInterrupt(P046_data->Plugin_046_nSELpin, Plugin_046_ISR_nSEL, CHANGE);
          }
          break;
        }
      }
      success = (nullptr != P046_data);
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      if (!P046_data) { break; }

      if (PCONFIG(0) == 0) {
        if (P046_data->Plugin_046_newData) {
          uint8_t crc = 0xff; // init = 0xff
          char    data;       // CRC = MAXIM with modified init: poly 0x31, init 0xff, refin 1; refout 1, xorout 0x00
          // Copy received data to buffer and check CRC
          P046_data->Plugin_046_databuffer[0] = P046_data->Plugin_046_ISR_Buffer[0];

          for (int i = 1; i < P046_data->Plugin_046_bytepointer; i++) {
            data                                = P046_data->Plugin_046_ISR_Buffer[i];
            P046_data->Plugin_046_databuffer[i] = data;

            for (int j = 0; j < 8; j++) // crc routine from Jim Studt
            {                           // Onewire library
              uint8_t mix = (crc ^ data) & 0x01;
              crc >>= 1;

              if (mix) { crc ^= 0x8C; }
              data >>= 1;
            }
          }
          P046_data->Plugin_046_MasterSlave = false;
          P046_data->Plugin_046_newData     = false;
            # ifndef BUILD_NO_DEBUG
            #  if PLUGIN_046_DEBUG
          String log = F("Ventus W266 Rcvd(");
          log += node_time.getTimeString(':');
          log += F(") ");

          for (int i = 0; i < Plugin_046_Payload; i++) {
            if ((i == 2) || (i == 3) || (i == 4) || (i == 9) || (i == 10) || (i == 14) || (i == 17) || (i == 18) || (i == 20)) {
              log += ':';
            }
            char myHex = (P046_data->Plugin_046_databuffer[i] >> 4) + 0x30;

            if (myHex > 0x39) { myHex += 7; }
            log  += myHex;
            myHex = (P046_data->Plugin_046_databuffer[i] & 0x0f) + 0x30;

            if (myHex > 0x39) { myHex += 7; }
            log += myHex;
          }
          log += F(" > ");
          char myHex = (crc >> 4) + 0x30;

          if (myHex > 0x39) { myHex += 7; }
          log  += myHex;
          myHex = (crc & 0x0f) + 0x30;

          if (myHex > 0x39) { myHex += 7; }
          log += myHex;
          addLogMove(LOG_LEVEL_INFO, log);
            #  endif // PLUGIN_046_DEBUG
            # endif // BUILD_NO_DEBUG

          if (crc != 00)
          {
            P046_data->Plugin_046_databuffer[0] = 0; // Not MagicByte, so not valid.
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (!P046_data) { break; }

      if (P046_data->Plugin_046_databuffer[0] == Plugin_046_MagicByte) // buffer[0] should be the MagicByte if valid
      {
        UserVar.setFloat(event->TaskIndex, 1, 0);
        uint8_t choice = PCONFIG(0);                                   // Which instance?

        switch (choice)
        {
          case (0):
          {
            int myTemp = int((P046_data->Plugin_046_databuffer[5] * 256) + P046_data->Plugin_046_databuffer[4]);

            if (myTemp > 0x8000) { myTemp |= 0xffff0000; }            // int @ esp8266 = 32 bits!
            float   temperature = static_cast<float>(myTemp) / 10.0f; // Temperature
            uint8_t myHum       = (P046_data->Plugin_046_databuffer[2] >> 4) * 10 + (P046_data->Plugin_046_databuffer[2] & 0x0f);
            UserVar.setFloat(event->TaskIndex, 0, temperature);
            UserVar.setFloat(event->TaskIndex, 1, static_cast<float>(myHum));
            event->sensorType = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
            break;
          }
          case (1):
          {
            const float average = static_cast<float>((P046_data->Plugin_046_databuffer[11] <<
            8) + P046_data->Plugin_046_databuffer[10]) / 10.0f;                                           // Wind speed average in m/s
            const float gust    = static_cast<float>((P046_data->Plugin_046_databuffer[13] <<
            8) + P046_data->Plugin_046_databuffer[12]) / 10.0f;                                           // Wind speed gust in m/s
            const float bearing = static_cast<float>(P046_data->Plugin_046_databuffer[9] & 0x0f) * 22.5f; // Wind bearing (0-359)
            UserVar.setFloat(event->TaskIndex, 0, bearing);                                               // degrees
            UserVar.setFloat(event->TaskIndex, 1, average);
            UserVar.setFloat(event->TaskIndex, 2, gust);
            event->sensorType = Sensor_VType::SENSOR_TYPE_WIND;
            break;
          }
          case (2):
          {
            float raincnt = static_cast<float>(((P046_data->Plugin_046_databuffer[15]) * 256 + P046_data->Plugin_046_databuffer[14])) / 4.0f;
            int   rainnow = int(raincnt);

            if (wdcounter < Plugin_046_lastrainctr) { Plugin_046_lastrainctr = wdcounter; }

            if (Plugin_046_lastraincount < 0) {             // first valid frame: take as reference
              Plugin_046_lastraincount = rainnow;
              Plugin_046_lastrainctr   = wdcounter;
            }

            if (wdcounter >= (Plugin_046_lastrainctr + 10)) // 5 min interval (10 x 30 s)
            {
              Plugin_046_lastrainctr = wdcounter;

              if (rainnow > Plugin_046_lastraincount)
              { // per 5 min * 12 = per hour
                Plugin_046_rainmmph      = static_cast<float>(rainnow - Plugin_046_lastraincount) * 12.0f;
                Plugin_046_lastraincount = rainnow;
              } else {
                Plugin_046_rainmmph = 0;
              }
            }
            UserVar.setFloat(event->TaskIndex, 0, Plugin_046_rainmmph);
            UserVar.setFloat(event->TaskIndex, 1, raincnt);
            break;
          }
          case (3):
          {
            float uvindex = static_cast<float>(P046_data->Plugin_046_databuffer[17]) / 10.0f;
            UserVar.setFloat(event->TaskIndex, 0, uvindex);
            break;
          }
          case (4):
          {
            const unsigned int strikesnow = (P046_data->Plugin_046_databuffer[21] << 8) | P046_data->Plugin_046_databuffer[20];

            if (Plugin_046_laststrikecount == UINT_MAX) {     // first valid frame: take as reference
              Plugin_046_laststrikecount = strikesnow;
              Plugin_046_laststrikectr   = wdcounter;
            }

            if (wdcounter < Plugin_046_laststrikectr) { Plugin_046_laststrikectr = wdcounter; }

            if (wdcounter >= (Plugin_046_laststrikectr + 10)) // 5 min interval (10 x 30 s)
            {
              Plugin_046_laststrikectr = wdcounter;
              Plugin_046_strikesph     = (strikesnow >= Plugin_046_laststrikecount)
                                         ? (strikesnow - Plugin_046_laststrikecount) : 0; // counter reset -> 0
              Plugin_046_laststrikecount = strikesnow;
            }
            UserVar.setFloat(event->TaskIndex, 0, Plugin_046_strikesph);
            UserVar.setFloat(event->TaskIndex, 1, strikesnow); // raw strike counter
            break;
          }
          case (5):
          {
            float distance = -1.0f;

            if (P046_data->Plugin_046_databuffer[18] != 0x3F)
            {
              distance = P046_data->Plugin_046_databuffer[18];
            }
            UserVar.setFloat(event->TaskIndex, 0, distance);
            break;
          }
          case (6):
          {
            UserVar.setFloat(event->TaskIndex, 0, P046_data->Plugin_046_databuffer[6]);
            break;
          }
          case (7):
          {
            UserVar.setFloat(event->TaskIndex, 0, P046_data->Plugin_046_databuffer[16]);
            break;
          }
          case (8):
          {
            UserVar.setFloat(event->TaskIndex, 0, P046_data->Plugin_046_databuffer[19]);
            break;
          }
        }                // switch
        success = true;
      } else {
        success = false; // No data received yet or incorrect CRC so do not publish any data
      }
      break;
    }                    // case READ
  }                      // switch
  return success;
}

void Plugin_046_ISR_nSEL() // Interrupt on nSEL change
{
  if (!P046_data) { return; }

  if (digitalRead(P046_data->Plugin_046_nSELpin)) {
    P046_data->Plugin_046_ReceiveActive = false;                     // nSEL high? Receive done.

    if (P046_data->Plugin_046_MasterSlave) {                         // If MISO not active, no data received
      if (P046_data->Plugin_046_bytepointer == Plugin_046_Payload) { // If not 23 then bad datapacket
        P046_data->Plugin_046_newData = true;                        // We have new data!
      }
    }
  } else {                                                           // nSEL low? Start receive
    if (!P046_data->Plugin_046_newData) {                            // Only accept new data if the old is processed
      P046_data->Plugin_046_bitpointer    = 7;                       // reset pointer (MSB first)
      P046_data->Plugin_046_bytepointer   = 0;                       // reset pointers & flags
      P046_data->Plugin_046_MasterSlave   = false;
      P046_data->Plugin_046_ReceiveActive = true;                    // We are now receiving data
    }
  }
}

void Plugin_046_ISR_SCLK() // Interrupt on SCLK rising
{
  if (!P046_data) { return; }

  if (P046_data->Plugin_046_ReceiveActive) { // Are we receiving or glitch?
    if (P046_data->Plugin_046_MasterSlave) { // Read MISO or MOSI?
      bitWrite(P046_data->Plugin_046_receivedData, P046_data->Plugin_046_bitpointer, digitalRead(P046_data->Plugin_046_MISOpin));
    } else {
      bitWrite(P046_data->Plugin_046_receivedData, P046_data->Plugin_046_bitpointer, digitalRead(P046_data->Plugin_046_MOSIpin));
    }

    if (P046_data->Plugin_046_bitpointer == 0) {                        // 8 bits done?
      P046_data->Plugin_046_bitpointer = 7;

      if (P046_data->Plugin_046_receivedData == Plugin_046_MagicByte) { // Switch data pins?
        P046_data->Plugin_046_MasterSlave = true;
      }
      P046_data->Plugin_046_ISR_Buffer[P046_data->Plugin_046_bytepointer] = P046_data->Plugin_046_receivedData;
      P046_data->Plugin_046_bytepointer++;           // TReady for the next uint8_t ...

      if (P046_data->Plugin_046_bytepointer >= Plugin_046_RAW_BUFFER_SIZE) { // Was '>': wrote 1 byte past ISR_Buffer into databuffer[0]
        P046_data->Plugin_046_ReceiveActive = false; // We don't want a bufferoverflow, so abort
        P046_data->Plugin_046_MasterSlave   = false;
      }
    } else {
      P046_data->Plugin_046_bitpointer--; // Not yet done with all bits ...
    }
  }
}

// #endif // PLUGIN_BUILD_DISABLED
#endif // USES_P046

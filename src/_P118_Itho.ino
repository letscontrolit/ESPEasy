// #######################################################################################################
// ############################## Plugin 118: Itho ventilation unit 868Mhz remote ########################
// #######################################################################################################

// author :jodur,       13-1-2018
// changed :jeroen, 2-11-2019
// changed :svollebregt, 30-1-2020 - changes to improve stability: volatile decleration of state,
//          disable logging within interrupts unles enabled, removed some unused code,
//          reduce noInterrupts() blockage of code fragments to prevent crashes
//			svollebregt, 16-2-2020 - ISR now sets flag which is checked by 50 per seconds plugin call as
//			receive ISR with Ticker was the cause of instability. Inspired by:
// https://github.com/arnemauer/Ducobox-ESPEasy-Plugin
//			svollebregt, 11-04-2020 - Minor changes to make code compatible with latest mega 20200410, removed SYNC1 option for
// now;
//			better to change this value in the Itho-lib code and compile it yourself
//			svollebreggt, 13-2-2021 - Now uses rewirtten library made by arjenhiemstra:
// https://github.com/arjenhiemstra/IthoEcoFanRFT
//			svollebregt, 11-2021 - Code improvements
//			tonhuisman, 27-12-2021 - Add setting for CS pin
//			tonhuisman, 27-12-2021 - Split into P118_data_struct to enable multiple instances, reduce memory footprint
//								   - Allow 3 simultaneous instances, each using an interrupt and CS
//								   - Remove unused code, reformat source using Uncrustify

// Recommended to disable RF receive logging to minimize code execution within interrupts

// List of commands:
// 1111 to join ESP8266 with Itho ventilation unit
// 9999 to leaveESP8266 with Itho ventilation unit
// 0 to set Itho ventilation unit to standby
// 1 - set Itho ventilation unit to low speed
// 2 - set Itho ventilation unit to medium speed
// 3 - set Itho ventilation unit to high speed
// 4 - set Itho ventilation unit to full speed
// 13 - set itho to high speed with hardware timer (10 min)
// 23 - set itho to high speed with hardware timer (20 min)
// 33 - set itho to high speed with hardware timer (30 min)

// List of States:

// 1 - Itho ventilation unit to lowest speed
// 2 - Itho ventilation unit to medium speed
// 3 - Itho ventilation unit to high speed
// 4 - Itho ventilation unit to full speed
// 13 -Itho to high speed with hardware timer (10 min)
// 23 -Itho to high speed with hardware timer (20 min)
// 33 -Itho to high speed with hardware timer (30 min)

// Usage for http (not case sensitive):
// http://ip/control?cmd=STATE,1111
// http://ip/control?cmd=STATE,1
// http://ip/control?cmd=STATE,2
// http://ip/control?cmd=STATE,3

// usage for example mosquito MQTT
// mosquitto_pub -t /Fan/cmd -m 'state 1111'
// mosquitto_pub -t /Fan/cmd -m 'state 1'
// mosquitto_pub -t /Fan/cmd -m 'state 2'
// mosquitto_pub -t /Fan/cmd -m 'state 3'


// This code needs the library made by 'arjenhiemstra': https://github.com/arjenhiemstra/IthoEcoFanRFT
// A CC1101 868Mhz transmitter is needed
// See https://gathering.tweakers.net/forum/list_messages/1690945 for more information
// code/idea was inspired by first release of code from 'Thinkpad'

#ifdef USES_P118

#include "_Plugin_Helper.h"
#include "./src/PluginStructs/P118_data_struct.h"

// volatile for interrupt function
volatile bool PLUGIN_118_Int[P118_INTERUPT_HANDLER_COUNT] = { false, false, false };
int8_t PLUGIN_118_Task[P118_INTERUPT_HANDLER_COUNT];
bool   PLUGIN_118_Task_Initialized = false;

#define PLUGIN_118
#define PLUGIN_ID_118         118
#define PLUGIN_NAME_118       "Communication - Itho ventilation"
#define PLUGIN_VALUENAME1_118 "State"
#define PLUGIN_VALUENAME2_118 "Timer"
#define PLUGIN_VALUENAME3_118 "LastIDindex"

boolean Plugin_118(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_118;
      Device[deviceCount].Type               = DEVICE_TYPE_SPI2;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_118);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_118));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_118));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_118));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_input(F("Interrupt pin (CC1101 GDO2)"));
      event->String2 = formatGpioName_output(F("CS pin (CC1101 CSN)"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:  // Set defaults address to the one used in old versions of the library for backwards compatability
    {
      PIN(0)     = -1;         // Interrupt pin undefined by default
      PIN(1)     = PIN_SPI_SS; // CS pin use the previous default of PIN_SPI_SS/gpio 15
      PCONFIG(0) = 1;
      PCONFIG(1) = 10;
      PCONFIG(2) = 87;
      PCONFIG(3) = 81;
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      #ifdef P118_DEBUG_LOG
      addLog(LOG_LEVEL_INFO, F("INIT PLUGIN_118"));
      #endif // ifdef P118_DEBUG_LOG
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P118_data_struct(PCONFIG(0)));
      P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P118_data) {
        return success;
      }
      success = P118_data->plugin_init(event); // Part 1

      if (success) {
        if (!PLUGIN_118_Task_Initialized) {
          for (uint8_t i = 0; i < P118_INTERUPT_HANDLER_COUNT; i++) {
            PLUGIN_118_Task[i] = -1;
            PLUGIN_118_Int[i]  = false;
          }
          PLUGIN_118_Task_Initialized = true;
        }
        int8_t offset = -1;

        for (uint8_t i = 0; i < P118_INTERUPT_HANDLER_COUNT && offset == -1; i++) {
          if (PLUGIN_118_Task[i] == -1) { // Find a free spot
            offset = i;
          }
        }

        if (offset > -1) {
          PLUGIN_118_Task[offset] = event->TaskIndex;
          pinMode(PIN(0), INPUT);

          switch (offset) { // Add more interrupt handlers when needed
            case 0:
              attachInterrupt(PIN(0), PLUGIN_118_ITHOinterrupt0, FALLING);
              break;
            case 1:
              attachInterrupt(PIN(0), PLUGIN_118_ITHOinterrupt1, FALLING);
              break;
            case 2:
              attachInterrupt(PIN(0), PLUGIN_118_ITHOinterrupt2, FALLING);
              break;
            default:
              break;
          }
          P118_data->plugin_init_part2();        // Start the data flow
          addLog(LOG_LEVEL_INFO, F("CC1101 868Mhz transmitter initialized"));
        } else {
          clearPluginTaskData(event->TaskIndex); // Destroy initialized data, init failed
          addLog(LOG_LEVEL_ERROR, F("CC1101 initialization failed!"));
          success = false;
        }
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      #ifdef P118_DEBUG_LOG
      addLog(LOG_LEVEL_INFO, F("EXIT PLUGIN_118"));
      #endif // ifdef P118_DEBUG_LOG
      P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P118_data) {
        return success;
      }
      int8_t offset = -1;

      for (uint8_t i = 0; i < P118_INTERUPT_HANDLER_COUNT && offset == -1; i++) {
        if (PLUGIN_118_Task[i] == event->TaskIndex) {
          offset = i;
        }
      }

      // detach interupt when plugin is 'removed'
      if (offset > -1) {
        detachInterrupt(PIN(0));
      }

      success = P118_data->plugin_exit(event);
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P118_data) {
        return success;
      }
      success = P118_data->plugin_once_a_second(event);

      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      int8_t offset = -1;

      // Find matching interrupt flag
      for (uint8_t i = 0; i < P118_INTERUPT_HANDLER_COUNT && offset == -1; i++) {
        if (PLUGIN_118_Task[i] == event->TaskIndex) {
          offset = i;
        }
      }

      if ((offset > -1) && PLUGIN_118_Int[offset])
      {
        P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr == P118_data) {
          return success;
        }
        success = P118_data->plugin_fifty_per_second(event);

        PLUGIN_118_Int[offset] = false; // reset flag
      }
      break;
    }


    case PLUGIN_READ: {
      P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P118_data) {
        return success;
      }
      success = P118_data->plugin_read(event);

      break;
    }

    case PLUGIN_WRITE: {
      P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P118_data) {
        return success;
      }
      success = P118_data->plugin_write(event, string);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      PLUGIN_118_ExtraSettingsStruct PLUGIN_118_ExtraSettings;
      LoadCustomTaskSettings(event->TaskIndex, (byte *)&PLUGIN_118_ExtraSettings, sizeof(PLUGIN_118_ExtraSettings));
      addFormSubHeader(F("Remote RF Controls"));
      addFormTextBox(F("Unit ID remote 1"), F("PLUGIN_118_ID1"), PLUGIN_118_ExtraSettings.ID1, 8);
      addFormTextBox(F("Unit ID remote 2"), F("PLUGIN_118_ID2"), PLUGIN_118_ExtraSettings.ID2, 8);
      addFormTextBox(F("Unit ID remote 3"), F("PLUGIN_118_ID3"), PLUGIN_118_ExtraSettings.ID3, 8);
      addFormCheckBox(F("Enable RF receive log"), F("p118_log"), PCONFIG(0)); // Makes RF logging optional to reduce clutter in the lof file
                                                                              // in RF noisy environments
      addFormNumericBox(F("Device ID byte 1"), F("p118_deviceid1"), PCONFIG(1), 0, 255);
      addFormNumericBox(F("Device ID byte 2"), F("p118_deviceid2"), PCONFIG(2), 0, 255);
      addFormNumericBox(F("Device ID byte 3"), F("p118_deviceid3"), PCONFIG(3), 0, 255);
      addFormNote(F(
                    "Device ID of your ESP, should not be the same as your neighbours ;-). Defaults to 10,87,81 which corresponds to the old Itho library"));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PLUGIN_118_ExtraSettingsStruct PLUGIN_118_ExtraSettings;
      strcpy(PLUGIN_118_ExtraSettings.ID1, web_server.arg(F("PLUGIN_118_ID1")).c_str());
      strcpy(PLUGIN_118_ExtraSettings.ID2, web_server.arg(F("PLUGIN_118_ID2")).c_str());
      strcpy(PLUGIN_118_ExtraSettings.ID3, web_server.arg(F("PLUGIN_118_ID3")).c_str());
      SaveCustomTaskSettings(event->TaskIndex, (byte *)&PLUGIN_118_ExtraSettings, sizeof(PLUGIN_118_ExtraSettings));

      PCONFIG(0) = isFormItemChecked(F("p118_log"));

      PCONFIG(1) = getFormItemInt(F("p118_deviceid1"), 10);
      PCONFIG(2) = getFormItemInt(F("p118_deviceid2"), 87);
      PCONFIG(3) = getFormItemInt(F("p118_deviceid3"), 81);
      success    = true;
      break;
    }
  }
  return success;
}

ICACHE_RAM_ATTR void PLUGIN_118_ITHOinterrupt0() {
  PLUGIN_118_Int[0] = true; // flag 0
}

ICACHE_RAM_ATTR void PLUGIN_118_ITHOinterrupt1() {
  PLUGIN_118_Int[1] = true; // flag 1
}

ICACHE_RAM_ATTR void PLUGIN_118_ITHOinterrupt2() {
  PLUGIN_118_Int[2] = true; // flag 2
}

#endif // USES_P118

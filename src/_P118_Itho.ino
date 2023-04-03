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
//			tonhuisman, 28-12-2021 - Move interrupt handling to Plugin_data_struct, lifting the limit on nr. of plugins
//      tonhuisman, 03-01-2022 - Review source after structural-crash report, fix interrupt handler
//      tonhuisman, 21-06-2022 - Minor improvements
//      tonhuisman, 10-08-2022 - Fix bugs, add 3 second limit to formerly perpetual while loops in IthoCC1101 library
//                               Restructure source somewhat, rename variables, clean up stuff generally
//      tonhuisman, 11-08-2022 - Fix issue with ESP32 support, the MISO pin was predefined, but not matching the ESPEasy
//                               actual configuration.
//                               Added time-out check (5s) to initialization, usually an indication of incorrect hardware
//                               configuration, defective or disconnected board.
//                               Reduced time-out checks in IthoCC1101 library to 1 second (from 3)
//                               Improved display of GPIO pins in Devices page
//      tonhuisman, 18-08-2022 - Merge Orcon related code from PR #4099 (https://github.com/letscontrolit/ESPEasy/pull/4099)
//                               Orcon code can be partially disabled by setting P118_FEATURE_ORCON 0 in P118_data_struc.h
//                               Support for orcon must be enabled in settings, to avoid possible interference with Itho.
//                               Re-enabled timer support for Orcon, as it is only a status update, NOT a ventilator update
//      tonhuisman, 18-09-2022 - Hide Debug log option in device configuration when Debug log is not available.
//      tonhuisman, 05-03-2023 - Deprecate 'state' command, and add support for 'itho' as the main command

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

// 100 - set Orcon ventilation unit to standby
// 101 - set Orcon ventilation unit to low speed
// 102 - set Orcon ventilation unit to medium speed
// 103 - set Orcon ventilation unit to high speed
// 104 - set Orcon ventilation unit to Auto
// 111 - set Orcon to standby with hardware timer (12 hours)
// 111 - set Orcon to low speed with hardware timer (1 hour)
// 112 - set Orcon to medium speed with hardware timer (13 hours)
// 113 - set Orcon to high speed with hardware timer (1 hour)
// 113 - set Orcon to AutoCO2 mode

// List of States:

// 1 - Itho ventilation unit to lowest speed
// 2 - Itho ventilation unit to medium speed
// 3 - Itho ventilation unit to high speed
// 4 - Itho ventilation unit to full speed
// 13 -Itho to high speed with hardware timer (10 min)
// 23 -Itho to high speed with hardware timer (20 min)
// 33 -Itho to high speed with hardware timer (30 min)

// 100 - Orcon ventilation unit to standby
// 101 - Orcon ventilation unit to low speed
// 102 - Orcon ventilation unit to medium speed
// 103 - Orcon ventilation unit to high speed
// 104 - Orcon ventilation unit to Auto
// 111 - Orcon to standby with hardware timer (12 hours)
// 111 - Orcon to low speed with hardware timer (1 hour)
// 112 - Orcon to medium speed with hardware timer (13 hours)
// 113 - Orcon to high speed with hardware timer (1 hour)
// 113 - Orcon to AutoCO2 mode

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

#include "_Plugin_Helper.h"

#ifdef USES_P118

# include "_Plugin_Helper.h"
# include "./src/PluginStructs/P118_data_struct.h"

# define PLUGIN_118
# define PLUGIN_ID_118         118
# define PLUGIN_NAME_118       "Communication - Itho ventilation"
# define PLUGIN_VALUENAME1_118 "State"
# define PLUGIN_VALUENAME2_118 "Timer"
# define PLUGIN_VALUENAME3_118 "LastIDindex"

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

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      string  = F("GDO2: ");
      string += formatGpioLabel(P118_IRQPIN, false);
      string += event->String1;
      string += F("CSN: ");
      string += formatGpioLabel(P118_CSPIN, false);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:          // Set defaults address to the one used in old versions of the library for backwards compatability
    {
      P118_IRQPIN        = -1;         // Interrupt pin undefined by default
      P118_CSPIN         = PIN_SPI_SS; // CS pin use the previous default of PIN_SPI_SS/gpio 15
      P118_CONFIG_LOG    = 0;          // RF DEBUG log disabled
      P118_CONFIG_DEVID1 = 10;
      P118_CONFIG_DEVID2 = 87;
      P118_CONFIG_DEVID3 = 81;
      P118_CONFIG_RF_LOG = 1; // RF INFO log enabled
      success            = true;
      break;
    }

    case PLUGIN_INIT:
    {
      # ifdef P118_DEBUG_LOG
      addLog(LOG_LEVEL_INFO, F("INIT PLUGIN_118"));
      # endif // ifdef P118_DEBUG_LOG

      if (validGpio(P118_CSPIN) && (P118_IRQPIN != P118_CSPIN)) {
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P118_data_struct(P118_CSPIN,
                                                                                 P118_IRQPIN,
                                                                                 P118_CONFIG_LOG == 1,
                                                                                 P118_CONFIG_RF_LOG == 1));
        P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

        success = (nullptr != P118_data) && P118_data->plugin_init(event);
      } else {
        addLog(LOG_LEVEL_ERROR, F("ITHO: CS pin not correctly configured, plugin can not start!"));
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      # ifdef P118_DEBUG_LOG
      addLog(LOG_LEVEL_INFO, F("EXIT PLUGIN_118"));
      # endif // ifdef P118_DEBUG_LOG
      P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P118_data) {
        success = P118_data->plugin_exit(event);
      }

      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P118_data) {
        success = P118_data->plugin_once_a_second(event);
      }

      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P118_data) {
        success = P118_data->plugin_fifty_per_second(event);
      }

      break;
    }


    case PLUGIN_READ: {
      P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P118_data) {
        success = P118_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE: {
      P118_data_struct *P118_data = static_cast<P118_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P118_data) {
        success = P118_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      PLUGIN_118_ExtraSettingsStruct PLUGIN_118_ExtraSettings;
      LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&PLUGIN_118_ExtraSettings), sizeof(PLUGIN_118_ExtraSettings));
      addFormSubHeader(F("Remote RF Controls"));
      addFormTextBox(F("Unit ID remote 1"), F("pID1"), PLUGIN_118_ExtraSettings.ID1, 8);
      # if P118_FEATURE_ORCON
      addFormNote(F("For Orcon: The addres of remote 1 will be used as source/sender address"));
      # endif // if P118_FEATURE_ORCON
      addFormTextBox(F("Unit ID remote 2"), F("pID2"), PLUGIN_118_ExtraSettings.ID2, 8);
      addFormTextBox(F("Unit ID remote 3"), F("pID3"), PLUGIN_118_ExtraSettings.ID3, 8);

      # ifndef BUILD_NO_DEBUG
      addFormCheckBox(F("Enable RF DEBUG log"),        F("plog"),   P118_CONFIG_LOG);    // Makes RF logging optional to reduce clutter in
                                                                                         // the log
      # endif // ifndef BUILD_NO_DEBUG
      addFormCheckBox(F("Enable minimal RF INFO log"), F("prflog"), P118_CONFIG_RF_LOG); // Log only the received Device ID's at INFO level

      addFormNumericBox(F("Device ID byte 1"), F("pdevid1"), P118_CONFIG_DEVID1, 0, 255);
      addFormNumericBox(F("Device ID byte 2"), F("pdevid2"), P118_CONFIG_DEVID2, 0, 255);
      addFormNumericBox(F("Device ID byte 3"), F("pdevid3"), P118_CONFIG_DEVID3, 0, 255);
      addFormNote(F("Device ID of your ESP, should not be the same as your neighbours ;-). "
                    "Defaults to 10,87,81 which corresponds to the old Itho library"));
      # if P118_FEATURE_ORCON
      addFormNote(F("For Orcon: This is the destination ID a.k.a. the ID of the Ventilation unit."));

      addFormCheckBox(F("Enable Orcon support"), F("orcon"), P118_CONFIG_ORCON);
      # endif // if P118_FEATURE_ORCON
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PLUGIN_118_ExtraSettingsStruct PLUGIN_118_ExtraSettings;
      strcpy(PLUGIN_118_ExtraSettings.ID1, web_server.arg(F("pID1")).c_str());
      strcpy(PLUGIN_118_ExtraSettings.ID2, web_server.arg(F("pID2")).c_str());
      strcpy(PLUGIN_118_ExtraSettings.ID3, web_server.arg(F("pID3")).c_str());
      SaveCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&PLUGIN_118_ExtraSettings), sizeof(PLUGIN_118_ExtraSettings));

      # ifndef BUILD_NO_DEBUG
      P118_CONFIG_LOG = isFormItemChecked(F("plog"));
      # endif // ifndef BUILD_NO_DEBUG
      P118_CONFIG_RF_LOG = isFormItemChecked(F("prflog"));

      P118_CONFIG_DEVID1 = getFormItemInt(F("pdevid1"), 10);
      P118_CONFIG_DEVID2 = getFormItemInt(F("pdevid2"), 87);
      P118_CONFIG_DEVID3 = getFormItemInt(F("pdevid3"), 81);
      # if P118_FEATURE_ORCON
      P118_CONFIG_ORCON = isFormItemChecked(F("orcon")) ? 1 : 0;
      # endif // if P118_FEATURE_ORCON
      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P118

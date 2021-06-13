#include "_Plugin_Helper.h"
#ifdef USES_P104

// #######################################################################################################
// #######################   Plugin 104 - dot matrix display plugin MAX7219       ########################
// #######################################################################################################
//
// Chips/displays supported:
//  MAX7219/21 -- 3 pins (SPI + CS) - Dot matrix display, 8x8 leds per unit, units can be connected daisy chained, up to 255 (theoretically,
// untested yet)
//
// TODO: Plugin Content can be setup as:
//
// TODO: Generic commands:
//
// History:
// 2021-06-13 tonhuisman: First working version, many improvemnts to make, like command handling, repeat timer implementaation
// 2021-05 tonhuisman: Store and retrieve settings for max 8 (ESP82xx) or 16 (ESP32) zones
// 2021-04 tonhuisman: Pickup and rework to get it working with ESPEasy
// 2021-03 rixtyan : Initial plugin setup, partially based on P073 MAX7219 LED display

# define PLUGIN_104
# define PLUGIN_ID_104           104
# define PLUGIN_NAME_104         "Display - MAX7219 dot matrix [DEVELOPMENT]"

# define PLUGIN_104_DEBUG        true // activate extra log info in the debug


# include "src/PluginStructs/P104_data_struct.h"


boolean Plugin_104(byte function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_104;
      Device[deviceCount].Type               = DEVICE_TYPE_SPI;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].TimerOptional      = false;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS: {
      CONFIG_PORT = -1;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_104);
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output("CS");
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      int8_t din_pin = -1;
      int8_t clk_pin = -1;
      int    pinnr = -1;
      bool   input, output, warning;
      # ifdef ESP32

      switch (Settings.InitSPI) {
        case 1:
          din_pin = 23;
          clk_pin = 19;
          break;
        case 2:
          din_pin = 13;
          clk_pin = 14;
          break;
      }
      # else // ESP82xx

      if (Settings.InitSPI == 1) {
        din_pin = 13;
        clk_pin = 14;
      }
      # endif // ifdef ESP32
      String note;
      note.reserve(72);
      note = F("SPI->MAX7219: MOSI");

      if (din_pin != -1) {
        note += '(';
        getGpioInfo(din_pin, pinnr, input, output, warning);
        note += createGPIO_label(din_pin, pinnr, true, true, false);
        note += ')';
      }
      note += F("->DIN-Pin, CLK");

      if (clk_pin != -1) {
        note += '(';
        getGpioInfo(clk_pin, pinnr, input, output, warning);
        note += createGPIO_label(clk_pin, pinnr, true, true, false);
        note += ')';
      }
      note += F("->CLK-Pin");
      addFormNote(note);

      P104_data_struct *P104_data = new (std::nothrow) P104_data_struct(static_cast<MD_MAX72XX::moduleType_t>(P104_CONFIG_HARDWARETYPE),
                                                                        event->TaskIndex,
                                                                        CONFIG_PIN1,
                                                                        P104_CONFIG_TOTAL_UNITS);

      if (nullptr == P104_data) {
        addFormNote(F("Memory allocation error, re-open task to load."));
        return success;
      }
      success = P104_data->webform_load(event);

      delete P104_data; // Clean up
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P104_data_struct *P104_data = static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      bool createdWhileActive = false;

      if (nullptr == P104_data) { // Create new object if not active atm.
        P104_data = new (std::nothrow) P104_data_struct(static_cast<MD_MAX72XX::moduleType_t>(P104_CONFIG_HARDWARETYPE),
                                                        event->TaskIndex,
                                                        CONFIG_PIN1,
                                                        -1);
        createdWhileActive = true;
      }

      if (nullptr != P104_data) {
        P104_data->setZones(P104_CONFIG_ZONE_COUNT);
        success = P104_data->webform_save(event);

        if (!success) {
          addHtmlError(P104_data->getError());
        }

        if (createdWhileActive) {
          delete P104_data; // Cleanup
        }
      }

      break;
    }

    case PLUGIN_INIT: {
      uint8_t numDevices = P104_CONFIG_TOTAL_UNITS;

      if (numDevices == 0) { // fallback value
        numDevices++;
      }
      # ifdef P104_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log;
        log.reserve(32);
        log  = F("P104: PLUGIN_INIT numDevices: ");
        log += numDevices;
        addLog(LOG_LEVEL_INFO, log);
      }
      # endif // ifdef P104_DEBUG

      initPluginTaskData(event->TaskIndex,
                         new (std::nothrow) P104_data_struct(static_cast<MD_MAX72XX::moduleType_t>(P104_CONFIG_HARDWARETYPE),
                                                             event->TaskIndex,
                                                             CONFIG_PIN1,
                                                             numDevices));

      P104_data_struct *P104_data =
        static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P104_data) {
        return success;
      }

      // initialise the LED display
      P104_data->begin();

      // Setup the zones from configuration
      P104_data->configureZones();

      //     invertUpperZone = (HARDWARE_TYPE == MD_MAX72XX::GENERIC_HW || HARDWARE_TYPE == MD_MAX72XX::PAROLA_HW);

      // Set up zones for 2 halves of the display
      // # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT

      // if (PCONFIG(1) == 2) {
      //   P104_data->P.setZone(ZONE_LOWER, 0,          PCONFIG(1) - 1);
      //   P104_data->P.setZone(ZONE_UPPER, PCONFIG(1), numDevices - 1);
      //   P104_data->P.setFont(numeric7SegDouble);
      //   P104_data->P.setCharSpacing(P104_data->P.getCharSpacing() * 2); // double height --> double spacing
      // }
      // # endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT


      //      if (invertUpperZone)
      //      {
      //        P.setZoneEffect(ZONE_UPPER, true, PA_FLIP_UD);
      //        P.setZoneEffect(ZONE_UPPER, true, PA_FLIP_LR);

      //        P.displayZoneText(ZONE_LOWER, szTimeL, PA_RIGHT, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
      //        P.displayZoneText(ZONE_UPPER, szTimeH, PA_LEFT, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
      //      }
      //      else
      //      {
      // P104_data->P.displayZoneText(ZONE_LOWER, szTimeL, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
      // P104_data->P.displayZoneText(ZONE_UPPER, szTimeH, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);

      //      }

      success = true;
      break;
    }

    case PLUGIN_WRITE: {
      P104_data_struct *P104_data = static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P104_data) {
        return success;
      }

      success = P104_data->handlePluginWrite(event->TaskIndex, string);

      break;
    }

    case PLUGIN_TEN_PER_SECOND: {
      P104_data_struct *P104_data = static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P104_data) {
        return success;
      }

      P104_data->P->displayAnimate();
      success = true;

      break;
    }

    case PLUGIN_ONCE_A_SECOND: {
      P104_data_struct *P104_data = static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P104_data) {
        return success;
      }

      if (P104_data->P->displayAnimate()) {          // At least 1 zone is ready
        bool allDone = true;                         // Assume all is done

        for (uint8_t z = 0; z < P104_CONFIG_ZONE_COUNT && allDone; z++) {
          allDone &= P104_data->P->getZoneStatus(z); // Check if really all is done
        }

        if (allDone) {
          P104_data->handlePluginOncePerSecond(event->TaskIndex);
        }
      }
    }
  }

  return success;
}

#endif // USES_P104

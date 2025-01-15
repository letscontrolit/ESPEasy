#include "_Plugin_Helper.h"
#ifdef USES_P080

// #######################################################################################################
// #################################### Plugin 080: iButton Sensor  DS1990A    ###########################
// #######################################################################################################

// Maxim Integrated

/** Changelog:
 * 2024-05-11 tonhuisman: Dallas_StartConversion() call not needed for iButton.
 *                        Reduce logging in Dallas_readiButton() function to on-change (only used for this plugin)
 * 2024-05-10 tonhuisman: Add support for Event with iButton address,
 *                        generating event: <taskname>#Address=<state>[,<buttonaddress_high4byte>,<buttonaddress_low4byte>],
 *                        enabling address to be processed in rules
 *                        Fix plugin VType setting as SENSOR_TYPE_ULONG isn't needed here, only storing 0/1 state
 *                        Make Interval optional, as with Event processing enabled, this state is not useful
 * 2024-05 tonhuisman: Start changelog
 */

# include "src/Helpers/Dallas1WireHelper.h"

# define PLUGIN_080
# define PLUGIN_ID_080         80
# define PLUGIN_NAME_080       "Input - iButton"
# define PLUGIN_VALUENAME1_080 "iButton"

# define P080_ADDRESS_EVENT     PCONFIG(0)
# define P080_EVENT_NAME        "Address"

boolean Plugin_080(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_080;
      dev.Type           = DEVICE_TYPE_SINGLE;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.ValueCount     = 1;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_080);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_080));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_bidirectional(F("1-Wire"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNote(F("External pull up resistor is needed, see docs!"));

      // Scan the onewire bus and fill dropdown list with devicecount on this GPIO.
      const int8_t Plugin_080_DallasPin = CONFIG_PIN1;

      if (validGpio(Plugin_080_DallasPin)) {
        Dallas_addr_selector_webform_load(event->TaskIndex, Plugin_080_DallasPin, Plugin_080_DallasPin);

        addFormCheckBox(F("Event with iButton address"), F("iaddr"), P080_ADDRESS_EVENT);
        addFormNote(F("When checked, <b>Device Address</b> should be '- None -'"));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // save the address for selected device and store into extra tasksettings
      Dallas_addr_selector_webform_save(event->TaskIndex, CONFIG_PIN1, CONFIG_PIN1);
      P080_ADDRESS_EVENT = isFormItemChecked(F("iaddr"));
      success            = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      uint8_t addr[8];
      Dallas_plugin_get_addr(addr, event->TaskIndex);
      string  = Dallas_format_address(addr);
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      const int8_t Plugin_080_DallasPin = CONFIG_PIN1;

      if (validGpio(Plugin_080_DallasPin)) {
        uint8_t addr[8];

        // Explicitly set the pinMode using the "slow" pinMode function
        // This way we know for sure the state of any pull-up or -down resistor is known.
        pinMode(Plugin_080_DallasPin, INPUT);

        Dallas_plugin_get_addr(addr, event->TaskIndex);

        if (Settings.TaskDeviceTimer[event->TaskIndex] == 0) { // Trigger at least once a PLUGIN_READ
          UserVar.setFloat(event->TaskIndex, 2, -1);
        }

        success = true;
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND: // PLUGIN_READ:
    {
      const int8_t Plugin_080_DallasPin = CONFIG_PIN1;
      uint8_t addr[8];
      Dallas_plugin_get_addr(addr, event->TaskIndex);

      if ((0x00 == addr[0]) && P080_ADDRESS_EVENT) { // Respond to any iButton presented?
        uint32_t state = 0;
        Dallas_reset(Plugin_080_DallasPin, Plugin_080_DallasPin);
        Dallas_reset_search();

        while (Dallas_search(addr, Plugin_080_DallasPin, Plugin_080_DallasPin)) {
          if (addr[0] == 0x01) { // Respond to first iButton device
            state = 1;
            break;
          }
        }

        if (0x01 == addr[0]) {
          if (state != UserVar.getFloat(event->TaskIndex, 0)) {
            UserVar.setFloat(event->TaskIndex, 0, state);
            eventQueue.add(event->TaskIndex, F(P080_EVENT_NAME),
                           strformat(F("%d,0x%s,0x%s"), // Address split in 2 hex parts
                                     state,
                                     formatToHex_array(addr,     4).c_str(),
                                     formatToHex_array(&addr[4], 4).c_str()));
          }
        } else {
          if (state != UserVar.getFloat(event->TaskIndex, 0)) {
            UserVar.setFloat(event->TaskIndex, 0, state);
            eventQueue.add(event->TaskIndex, F(P080_EVENT_NAME),
                           state);
          }
        }

        // No (debug) logging is added as the generated event is already logged at INFO level.
        success = true;
        addr[0] = 0; // Ignore other devices on the wire
      } else

      if (0 != addr[0]) {
        if (Dallas_readiButton(addr, Plugin_080_DallasPin, Plugin_080_DallasPin, UserVar.getFloat(event->TaskIndex, 0))) {
          UserVar.setFloat(event->TaskIndex, 0, 1);
          success = true;
        } else {
          UserVar.setFloat(event->TaskIndex, 0, 0);
        }

        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("DS   : iButton: ");

          if (success) {
            log += formatUserVarNoCheck(event, 0);
          } else {
            log += F("Not Present!");
          }
          addLogMove(LOG_LEVEL_DEBUG, log);
        }
        # endif // ifndef BUILD_NO_DEBUG
      }
      break;
    }
    case PLUGIN_READ:
    {
      success = UserVar.getFloat(event->TaskIndex, 0) != UserVar.getFloat(event->TaskIndex, 2); // Changed?

      // Keep previous state
      UserVar.setFloat(event->TaskIndex, 2, UserVar.getFloat(event->TaskIndex, 0));
      break;
    }
  }
  return success;
}

#endif // USES_P080

#include "_Plugin_Helper.h"
#ifdef USES_P005

// #######################################################################################################
// ######################## Plugin 005: Temperature and Humidity sensor DHT 11/22 ########################
// #######################################################################################################

# include "src/PluginStructs/P005_data_struct.h"

# define PLUGIN_005
# define PLUGIN_ID_005         5
# define PLUGIN_NAME_005       "Environment - DHT11/12/22  SONOFF2301/7021/MS01"
# define PLUGIN_VALUENAME1_005      "Temperature"
# define PLUGIN_VALUENAME1_005_MS01 "RAW"
# define PLUGIN_VALUENAME2_005      "Humidity"


boolean Plugin_005(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_005;
      dev.Type               = DEVICE_TYPE_SINGLE;
      dev.VType              = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
      dev.FormulaOption      = true;
      dev.ValueCount         = 2;
      dev.SendDataOption     = true;
      dev.TimerOption        = true;
      dev.PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_005);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      if (PCONFIG(0) == P005_MS01) {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_005_MS01));
      } else {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_005));
      }
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
      const __FlashStringHelper *options[] = { F("DHT 11"), F("DHT 22"), F("DHT 12"), F("Sonoff am2301"), F("Sonoff si7021"),
                                               F("Sonoff MS01") };
      const int indices[] = { P005_DHT11, P005_DHT22, P005_DHT12, P005_AM2301, P005_SI7021, P005_MS01 };

      constexpr size_t nrElements = NR_ELEMENTS(indices);

      addFormSelector(F("Sensor model"), F("dhttype"), nrElements, options, indices, PCONFIG(0));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("dhttype"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P005_data_struct(event));
      break;
    }

    case PLUGIN_READ:
    {
      P005_data_struct *P005_data =
        static_cast<P005_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P005_data) {
        success = P005_data->readDHT(event);
      }
      break;
    }
  }
  return success;
}

#endif // USES_P005

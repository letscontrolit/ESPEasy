#include "_Plugin_Helper.h"
#ifdef USES_P047

// #######################################################################################################
// #################### Plugin 047 Moisture & Temperature & Light I2C Soil moisture sensor  ##############
// #######################################################################################################
//
// Capacitive soil moisture sensor
// like this one: https://www.tindie.com/products/miceuz/i2c-soil-moisture-sensor/
// based on this library: https://github.com/Apollon77/I2CSoilMoistureSensor
// this code is based on version 1.1.2 of the above library
//

/** Changelog:
 * 2024-05-09 tonhuisman: Add support for BeFlE v3.x (low power) Moisture sensor
 *                        Code improvements
 *                        ** Fix bug in setting a new I2C address for BeFlE sensors (needs a left-shift by 1)
 * 2024-04-05 tonhuisman: Complete implementation for Afafruit I2C Capacitive Moisture sensor.
 *                        Log sensor name and version (or 0 when not available) at plugin startup.
 * 2024-03-23 tonhuisman: Start implementation of Adafruit I2C Capacitive Moisture Sensor (product ID 4026)
 *                        From a forum request: https://www.letscontrolit.com/forum/viewtopic.php?t=10107
 * 2023-04-07 tonhuisman: Correct typo BelFlE to BeFlE
 * 2023-04-01 tonhuisman: Implement staged reading instead of a fixed delay during PLUGIN_READ
 *                        Add range-check on save for I2C address inputs (0x01..0x7F)
 * 2023-03-31 tonhuisman: Add support for BelFlE I2C Moisture sensor,
 *                        from a forum request: https://www.letscontrolit.com/forum/viewtopic.php?t=9581
 *                        Move some code to PluginStruct files
 * 2023-03-31 tonhuisman: Start of changelog, older changes not logged
 */

# define PLUGIN_047
# define PLUGIN_ID_047          47
# define PLUGIN_NAME_047        "Environment - Soil moisture sensor"
# define PLUGIN_VALUENAME1_047  "Temperature"
# define PLUGIN_VALUENAME2_047  "Moisture"
# define PLUGIN_VALUENAME3_047  "Light"

# include "src/PluginStructs/P047_data_struct.h"


boolean Plugin_047(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_047;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_TRIPLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 3;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_047);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_047));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_047));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_047));
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P047_MODEL_CATNIP == static_cast<P047_SensorModels>(P047_MODEL) ? 3 : 2;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(P047_MODEL_CATNIP == static_cast<P047_SensorModels>(P047_MODEL) ? 3 : 2);
      event->idx        = P047_MODEL_CATNIP == static_cast<P047_SensorModels>(P047_MODEL) ? 3 : 2;
      success           = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P047_I2C_ADDR = P047_CATNIP_DEFAULT_ADDR;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      # if P047_FEATURE_ADAFRUIT

      if (P047_MODEL_ADAFRUIT == static_cast<P047_SensorModels>(P047_MODEL)) {
        const uint8_t i2cAddressValues[] = { P047_ADAFRUIT_DEFAULT_ADDR, 0x37, 0x38, 0x39 };

        addFormSelectorI2C(F("i2c_addr"), 4, i2cAddressValues, P047_I2C_ADDR);
      } else
      # endif // if P047_FEATURE_ADAFRUIT
      {
        addFormTextBox(F("I2C Address (Hex)"), F("i2c_addr"),
                       formatToHex_decimal(P047_I2C_ADDR), 4);
        addUnit(F("0x01..0x7F"));
      }

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = event->Par1 == P047_I2C_ADDR; // Show for currently configured address
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P047_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *SensorModels[] = {
          toString(P047_MODEL_CATNIP),
          toString(P047_MODEL_BEFLE),
          # if P047_FEATURE_BEFLE_V3
          toString(P047_MODEL_BEFLE_V3),
          # endif // if P047_FEATURE_BEFLE_V3
          # if P047_FEATURE_ADAFRUIT
          toString(P047_MODEL_ADAFRUIT),
          # endif // if P047_FEATURE_ADAFRUIT
        };
        const int SensorModelIds[] = {
          static_cast<int>(P047_MODEL_CATNIP),
          static_cast<int>(P047_MODEL_BEFLE),
          # if P047_FEATURE_BEFLE_V3
          static_cast<int>(P047_MODEL_BEFLE_V3),
          # endif // if P047_FEATURE_BEFLE_V3
          # if P047_FEATURE_ADAFRUIT
          static_cast<int>(P047_MODEL_ADAFRUIT),
          # endif // if P047_FEATURE_ADAFRUIT
        };
        constexpr size_t P047_MODEL_OPTIONS = NR_ELEMENTS(SensorModelIds);
        addFormSelector(F("Sensor model"), F("model"), P047_MODEL_OPTIONS, SensorModels, SensorModelIds, P047_MODEL, true);
        addFormNote(F("Changing the Sensor model will reload the page."));
      }

      if ((P047_MODEL_CATNIP == static_cast<P047_SensorModels>(P047_MODEL))
          # if P047_FEATURE_BEFLE_V3
          || (P047_MODEL_BEFLE_V3 == static_cast<P047_SensorModels>(P047_MODEL))
          # endif // if P047_FEATURE_BEFLE_V3
          ) {
        addFormSeparator(2);

        addFormCheckBox(F("Send sensor to sleep"), F("sleep"), P047_SENSOR_SLEEP);

        # if P047_FEATURE_BEFLE_V3

        if (P047_MODEL_CATNIP == static_cast<P047_SensorModels>(P047_MODEL))
        # endif // if P047_FEATURE_BEFLE_V3
        {
          addFormCheckBox(F("Check sensor version"), F("version"), P047_CHECK_VERSION);
        }
      }

      # if P047_FEATURE_ADAFRUIT

      if (P047_MODEL_ADAFRUIT != static_cast<P047_SensorModels>(P047_MODEL))
      # endif // if P047_FEATURE_ADAFRUIT
      {
        addFormSeparator(2);

        addFormCheckBox(F("Change Sensor address"), F("changeAddr"), false);
        addFormTextBox(F("Change I2C Addr. to (Hex)"), F("newAddr"),
                       formatToHex_decimal(P047_I2C_ADDR), 4);
        addUnit(F("0x01..0x7F"));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = true;
      String webarg;
      int    addr;

      # if P047_FEATURE_ADAFRUIT

      if (P047_MODEL_ADAFRUIT == static_cast<P047_SensorModels>(P047_MODEL)) {
        P047_I2C_ADDR = getFormItemInt(F("i2c_addr"));
      } else
      # endif // if P047_FEATURE_ADAFRUIT
      {
        webarg = webArg(F("i2c_addr"));
        addr   = static_cast<int>(strtol(webarg.c_str(), 0, 16));

        if ((addr > 0x00) && (addr < 0x80)) {
          P047_I2C_ADDR = addr;
        } else {
          addHtmlError(F("I2C Address (Hex) error, range: 0x01..0x7F"));
          success = false;
        }
      }

      uint8_t model = getFormItemInt(F("model"));

      if (model != P047_MODEL) {
        P047_MODEL = model;

        if (P047_MODEL_CATNIP == static_cast<P047_SensorModels>(model)) {
          P047_I2C_ADDR = P047_CATNIP_DEFAULT_ADDR;
          strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_047)); // Gets wiped when switching nr. of values
        } else if (P047_MODEL_BEFLE == static_cast<P047_SensorModels>(model)) {
          P047_I2C_ADDR = P047_BEFLE_DEFAULT_ADDR;
          # if P047_FEATURE_ADAFRUIT
        } else if (P047_MODEL_ADAFRUIT == static_cast<P047_SensorModels>(model)) {
          P047_I2C_ADDR = P047_ADAFRUIT_DEFAULT_ADDR;
          # endif // if P047_FEATURE_ADAFRUIT
          # if P047_FEATURE_BEFLE_V3
        } else if (P047_MODEL_BEFLE_V3 == static_cast<P047_SensorModels>(model)) {
          P047_I2C_ADDR = P047_BEFLE_V3_DEFAULT_ADDR;
          # endif // if P047_FEATURE_BEFLE_V3
        }
      }

      P047_SENSOR_SLEEP = isFormItemChecked(F("sleep"));

      P047_CHECK_VERSION = isFormItemChecked(F("version"));

      # if P047_FEATURE_ADAFRUIT

      if (P047_MODEL_ADAFRUIT != static_cast<P047_SensorModels>(P047_MODEL))
      # endif // if P047_FEATURE_ADAFRUIT
      {
        webarg = webArg(F("newAddr"));

        if (!webarg.isEmpty()) {
          addr = static_cast<int>(strtol(webarg.c_str(), 0, 16));

          if ((addr > 0x00) && (addr < 0x80)) {
            P047_NEW_ADDR    = addr;
            P047_CHANGE_ADDR = isFormItemChecked(F("changeAddr"));
          } else {
            addHtmlError(F("Change I2C Addr. to (Hex) error, range: 0x01..0x7F"));
            success = false;
          }
        }
      }

      break;
    }

    case PLUGIN_INIT:
    {
      success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P047_data_struct(P047_I2C_ADDR, P047_MODEL));
      break;
    }

    case PLUGIN_READ:
    {
      P047_data_struct *P047_data =
        static_cast<P047_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P047_data) {
        success = P047_data->plugin_read(event);
      }

      break;
    }
  }

  return success;
}

#endif // USES_P047

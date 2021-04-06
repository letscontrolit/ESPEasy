#include "_CPlugin_LoRa_TTN_helper.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../Globals/Settings.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../../_Plugin_Helper.h"

// #######################################################################################################
// #  Helper functions to encode data for use on LoRa/TTN network.
// #######################################################################################################

#if defined(USES_PACKED_RAW_DATA)


String getPackedFromPlugin(struct EventStruct *event, uint8_t sampleSetCount)
{
  byte   value_count = getValueCountForTask(event->TaskIndex);
  String raw_packed;

  if (PluginCall(PLUGIN_GET_PACKED_RAW_DATA, event, raw_packed)) {
    value_count = event->Par1;
  }
  String packed;
  packed.reserve(32);
  packed += LoRa_addInt(Settings.TaskDeviceNumber[event->TaskIndex], PackedData_uint8);
  packed += LoRa_addInt(event->idx, PackedData_uint16);
  packed += LoRa_addInt(sampleSetCount, PackedData_uint8);
  packed += LoRa_addInt(value_count, PackedData_uint8);
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("packed header: ");
    log += packed;
    if (raw_packed.length() > 0) {
      log += F(" RAW: ");
      log += raw_packed;
    }
    addLog(LOG_LEVEL_INFO, log);
  }

  if (raw_packed.length() > 0) {
    packed += raw_packed;
  } else {
    switch (event->getSensorType())
    {
      case Sensor_VType::SENSOR_TYPE_LONG:
      {
        unsigned long longval = UserVar.getSensorTypeLong(event->TaskIndex);
        packed += LoRa_addInt(longval, PackedData_uint32);
        break;
      }

      default:

        for (byte i = 0; i < value_count && i < VARS_PER_TASK; ++i) {
          // For now, just store the floats as an int32 by multiplying the value with 10000.
          packed += LoRa_addFloat(UserVar[event->BaseVarIndex + i], PackedData_int32_1e4);
        }
        break;
    }
  }
  return packed;
}

float getLoRaAirTime(uint8_t pl, uint8_t sf, uint16_t bw, uint8_t cr, uint8_t n_preamble, bool header, bool crc)
{
  if (sf > 12) {
    sf = 12;
  } else if (sf < 7) {
    sf = 7;
  }

  if (cr > 4) {
    cr = 4;
  } else if (cr < 1) {
    cr = 1;
  }

  // Symbols in frame
  int payload_length = 8;
  {
    int beta_offset = 28;

    if (crc) { beta_offset += 16; }

    if (!header) { beta_offset -= 20; }
    float beta_f                  = 8.0f * pl - 4.0f * sf + beta_offset;
    bool  lowDataRateOptimization = (bw == 125 && sf >= 11);

    if (lowDataRateOptimization) {
      beta_f = beta_f / (4.0f * (sf - 2));
    } else {
      beta_f = beta_f / (4.0f * sf);
    }
    int beta = static_cast<int>(beta_f + 1.0f); // ceil

    if (beta > 0) {
      payload_length += (beta * (cr + 4));
    }
  }

  // t_symbol and t_air in msec
  float t_symbol = (1 << sf) / bw;
  float t_air    = ((n_preamble + 4.25f) + payload_length) * t_symbol;
  return t_air;
}

#endif // USES_PACKED_RAW_DATA

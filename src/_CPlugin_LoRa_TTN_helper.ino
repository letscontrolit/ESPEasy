// #######################################################################################################
// #  Helper functions to encode data for use on LoRa/TTN network.
// #######################################################################################################

#if defined(USES_PACKED_RAW_DATA)



String getPackedFromPlugin(struct EventStruct *event, uint8_t sampleSetCount)
{
  byte value_count = getValueCountFromSensorType(event->sensorType);
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

  if (raw_packed.length() > 0) {
    packed += raw_packed;
  } else {
    const byte BaseVarIndex = event->TaskIndex * VARS_PER_TASK;
    switch (event->sensorType)
    {
      case SENSOR_TYPE_LONG:
      {
        unsigned long longval = (unsigned long)UserVar[BaseVarIndex] + ((unsigned long)UserVar[BaseVarIndex + 1] << 16);
        packed += LoRa_addInt(longval, PackedData_uint32);
        break;
      }

      default:
        for (byte i = 0; i < value_count && i < VARS_PER_TASK; ++i) {
          // For now, just store the floats as an int32 by multiplying the value with 10000.
          packed += LoRa_addFloat(value_count, PackedData_int32_1e4);
        }
        break;
    }
  }
  return packed;
}

#endif // USES_PACKED_RAW_DATA
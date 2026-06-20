#include "../PluginStructs/P187_data_struct.h"

#ifdef USES_P187

# include "../Helpers/CRC_functions.h"

bool P187_data_struct::init(struct EventStruct *event) {

  if (event != nullptr) {
    LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&(P187_param), sizeof(P187_param));  // load configuration from flash

    for (int i = 0; i < VARS_PER_TASK; i++) {
      P187_time[i] = 0.0f;   // initialize time
    }
   initialized = true;
  }

  return isInitialized();
}


bool P187_data_struct::plugin_read(struct EventStruct *event) {
  bool success = false;

  if (isInitialized() && (event != nullptr)) {

      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

      for (int i = 0; i < valueCount; i++)
      {
        switch (P187_OUTPUT_OPTIONx_CONFIG(i))
        {
          case P187_OUTPUT_SINUS:
            UserVar.setFloat(event->TaskIndex, i, P187_param[i].param1 + (P187_param[i].param0 * sinf((P187_time[i] + P187_param[i].param3) * PI / 180.0f)));
            break;
          case P187_OUTPUT_TRAPEZ:

            if ((P187_time[i] * P187_param[i].param2 / 360.0f)  < P187_param[i].param4) // rising edge
            {
              UserVar.setFloat(event->TaskIndex, i,
                               P187_param[i].param1 + ((P187_param[i].param0- P187_param[i].param1) * (P187_time[i] * P187_param[i].param2 / 360.0f) / P187_param[i].param4));
            }
            else if ((P187_time[i] * P187_param[i].param2 / 360.0f)  < (P187_param[i].param4 + P187_param[i].param3)) // on level
            {
              UserVar.setFloat(event->TaskIndex, i, P187_param[i].param0);
            }
            else if ((P187_time[i] * P187_param[i].param2 / 360.0f)  < (P187_param[i].param4 + P187_param[i].param3 + P187_param[i].param5)) // falling edge
            {
              UserVar.setFloat(event->TaskIndex, i,
                               P187_param[i].param1 +
                               ((P187_param[i].param0 - P187_param[i].param1) *
                                (1.0f - (((P187_time[i] * P187_param[i].param2 / 360.0f) - P187_param[i].param4 - P187_param[i].param3) / P187_param[i].param5))));
            }
            else // off level
            {
              UserVar.setFloat(event->TaskIndex, i, P187_param[i].param1);
            }
            break;
          case P187_OUTPUT_RANDOM:
            UserVar.setFloat(event->TaskIndex, i, P187_param[i].param1 + (random(0, 10000) / 10000.0f) * (P187_param[i].param0 - P187_param[i].param1));
            break;
        }
      }
      success = true;
  }

  return success;
}

bool P187_data_struct::loop(struct EventStruct *event)
{
  const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

  for (int i = 0; i < valueCount; i++)
  {
    if (P187_param[i].param2) // check for period value != 0
      P187_time[i] += 36.0f / P187_param[i].param2; // advance time for each output channel

    if (P187_time[i] > 360.0f) // timer range is 0 - 360 (convenient for sinus output)
      P187_time[i] -= 360.0f;  // so wrap around after one period
  }
  return true;
}
#endif // ifdef USES_P187

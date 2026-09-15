#include "../PluginStructs/P187_data_struct.h"

#ifdef USES_P187

# include "../Helpers/CRC_functions.h"

bool P187_data_struct::init(struct EventStruct *event) {

  if (event != nullptr) {
    unsigned long now = millis();                                                           // use same point in time for all outputs
    LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&(P187_param), sizeof(P187_param)); // load configuration from flash

    for (int i = 0; i < VARS_PER_TASK; i++) {
      periodStart[i] = now;                                                                 // initialize period start time
    }
    initialized = true;
  }

  return isInitialized();
}

# define PARAMms2(x) (P187_param[x].param2 * 1000)
# define PARAMms3(x) (P187_param[x].param3 * 1000)
# define PARAMms4(x) (P187_param[x].param4 * 1000)
# define PARAMms5(x) (P187_param[x].param5 * 1000)

bool P187_data_struct::plugin_read(struct EventStruct *event) {
  bool success = false;

  if (isInitialized() && (event != nullptr)) {
    unsigned long now = millis(); // use same point in time for all outputs

    const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P187_OUTPUT_TYPE));

    for (int i = 0; i < valueCount; i++)
    {
      int t_ms;                   // time within period in ms

      if (now < periodStart[i]) { // check for overflow (happens each ~49 days)
        t_ms = (now + (ULONG_MAX - periodStart[i]));
      }
      else {
        t_ms = now - periodStart[i];
      }

      if (t_ms > PARAMms2(i)) // period wrap around
      {
        periodStart[i] = periodStart[i] + PARAMms2(i);
        t_ms          -= PARAMms2(i);
      }

      switch (P187_OUTPUT_OPTIONx_CONFIG(i))
      {
        case P187_OUTPUT_SINUS:
          UserVar.setFloat(event->TaskIndex, i,
                           P187_param[i].param1 +
                           ((float)P187_param[i].param0 *
                            sinf((2 * PI * (float)t_ms / (float)(PARAMms2(i))) + ((float)P187_param[i].param3 * PI / 180.0f))));
          break;

        case P187_OUTPUT_TRAPEZ:

          if (t_ms < PARAMms4(i)) // rising edge
          {
            UserVar.setFloat(event->TaskIndex, i,
                             P187_param[i].param1 +
                             ((float)(P187_param[i].param0 - P187_param[i].param1) * (float)t_ms / (float)PARAMms4(i)));
          }
          else if (t_ms < (PARAMms4(i) + PARAMms3(i))) // on level
          {
            UserVar.setFloat(event->TaskIndex, i, P187_param[i].param0);
          }
          else if (t_ms < (PARAMms4(i) + PARAMms3(i) + PARAMms5(i))) // falling edge
          {
            UserVar.setFloat(event->TaskIndex, i,
                             P187_param[i].param1 +
                             ((P187_param[i].param0 - P187_param[i].param1) *
                              (1.0f - (((float)(t_ms - PARAMms4(i) - PARAMms3(i)) / (float)PARAMms5(i))))));
          }
          else // off level
          {
            UserVar.setFloat(event->TaskIndex,
                             i,
                             P187_param[i].param1);
          }
          break;
        case P187_OUTPUT_RANDOM:
          UserVar.setFloat(event->TaskIndex, i,
                           P187_param[i].param1 + (random(0, 10000) / 10000.0f) * (P187_param[i].param0 - P187_param[i].param1));
          break;
      }
    }
    success = true;
  }

  return success;
}

#endif // ifdef USES_P187

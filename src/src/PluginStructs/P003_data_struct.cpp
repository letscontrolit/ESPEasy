#include "../PluginStructs/P003_data_struct.h"


#ifdef USES_P003
P003_data_struct::P003_data_struct(const Internal_GPIO_pulseHelper::pulseCounterConfig& config)
  : pulseHelper(config) {}

bool P003_data_struct::plugin_read(struct EventStruct *event) {
  bool success = true;
  unsigned long pulseCounter, pulseCounterTotal;
  float pulseTime_msec;

  pulseHelper.getPulseCounters(pulseCounter, pulseCounterTotal, pulseTime_msec);
  pulseHelper.resetPulseCounter();


  if (PCONFIG(P003_IDX_IGNORE_ZERO) && (0 == pulseCounter) && lastDeltaZero) {
    success = false;
  }
  lastDeltaZero = (0 == pulseCounter);

  // store the current counter values into UserVar (RTC-memory)
  // FIXME TD-er: Must check we're interacting with the raw values in this PulseCounter plugin
  // For backward compatibility, set all values
  UserVar.setFloat(event->TaskIndex, P003_IDX_pulseCounter,      pulseCounter);
  UserVar.setFloat(event->TaskIndex, P003_IDX_pulseTotalCounter, pulseCounterTotal);
  UserVar.setFloat(event->TaskIndex, P003_IDX_pulseTime,         pulseTime_msec);

  switch (PCONFIG(P003_IDX_COUNTERTYPE)) {
    case P003_CT_INDEX_COUNTER:            // No updates
    case P003_CT_INDEX_COUNTER_TOTAL:      // No updates
      break;
    case P003_CT_INDEX_TOTAL:              // Replace first value
      UserVar.setFloat(event->TaskIndex, P003_IDX_pulseCounter, pulseCounterTotal);
      break;
    case P003_CT_INDEX_COUNTER_TOTAL_TIME: // No updates
      break;
    # if P003_USE_EXTRA_COUNTERTYPES
    case P003_CT_INDEX_TIME:               // Replace first value
      UserVar.setFloat(event->TaskIndex, P003_IDX_pulseCounter,      pulseTime_msec);
      break;
    case P003_CT_INDEX_TOTAL_TIME:         // Replace first 2 values
      UserVar.setFloat(event->TaskIndex, P003_IDX_pulseCounter,      pulseCounterTotal);
      UserVar.setFloat(event->TaskIndex, P003_IDX_pulseTotalCounter, pulseTime_msec);
      break;
    case P003_CT_INDEX_TIME_COUNTER: // Replace first 2 values
      UserVar.setFloat(event->TaskIndex, P003_IDX_pulseCounter,      pulseTime_msec);
      UserVar.setFloat(event->TaskIndex, P003_IDX_pulseTotalCounter, pulseCounter);
      break;
    # endif // if P003_USE_EXTRA_COUNTERTYPES
  }

  // Store the raw value in the unused 4th position.
  // This is needed to restore the value from RTC as it may be converted into another output value using a formula.
  UserVar.setFloat(event->TaskIndex, P003_IDX_persistedTotalCounter, pulseCounterTotal);
  return success;
}

bool P003_data_struct::plugin_peek(struct EventStruct *event) {
  unsigned long pulseCounter, pulseCounterTotal;
  float pulseTime_msec;

  pulseHelper.getPulseCounters(pulseCounter, pulseCounterTotal, pulseTime_msec);

  const bool success = (PCONFIG(P003_IDX_IGNORE_ZERO) && (0 != pulseCounter));

  return success;
}

#endif // ifdef USES_P003

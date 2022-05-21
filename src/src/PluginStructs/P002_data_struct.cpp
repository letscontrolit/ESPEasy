#include "../PluginStructs/P002_data_struct.h"

#ifdef USES_P002


P002_data_struct::~P002_data_struct() {
  reset();
}

void P002_data_struct::reset() {
  OversamplingValue  = 0;
  OversamplingCount  = 0;
  OversamplingMinVal = P002_MAX_ADC_VALUE;
  OversamplingMaxVal = -P002_MAX_ADC_VALUE;
}

void P002_data_struct::addOversamplingValue(int currentValue) {
  // Extra check to only add min or max readings once.
  // They will be taken out of the averaging only one time.
  if ((currentValue == 0) && (currentValue == OversamplingMinVal)) {
    return;
  }

  if ((currentValue == P002_MAX_ADC_VALUE) && (currentValue == OversamplingMaxVal)) {
    return;
  }

  OversamplingValue += currentValue;
  ++OversamplingCount;

  if (currentValue > OversamplingMaxVal) {
    OversamplingMaxVal = currentValue;
  }

  if (currentValue < OversamplingMinVal) {
    OversamplingMinVal = currentValue;
  }
}

bool P002_data_struct::getOversamplingValue(float& float_value, int& raw_value) const {
  if (OversamplingCount > 0) {
    float sum   = static_cast<float>(OversamplingValue);
    float count = static_cast<float>(OversamplingCount);

    if (OversamplingCount >= 3) {
      sum   -= OversamplingMaxVal;
      sum   -= OversamplingMinVal;
      count -= 2;
    }
    float_value = sum / count;
    raw_value   = static_cast<int16_t>(float_value);
    return true;
  }
  return false;
}

#endif // ifdef USES_P002

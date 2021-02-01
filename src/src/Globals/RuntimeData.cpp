#include "../Globals/RuntimeData.h"


std::map<uint32_t, double> customFloatVar;

float UserVar[VARS_PER_TASK * TASKS_MAX];


double getCustomFloatVar(uint32_t index) {
  auto it = customFloatVar.find(index);

  if (it != customFloatVar.end()) {
    return it->second;
  }
  return 0.0;
}

void setCustomFloatVar(uint32_t index, const double& value) {
  customFloatVar[index] = value;
}

bool getNextCustomFloatVar(uint32_t& index, double& value) {
  auto it = customFloatVar.find(index);

  if (it == customFloatVar.end()) { return false; }
  ++it;

  if (it == customFloatVar.end()) { return false; }
  index = it->first;
  value = it->second;
  return true;
}

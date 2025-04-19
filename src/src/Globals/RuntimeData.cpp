#include "../Globals/RuntimeData.h"


std::map<String, ESPEASY_RULES_FLOAT_TYPE> customFloatVar;

//float UserVar[VARS_PER_TASK * TASKS_MAX];

UserVarStruct UserVar;


ESPEASY_RULES_FLOAT_TYPE getCustomFloatVar(String indexName, ESPEASY_RULES_FLOAT_TYPE defaultValue) {
  indexName.toLowerCase();
  auto it = customFloatVar.find(indexName);

  if (it != customFloatVar.end()) {
    return it->second;
  }
  return defaultValue;
}

void setCustomFloatVar(String indexName, const ESPEASY_RULES_FLOAT_TYPE& value) {
  indexName.toLowerCase();
  // std::map doesn't handle 2nd heap well, so make sure we keep using the default heap.
  # ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  # endif // ifdef USE_SECOND_HEAP

  customFloatVar[indexName] = value;
}

bool getNextCustomFloatVar(String& indexName, ESPEASY_RULES_FLOAT_TYPE& value) {
  String valueName(indexName);
  valueName.toLowerCase();
  auto it = customFloatVar.find(valueName);

  if (it == customFloatVar.end()) { return false; }
  ++it;

  if (it == customFloatVar.end()) { return false; }
  indexName = it->first;
  value = it->second;
  return true;
}

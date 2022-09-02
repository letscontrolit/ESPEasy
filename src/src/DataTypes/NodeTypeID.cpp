#include "../DataTypes/NodeTypeID.h"


const __FlashStringHelper* toNodeTypeDisplayString(uint8_t nodeType) {
  switch (nodeType)
  {
    case NODE_TYPE_ID_ESP_EASY_STD:     return F("ESP Easy");
    case NODE_TYPE_ID_ESP_EASYM_STD:    return F("ESP Easy Mega");
    case NODE_TYPE_ID_ESP_EASY32_STD:   return F("ESP Easy 32");
    case NODE_TYPE_ID_ESP_EASY32S2_STD: return F("ESP Easy 32-s2");
    case NODE_TYPE_ID_ESP_EASY32C3_STD: return F("ESP Easy 32-c3");
    case NODE_TYPE_ID_RPI_EASY_STD:     return F("RPI Easy");
    case NODE_TYPE_ID_ARDUINO_EASY_STD: return F("Arduino Easy");
    case NODE_TYPE_ID_NANO_EASY_STD:    return F("Nano Easy");
  }
  return F("");
}
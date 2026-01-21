#include "../DataTypes/NodeTypeID.h"

#include "../Helpers/StringConverter.h"

String toNodeTypeDisplayString(uint8_t nodeType) {
  const __FlashStringHelper* suffix = F("");

  switch (nodeType)
  {
    case NODE_TYPE_ID_ESP_EASY_STD:        return F("ESP Easy");
    case NODE_TYPE_ID_ESP_EASYM_STD:       return F("ESP Easy Mega");
    case NODE_TYPE_ID_ESP_EASY32_STD:      break;
    case NODE_TYPE_ID_ESP_EASY32S2_STD:    suffix = F("-S2"); break;
    case NODE_TYPE_ID_ESP_EASY32S3_STD:    suffix = F("-S3"); break;
    case NODE_TYPE_ID_ESP_EASY32C2_STD:    suffix = F("-C2"); break;
    case NODE_TYPE_ID_ESP_EASY32C3_STD:    suffix = F("-C3"); break;
    case NODE_TYPE_ID_ESP_EASY32C5_STD:    suffix = F("-C5"); break;
    case NODE_TYPE_ID_ESP_EASY32C6_STD:    suffix = F("-C6"); break;
    case NODE_TYPE_ID_ESP_EASY32C61_STD:   suffix = F("-C61"); break;
    case NODE_TYPE_ID_ESP_EASY32H2_STD:    suffix = F("-H2"); break;
    case NODE_TYPE_ID_ESP_EASY32H21_STD:   suffix = F("-H21"); break;
    case NODE_TYPE_ID_ESP_EASY32H4_STD:    suffix = F("-H4"); break;
    case NODE_TYPE_ID_ESP_EASY32P4_STD:    suffix = F("-P4"); break;
    case NODE_TYPE_ID_ESP_EASY32P4r3_STD:  suffix = F("-P4r3"); break;
    case NODE_TYPE_ID_RPI_EASY_STD:        return F("RPI Easy");
    case NODE_TYPE_ID_ARDUINO_EASY_STD:    return F("Arduino Easy");
    case NODE_TYPE_ID_NANO_EASY_STD:       return F("Nano Easy");
    default:
    return EMPTY_STRING;
  }
  return concat(F("ESP Easy 32"), suffix);  
}

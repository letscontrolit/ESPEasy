#include "../Helpers/StringGenerator_Plugin.h"



String Plugin_valuename(const __FlashStringHelper * name_prefix, uint8_t value_nr, bool displayString) {
  String name = name_prefix;

  if (value_nr != 0) {
    name += value_nr + 1;
  }

  if (!displayString) {
    name.toLowerCase();
  }
  return name;
}

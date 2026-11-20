#include "../Helpers/ESPEasy_Build_Description.h"

#if FEATURE_BUILD_DESCRIPTION
# include "../Helpers/_Plugin_init.h"
# include "../Helpers/_CPlugin_init.h"
# include "../Helpers/_NPlugin_init.h"
# include "../../ESPEasy/net/Helpers/_NWPlugin_init.h"
# include "../Helpers/_Feature_init.h"
# include "../Helpers/StringConverter.h"

# include "../Globals/CPlugins.h"
# include "../Globals/NPlugins.h"
# include "../Globals/Plugins.h"
# include "../../ESPEasy/net/Globals/NWPlugins.h"

xPluginEnumerator::xPluginEnumerator() {}

bool xPluginEnumerator::isEmpty() {
  bool result = true;

  for (size_t i = 0; i < _bitmap.size() && result; ++i) {
    result = result && _bitmap[i] == 0;
  }
  return result;
}

void xPluginEnumerator::fill(const uint16_t arr[]) {
  _bitmap.resize(8, 0u);

  for (uint8_t i = 0; i < 8; i++) {
    _bitmap[i] = arr[i];
  }
}

String xPluginEnumerator::getString(char separator) const {
  String result;

  result.reserve(_bitmap.size() * 5); // 4 HEX characters per 16 bit value + separator
  size_t zeroCount  = 0;
  bool   haveZeroes = false;          // Did we already handle multiple 0-words? (Use IPv6-style bitmap compression)

  for (size_t i = 0; i < _bitmap.size(); ++i) {
    if ((_bitmap[i] == 0) && !haveZeroes) {
      ++zeroCount;
    } else if (zeroCount > 1) { // At least 2 succeeding words must be 0 to be compressed into ::
      result    += separator;
      zeroCount  = 0;
      haveZeroes = true;
    } else if (zeroCount == 1) { // Only one word is 0
      result   += separator;
      zeroCount = 0;
    }

    if (zeroCount == 0) {
      if (_bitmap[i] != 0) {
        result += String(_bitmap[i], HEX);
      }
      result += separator;
    }
  }

  if (zeroCount > 1) {
    result += separator; // Add closing separator if last 2 or more words are 0
  }
  return result;
}

String CreateBuildDescription(char separator) {
  String result;

  // Plugins part 1, 1..128
  xPluginEnumerator statusMap;

  statusMap.fill(pluginsBitmap);

  if (!statusMap.isEmpty()) {
    result += F("P1=");
    result += statusMap.getString(separator);
    result += ',';
  }

  // Plugins part 2, 129..255
  statusMap.fill(&pluginsBitmap[8]);

  if (!statusMap.isEmpty()) {
    result += F("P2=");
    result += statusMap.getString(separator);
    result += ',';
  }

  // Controllers part 1, 1..128
  statusMap.fill(controllersBitmap);

  if (!statusMap.isEmpty()) {
    result += F("T1=");
    result += statusMap.getString(separator);
    result += ',';
  }

  // Controllers part 2, 129..255
  statusMap.fill(&controllersBitmap[8]);

  if (!statusMap.isEmpty()) {
    result += F("T2=");
    result += statusMap.getString(separator);
    result += ',';
  }

  # if FEATURE_NOTIFIER && !defined(NOTIFIER_SET_NONE)
  statusMap.fill(notifierBitmaps);

  if (!statusMap.isEmpty()) {
    result += F("N1=");
    result += statusMap.getString(separator);
    result += ',';
  }
  # endif // if FEATURE_NOTIFIER && !defined(NOTIFIER_SET_NONE)

  statusMap.fill(ESPEasy::net::networksBitmap);

  if (!statusMap.isEmpty()) {
    result += F("W1=");
    result += statusMap.getString(separator);
    result += ',';
  }

  statusMap.fill(&ESPEasy::net::networksBitmap[8]);

  if (!statusMap.isEmpty()) {
    result += F("W2=");
    result += statusMap.getString(separator);
    result += ',';
  }

  statusMap.fill(featuresBitmap);

  if (!statusMap.isEmpty()) {
    result += F("U1=");
    result += statusMap.getString(separator);
    result += ',';
  }

  statusMap.fill(&featuresBitmap[8]);

  if (!statusMap.isEmpty()) {
    result += F("U2=");
    result += statusMap.getString(separator);
    result += ',';
  }

  if (result.endsWith(F(","))) {
    result = result.substring(0, result.length() - 1);
  }

  return result;
}

#endif // if FEATURE_BUILD_DESCRIPTION

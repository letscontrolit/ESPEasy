#include "../Helpers/JSON_helper.h"

/** Changelog:
 * 2026-08-01 tonhuisman: Extracted JSON value parser from HTTPResponseParser by @chromoxdor
 *                        Support bool type by returning 1/0 for true/false
 */

#if FEATURE_JSON_PARSE
# include "../Helpers/StringConverter.h"

// Private for now
String getJsonValue(JsonVariant element,
                    bool        asJson);

String getJsonValue(DynamicJsonDocument *root,
                    String               key,
                    bool                 asJson) {
  // Process the key and navigate the JSON
  JsonVariant value = *root;
  size_t start      = 0;
  size_t end;
  String result;

  while ((end = key.indexOf('.', start)) != -1) {
    const String part = key.substring(start, end);
    start = end + 1;

    // Look for an array e.g., "result[0]" → object "result", index 0
    const int bracketStart = part.indexOf('[');

    if (bracketStart != -1) {
      const String objectName = part.substring(0, bracketStart);
      const String indexStr   = part.substring(bracketStart + 1, part.indexOf(']', bracketStart));

      if (objectName.length() > 0) {
        value = value[objectName]; // Access the object
      }

      if (value.is<JsonArray>()) {
        value = value[indexStr.toInt()];
      } else {
        value = value[indexStr]; // fallback if not actually array
      }
    } else {
      // Normal object access without array
      value = value[part];
    }

    if (value.isNull()) {
      break; // Key path is invalid
    }
  }

  if (!value.isNull()) {
    const String lastPart     = key.substring(start);
    const int    bracketStart = lastPart.indexOf('[');

    if (bracketStart != -1) {
      const String objectName = lastPart.substring(0, bracketStart);
      const String indexStr   = lastPart.substring(bracketStart + 1, lastPart.indexOf(']', bracketStart));

      if (objectName.length() > 0) {
        value = value[objectName];
      }

      if (value.is<JsonArray>()) {
        value = value[indexStr.toInt()];
      } else {
        value = value[indexStr];
      }
    } else {
      value = value[lastPart];
    }
  }

  // Append the value to the CSV string if it exists
  if (!value.isNull()) {
    result += getJsonValue(value, asJson);

  }
  return result;
}

String getJsonValue(JsonVariant value, bool asJson) {
  # if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  const int nr_decimals = ESPEASY_DOUBLE_NR_DECIMALS;
  # else // ifdef FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  const int nr_decimals = ESPEASY_FLOAT_NR_DECIMALS;
  # endif // ifdef FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE

  String result;

  if (value.is<int>()) {
    result += String(value.as<int>());
  } else if (value.is<float>()) {
    result += doubleToString(value.as<double>(), nr_decimals, true);
  } else if (value.is<const char *>()) {
    if (asJson) {
      result += wrap_String(String(value.as<const char *>()), '\"'); // JSON quotes are always "
    } else {
      result += String(value.as<const char *>());
    }
  } else if (value.is<bool>()) {
    if (asJson) {
      result += value.as<bool>() ? F("true") : F("false");
    } else {
      result += String(value.as<bool>() ? 1 : 0);
    }
  } else if (value.is<JsonArray>()) {
    // If the value is an array, iterate over its elements
    JsonArray array        = value.as<JsonArray>();
    size_t    arraySize    = array.size(); // Get the total number of elements in the array
    size_t    currentIndex = 0;            // Track the current index

    if (asJson) {
      result += '[';
    }

    for (JsonVariant element : array) {
      result += getJsonValue(element, asJson);

      // Add a comma unless it's the last element
      currentIndex++;

      if (currentIndex < arraySize) {
        result += ',';
      }
    }

    if (asJson) {
      result += ']';
    }
  } else if (value.is<JsonObject>()) {
    // if the value is a JSON Object, iterate over the attributes and return <name>,<value> pair(s), recursive
    auto it           = value.as<JsonObject>().begin();
    int  objectSize   = value.as<JsonObject>().size();
    int  currentIndex = 0;

    while (it != value.as<JsonObject>().end()) {
      if (asJson) {
        result += '{';
        result += wrap_String(it->key().c_str(), '\"'); // JSON quotes are always "
      } else {
        result += String(it->key().c_str());
      }
      result += ',';
      result += getJsonValue(it->value(), asJson);

      if (asJson) {
        result += '}';
      }
      currentIndex++;

      if (currentIndex < objectSize) {
        result += ',';
      }
      ++it;
    }

  } else {
    result += F("unknown");
  }

  return result;
}

#endif // if FEATURE_JSON_PARSE

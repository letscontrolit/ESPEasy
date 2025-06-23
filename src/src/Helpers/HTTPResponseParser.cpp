#include "../Helpers/HTTPResponseParser.h"

#if RESPONSE_PARSER_SUPPORT
# include "../Helpers/Networking.h"
# include "../../ESPEasy_common.h"
# include "../../_Plugin_Helper.h"
# include "../Globals/EventQueue.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/StringProvider.h"

# if FEATURE_JSON_EVENT
#  include "../Helpers/ESPEasy_Storage.h"
#  include "../WebServer/LoadFromFS.h"

#  include <ArduinoJson.h>

# endif // if FEATURE_JSON_EVENT


void eventFromResponse(const String& host, const int& httpCode, const String& uri, HTTPClient& http) {
  if ((httpCode == 200)) {
    // -------------------------------------------------------------------------------------------Thingspeak
  # if FEATURE_THINGSPEAK_EVENT

    // Generate event with the response of a
    // thingspeak request (https://de.mathworks.com/help/thingspeak/readlastfieldentry.html &
    // https://de.mathworks.com/help/thingspeak/readdata.html)
    // e.g. command for a specific field: "sendToHTTP,api.thingspeak.com,80,/channels/1637928/fields/5/last.csv"
    // command for all fields: "sendToHTTP,api.thingspeak.com,80,/channels/1637928/feeds/last.csv"
    // where first eventvalue is the channel number and the second to the nineth event values
    // are the field values
    // Example of the event: "EVENT: ThingspeakReply=1637928,5,24.2,12,900,..."
    //                                                  ^    ^ └------┬------┘
    //                                   channel number ┘    |        └ received values
    //                                                   field number (only available for a "single-value-event")
    // In rules you can grep the reply by "On ThingspeakReply Do ..."
    // -----------------------------------------------------------------------------------------------------------------------------
    // 2024-02-05 - Added the option to get a single value of a field or all values of a channel at a certain time (not only the last entry)
    // Examples:
    // Single channel: "sendtohttp,api.thingspeak.com,80,channels/1637928/fields/1.csv?end=2024-01-01%2023:59:00&results=1"
    // => gets the value of field 1 at (or the last entry before) 23:59:00 of the channel 1637928
    // All channels: "sendtohttp,api.thingspeak.com,80,channels/1637928/feeds.csv?end=2024-01-01%2023:59:00&results=1"
    // => gets the value of each field of the channel 1637928 at (or the last entry before) 23:59:00
    // -----------------------------------------------------------------------------------------------------------------------------

    if (equals(host, F("api.thingspeak.com")) &&
        (uri.endsWith(F("/last.csv")) || ((uri.indexOf(F("results=1")) >= 0) && (uri.indexOf(F(".csv")) >= 0)))) {
      String result = http.getString();

      result.replace(' ', '_'); // if using a single field with a certain time, the result contains a space and would break the code
      const int posTimestamp = result.lastIndexOf(':');

      if (posTimestamp >= 0) {
        result = parseStringToEndKeepCase(result.substring(posTimestamp), 3);

        if (uri.indexOf(F("fields")) >= 0) {                                        // when there is a single field call add the field
                                                                                    // number
                                                                                    // before the value
          result = parseStringKeepCase(uri, 4, '/').substring(0, 1) + "," + result; // since the field number is always the fourth part of
                                                                                    // the
                                                                                    // url and is always a single digit, we can use this to
                                                                                    // extract the fieldnumber
        }
        eventQueue.addMove(strformat(
                             F("ThingspeakReply=%s,%s"),
                             parseStringKeepCase(uri, 2, '/').c_str(),
                             result.c_str()));
      }
    }
  # endif // if FEATURE_THINGSPEAK_EVENT

    // ------------------------------------------------------------------------------------------- OpenMeteo
  # if FEATURE_OPENMETEO_EVENT

    // Generate an event with the response of an open-meteo request.
    // Example command:
    // sendtohttp,api.open-meteo.com,80,"/v1/forecast?latitude=52.52&longitude=13.41¤t=temperature_2m,weather_code,wind_speed_10m&daily=temperature_2m_max,temperature_2m_min&forecast_days=1"
    // No need for an api key and it is free (daily requests are limited to 10,000 in the free version)
    // Visit the URL (https://open-meteo.com/en/docs) and build your personal URL by selecting the location and values you want to receive.
    // Supported variable kinds are current, hourly, daily!
    // In rules you can grep the reply by the kind of weather variables with "On Openmeteo#<type> Do ..."
    // e.g. "On Openmeteo#current Do ..."
    // Note: hourly and daily results are arrays which can become very long.
    // Best to make seperate calls. Especially for hourly results.

    // define limits
  #  define WEATHER_KEYS_MAX 10

    if (equals(host, F("api.open-meteo.com"))) {
      const String str = http.getString();

      if (str.length() > RESPONSE_MAX_LENGTH) {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          addLog(LOG_LEVEL_ERROR,
                 strformat(F("Response exceeds %d characters which could cause instabilities or crashes!"), RESPONSE_MAX_LENGTH));
        }
      }

      auto processAndQueueParams = [](const String& url, const String& str, const String& eventName) {
                                     // Extract the parameters from the URL
                                     int start = url.indexOf(eventName + '=');

                                     if (start == -1) {
                                       return; // No parameters found for the given eventName
                                     }
                                     start += eventName.length() + 1;
                                     const int end       = url.indexOf('&', start);
                                     const String params = (end == -1) ? url.substring(start) : url.substring(start, end);

                                     if (!params.isEmpty()) {
                                       String keys[WEATHER_KEYS_MAX];
                                       int    keyCount   = 0;
                                       int    startIndex = 0;
                                       int    commaIndex = params.indexOf(',');

                                       // Split and add keys to the array
                                       while (commaIndex != -1) {
                                         if (keyCount >= WEATHER_KEYS_MAX) {
                                           // Stop adding keys if array is full
                                           if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
                                             addLog(LOG_LEVEL_ERROR,
                                                    strformat(F(
                                                                "More than %d keys in the URL, this could cause instabilities or crashes! Try to split up the calls.."),
                                                              WEATHER_KEYS_MAX));
                                           }
                                           break;
                                         }
                                         String key = params.substring(startIndex, commaIndex);
                                         keys[keyCount++] = key;
                                         startIndex       = commaIndex + 1;
                                         commaIndex       = params.indexOf(',', startIndex);
                                       }

                                       // Add the last key
                                       if (keyCount < WEATHER_KEYS_MAX) {
                                         const String lastKey = params.substring(startIndex);
                                         keys[keyCount++] = lastKey;
                                       }

                                       String csv;
                                       const int startStringIndex = str.indexOf(strformat(F("\"%s\":"),
                                                                                          eventName.c_str())) +
                                                                    eventName.length() + 4;

                                       // example( },"current":{"time":... ) we want to start after current":{

                                       for (int i = 0; i < keyCount; i++) // Use keyCount to limit the iteration
                                       {
                                         String key = keys[i];
                                         String value;
                                         int    startIndex = str.indexOf(strformat(F("%s\":"), key.c_str()), startStringIndex);

                                         if (startIndex == -1) {
                                           // Handle case where key is not found
                                           value = F("-256"); // Placeholder value
                                         } else {
                                           int endIndex = 0;

                                           if (!equals(eventName, F("current"))) {
                                             // In daily and hourly, the values are stored in an array
                                             startIndex += key.length() + 3; // Move index past the key
                                             endIndex    = str.indexOf(']', startIndex);
                                           } else {
                                             startIndex += key.length() + 2; // Move index past the key
                                             endIndex    = str.indexOf(',', startIndex);
                                           }

                                           // Find the index of the next comma
                                           if ((endIndex == -1) || (endIndex > str.indexOf("}", startIndex))) {
                                             endIndex = str.indexOf('}', startIndex); // If no comma is found or comma comes after },
                                                                                      // take the rest of the string
                                           }

                                           value = str.substring(startIndex, endIndex);
                                           value.trim(); // Remove any surrounding whitespace
                                         }

                                         if (!csv.isEmpty()) {
                                           csv += ',';
                                         }
                                         csv += value;
                                       }
                                       eventQueue.addMove(strformat(F("OpenMeteo#%s=%s"), eventName.c_str(), csv.c_str()));
                                     }
                                   };

      processAndQueueParams(uri, str, F("current"));
      processAndQueueParams(uri, str, F("hourly"));
      processAndQueueParams(uri, str, F("daily"));
    }

  # endif // if FEATURE_OMETEO_EVENT

    // ------------------------------------------------------------------------------------------- JSONevent
  # if FEATURE_JSON_EVENT

    if ((uri.indexOf(F("#json")) != -1) || (uri.indexOf(F("?json")) != -1)) {
      int numJson = 0; // Default value

      int i = uri.length() - 1;

      // Check if the URL ends with a number
      if ((i > -1) && isDigit(uri.charAt(i))) {
        // Find the position of the last non-digit character
        while (i >= 0 && isDigit(uri.charAt(i))) {
          i--;
        }

        // Extract the number from the string
        numJson = uri.substring(i + 1).toInt();
      }

      const String message = http.getString();

      if (message.length() > RESPONSE_MAX_LENGTH) {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          addLog(LOG_LEVEL_ERROR,
                 strformat(F("Response exceeds %d characters which could cause instabilities or crashes!"), RESPONSE_MAX_LENGTH));
        }
      }

      DynamicJsonDocument*root       = nullptr;
      uint16_t lastJsonMessageLength = 512;

      // Cleanup lambda to deallocate resources
      auto cleanupJSON = [&root]() {
                           if (root != nullptr) {
                             root->clear();
                             delete root;
                             root = nullptr;
                           }
                         };

      // Check if root needs resizing or cleanup
      if ((root != nullptr) && (message.length() * 2.5 > lastJsonMessageLength)) {
        cleanupJSON();
      }

      // Resize lastJsonMessageLength if needed
      if (message.length() * 2 > lastJsonMessageLength) {
        lastJsonMessageLength = message.length() * 2;
      }

      // Allocate memory for root if needed
      if (root == nullptr) {
  #  ifdef USE_SECOND_HEAP
        HeapSelectIram ephemeral;
  #  endif // ifdef USE_SECOND_HEAP
        root = new (std::nothrow) DynamicJsonDocument(lastJsonMessageLength);
      }

      if (root != nullptr) {
        // Parse the JSON
        DeserializationError error = deserializeJson(*root, message);

        if (!error) {
          // Process the keys from the file
          readAndProcessJsonKeys(root, numJson);
        } else {
          if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
            addLog(LOG_LEVEL_ERROR, strformat(F("Parsing JSON failed: %s"), error.c_str()));
          }
        }

        // Cleanup JSON resources
        cleanupJSON();
      }
    }
    # endif // if FEATURE_JSON_EVENT
  }
}

// ------------------------------------------------------------------------------------------- JSONevent Key processing
   # if FEATURE_JSON_EVENT

void readAndProcessJsonKeys(DynamicJsonDocument *root, int numJson) {
#  if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  int nr_decimals = ESPEASY_DOUBLE_NR_DECIMALS;
#  else // ifdef FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  int nr_decimals = ESPEASY_FLOAT_NR_DECIMALS;
#  endif // ifdef FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE

  // Open the `json.keys` file
#  ifdef ESP8266
  const String fileName = F("json.keys");
#  else // ifdef ESP8266
  const String fileName = F("/json.keys");
#  endif // ifdef ESP8266

  if (!fileExists(fileName)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      addLogMove(LOG_LEVEL_ERROR, strformat(F("%s does not exist!"), fileName.c_str()));
    }
    return;
  }
  File keyFile = ESPEASY_FS.open(fileName, "r");

  int keyCount                   = 0;
  int successfullyProcessedCount = 0; // Counter for successfully processed keys
  String csvOutput;                   // Collect values as a CSV string

  while (keyFile.available()) {
    String key = keyFile.readStringUntil('\n');
    key.trim(); // Remove extra spaces or newlines

    if (key.isEmpty()) {
      continue;
    }
    int keyNumber = 0;

    // Extract the number prefix (before the first dot)
    if (key.indexOf(':') != -1) {
      keyNumber = key.substring(0, key.indexOf(':')).toInt();
      key       = key.substring(key.indexOf(':') + 1);
    }

    // Only process keys that match the filterNumber (or process all if no match)
    if ((numJson > 0) && (keyNumber != numJson)) {
      continue; // Skip this key if it doesn't match the filter number
    }
    keyCount++;

    if (keyCount > MAX_KEYS) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLogMove(LOG_LEVEL_ERROR, strformat(F("Warning: More than %d keys in %s"), MAX_KEYS, fileName.c_str()));
      }
    }

    // Process the key and navigate the JSON
    JsonVariant value = *root;
    size_t start = 0, end;

    while ((end = key.indexOf('.', start)) != (unsigned int)-1) {
      const String part = key.substring(start, end);
      value = value[part];

      if (value.isNull()) {
        break; // Key path is invalid
      }
      start = end + 1;
    }

    // Handle the last part of the key
    if (!value.isNull()) {
      const String lastPart = key.substring(start);
      value = value[lastPart];
      successfullyProcessedCount++;
    }

    // Append the value to the CSV string if it exists
    if (!value.isNull()) {
      if (value.is<int>()) {
        csvOutput += String(value.as<int>());
      } else if (value.is<float>()) {
        csvOutput += doubleToString(value.as<double>(), nr_decimals, true);
      } else if (value.is<const char *>()) {
        csvOutput += String(value.as<const char *>());
      } else if (value.is<JsonArray>()) {
        // If the value is an array, iterate over its elements
        JsonArray array        = value.as<JsonArray>();
        size_t    arraySize    = array.size(); // Get the total number of elements in the array
        size_t    currentIndex = 0;            // Track the current index

        for (JsonVariant element : array) {
          if (element.is<int>()) {
            csvOutput += String(element.as<int>());
          } else if (element.is<float>()) {
            csvOutput += doubleToString(element.as<double>(), nr_decimals, true);
          } else if (element.is<const char *>()) {
            csvOutput += String(element.as<const char *>());
          } else {
            csvOutput += F("unknown");
          }

          // Add a comma unless it's the last element
          currentIndex++;

          if (currentIndex < arraySize) {
            csvOutput += ',';
          }
        }
      } else {
        csvOutput += F("unknown");
      }
    } else {
      csvOutput += F("null"); // Indicate missing value
    }
    csvOutput += ',';
  }

  keyFile.close();

  if (!csvOutput.isEmpty()) {                   // If there is nothing to show don't create an event
    if (csvOutput.charAt(csvOutput.length() - 1) == ',') {
      // Check if the last character is a comma
      csvOutput.remove(csvOutput.length() - 1); // Remove the last character
    }

    // Log the results
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("Successfully processed %d out of %d keys"), successfullyProcessedCount, keyCount));
      eventQueue.addMove(strformat(F("JsonReply%s%s=%s"),
                                   numJson != 0 ? "#" : "",
                                   toStringNoZero(numJson).c_str(),
                                   csvOutput.c_str()));
    }
  }
}

  # endif // if FEATURE_JSON_EVENT
#endif // if RESPONSE_PARSER_SUPPORT

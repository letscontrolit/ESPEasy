#ifndef HELPERS_HTTPRESPONSEPARSER_H
#define HELPERS_HTTPRESPONSEPARSER_H

#include "../../ESPEasy_common.h"
#include "../../_Plugin_Helper.h"

#if RESPONSE_PARSER_SUPPORT
# include "../Helpers/Networking.h"

# include <ArduinoJson.h>

# ifdef ESP8266
#  include <ESP8266HTTPClient.h>
# endif // ifdef ESP8266 #

# define MAX_KEYS 20 // Maximum number of keys allowed in json.keys
# define RESPONSE_MAX_LENGTH 5000

/**
 * @brief Reads and processes keys from a json.keys file and navigates the JSON document.
 */
void readAndProcessJsonKeys(DynamicJsonDocument*root,
                            int                 numJson);

/**
 * @brief Processes the response of an HTTP request, extracts the JSON, and processes it using keys from `json.keys`.
 */
void eventFromResponse(const String& host,
                       const int   & httpCode,
                       const String& uri,
                       HTTPClient  & http);
#endif // RESPONSE_PARSER_SUPPORT
#endif // HELPERS_HTTPRESPONSEPARSER_H

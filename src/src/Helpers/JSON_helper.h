#pragma once

/** See */
#if FEATURE_JSON_PARSE
# include <ArduinoJson.h>
# include "../Helpers/StringConverter_Numerical.h"

String getJsonValue(DynamicJsonDocument *root,
                    String               key,
                    bool                 asJson); // Format objects and arrays as JSON with {} and [] wrappers

#endif // if FEATURE_JSON_PARSE

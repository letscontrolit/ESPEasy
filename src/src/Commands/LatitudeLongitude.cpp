#include "../Commands/Common.h"
#include "../Commands/LatitudeLongitude.h"
#include "../Globals/Settings.h"

String Command_Latitude(struct EventStruct *event,
                        const char         *Line) {
  return Command_GetORSetFloatMinMax(event, F("Latitude:"), Line, &Settings.Latitude, 1, -90.0001f, 90.0001f);
}

String Command_Longitude(struct EventStruct *event,
                         const char         *Line) {
  return Command_GetORSetFloatMinMax(event, F("Longitude:"), Line, &Settings.Longitude, 1, -180.0001f, 180.0001f);
}

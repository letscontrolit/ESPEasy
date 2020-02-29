#include <inttypes.h>
#include <time.h>

extern double    sysTime;             // Use high resolution double to get better sync between nodes when using NTP

uint32_t makeTime(const struct tm &tm);

String getDateTimeString(char dateDelimiter, char timeDelimiter, char dateTimeDelimiter);
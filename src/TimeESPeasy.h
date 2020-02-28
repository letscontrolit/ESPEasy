#include <inttypes.h>
#include <time.h>

#ifndef SECS_PER_MIN
#define SECS_PER_MIN  (60UL)
#endif

#ifndef SECS_PER_HOUR
#define SECS_PER_HOUR (3600UL)
#endif

#ifndef SECS_PER_DAY
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#endif

#ifndef DAYS_PER_WEEK
#define DAYS_PER_WEEK (7UL)
#endif

#ifndef SECS_PER_WEEK
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#endif

#ifndef SECS_PER_YEAR
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#endif

#ifndef SECS_YR_2000
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k
#endif

#ifndef LEAP_YEAR
#define LEAP_YEAR(Y) (((1970 + Y) > 0) && !((1970 + Y) % 4) && (((1970 + Y) % 100) || !((1970 + Y) % 400)))
#endif


extern double    sysTime;             // Use high resolution double to get better sync between nodes when using NTP

extern uint32_t makeTime(const struct tm &tm);


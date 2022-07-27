#ifndef HELPERS_ESPEASYSTATISTICS_H
#define HELPERS_ESPEASYSTATISTICS_H


#include "../../ESPEasy_common.h"


#if FEATURE_TIMING_STATS

#include "../DataStructs/TimingStats.h"

//void logStatistics(uint8_t loglevel, bool clearStats);

void stream_json_timing_stats(const TimingStats& stats, long timeSinceLastReset);

void jsonStatistics(bool clearStats);

#endif // if FEATURE_TIMING_STATS


#endif

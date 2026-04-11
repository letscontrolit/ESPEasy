#ifndef WEBSERVER_WEBSERVER_TIMINGSTATS_H
#define WEBSERVER_WEBSERVER_TIMINGSTATS_H

#include "../WebServer/common.h"

#if defined(WEBSERVER_TIMINGSTATS) && FEATURE_TIMING_STATS

#include "../DataStructs/TimingStats.h"

void handle_timingstats();

// ********************************************************************************
// HTML table formatted timing statistics
// ********************************************************************************
void format_using_threshhold(uint32_t value);

void stream_html_timing_stats(const TimingStats& stats, int32_t timeSinceLastReset);

int32_t stream_timing_statistics(bool clearStats);

#endif 


#endif
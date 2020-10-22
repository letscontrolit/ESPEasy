#ifndef WEBSERVER_WEBSERVER_TIMINGSTATS_H
#define WEBSERVER_WEBSERVER_TIMINGSTATS_H

#include "../WebServer/common.h"

#if defined(WEBSERVER_TIMINGSTATS) && defined(USES_TIMING_STATS)

#include "../DataStructs/TimingStats.h"

void handle_timingstats();

// ********************************************************************************
// HTML table formatted timing statistics
// ********************************************************************************
void format_using_threshhold(unsigned long value);

void stream_html_timing_stats(const TimingStats& stats, long timeSinceLastReset);

long stream_timing_statistics(bool clearStats);

#endif 


#endif
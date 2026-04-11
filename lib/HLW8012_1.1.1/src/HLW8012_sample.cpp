#include "HLW8012_sample.h"
#include <Arduino.h>

#include <GPIO_Direct_Access.h>

#ifndef CORE_POST_3_0_0
#ifdef ESP8266
#define IRAM_ATTR ICACHE_RAM_ATTR
#endif
#endif

bool HLW8012_finished_sample::getPulseFreq(float &pulseFreq)
{
    // Copy volatile values first before checking
    const int32_t dur = duration_usec;
    const uint32_t cnt = count;
    valid = (dur != 0 && cnt != 0);
    pulseFreq = valid
      ? static_cast<float>(cnt) / static_cast<float>(dur)
      : 0.0f;
    return valid;
}

void HLW8012_sample::reset()
{
    // Copy volatile values first before checking
    const uint32_t prev = first_pulse_usec;
    const uint32_t next = last_pulse_usec;
    const uint32_t cnt = count;
    if (getState() == HLW8012_sample::result_e::Enough)
    {
        finished.duration_usec = timeDiff(prev, next);
        finished.count = cnt;
        finished.updated = true;
    }
    else
    {
        finished.clear();
    }
    count = 0;
    last_pulse_usec = 0;
    first_pulse_usec = 0;
    start_usec = micros();
}

HLW8012_sample::result_e HLW8012_sample::add()
{
    const uint32_t now = micros();
    const int32_t duration_since_start_usec(timeDiff(start_usec, now));
    if (duration_since_start_usec < HLW8012_UNSTABLE_SAMPLE_DURATION_USEC) {
        return HLW8012_sample::result_e::NotEnough;
    }
    if (first_pulse_usec == 0) {
        first_pulse_usec = now;
        return HLW8012_sample::result_e::NotEnough;
    }
    ++count;
    last_pulse_usec = now;
    if (timeDiff(first_pulse_usec, now) > HLW8012_MINIMUM_SAMPLE_DURATION_USEC)
    {
        return HLW8012_sample::result_e::Enough;
    }
    if (duration_since_start_usec >= HLW8012_MAXIMUM_SAMPLE_DURATION_USEC)
    {
        return HLW8012_sample::result_e::Expired;
    }
    return HLW8012_sample::result_e::NotEnough;
}

HLW8012_sample::result_e HLW8012_sample::getState() const
{
    const int32_t duration_since_start_usec(timeDiff(start_usec, last_pulse_usec));
    if (duration_since_start_usec < HLW8012_UNSTABLE_SAMPLE_DURATION_USEC && count == 0)
    {
        return HLW8012_sample::result_e::NoisePeriod;
    }
    if (duration_since_start_usec >= HLW8012_MAXIMUM_SAMPLE_DURATION_USEC && count == 0)
    {
        return HLW8012_sample::result_e::Expired;
    }
    const int32_t duration_usec(timeDiff(first_pulse_usec, last_pulse_usec));
    if (duration_usec >= HLW8012_MINIMUM_SAMPLE_DURATION_USEC && count > 0)
    {
        return HLW8012_sample::result_e::Enough;
    }
    return HLW8012_sample::result_e::NotEnough;
}

bool HLW8012_sample::getPulseFreq(float &pulsefreq)
{
    return finished.getPulseFreq(pulsefreq);
}

bool HLW8012_sample::updated(bool& valid)
{
    const bool res = finished.updated == 1;
    valid = finished.valid;
    if (res) {
      // Do not try to clear the volatile value if it was not set
      finished.updated = 0;
    }
    return res;
}
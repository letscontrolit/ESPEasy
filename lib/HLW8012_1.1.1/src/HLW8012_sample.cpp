#include "HLW8012_sample.h"
#include <Arduino.h>

#include <GPIO_Direct_Access.h>

#ifndef CORE_POST_3_0_0
#ifdef ESP8266
#define IRAM_ATTR ICACHE_RAM_ATTR
#endif
#endif

bool HLW8012_finished_sample::getPulseFreq(float &pulseFreq) const
{
    // Copy volatile values first before checking
    const int32_t dur = duration_usec;
    const uint32_t cnt = count;
    if (dur == 0 || cnt == 0)
    {
        pulseFreq = 0.0f;
        return false;
    }
    pulseFreq = cnt;
    pulseFreq /= dur;
    return true;
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
    ++count;
    last_pulse_usec = now;

    const auto res = getState();
    if (res == HLW8012_sample::result_e::NoisePeriod ||
        first_pulse_usec == 0)
    {
        count = 0;
        first_pulse_usec =
            (res == HLW8012_sample::result_e::NoisePeriod)
                ? 0
                : now;
        return HLW8012_sample::result_e::NotEnough;
    }
    return res;
}

HLW8012_sample::result_e HLW8012_sample::getState() const
{
    if (last_pulse_usec == 0)
    {
        return HLW8012_sample::result_e::Cleared;
    }
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

HLW8012_sample::result_e HLW8012_sample::getPulseFreq(float &pulsefreq) const
{
    if (finished.getPulseFreq(pulsefreq))
    {
        return HLW8012_sample::result_e::Enough;
    }
    return HLW8012_sample::result_e::Expired;
}

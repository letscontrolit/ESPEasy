#pragma once

#ifndef CORE_POST_3_0_0
#ifdef ESP8266
#define IRAM_ATTR ICACHE_RAM_ATTR
#endif
#endif

#if ESP_IDF_VERSION_MAJOR >= 5
#include <atomic>
typedef std::atomic<uint32_t> HLW8012_VOLATILE_UINT32;
typedef std::atomic<unsigned char> HLW8012_VOLATILE_UCHAR;

#define HLW8012_IRAM
#else
typedef volatile uint32_t HLW8012_VOLATILE_UINT32;
typedef volatile unsigned char HLW8012_VOLATILE_UCHAR;

#define HLW8012_IRAM IRAM_ATTR
#endif

#define HLW8012_UNSTABLE_SAMPLE_DURATION_USEC 20000   // 20 msec
#define HLW8012_MINIMUM_SAMPLE_DURATION_USEC 1500000  // 1.5 sec
#define HLW8012_MAXIMUM_SAMPLE_DURATION_USEC 10000000 // 10 sec

struct HLW8012_finished_sample
{
    bool getPulseFreq(float &pulseFreq) const;
    void clear()
    {
        duration_usec = 0;
        count = 0;
    }

    int32_t duration_usec{};
    uint32_t count{};
};

struct HLW8012_sample
{

    enum class result_e
    {
        NotEnough = 0,
        Cleared,
        NoisePeriod,
        Enough,
        Expired
    };

    HLW8012_sample() = default;

    void reset() HLW8012_IRAM;

    // Add a new recorded pulse
    // Return true when enough has been captured
    result_e add() HLW8012_IRAM;

    // Check to make sure we have long enough duration and at least 1 sample
    result_e getState() const HLW8012_IRAM;

    result_e getPulseFreq(float &pulsefreq) const;

private:
    static inline int32_t timeDiff(const unsigned long prev, const unsigned long next)
    {
        return ((int32_t)(next - prev));
    }

    HLW8012_VOLATILE_UINT32 count{};
    HLW8012_VOLATILE_UINT32 last_pulse_usec{};
    HLW8012_VOLATILE_UINT32 first_pulse_usec{};

    HLW8012_VOLATILE_UINT32 start_usec{};

    HLW8012_finished_sample finished{};
};

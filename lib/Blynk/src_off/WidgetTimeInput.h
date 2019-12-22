/**
 * @file       WidgetTimeInput.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Aug 2016
 * @brief
 *
 */

#ifndef WidgetTimeInput_h
#define WidgetTimeInput_h

#include <Blynk/BlynkApi.h>
#include <utility/BlynkDateTime.h>
#include <utility/BlynkUtility.h>

class TimeInputParam
{
public:
    enum TimeMode {
        TIME_UNDEFINED,
        TIME_SUNSET,
        TIME_SUNRISE,
        TIME_SPECIFIED
    };

    TimeInputParam(const BlynkParam& param)
    {
        mStartMode = TIME_UNDEFINED;
        mStopMode = TIME_UNDEFINED;
        mTZ[0] = '\0';
        mWeekdays = -1; // All set
        mTZ_Offset = 0;

        BlynkParam::iterator it = param.begin();
        if (it >= param.end())
            return;

        if (0 == strcmp(it.asStr(), "sr")) {
            mStartMode = TIME_SUNRISE;
        } else if (0 == strcmp(it.asStr(), "ss")) {
            mStartMode = TIME_SUNSET;
        } else if (!it.isEmpty()) {
            mStart = BlynkTime(it.asLong());
            if (mStart.isValid()) {
                mStartMode = TIME_SPECIFIED;
            }
        }

        if (++it >= param.end())
            return;

        if (0 == strcmp(it.asStr(), "sr")) {
            mStopMode = TIME_SUNRISE;
        } else if (0 == strcmp(it.asStr(), "ss")) {
            mStopMode = TIME_SUNSET;
        } else if (!it.isEmpty()) {
            mStop = BlynkTime(it.asLong());
            if (mStop.isValid()) {
                mStopMode = TIME_SPECIFIED;
            }
        }

        if (++it >= param.end())
            return;

        strncpy(mTZ, it.asStr(), sizeof(mTZ));

        if (++it >= param.end())
            return;

        if (!it.isEmpty()) {
            mWeekdays = 0;
            const char* p = it.asStr();

            while (int c = *p++) {
                if (c >= '1' && c <= '7') {
                    BlynkBitSet(mWeekdays, c - '1');
                }
            }
        }

        if (++it >= param.end())
            return;

        mTZ_Offset = it.asLong();
    }

    BlynkTime& getStart(void) { return mStart; }
    BlynkTime& getStop(void)  { return mStop;  }

    TimeMode getStartMode(void) const { return mStartMode; }
    TimeMode getStopMode(void)  const { return mStopMode; }

    bool hasStartTime(void)   const { return mStartMode == TIME_SPECIFIED; }
    bool isStartSunrise(void) const { return mStartMode == TIME_SUNRISE; }
    bool isStartSunset(void)  const { return mStartMode == TIME_SUNSET; }
    int getStartHour(void)    const { return mStart.hour(); }
    int getStartMinute(void)  const { return mStart.minute(); }
    int getStartSecond(void)  const { return mStart.second(); }

    bool hasStopTime(void)    const { return mStopMode == TIME_SPECIFIED; }
    bool isStopSunrise(void)  const { return mStopMode == TIME_SUNRISE; }
    bool isStopSunset(void)   const { return mStopMode == TIME_SUNSET; }
    int getStopHour(void)     const { return mStop.hour(); }
    int getStopMinute(void)   const { return mStop.minute(); }
    int getStopSecond(void)   const { return mStop.second(); }

    const char* getTZ(void)   const { return mTZ; }
    int32_t getTZ_Offset(void) const { return mTZ_Offset; }

    bool isWeekdaySelected(int day) const {
        return BlynkBitRead(mWeekdays, (day - 1) % 7);
    }

private:
    BlynkTime mStart;
    BlynkTime mStop;
    char      mTZ[32];
    int32_t   mTZ_Offset;

    TimeMode  mStopMode;
    TimeMode  mStartMode;

    uint8_t   mWeekdays;
};

#endif

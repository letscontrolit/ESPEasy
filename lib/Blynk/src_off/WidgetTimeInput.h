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

        BlynkParam::iterator it = param.begin(void);
        if (it >= param.end(void))
            return;

        if (0 == strcmp(it.asStr(void), "sr")) {
            mStartMode = TIME_SUNRISE;
        } else if (0 == strcmp(it.asStr(void), "ss")) {
            mStartMode = TIME_SUNSET;
        } else if (!it.isEmpty(void)) {
            mStart = BlynkTime(it.asLong(void));
            if (mStart.isValid(void)) {
                mStartMode = TIME_SPECIFIED;
            }
        }

        if (++it >= param.end(void))
            return;

        if (0 == strcmp(it.asStr(void), "sr")) {
            mStopMode = TIME_SUNRISE;
        } else if (0 == strcmp(it.asStr(void), "ss")) {
            mStopMode = TIME_SUNSET;
        } else if (!it.isEmpty(void)) {
            mStop = BlynkTime(it.asLong(void));
            if (mStop.isValid(void)) {
                mStopMode = TIME_SPECIFIED;
            }
        }

        if (++it >= param.end(void))
            return;

        strncpy(mTZ, it.asStr(void), sizeof(mTZ));

        if (++it >= param.end(void))
            return;

        if (!it.isEmpty(void)) {
            mWeekdays = 0;
            const char* p = it.asStr(void);

            while (int c = *p++) {
                if (c >= '1' && c <= '7') {
                    BlynkBitSet(mWeekdays, c - '1');
                }
            }
        }

        if (++it >= param.end(void))
            return;

        mTZ_Offset = it.asLong(void);
    }

    BlynkTime& getStart(void) { return mStart; }
    BlynkTime& getStop(void)  { return mStop;  }

    TimeMode getStartMode(void) const { return mStartMode; }
    TimeMode getStopMode(void)  const { return mStopMode; }

    bool hasStartTime(void)   const { return mStartMode == TIME_SPECIFIED; }
    bool isStartSunrise(void) const { return mStartMode == TIME_SUNRISE; }
    bool isStartSunset(void)  const { return mStartMode == TIME_SUNSET; }
    int getStartHour(void)    const { return mStart.hour(void); }
    int getStartMinute(void)  const { return mStart.minute(void); }
    int getStartSecond(void)  const { return mStart.second(void); }

    bool hasStopTime(void)    const { return mStopMode == TIME_SPECIFIED; }
    bool isStopSunrise(void)  const { return mStopMode == TIME_SUNRISE; }
    bool isStopSunset(void)   const { return mStopMode == TIME_SUNSET; }
    int getStopHour(void)     const { return mStop.hour(void); }
    int getStopMinute(void)   const { return mStop.minute(void); }
    int getStopSecond(void)   const { return mStop.second(void); }

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

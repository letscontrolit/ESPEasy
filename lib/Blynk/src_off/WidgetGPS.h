/**
 * @file       WidgetGPS.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Oct 2016
 * @brief
 *
 */

#ifndef WidgetGPS_h
#define WidgetGPS_h

#ifndef BLYNK_NO_FLOAT

#include <Blynk/BlynkWidgetBase.h>

class GpsParam
{
public:

    GpsParam(const BlynkParam& param)
        : mLat (0)
        , mLon (0)
        , mAlt (0)
        , mSpeed (0)
    {
        BlynkParam::iterator it = param.begin(void);
        if (it >= param.end(void))
            return;

        mLat = it.asDouble(void);

        if (++it >= param.end(void))
            return;

        mLon = it.asDouble(void);

        if (++it >= param.end(void))
            return;

        mAlt = it.asDouble(void);

        if (++it >= param.end(void))
            return;

        mSpeed = it.asDouble(void);
    }


    double getLat(void) const { return mLat; }
    double getLon(void) const { return mLon; }
    double getAltitude(void) const { return mAlt; }
    double getSpeed(void) const { return mSpeed; }

private:
    double mLat;
    double mLon;
    double mAlt;
    double mSpeed;
};

#endif

#endif

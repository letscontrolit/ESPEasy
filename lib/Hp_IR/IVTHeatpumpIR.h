/*
    IVT heatpump control for IVT AY-XP12FR-N, remote control CRMC-A673JBEZ (almost the same as Sharp)

    Also see: https://github.com/skarlsso/IRRemoteIVT/blob/master/IRRemoteIVT
*/
#ifndef IVTHeatpumpIR_h
#define IVTHeatpumpIR_h

#include <SharpHeatpumpIR.h>


class IVTHeatpumpIR : public SharpHeatpumpIR
{
  public:
    IVTHeatpumpIR();
};

#endif

#ifndef HeatpumpIRFactory_h
#define HeatpumpIRFactory_h

#include <AUXHeatpumpIR.h>
#include <BalluHeatpumpIR.h>
#include <CarrierHeatpumpIR.h>
#include <DaikinHeatpumpIR.h>
#include <DaikinHeatpumpARC417IR.h>
#include <DaikinHeatpumpARC480A14IR.h>
#include <FuegoHeatpumpIR.h>
#include <FujitsuHeatpumpIR.h>
#include <GreeHeatpumpIR.h>
#include <HisenseHeatpumpIR.h>
#include <HitachiHeatpumpIR.h>
#include <HyundaiHeatpumpIR.h>
#include <IVTHeatpumpIR.h>
#include <MideaHeatpumpIR.h>
#include <MitsubishiHeatpumpIR.h>
#include <MitsubishiHeavyFDTCHeatpumpIR.h>
#include <MitsubishiHeavyHeatpumpIR.h>
#include <MitsubishiMSCHeatpumpIR.h>
#include <MitsubishiSEZKDXXHeatpumpIR.h>
#include <PanasonicCKPHeatpumpIR.h>
#include <PanasonicHeatpumpIR.h>
#include <SamsungHeatpumpIR.h>
#include <SharpHeatpumpIR.h>
#include <ToshibaDaiseikaiHeatpumpIR.h>
#include <ToshibaHeatpumpIR.h>


class HeatpumpIRFactory
{
  protected:
    HeatpumpIRFactory();

  public:
    static HeatpumpIR* create(const char *modelName);
};

#endif
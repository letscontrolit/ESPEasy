#ifndef HeatpumpIRFactory_h
#define HeatpumpIRFactory_h

#include <AUXHeatpumpIR.h>
#include <AIRWAYHeatpumpIR.h>
#include <BalluHeatpumpIR.h>
#include <BGHHeatpumpIR.h>
#include <CarrierHeatpumpIR.h>
#include <DaikinHeatpumpIR.h>
#include <DaikinHeatpumpARC417IR.h>
#include <DaikinHeatpumpARC480A14IR.h>
#include <ElectroluxHeatpumpIR.h>
#include <FuegoHeatpumpIR.h>
#include <FujitsuHeatpumpIR.h>
#include <GreeHeatpumpIR.h>
#include <HisenseHeatpumpIR.h>
#include <HitachiHeatpumpIR.h>
#include <HyundaiHeatpumpIR.h>
#include <IVTHeatpumpIR.h>
#include <NibeHeatpumpIR.h>
#include <MideaHeatpumpIR.h>
#include <MitsubishiHeatpumpIR.h>
#include <MitsubishiHeavyFDTCHeatpumpIR.h>
#include <MitsubishiHeavyHeatpumpIR.h>
#include <MitsubishiMSCHeatpumpIR.h>
#include <MitsubishiSEZKDXXHeatpumpIR.h>
#include <PanasonicAltDKEHeatpumpIR.h>
#include <PanasonicCKPHeatpumpIR.h>
#include <PanasonicHeatpumpIR.h>
#include <PhilcoPHS32HeatpumpIR.h>
#include <R51MHeatpumpIR.h>
#include <SamsungHeatpumpIR.h>
#include <SharpHeatpumpIR.h>
#include <ToshibaDaiseikaiHeatpumpIR.h>
#include <ToshibaHeatpumpIR.h>
#include <VaillantHeatpumpIR.h>
#include <ZHJG01HeatpumpIR.h>
#include <ZHLT01HeatpumpIR.h>
#include <KY26HeatpumpIR.h>

class HeatpumpIRFactory
{
  protected:
    HeatpumpIRFactory();

  public:
    static HeatpumpIR* create(const char *modelName);
};

#endif

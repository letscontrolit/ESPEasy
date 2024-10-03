#include <IVTHeatpumpIR.h>


IVTHeatpumpIR::IVTHeatpumpIR() : SharpHeatpumpIR()
{
  static const char model[] PROGMEM = "ivt";
  static const char info[]  PROGMEM = "{\"mdl\":\"ivt\",\"dn\":\"IVT AY-XP12FR-N\",\"mT\":18,\"xT\":32,\"fs\":3,\"maint\":[10]}}";

  _model = model;
  _info = info;

  _sharpModel = MODEL_IVT;
}

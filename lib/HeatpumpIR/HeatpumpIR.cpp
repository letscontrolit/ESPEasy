#include <HeatpumpIR.h>

HeatpumpIR::HeatpumpIR()
{
}

HeatpumpIR::~HeatpumpIR()
{
}

// This is a virtual function, i.e. never called
void HeatpumpIR::send(IRSender&, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t)
{
}

// Heatpump model and info getters
const char PROGMEM* HeatpumpIR::model()
{
  return _model;
}

const char PROGMEM* HeatpumpIR::info()
{
  return _info;
}
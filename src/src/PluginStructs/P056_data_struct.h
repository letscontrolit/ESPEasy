#pragma once

#include "../../_Plugin_Helper.h"
#ifdef USES_P056

# include <jkSDS011.h>

struct P056_data_struct : public PluginTaskData_base {
  P056_data_struct(EventStruct *event);
  ~P056_data_struct();

  bool isInitialized() {
    return nullptr != SDS;
  }

  void  Process();
  bool  available();
  float GetPM2_5();
  float GetPM10_();
  bool  ReadAverage(float pm25,
                    float pm10);
  void  SetWorkingPeriod(int minutes);

private:

  CjkSDS011 *SDS = nullptr;
};
#endif // ifdef USES_P056

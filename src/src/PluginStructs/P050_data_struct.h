#ifndef PLUGINSTRUCTS_P050_DATA_STRUCT_H
#define PLUGINSTRUCTS_P050_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P050

# include <Adafruit_TCS34725.h>

typedef struct {
  float matrix[3][3];
} tcsTransformationSettings_t;

struct P050_data_struct : public PluginTaskData_base {

public:

  P050_data_struct(uint16_t integrationSetting, uint16_t gainSetting);

  bool loadSettings(taskIndex_t taskIndex);
  bool saveSettings(taskIndex_t taskIndex);
  void resetTransformation();
  void applyTransformation(uint16_t r, uint16_t g, uint16_t b, float *rc, float *gc, float *bc);
  void applyTransformation(float nr, float ng, float nb, float *rc, float *gc, float *bc);

  Adafruit_TCS34725           tcs;

  tcsTransformationSettings_t TransformationSettings;

private:
  tcs34725IntegrationTime_t   _integrationTime;
  tcs34725Gain_t              _gain;
};

#endif // ifdef USES_P050
#endif // ifndef PLUGINSTRUCTS_P050_DATA_STRUCT_H

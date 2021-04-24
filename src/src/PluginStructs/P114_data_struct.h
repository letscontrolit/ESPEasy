#ifndef PLUGINSTRUCTS_P114_DATA_STRUCT_H
#define PLUGINSTRUCTS_P114_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P114

////////////////////////////
// VEML6075 Command Codes //
////////////////////////////
# define VEML6075_UV_CONF       0x00 // command codes
# define VEML6075_UVA_DATA      0x07 // 2 bytes
# define VEML6075_UVDUMMY_DATA  0x08
# define VEML6075_UVB_DATA      0x09
# define VEML6075_UVCOMP1_DATA  0x0A
# define VEML6075_UVCOMP2_DATA  0x0B
# define VEML6075_UV_ID         0x0C // should retrn 0x26

// Calculation factors
# define ACoef 3.33f
# define BCoef 2.5f
# define CCoef 3.66f
# define DCoef 2.75f
# define UVAresponsivity  0.0011f
# define UVBresponsivity  0.00125f


enum IT {
  IT_50  = 0, //   50 ms
  IT_100 = 1, //  100 ms
  IT_200 = 2, //  200 ms
  IT_400 = 3, //  400 ms
  IT_800 = 4  //  800 ms
};

struct P114_data_struct : public PluginTaskData_base {
public:

  P114_data_struct(uint8_t i2c_addr,
                   uint8_t integration_time,
                   bool    highDensity);

  bool read_sensor(float& _UVA,
                   float& _UVB,
                   float& _UVIndex);

private:

  bool init_sensor();

  uint8_t i2cAddress;

  // Specify VEML6075 Integration time
  uint8_t IT;
  bool    HD;

  uint16_t UVData[5] = { 0, 0, 0, 0, 0 }; // UVA, Dummy, UVB, UVComp1, UVComp2
  float    UVAComp;
  float    UVBComp;

  bool initialised;
};
#endif // ifdef USES_P114
#endif // ifndef PLUGINSTRUCTS_P114_DATA_STRUCT_H

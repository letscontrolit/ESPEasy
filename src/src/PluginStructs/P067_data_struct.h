#ifndef PLUGINSTRUCTS_P067_DATA_STRUCT_H
#define PLUGINSTRUCTS_P067_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P067

# define P067_CONFIG_FLAGS          PCONFIG(0)
# define P067_CONFIG_CHANNEL_A_OS   0
# define P067_GET_CHANNEL_A_OS      bitRead(P067_CONFIG_FLAGS, P067_CONFIG_CHANNEL_A_OS)
# define P067_SET_CHANNEL_A_OS(X) bitWrite(P067_CONFIG_FLAGS, P067_CONFIG_CHANNEL_A_OS, X)
# define P067_CONFIG_CHANNEL_B_OS   1
# define P067_GET_CHANNEL_B_OS      bitRead(P067_CONFIG_FLAGS, P067_CONFIG_CHANNEL_B_OS)
# define P067_SET_CHANNEL_B_OS(X) bitWrite(P067_CONFIG_FLAGS, P067_CONFIG_CHANNEL_B_OS, X)
# define P067_CONFIG_CHANNEL_A_MODE 2
# define P067_GET_CHANNEL_A_MODE    get2BitFromUL(P067_CONFIG_FLAGS, P067_CONFIG_CHANNEL_A_MODE)
# define P067_GET_CHANNEL_A_MODE_e  static_cast<P067_ChannelA_State_e>(P067_GET_CHANNEL_A_MODE)
# define P067_CONFIG_CHANNEL_B_MODE 4
# define P067_GET_CHANNEL_B_MODE    bitRead(P067_CONFIG_FLAGS, P067_CONFIG_CHANNEL_B_MODE)
# define P067_GET_CHANNEL_B_MODE_e  static_cast<P067_ChannelB_State_e>(P067_GET_CHANNEL_B_MODE)
# define P067_SET_CHANNEL_B_MODE(X) bitWrite(P067_CONFIG_FLAGS, P067_CONFIG_CHANNEL_B_MODE, X)
# define P067_CONFIG_CHANNEL_A_CALIB  5
# define P067_GET_CHANNEL_A_CALIB   bitRead(P067_CONFIG_FLAGS, P067_CONFIG_CHANNEL_A_CALIB)
# define P067_SET_CHANNEL_A_CALIB(X) bitWrite(P067_CONFIG_FLAGS, P067_CONFIG_CHANNEL_A_CALIB, X)
# define P067_CONFIG_CHANNEL_B_CALIB  6
# define P067_GET_CHANNEL_B_CALIB   bitRead(P067_CONFIG_FLAGS, P067_CONFIG_CHANNEL_B_CALIB)
# define P067_SET_CHANNEL_B_CALIB(X) bitWrite(P067_CONFIG_FLAGS, P067_CONFIG_CHANNEL_B_CALIB, X)

# define P067_CONFIG_CHANNEL_A_ADC1   PCONFIG_LONG(0)
# define P067_CONFIG_CHANNEL_A_ADC2   PCONFIG_LONG(1)
# define P067_CONFIG_CHANNEL_A_OUT1   PCONFIG_FLOAT(0)
# define P067_CONFIG_CHANNEL_A_OUT2   PCONFIG_FLOAT(1)
# define P067_CONFIG_CHANNEL_B_ADC1   PCONFIG_LONG(2)
# define P067_CONFIG_CHANNEL_B_ADC2   PCONFIG_LONG(3)
# define P067_CONFIG_CHANNEL_B_OUT1   PCONFIG_FLOAT(2)
# define P067_CONFIG_CHANNEL_B_OUT2   PCONFIG_FLOAT(3)

# define P067_OFFSET_CHANNEL_A_1      PCONFIG(1)
# define P067_OFFSET_CHANNEL_A_2      PCONFIG(2)
# define P067_OFFSET_CHANNEL_B_1      PCONFIG(3)
# define P067_OFFSET_CHANNEL_B_2      PCONFIG(4)

enum class P067_ChannelA_State_e : uint8_t {
  modeAoff = 0u,
  modeA64  = 1u,
  modeA128 = 2u
};

enum class P067_ChannelB_State_e : uint8_t {
  modeBoff = 0u,
  modeB32  = 1u
};

enum class P067_Channel_e : uint8_t {
  chanA128 = 0u,
  chanB32  = 1u,
  chanA64  = 2u
};

void P067_float2int(float    valFloat,
                    int16_t *valInt0,
                    int16_t *valInt1);
void P067_int2float(int16_t valInt0,
                    int16_t valInt1,
                    float  *valFloat);

struct P067_data_struct : public PluginTaskData_base {
public:

  P067_data_struct(struct EventStruct *event,
                   int8_t              pinSCL,
                   int8_t              pinDOUT);

  P067_data_struct() = delete;
  virtual ~P067_data_struct();

  bool init(struct EventStruct *event);

  bool plugin_read(struct EventStruct *event);
  bool plugin_fifty_per_second(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);

private:

  bool    isInitialized();
  bool    isDataReady();
  int32_t readHX711();

  int8_t _pinSCL  = -1;
  int8_t _pinDOUT = -1;

  bool           _channelToggle = false;
  P067_Channel_e _nextChannel   = P067_Channel_e::chanA128;
  P067_Channel_e _channelRead   = P067_Channel_e::chanA128;

  P067_ChannelA_State_e _modeChanA = P067_ChannelA_State_e::modeAoff;
  P067_ChannelB_State_e _modeChanB = P067_ChannelB_State_e::modeBoff;

  int32_t OversamplingValueChanA = 0;
  int16_t OversamplingCountChanA = 0;
  int32_t OversamplingValueChanB = 0;
  int16_t OversamplingCountChanB = 0;

  float _offsetChanA = 0.0f;
  float _offsetChanB = 0.0f;

  bool firstRead = true;
};

#endif // ifdef USES_P067
#endif // ifndef PLUGINSTRUCTS_P067_DATA_STRUCT_H

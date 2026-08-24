#ifndef PLUGINSTRUCTS_P188_DATA_STRUCT_H
#define PLUGINSTRUCTS_P188_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P188

#ifndef P188_FEATURE_RESISTOR_MEASUREMENT
  #define P188_FEATURE_RESISTOR_MEASUREMENT 1 // default: enable resistor measurement mode, can be dsiabled in Custom.h
#endif // #ifndef P188_FEATURE_RESISTOR_MEASUREMENT

/**********************************************/
/********** TLA2528 specific defines **********/
/**********************************************/

#define TLA2528_CHANNEL_NUM           8

#define TLA2528_OPC_SINGLE_READ       0x10
#define TLA2528_OPC_SINGLE_WRITE      0x08
#define TLA2528_OPC_SET_BIT           0x18
#define TLA2528_OPC_CLR_BIT           0x20
#define TLA2528_OPC_MULT_READ         0x30
#define TLA2528_OPC_MULT_WRITE        0x28


#define TLA2528_REG_SYSTEM_STATUS     0x00

#define TLA2528_REG_GENERAL_CFG       0x01
#define TLA2528_BIT_GC_CNVST          0x08
#define TLA2528_BIT_GC_CH_RST         0x04
#define TLA2528_BIT_GC_CAL            0x02
#define TLA2528_BIT_GC_RST            0x01

#define TLA2528_REG_DATA_CFG          0x02
#define TLA2528_BIT_DC_FIX_PAT        0x80
#define TLA2528_BIT_DC_APPEND_STATUS  0x10

#define TLA2528_REG_OSR_CFG           0x03
#define TLA2528_VAL_OSR_0             0x00
#define TLA2528_VAL_OSR_2             0x01
#define TLA2528_VAL_OSR_4             0x02
#define TLA2528_VAL_OSR_8             0x03
#define TLA2528_VAL_OSR_16            0x04
#define TLA2528_VAL_OSR_32            0x05
#define TLA2528_VAL_OSR_64            0x06
#define TLA2528_VAL_OSR_128           0x07

#define TLA2528_REG_OPMODE_CFG        0x04
#define TLA2528_VAL_OC_HS_1us         0x00
#define TLA2528_VAL_OC_HS_1_5us       0x01
#define TLA2528_VAL_OC_HS_2us         0x02
#define TLA2528_VAL_OC_HS_3us         0x03
#define TLA2528_VAL_OC_HS_4us         0x04
#define TLA2528_VAL_OC_HS_6us         0x05
#define TLA2528_VAL_OC_HS_8us         0x06
#define TLA2528_VAL_OC_HS_12us        0x07
#define TLA2528_VAL_OC_HS_16us        0x08
#define TLA2528_VAL_OC_HS_24us        0x09
#define TLA2528_VAL_OC_HS_32us        0x0a
#define TLA2528_VAL_OC_HS_48us        0x0b
#define TLA2528_VAL_OC_HS_64us        0x0c
#define TLA2528_VAL_OC_HS_96us        0x0d
#define TLA2528_VAL_OC_HS_128us       0x0e
#define TLA2528_VAL_OC_HS_192us       0x0f
#define TLA2528_VAL_OC_LP_32us        0x10
#define TLA2528_VAL_OC_LP_48us        0x11
#define TLA2528_VAL_OC_LP_64us        0x12
#define TLA2528_VAL_OC_LP_96us        0x13
#define TLA2528_VAL_OC_LP_128us       0x14
#define TLA2528_VAL_OC_LP_192us       0x15
#define TLA2528_VAL_OC_LP_256us       0x16
#define TLA2528_VAL_OC_LP_384us       0x17
#define TLA2528_VAL_OC_LP_512us       0x18
#define TLA2528_VAL_OC_LP_768us       0x19
#define TLA2528_VAL_OC_LP_1024us      0x1a
#define TLA2528_VAL_OC_LP_1536us      0x1b
#define TLA2528_VAL_OC_LP_2048us      0x1c
#define TLA2528_VAL_OC_LP_3072us      0x1d
#define TLA2528_VAL_OC_LP_6096us      0x1e
#define TLA2528_VAL_OC_LP_6144us      0x1f

#define TLA2528_REG_SEQUENCE_CFG      0x10
#define TLA2528_BIT_SC_SEQ_START      0x10
#define TLA2528_BIT_SC_SEQ_MODE       0x01

#define TLA2528_REG_CHANNEL_SEL       0x11
#define TLA2528_MSK_CS_MANUAL_CHID    0x07

#define TLA2528_REG_AUTO_SEQ_CH_SEL   0x12

#ifdef P188_FEATURE_RESISTOR_MEASUREMENT
  #define P188_OUTPUT_OPTION_CNT 16
#else
  #define P188_OUTPUT_OPTION_CNT 12
#endif

#define P188_OUTPUT_MAPPING_OFFSET 0
#define P188_OUTPUT_MAPPING_DIFFERENCE_OFFSET 7
#define P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET 11

#define P188_OUTPUT_MAPPING_0 PCONFIG(0)
#define P188_OUTPUT_MAPPING_1 PCONFIG(1)
#define P188_OUTPUT_MAPPING_2 PCONFIG(2)
#define P188_OUTPUT_MAPPING_3 PCONFIG(3)

#define P188_NR_OUTPUT_VALUES 4

#define P188_I2C_ADDR         PCONFIG(4)
#define P188_OUTPUT_TYPE_INDEX 5
#define P188_OUTPUT_TYPE      PCONFIG(5)
#define P188_CONFIG_BITS      PCONFIG(6)

struct P188_CONFIG_BITS_t {
  union {
    struct {
      uint16_t en_cal     : 4; // one bit for each channel
      uint16_t raw_val    : 4; // one bit for each channel
      uint16_t unused     : 8;
    };
    uint16_t _regValue{};
  };

  P188_CONFIG_BITS_t(int16_t value);

  int16_t pconfigvalue() const { return _regValue; }
};

const __FlashStringHelper* Plugin_188_output_mapping_name(uint8_t value_nr, bool displayString);

struct P188_config_struct {
    uint8_t  i2cAddress{};
    float ADC_Vref{};
    float R_Clip{};

    float CalIn[4][2]{}; // 4 calibration points, each with 2 ADC values
    float CalOut[4][2]{}; // 4 calibration points, each with 2 output values
#ifdef P188_FEATURE_RESISTOR_MEASUREMENT
    uint32_t Rref[VARS_PER_TASK]{};
    uint32_t Rpar[VARS_PER_TASK]{};
#endif // P188_FEATURE_RESISTOR_MEASUREMENT
};

struct P188_data_struct : public PluginTaskData_base {
public:

  P188_data_struct(struct EventStruct *event);
  P188_data_struct()          = delete;
  virtual ~P188_data_struct() = default;

  static uint8_t TLA2528_read_single_reg(uint8_t i2caddr, uint8_t reg);
  static bool TLA2528_write_single_reg(uint8_t i2caddr, uint8_t reg, uint8_t data);

  bool init(struct EventStruct *event);
  bool read_raw(struct EventStruct *event, float& value, uint8_t ch_num) const;
  bool sample(void);

  P188_config_struct P188_config;

  uint16_t raw_samples[TLA2528_CHANNEL_NUM]; 
  // bits 16..4 ADC raw value
  // bits  3..0 channel number 
  
  bool isInitialized() const {
    return initialized;
  }

private:
  uint8_t _sample_cnt;
  bool  initialized = false;
};

#endif // ifdef USES_P188
#endif // ifndef PLUGINSTRUCTS_P188_DATA_STRUCT_H

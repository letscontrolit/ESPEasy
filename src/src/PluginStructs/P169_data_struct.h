#ifndef PLUGINSTRUCTS_P169_DATA_STRUCT_H
#define PLUGINSTRUCTS_P169_DATA_STRUCT_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// Using AS3935MI library written by Gregor Christandl
// https://bitbucket.org/christandlg/as3935mi/issues
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../_Plugin_Helper.h"
#ifdef USES_P169

# include "../ESPEasyCore/ESPEasyGPIO.h"

# include <AS3935I2C.h>

# define DEFAULT_SENSE_INCREASE_INTERVAL   15000 // 15 s sensitivity increase interval

# define P169_IRQ_PIN                   CONFIG_PIN1
# define P169_IRQ_PIN_LABEL             "taskdevicepin1"

# define P169_I2C_ADDRESS               PCONFIG(0)
# define P169_I2C_ADDRESS_LABEL         PCONFIG_LABEL(0)

# define P169_LIGHTNING_THRESHOLD       PCONFIG(1)
# define P169_LIGHTNING_THRESHOLD_LABEL PCONFIG_LABEL(1)

# define P169_GET_INDOOR                bitRead(PCONFIG(2), 0)
# define P169_SET_INDOOR(X) bitWrite(PCONFIG(2), 0, X)
# define P169_INDOOR_LABEL              "mode"

# define P169_GET_MASK_DISTURBANCE      bitRead(PCONFIG(2), 1)
# define P169_SET_MASK_DISTURBANCE(X) bitWrite(PCONFIG(2), 1, X)
# define P169_MASK_DISTURBANCE_LABEL    "maskdist"

# define P169_GET_SEND_ONLY_ON_LIGHTNING    bitRead(PCONFIG(2), 2)
# define P169_SET_SEND_ONLY_ON_LIGHTNING(X) bitWrite(PCONFIG(2), 2, X)
# define P169_SEND_ONLY_ON_LIGHTNING_LABEL  "sendonlightning"

# define P169_GET_TOLERANT_CALIBRATION_RANGE    bitRead(PCONFIG(2), 3)
# define P169_SET_TOLERANT_CALIBRATION_RANGE(X) bitWrite(PCONFIG(2), 3, X)
# define P169_TOLERANT_CALIBRATION_RANGE_LABEL  "tolerantcalib"

# define P169_GET_SLOW_LCO_CALIBRATION    bitRead(PCONFIG(2), 4)
# define P169_SET_SLOW_LCO_CALIBRATION(X) bitWrite(PCONFIG(2), 4, X)
# define P169_SLOW_LCO_CALIBRATION_LABEL  "slowcalib"

// The device addresses for the AS3935 in read or write mode are defined by:
// 0-0-0-0-0-a1-a0-0: write mode device address (DW)
// 0-0-0-0-0-a1-a0-1: read mode device address (DR)
// Where a0 and a1 are defined by the pins 5 (ADD0) and 6 (ADD1).
// The combination a0 = 0 (low) and a1 =0 (low) is explicitly not allowed for I²C communication.
# define P169_I2C_ADDRESS_DFLT      0x03

// Franklin AS3935 has 10k pull-up on the SDA line.
// When no other I2C devices used:      Max 400 kHz I2C clock, add 10k as pull-up on SCL.
// Along with upto 3 other I2C devices: Max 100 kHz I2C clock, add 10k on SDA and add 4k7 pull-up on SCL.


struct P169_data_struct : public PluginTaskData_base
{
public:

  P169_data_struct(struct EventStruct *event);
  virtual ~P169_data_struct();

  bool     loop(struct EventStruct *event);

  bool     plugin_init(struct EventStruct *event);
  bool     plugin_write(struct EventStruct *event,
                        String            & string);

  bool     plugin_get_config_value(struct EventStruct *event,
                                   String            & string);

  void     html_show_sensor_info(struct EventStruct *event);

  // Read distance in km
  int      getDistance();

  // Get lightning strike energy in some raw value (no unit)
  uint32_t getEnergy();

  uint32_t getLightningCount() const {
    return _lightningCount;
  }

  uint32_t getAndClearLightningCount();


  // Clear lightning distance estimation statistics
  void clearStatistics();

private:

  static float computeDeviationPct(uint32_t LCO_freq);

  bool         calibrate(struct EventStruct *event);

  void         adjustForNoise(struct EventStruct *event);

  void         adjustForDisturbances(struct EventStruct *event);

  void         tryIncreasedSensitivity(struct EventStruct *event);

# if FEATURE_CHART_JS
  void         addCalibrationChart(struct EventStruct *event);
# endif // if FEATURE_CHART_JS


  AS3935I2C _sensor;
  uint8_t   _irqPin;

  uint32_t _sense_adj_last = 0;

  uint32_t _sense_increase_interval = DEFAULT_SENSE_INCREASE_INTERVAL;

  uint32_t _lightningCount = 0;
};

#endif // ifdef USES_P169
#endif // ifndef PLUGINSTRUCTS_P169_DATA_STRUCT_H

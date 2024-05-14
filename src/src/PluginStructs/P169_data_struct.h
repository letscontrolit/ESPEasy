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

# include <atomic>

# include <AS3935I2C.h>

# define DEFAULT_SENSE_INCREASE_INTERVAL   15000 // 15 s sensitivity increase interval

# define P169_IRQ_PIN                   CONFIG_PIN1
# define P169_IRQ_PIN_LABEL             "taskdevicepin1"

# define P169_I2C_ADDRESS               PCONFIG(0)
# define P169_I2C_ADDRESS_LABEL         PCONFIG_LABEL(0)
# define P169_NOISE                     PCONFIG(1)
# define P169_NOISE_LABEL               PCONFIG_LABEL(1)
# define P169_WATCHDOG                  PCONFIG(2)
# define P169_WATCHDOG_LABEL            PCONFIG_LABEL(2)
# define P169_SPIKE_REJECTION           PCONFIG(3)
# define P169_SPIKE_REJECTION_LABEL     PCONFIG_LABEL(3)
# define P169_LIGHTNING_THRESHOLD       PCONFIG(4)
# define P169_LIGHTNING_THRESHOLD_LABEL PCONFIG_LABEL(4)

# define P169_GET_INDOOR                bitRead(PCONFIG(5), 0)
# define P169_SET_INDOOR(X)             bitWrite(PCONFIG(5), 0, X)
# define P169_INDOOR_LABEL              "mode"
# define P169_GET_MASK_DISTURBANCE      bitRead(PCONFIG(5), 1)
# define P169_SET_MASK_DISTURBANCE(X)   bitWrite(PCONFIG(5), 1, X)
# define P169_MASK_DISTURBANCE_LABEL    "maskdist"

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

  enum class P169_IRQ_frequency_source {
    LCO, // 500 kHz resonance freq
    SRCO, // 1.1 MHz signal
    TRCO // 32768 Hz signal
  };

  P169_data_struct(struct EventStruct *event);
  virtual ~P169_data_struct();

  bool loop();

  bool plugin_init(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);

  bool validateCurrentResonanceFrequency(int32_t& frequency);

  int32_t measureResonanceFrequency(P169_IRQ_frequency_source source);

  // Read distance in km
  int getDistance();

  // Get lightning strike energy in some raw value (no unit)
  uint32_t getEnergy();


private:

  enum class P169_InterruptMode {
    detached,
    normal,
    calibration
  };

  static void IRAM_ATTR P169_interrupt_ISR(P169_data_struct *self);
  static void IRAM_ATTR P169_calibrate_ISR(P169_data_struct *self);

  uint32_t              computeCalibratedFrequency(int32_t divider);


  // Internal Tuning Capacitors (from 0 to 120pF in steps of 8pF)
  uint32_t              measureResonanceFrequency(P169_IRQ_frequency_source source, uint8_t tuningCapacitance);

  bool                  calibrateResonanceFrequency(int32_t& frequency);


  void                  set_P169_interruptMode(P169_InterruptMode mode);

  void                  adjustForNoise();

  void                  adjustForDisturbances();

  void                  tryIncreasedSensitivity();


  AS3935I2C _sensor;
  uint8_t   _irqPin;

  #if ESP_IDF_VERSION_MAJOR >= 5
  #define P169_VOLATILE_TYPE std::atomic<uint32_t>
  #else
  #define P169_VOLATILE_TYPE volatile uint32_t
  #endif


  P169_VOLATILE_TYPE _interrupt_timestamp = 0;
  P169_VOLATILE_TYPE _interrupt_count     = 0;

  // Store the time micros as 32-bit int so it can be stored and comprared as an atomic operation.
  // Expected duration will be much less than 2^32 usec, thus overflow isn't an issue here
  P169_VOLATILE_TYPE _calibration_start_micros = 0;
  P169_VOLATILE_TYPE _calibration_end_micros   = 0;
  uint32_t _sense_adj_last = 0;

  uint32_t _sense_increase_interval = DEFAULT_SENSE_INCREASE_INTERVAL;

  P169_InterruptMode _mode = P169_InterruptMode::detached;
};

#endif // ifdef USES_P169
#endif // ifndef PLUGINSTRUCTS_P169_DATA_STRUCT_H

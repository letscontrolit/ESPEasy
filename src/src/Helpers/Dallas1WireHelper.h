#ifndef HELPERS_DALLAS1WIREHELPER_H
#define HELPERS_DALLAS1WIREHELPER_H

#include "../../ESPEasy_common.h"

#if FEATURE_DALLAS_HELPER

# include "../DataTypes/TaskIndex.h"
# include "../DataTypes/PluginID.h"


// Used timings based on Maxim documentation.
// See https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html
// We use the "standard speed" timings, not the "Overdrive speed"


struct Dallas_SensorData {
  Dallas_SensorData() = default;

  void clear();

  bool check_sensor(int8_t gpio_rx,
                    int8_t gpio_tx,
                    int8_t res);

  void set_measurement_inactive();

  bool initiate_read(int8_t gpio_rx,
                     int8_t gpio_tx,
                     int8_t res);

  bool   collect_value(int8_t gpio_rx,
                       int8_t gpio_tx);

  String get_formatted_address() const;

  uint64_t addr{};
  float    value{};
  uint32_t start_read_failed{};
  uint32_t start_read_retry{};
  uint32_t read_success{};
  uint32_t sensor_power_on_reset{};
  uint32_t read_CRC{};
  uint32_t read_retry{};
  uint32_t read_failed{};
  uint32_t reinit_count{};
  uint8_t  actual_res{};

  bool measurementActive{};
  bool valueRead{};
  bool lastReadError{};
  bool fixed_resolution{};
# ifndef LIMIT_BUILD_SIZE
  bool parasitePowered{};
# endif

};


/*********************************************************************************************\
   Variables used to keep track of scanning the bus
   N.B. these should not be shared for simultaneous scans on different pins
\*********************************************************************************************/
extern unsigned char ROM_NO[8];
extern uint8_t LastDiscrepancy;
extern uint8_t LastFamilyDiscrepancy;
extern bool    LastDeviceFlag;


/*********************************************************************************************\
   Timings for diagnostics regarding the reset + presence detection
\*********************************************************************************************/
extern int32_t usec_release;   // Time needed for the line to rise (typ: < 1 usec)
extern int32_t presence_start; // Start presence condition after release by master (typ: 30 usec)
extern int32_t presence_end;   // End presence condition (minimal 60 usec, typ: 100 usec)


/*********************************************************************************************\
   Format 1-wire address
\*********************************************************************************************/
String   Dallas_format_address(const uint8_t addr[],
                               const bool    hasFixedResolution = false);

uint64_t Dallas_addr_to_uint64(const uint8_t addr[]);

void     Dallas_addr_selector_webform_load(taskIndex_t TaskIndex,
                                           int8_t      gpio_pin_rx,
                                           int8_t      gpio_pin_tx,
                                           uint8_t     nrVariables = 1);

# ifndef LIMIT_BUILD_SIZE
void Dallas_show_sensor_stats_webform_load(const Dallas_SensorData& sensor_data);
# endif

void Dallas_addr_selector_webform_save(taskIndex_t TaskIndex,
                                       int8_t      gpio_pin_rx,
                                       int8_t      gpio_pin_tx,
                                       uint8_t     nrVariables = 1);

// Load ROM address from tasksettings
void Dallas_plugin_get_addr(uint8_t     addr[],
                            taskIndex_t TaskIndex,
                            uint8_t     var_index = 0);


/*********************************************************************************************\
   Dallas Scan bus
\*********************************************************************************************/
int     Dallas_measure_rise_time(int8_t gpio_pin_rx,
                                 int8_t gpio_pin_tx);

uint8_t Dallas_scan(uint8_t  getDeviceROM,
                    uint8_t *ROM,
                    int8_t   gpio_pin_rx,
                    int8_t   gpio_pin_tx);

/*
   void Dallas_startConversion(const uint8_t ROM[8],
                            int8_t        gpio_pin_rx,
                            int8_t        gpio_pin_tx);
 */

/*********************************************************************************************\
*  Dallas data from scratchpad
\*********************************************************************************************/

# ifdef USES_P080
bool Dallas_readiButton(const uint8_t addr[8],
                        int8_t        gpio_pin_rx,
                        int8_t        gpio_pin_tx,
                        int8_t        lastState = -1);
# endif // ifdef USES_P080

# ifdef USES_P100
bool Dallas_readCounter(const uint8_t ROM[8],
                        float        *value,
                        int8_t        gpio_pin_rx,
                        int8_t        gpio_pin_tx,
                        uint8_t       counter);
# endif // ifdef USES_P100

/*********************************************************************************************\
* Dallas Get Resolution
\*********************************************************************************************/
uint8_t Dallas_getResolution(const uint8_t ROM[8],
                             int8_t        gpio_pin_rx,
                             int8_t        gpio_pin_tx);
uint8_t Dallas_getResolution(const uint8_t ROM[8],
                             int8_t        gpio_pin_rx,
                             int8_t        gpio_pin_tx,
                             bool        & hasFixedResolution);

/*********************************************************************************************\
* Dallas Set Resolution
\*********************************************************************************************/
bool Dallas_setResolution(const uint8_t ROM[8],
                          uint8_t       res,
                          int8_t        gpio_pin_rx,
                          int8_t        gpio_pin_tx);

/*********************************************************************************************\
*  Dallas Reset
\*********************************************************************************************/
uint8_t Dallas_reset(int8_t gpio_pin_rx,
                     int8_t gpio_pin_tx);


/*********************************************************************************************\
*  Dallas Reset Search
\*********************************************************************************************/
void    Dallas_reset_search();

/*********************************************************************************************\
*  Dallas Search bus
\*********************************************************************************************/
uint8_t Dallas_search(uint8_t *newAddr,
                      int8_t   gpio_pin_rx,
                      int8_t   gpio_pin_tx);


/*********************************************************************************************\
*  Dallas Read bit
*  See https://github.com/espressif/arduino-esp32/issues/1335
\*********************************************************************************************/
/*
   uint8_t Dallas_read_bit(int8_t gpio_pin_rx,
                        int8_t gpio_pin_tx);
   uint8_t Dallas_read_bit_ISR(int8_t    gpio_pin_rx,
                            int8_t    gpio_pin_tx,
                            uint32_t& start);
 */
/*********************************************************************************************\
*  Dallas Write bit
*  See https://github.com/espressif/arduino-esp32/issues/1335
\*********************************************************************************************/
/*
   void Dallas_write_bit(uint8_t v,
                      int8_t  gpio_pin_rx,
                      int8_t  gpio_pin_tx);

   void Dallas_write_bit_ISR(uint8_t   v,
                          int8_t    gpio_pin_rx,
                          int8_t    gpio_pin_tx,
                          int32_t      low_time,
                          int32_t      high_time,
                          uint32_t& start);
 */

#endif // if FEATURE_DALLAS_HELPER

#endif // ifndef HELPERS_DALLAS1WIREHELPER_H

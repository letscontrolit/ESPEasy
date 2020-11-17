#ifndef PLUGINSTRUCTS_P004_DATA_STRUCT_H
#define PLUGINSTRUCTS_P004_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P004

# include "../../ESPEasy_common.h"

# include "../Helpers/Dallas1WireHelper.h"

struct P004_data_struct : public PluginTaskData_base {
  /*********************************************************************************************\
  * Task data struct to simplify taking measurements of upto 4 Dallas DS18b20 (or compatible)
  * temperature sensors at once.
  *
  * Limitations:
  * - Use the same GPIO pin
  * - Use the same resolution for all sensors of the same task
  * - Max 4 sensors queried at the same time
  *
  * The limit of 4 sensors is determined by the way the settings are stored and it
  * is a practical limit to make sure we don't spend too much time in a single call.
  *
  * Using the same resolution is to make it (a lot) simpler as all sensors then need the
  * same measurement time.
  *
  * If those limitations are not desired, use multiple tasks.
  \*********************************************************************************************/

  // @param pin  The GPIO pin used to communicate to the Dallas sensors in this task
  // @param addr Address of the (1st) Dallas sensor (index = 0) in this task
  // @param res  The resolution of the Dallas sensor(s) used in this task
  P004_data_struct(int8_t        pin,
                   const uint8_t addr[],
                   uint8_t       res);

  // Add extra sensor address
  // @param addr The address to add
  // @param index  The index (0...3) to store this address
  void add_addr(const uint8_t addr[],
                uint8_t       index);

  // Send the start measuremnt command to all set sensors which have a non-zero address
  // Their index determines the order in which the sensors receive this command.
  bool initiate_read();

  bool collect_values();

  // Read temperature from the sensor at given index.
  // May return false if the sensor is not present or address is zero.
  bool          read_temp(float & value,
                          uint8_t index = 0) const;

  String        get_formatted_address(uint8_t index = 0) const;

  unsigned long get_timer() const {
    return _timer;
  }

  unsigned long get_measurement_start() const {
    return _measurementStart;
  }

  int8_t get_gpio() const {
      return _gpio;
  }

  bool measurement_active() const;
  bool measurement_active(uint8_t index) const;
  void set_measurement_inactive();

  Dallas_SensorData get_sensor_data(uint8_t index) const;


private:

  // Do not set the _timer to 0, since it may cause issues
  // if this object is created (settings edited or task enabled)
  // while the node is up some time between 24.9 and 49.7 days.
  unsigned long   _timer            = millis();
  unsigned long   _measurementStart = millis();
  Dallas_SensorData _sensors[4];
  int8_t          _gpio = -1;
  uint8_t         _res  = 0;
};

#endif // ifdef USES_P004
#endif // ifndef PLUGINSTRUCTS_P004_DATA_STRUCT_H

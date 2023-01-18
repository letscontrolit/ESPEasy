#include "../PluginStructs/P004_data_struct.h"

#ifdef USES_P004

P004_data_struct::P004_data_struct(
  taskIndex_t taskIndex,
  int8_t      pin_rx,
  int8_t      pin_tx,
  uint8_t     res,
  bool        scanOnInit) :
  _taskIndex(taskIndex),
  _gpio_rx(pin_rx), _gpio_tx(pin_tx),
  _res(res), _scanOnInit(scanOnInit)
{
  _timer            = millis();
  _measurementStart = _timer;

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    _sensors[i].clear();
  }

  if ((_res < 9) || (_res > 12)) {
    _res = 12;
  }
}

bool P004_data_struct::sensorAddressSet() const
{
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_sensors[i].addr != 0) return true;
  }
  return false;
}

void P004_data_struct::init()
{
  // Explicitly set the pinMode using the "slow" pinMode function
  // This way we know for sure the state of any pull-up or -down resistor is known.
  pinMode(_gpio_rx, INPUT);

  // The Shelly 1 temp. addon uses separate
  // input and output pins, and therefore
  // doesn't switch between input and output
  // when running.
  if (_gpio_rx != _gpio_tx) {
    pinMode(_gpio_tx, OUTPUT);
  }
  set_measurement_inactive();

  if (_scanOnInit) {
    // Scan for any sensor and use it.
    // This can only be used when we use only 1 sensor.
    uint8_t addr[8] = { 0 };
    Dallas_reset_search();

    if (!Dallas_search(addr, _gpio_rx, _gpio_tx)) {
      // Nothing found, for now restore from the settings (if anything is set)
      Dallas_plugin_get_addr(addr, _taskIndex);
    }

    if (addr[0] != 0) {
      add_addr(addr, 0);
    }
  }
}

void P004_data_struct::add_addr(const uint8_t addr[], uint8_t index) {
  if (index < VARS_PER_TASK) {
    const uint64_t new_addr = Dallas_addr_to_uint64(addr);

    if (new_addr == 0) { return; }

    if (new_addr != _sensors[index].addr) {
      _sensors[index].clear();
      _sensors[index].addr = new_addr;
    }

    // If the address already exists, set it to 0 to avoid duplicates
    for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
      if (index != i) {
        if (new_addr == _sensors[i].addr) {
          _sensors[index].clear();
          return;
        }
      }
    }
    _sensors[index].check_sensor(_gpio_rx, _gpio_tx, _res);
  }
}

bool P004_data_struct::initiate_read() {
  _measurementStart = millis();

  bool mustInit = false;

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_sensors[i].initiate_read(_gpio_rx, _gpio_tx, _res)) {
      if (!measurement_active()) {
        // Set the timer right after initiating the first sensor


        /*********************************************************************************************\
        *  Dallas Start Temperature Conversion, expected max duration:
        *    9 bits resolution ->  93.75 ms
        *   10 bits resolution -> 187.5 ms
        *   11 bits resolution -> 375 ms
        *   12 bits resolution -> 750 ms
        \*********************************************************************************************/
        _timer = millis() + (800 / (1 << (12 - _res)));
      }
      _sensors[i].measurementActive = true;
    } else {
      if (_scanOnInit && (_sensors[i].start_read_failed > (_sensors[i].reinit_count * 10))) {
        _sensors[i].reinit_count++;
        mustInit = true;
      }
    }
  }

  if (mustInit) {
    init();
    return false;
  }

  return measurement_active();
}

bool P004_data_struct::collect_values() {
  bool success = false;

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_sensors[i].collect_value(_gpio_rx, _gpio_tx)) {
      success = true;
    }
  }
  return success;
}

bool P004_data_struct::read_temp(float& value, uint8_t index) const {
  if (index >= VARS_PER_TASK) { return false; }

  if ((_sensors[index].addr == 0) || !_sensors[index].valueRead) { return false; }

  value = _sensors[index].value;
  return true;
}

String P004_data_struct::get_formatted_address(uint8_t index) const {
  if (index < VARS_PER_TASK) {
    return _sensors[index].get_formatted_address();
  }
  return EMPTY_STRING;
}

bool P004_data_struct::measurement_active() const {
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (_sensors[i].measurementActive) { return true; }
  }

  return false;
}

bool P004_data_struct::measurement_active(uint8_t index) const {
  if (index < VARS_PER_TASK) {
    return _sensors[index].measurementActive;
  }
  return false;
}

void P004_data_struct::set_measurement_inactive() {
  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    _sensors[i].set_measurement_inactive();
  }
}

Dallas_SensorData P004_data_struct::get_sensor_data(uint8_t index) const {
  if (index < VARS_PER_TASK) { return _sensors[index]; }
  Dallas_SensorData res;

  return res;
}

#endif // ifdef USES_P004

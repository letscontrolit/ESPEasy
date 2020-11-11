#include "P004_data_struct.h"

#ifdef USES_P004

# include "../Helpers/Dallas1WireHelper.h"

P004_data_struct::P004_data_struct(int8_t pin, const uint8_t addr[], uint8_t res) : _gpio(pin), _res(res)
{
  if ((_res < 9) || (_res > 12)) { _res = 12; }

  _addr[0] = Dallas_addr_to_uint64(addr);
  set_measurement_inactive();
}

void P004_data_struct::add_addr(const uint8_t addr[], uint8_t index) {
  if (index < 4) {
    _addr[index] = Dallas_addr_to_uint64(addr);

    // If the address already exists, set it to 0 to avoid duplicates
    for (uint8_t i = 0; i < 4; ++i) {
      if (index != i) {
        if (_addr[index] == _addr[i]) {
          _addr[index] = 0;
        }
      }
    }
  }
}

bool P004_data_struct::initiate_read() {
  _measurementStart = millis();

  for (byte i = 0; i < 4; ++i) {
    if (_addr[i] != 0) {
      uint8_t addr[8];
      Dallas_uint64_to_addr(_addr[i], addr);

      const byte cur_resolution = Dallas_getResolution(addr, _gpio);

      if (cur_resolution == 0) { continue; }

      if (_res != cur_resolution) {
        if (!Dallas_setResolution(addr, _res, _gpio)) {
          continue;
        }
      }
      Dallas_address_ROM(addr, _gpio);
      Dallas_write(0x44, _gpio); // Take temperature mesurement

      if (!measurement_active()) {
        // Set the timer right after initiating the first sensor


        /*********************************************************************************************\
        *  Dallas Start Temperature Conversion, expected max duration:
        *    9 bits resolution ->  93.75 ms
        *   10 bits resolution -> 187.5 ms
        *   11 bits resolution -> 375 ms
        *   12 bits resolution -> 750 ms
        \*********************************************************************************************/
        uint8_t res = _res;

        if ((res < 9) || (res > 12)) { res = 12; }
        _timer                = millis() + (800 / (1 << (12 - res)));
      }
      _measurementActive[i] = true;
    }
  }

  return measurement_active();
}

bool P004_data_struct::read_temp(float& value, uint8_t index) const {
  if ((_addr[index] == 0) || !_measurementActive[index]) { return false; }

  uint8_t addr[8];

  Dallas_uint64_to_addr(_addr[index], addr);

  return Dallas_readTemp(addr, &value, _gpio);
}

String P004_data_struct::get_formatted_address(uint8_t index) const {
  if (_addr[index] == 0) { return ""; }

  uint8_t addr[8];

  Dallas_uint64_to_addr(_addr[index], addr);
  return Dallas_format_address(addr);
}

bool P004_data_struct::measurement_active() const {
  for (uint8_t i = 0; i < 4; ++i) {
    if (_measurementActive[i]) { return true; }
  }

  return false;
}

bool P004_data_struct::measurement_active(uint8_t index) const {
  if (index < 4) {
    return _measurementActive[index];
  }
  return false;
}

void P004_data_struct::set_measurement_inactive() {
  for (uint8_t i = 0; i < 4; ++i) {
    _measurementActive[i] = false;
  }
}

#endif // ifdef USES_P004

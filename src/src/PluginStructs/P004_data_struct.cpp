#include "P004_data_struct.h"

#ifdef USES_P004

# include "../Helpers/Dallas1WireHelper.h"

P004_data_struct::P004_data_struct(int8_t pin, const uint8_t addr[], uint8_t res) : _gpio(pin), _res(res)
{
  if ((_res < 9) || (_res > 12)) { _res = 12; }

  for (uint8_t i = 0; i < 8; ++i) {
    _addr[i] = addr[i];
  }
}

bool P004_data_struct::initiate_read() {
  _measurementStart = millis();
  const byte cur_resolution = Dallas_getResolution(_addr, _gpio);

  if (cur_resolution == 0) { return false; }

  if (_res != cur_resolution) {
    if (!Dallas_setResolution(_addr, _res, _gpio)) {
      return false;
    }
  }
  Dallas_address_ROM(_addr, _gpio);
  Dallas_write(0x44, _gpio); // Take temperature mesurement

  /*********************************************************************************************\
  *  Dallas Start Temperature Conversion, expected max duration:
  *    9 bits resolution ->  93.75 ms
  *   10 bits resolution -> 187.5 ms
  *   11 bits resolution -> 375 ms
  *   12 bits resolution -> 750 ms
  \*********************************************************************************************/
  uint8_t res = _res;

  if ((res < 9) || (res > 12)) { res = 12; }
  _timer = millis() + (800 / (1 << (12 - res)));

  _measurementActive = true;

  return true;
}

bool P004_data_struct::read_temp(float& value) const {
  return Dallas_readTemp(_addr, &value, _gpio);
}

String P004_data_struct::get_formatted_address() const {
  return Dallas_format_address(_addr);
}

#endif // ifdef USES_P004

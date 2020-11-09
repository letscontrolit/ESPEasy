#include "P004_data_struct.h"

#ifdef USES_P004

P004_data_struct::P004_data_struct(int8_t pin, const uint8_t addr[], uint8_t res) : _gpio(pin), _res(res)
{
  if ((_res < 9) || (_res > 12)) { _res = 12; }
  for (uint8_t i = 0; i < 8; ++i) {
    _addr[i] = addr[i];
  }
}

void P004_data_struct::set_timeout() {
  uint8_t res = _res;

  if ((res < 9) || (res > 12)) { res = 12; }
  _timer = millis() + (800 / (1 << (12 - res)));
}

#endif // ifdef USES_P004

#include "../Helpers/PrintToString.h"

void   PrintToString::clear() { _str.clear(); }

size_t PrintToString::length() const
{
  return _str.length();
}

bool   PrintToString::reserve(unsigned int size) { return _str.reserve(size); }

size_t PrintToString::write(uint8_t c)
{
  _str += (char)c;
  return 1;
}

const String& PrintToString::get() const
{
  return _str;
}

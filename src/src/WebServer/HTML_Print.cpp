#include "../WebServer/HTML_Print.h"

#include "../Globals/TXBuffer.h"

size_t HTML_Print::write(uint8_t c)
{
  TXBuffer += (char)c;
  return 1;
}

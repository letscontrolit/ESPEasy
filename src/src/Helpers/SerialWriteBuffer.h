#ifndef HELPERS_SERIALWRITEBUFFER_H
#define HELPERS_SERIALWRITEBUFFER_H

#include "../../ESPEasy_common.h"

#include <deque>

#ifndef MAX_SERIALWRITEBUFFER_SIZE
# ifdef ESP8266
#  define MAX_SERIALWRITEBUFFER_SIZE 1024
# endif // ifdef ESP8266
# ifdef ESP32
#  define MAX_SERIALWRITEBUFFER_SIZE 10240
# endif // ifdef ESP32
#endif // ifndef MAX_SERIALWRITEBUFFER_SIZE

class SerialWriteBuffer_t {
public:

  SerialWriteBuffer_t(size_t maxSize = MAX_SERIALWRITEBUFFER_SIZE)
    : _maxSize(maxSize) {}

  void   add(const String& line);
  void   add(const __FlashStringHelper *line);
  void   add(char c);

  void   addNewline();

  void   clear();

  size_t write(Stream& stream,
               size_t  nrBytesToWrite);

private:

  int getRoomLeft() const;

  std::deque<char>_buffer;
  size_t _maxSize = MAX_SERIALWRITEBUFFER_SIZE;
};

#endif // ifndef HELPERS_SERIALWRITEBUFFER_H

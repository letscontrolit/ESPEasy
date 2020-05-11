#ifndef DATASTRUCTS_ESPEASY_NOW_INCOMING_H
#define DATASTRUCTS_ESPEASY_NOW_INCOMING_H

#include <Arduino.h>

class ESPEasy_Now_incoming {
public:

  ESPEasy_Now_incoming(const uint8_t  mac[6],
                       const uint8_t *buf,
                       size_t         count);

  size_t getSize() const;

  // Return a string starting from position pos in the buffer.
  String getString(size_t pos = 0) const;


  uint8_t _mac[6] = { 0 };
  std::vector<uint8_t>_buf;
};


#endif // DATASTRUCTS_ESPEASY_NOW_INCOMING_H

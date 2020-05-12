#ifndef DATASTRUCTS_ESPEASY_NOW_INCOMING_H
#define DATASTRUCTS_ESPEASY_NOW_INCOMING_H

#include <Arduino.h>

#include "../Globals/ESPEasy_now_state.h"
#ifdef USES_ESPEASY_NOW

# include "ESPEasy_now_hdr.h"
class ESPEasy_Now_packet {
public:

  // Constructor for sending a packet
  ESPEasy_Now_packet(const ESPEasy_now_hdr& header,
                     size_t                 payloadSize);

  // Constructor for receiving a packet
  ESPEasy_Now_packet(const uint8_t  mac[6],
                     const uint8_t *buf,
                     size_t         packetSize);

  size_t          getSize() const;

  size_t          getPayloadSize() const;

  ESPEasy_now_hdr getHeader() const;

  void            setHeader(ESPEasy_now_hdr header);

  // Add a string to the packet, starting at payload position payload_pos
  size_t          addString(const String& string,
                            size_t        payload_pos = 0);

  // Return a string starting from position pos in the buffer.
  String          getString(size_t pos = 0) const;

  const uint8_t * operator[](size_t idx) const {
    return &_buf[idx];
  }

  uint8_t _mac[6] = { 0 };

private:

  std::vector<uint8_t>_buf;

  void setSize(size_t packetSize);
};

#endif // ifdef USES_ESPEASY_NOW

#endif // DATASTRUCTS_ESPEASY_NOW_INCOMING_H

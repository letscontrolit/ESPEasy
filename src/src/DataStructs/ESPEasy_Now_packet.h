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

  void            setMac(uint8_t mac[6]);

  void            setBroadcast();

  // Add a string to the packet, starting at payload position payload_pos
  // Return the number of bytes added (can be 1 more than the given string)
  size_t addString(const String& string,
                   size_t      & payload_pos);

  // Return a string starting from position pos in the buffer.
  // payload_pos will contain the new position to start for a next string
  String          getString(size_t& payload_pos) const;

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

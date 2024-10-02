#ifndef DATASTRUCTS_NTP_PACKET_H
#define DATASTRUCTS_NTP_PACKET_H

#include <Arduino.h>

struct  __attribute__((__packed__)) NTP_packet
{
  NTP_packet();
  bool isUnsynchronized() const;

  // Reference Timestamp: Time when the system clock was last set or corrected, in NTP timestamp format.
  // Returned timestamp is Unixtime in microseconds
  uint64_t getReferenceTimestamp_usec() const;

  // Origin Timestamp (org): Time at the client when the request departed for the server, in NTP timestamp format.
  // Returned timestamp is Unixtime in microseconds
  uint64_t getOriginTimestamp_usec() const;

  // Receive Timestamp (rec): Time at the server when the request arrived from the client, in NTP timestamp format.
  // Returned timestamp is Unixtime in microseconds
  uint64_t getReceiveTimestamp_usec() const;

  // Transmit Timestamp (xmt): Time at the server when the response left for the client, in NTP timestamp format.
  // N.B. when requesting the time, the client should set its local system time here.
  // In the reply packet, this will be moved to the origin timestamp field.
  // Returned timestamp is Unixtime in microseconds
  uint64_t getTransmitTimestamp_usec() const;


  // Before sending, the TX-timestamp of the local machine must be set.
  // This will be returned in the reply as "Origin" timestamp
  void setTxTimestamp(uint64_t micros);

  // The "Offset", the time difference of the two computer clocks
  // The "Delay", the time that was needed to transfer the packet in the network
  bool compute_usec(
    uint64_t localTXTimestamp_usec,
    uint64_t localRxTimestamp_usec,
    int64_t& offset_usec,
    int64_t& roundtripDelay_usec) const;

  String getRefID_str(bool& isError) const;

#ifndef BUILD_NO_DEBUG
  String toDebugString() const;
#endif // ifndef BUILD_NO_DEBUG

  uint8_t data[48]{};

private:

  uint32_t readWord(uint8_t startIndex) const;
  void     writeWord(uint32_t value,
                     uint8_t  startIndex);
  uint64_t ntp_timestamp_to_Unix_time(uint8_t startIndex) const;
};

#endif // ifndef DATASTRUCTS_NTP_PACKET_H

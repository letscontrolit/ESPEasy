#include "../DataStructs/NTP_packet.h"

#include "../Helpers/ESPEasy_time_calc.h"

#include "../Helpers/StringConverter.h"

NTP_packet::NTP_packet()
{
  // li, vn, and mode:
  // - li.   2 bits. Leap indicator.
  //    0 = no warning
  //    1 = last minute of the day has 61 seconds
  //    2 = last minute of the day has 59 seconds
  //    3 = unknown (clock unsynchronized)
  // - vn.   3 bits. Version number of the protocol. (0b100 = v4)
  // - mode. 3 bits. Client will pick mode 3 for client.
  //    0 = reserved
  //    1 = symmetric active
  //    2 = symmetric passive
  //    3 = client
  //    4 = server
  //    5 = broadcast
  //    6 = NTP control message
  //    7 = reserved for private use
  data[0] = 0b11100011; // Unsynchronized, V4, client mode

  // Stratum level of the local clock.
  //    0      = unspecified or invalid
  //    1      = primary server (e.g., equipped with a GPS receiver)
  //    2-15   = secondary server (via NTP)
  //    16     = unsynchronized
  //    17-255 = reserved
  data[1] = 0u;

  // Poll: 8-bit signed integer representing the maximum interval between
  //       successive messages, in log2 seconds.  Suggested default limits for
  //       minimum and maximum poll intervals are 6 and 10, respectively.
  data[2] = 6u;

  // Precision: 8-bit signed integer representing the precision of the
  //    system clock, in log2 seconds.  For instance, a value of -18
  //    corresponds to a precision of about one microsecond.  The precision
  //    can be determined when the service first starts up as the minimum
  //    time of several iterations to read the system clock.
  data[3] = 0xEC; // -20 -> 2^-20 sec -> microsec precision.

  constexpr int8_t precision = 0xEC;

  // Reference clock identifier. ASCII: "1N14"
  data[12] = 0x31;
  data[13] = 0x4E;
  data[14] = 0x31;
  data[15] = 0x34;
}

uint32_t NTP_packet::readWord(uint8_t startIndex) const
{
  uint32_t res{};

  res  = (uint32_t)data[startIndex] << 24;
  res |= (uint32_t)data[startIndex + 1] << 16;
  res |= (uint32_t)data[startIndex + 2] << 8;
  res |= (uint32_t)data[startIndex + 3];
  return res;
}

uint64_t NTP_packet::ntp_timestamp_to_Unix_time(uint8_t startIndex) const {
  // Apply offset from 1900/01/01 to 1970/01/01
  constexpr uint64_t offset_since_1900 = 2208988800ULL * 1000000ull;

  const uint32_t Tm_s      = readWord(startIndex);
  const uint32_t Tm_f      = readWord(startIndex + 4);
  uint64_t usec_since_1900 = sec_time_frac_to_Micros(Tm_s, Tm_f);

  if (usec_since_1900 < offset_since_1900) {
    // Fix overflow which will occur in 2036
    usec_since_1900 += (4294967296ull * 1000000ull);
  }
  return usec_since_1900 - offset_since_1900;
}

void NTP_packet::writeWord(uint32_t value, uint8_t startIndex)
{
  data[startIndex]     = (value >> 24) & 0xFF;
  data[startIndex + 1] = (value >> 16) & 0xFF;
  data[startIndex + 2] = (value >> 8) & 0xFF;
  data[startIndex + 3] = (value) & 0xFF;
}

bool NTP_packet::isUnsynchronized() const
{
  return (data[0] /*li_vn_mode*/ & 0b11000000) == 0b11000000;
}

uint64_t NTP_packet::getReferenceTimestamp_usec() const
{
  return ntp_timestamp_to_Unix_time(16);
}

uint64_t NTP_packet::getOriginTimestamp_usec() const
{
  return ntp_timestamp_to_Unix_time(24);
}

uint64_t NTP_packet::getReceiveTimestamp_usec() const
{
  return ntp_timestamp_to_Unix_time(32);
}

uint64_t NTP_packet::getTransmitTimestamp_usec() const
{
  return ntp_timestamp_to_Unix_time(40);
}

void NTP_packet::setTxTimestamp(uint64_t micros)
{
  constexpr uint64_t offset_since_1900 = 2208988800ULL * 1000000ull;

  micros += offset_since_1900;
  uint32_t tmp_origTm_f{};

  writeWord(micros_to_sec_time_frac(micros, tmp_origTm_f), 40);
  writeWord(tmp_origTm_f,                                  44);
}

bool NTP_packet::compute_usec(
  uint64_t localTXTimestamp_usec,
  uint64_t localRxTimestamp_usec,
  int64_t& offset_usec,
  int64_t& roundtripDelay_usec) const
{
  int64_t t1 = getOriginTimestamp_usec();

  if (t1 == 0) {
    t1 = localTXTimestamp_usec;
  }

  const int64_t t2 = getReceiveTimestamp_usec();
  const int64_t t3 = getTransmitTimestamp_usec();

  if ((t3 == 0) || (t3 < t2)) {
    // No time stamp received
    return false;
  }
  const int64_t t4 = localRxTimestamp_usec;

  offset_usec  = (t2 - t1) + (t3 - t4);
  offset_usec /= 2;

  roundtripDelay_usec = (t4 - t1) - (t3 - t2);
  return true;
}

String NTP_packet::getRefID_str(bool& isError) const
{
  String refID;

  const uint8_t stratum = data[1];

  isError = false;

  if ((stratum == 0) || (stratum == 1)) {
    refID = strformat(F("%c%c%c%c"),
                      static_cast<char>(data[12] & 0x7F),
                      static_cast<char>(data[13] & 0x7F),
                      static_cast<char>(data[14] & 0x7F),
                      static_cast<char>(data[15] & 0x7F));

    if (stratum == 0) {
      if (refID.equals(F("DENY")) ||
          refID.equals(F("RSTR"))) {
        // For kiss codes DENY and RSTR, the client MUST
        // demobilize any associations to that server and
        // stop sending packets to that server;
        // DENY = Access denied by remote server.
        // RSTR = Access denied due to local policy.
        isError = true;
      } else if (refID.equals(F("RATE"))) {
        // For kiss code RATE, the client MUST immediately reduce its
        // polling interval to that server and continue to reduce it each
        // time it receives a RATE kiss code.
      }
    }
  } else {
    const IPAddress addrv4(readWord(12));
    refID = addrv4.toString();
  }
  return refID;
}

#ifndef BUILD_NO_DEBUG
String NTP_packet::toDebugString() const
{
  const uint8_t li   = (data[0] >> 6) & 0x3; // Leap Indicator
  const uint8_t ver  = (data[0] >> 3) & 0x7; // Version
  const uint8_t mode = data[0] & 0x7;        // Mode

  bool isError{};

  return strformat(
    F("    li: %u ver: %u mode: %u\n"
      "    strat: %u poll: %d prec: %d\n"
      "    del:   %u disp: %u refID: '%s'\n"
      "    refTm_s : %u refTm_f : %u\n"
      "    origTm_s: %u origTm_f: %u\n"
      "    rxTm_s  : %u rxTm_f  : %u\n"
      "    txTm_s  : %u txTm_f  : %u\n"),
    li, ver, mode,                                        // li_vn_mode
    data[1],                                              // stratum,
    (int8_t)(data[2]),                                    // poll in log2 seconds
    (int8_t)(data[3]),                                    // precision in log2 seconds
    readWord(4), readWord(8),                             // rootDelay, rootDispersion,
    getRefID_str(isError).c_str(),
    readWord(16), unix_time_frac_to_micros(readWord(20)), // refTm_s, unix_time_frac_to_micros(refTm_f),
    readWord(24), unix_time_frac_to_micros(readWord(28)), // origTm_s, unix_time_frac_to_micros(origTm_f),
    readWord(32), unix_time_frac_to_micros(readWord(36)), // rxTm_s, unix_time_frac_to_micros(rxTm_f),
    readWord(40), unix_time_frac_to_micros(readWord(44))  // txTm_s, unix_time_frac_to_micros(txTm_f)

    );
}

#endif // ifndef BUILD_NO_DEBUG

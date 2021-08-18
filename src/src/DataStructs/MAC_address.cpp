#include "../DataStructs/MAC_address.h"

#include "../../ESPEasy_common.h"

MAC_address::MAC_address()
{}

MAC_address::MAC_address(const uint8_t new_mac[6])
{
  memcpy(mac, new_mac, 6);
}

bool MAC_address::set(const char *string)
{
  unsigned u[6];
  int c = sscanf(string, "%x:%x:%x:%x:%x:%x", u, u + 1, u + 2, u + 3, u + 4, u + 5);

  if (c != 6) {
    return false;
  }

  for (int i = 0; i < 6; ++i) {
    mac[i] = static_cast<uint8_t>(u[i]);
  }
  return true;
}

void MAC_address::set(const uint8_t other[6])
{
  memcpy(mac, other, 6);
}

void MAC_address::get(uint8_t mac_out[6]) const
{
  memcpy(mac_out, mac, 6);
}

bool MAC_address::all_zero() const
{
  for (int i = 0; i < 6; ++i) {
    if (mac[i] != 0) {
      return false;
    }
  }
  return true;
}

bool MAC_address::all_one() const
{
  for (int i = 0; i < 6; ++i) {
    if (mac[i] != 0xFF) {
      return false;
    }
  }
  return true;
}

String MAC_address::toString() const
{
  char str[18] = { 0 };
  sprintf_P(str, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(str);
}

bool MAC_address::mac_addr_cmp(const uint8_t other[6]) const
{
  for (int i = 0; i < 6; ++i) {
    if (mac[i] != other[i]) {
      return false;
    }
  }
  return true;
}
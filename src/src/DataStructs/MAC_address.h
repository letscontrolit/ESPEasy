#ifndef DATASTRUCTS_MAC_ADDRESS_H
#define DATASTRUCTS_MAC_ADDRESS_H

#include <stdint.h>
#include <WString.h>

class __attribute__((__packed__)) MAC_address {
public:

  MAC_address();

  MAC_address(const uint8_t new_mac[6]);

  bool operator==(const MAC_address& other) const {
    return mac_addr_cmp(other.mac);
  }

  bool operator!=(const MAC_address& other) const {
    return !mac_addr_cmp(other.mac);
  }

  bool operator==(const uint8_t other[6]) const {
    return mac_addr_cmp(other);
  }

  bool operator!=(const uint8_t other[6]) const {
    return !mac_addr_cmp(other);
  }

  // Parse string with MAC address.
  // Returns false if the given string has no valid formatted mac address.
  bool   set(const char *string);

  void   set(const uint8_t other[6]);

  void   get(uint8_t mac_out[6]) const;

  bool   all_zero() const;

  String toString() const;

  void toString(char (& strMAC)[20]) const;

  uint8_t mac[6] = { 0 };

private:

  bool mac_addr_cmp(const uint8_t other[6]) const;
};

#endif // DATASTRUCTS_MAC_ADDRESS_H

#ifndef DATATYPES_NETWORKMEDIUM_H
#define DATATYPES_NETWORKMEDIUM_H

#include <Arduino.h>

// Is stored in settings
enum class NetworkMedium_t : unsigned char {
  WIFI     = 0,
  Ethernet = 1
};

bool   isValid(NetworkMedium_t medium);

const __FlashStringHelper * toString(NetworkMedium_t medium);


#endif // DATATYPES_NETWORKMEDIUM_H

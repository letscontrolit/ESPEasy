#ifndef HELPERS_STRINGCONVERTER_NUMERICAL_H
#define HELPERS_STRINGCONVERTER_NUMERICAL_H

#include "../../ESPEasy_common.h"

/********************************************************************************************\
   Convert a char string to integer
 \*********************************************************************************************/

// FIXME: change original code so it uses String and String.toInt()
unsigned long str2int(const char *string);

/********************************************************************************************\
   Check if valid float and convert string to float.
 \*********************************************************************************************/
bool          string2float(const String& string,
                           float       & floatvalue);

/*********************************************************************************************\
   Workaround for removing trailing white space when String() converts a float with 0 decimals
\*********************************************************************************************/
String toString(const float& value,
                unsigned int decimalPlaces = 2,
                bool         trimTrailingZeros = false);

bool toValidString(String& str,
                  const float& value,
                  unsigned int decimalPlaces = 2,
                  bool         trimTrailingZeros = false);
  

String ull2String(uint64_t value,
                  uint8_t  base = 10);

String ll2String(int64_t value,
                 uint8_t base = 10);

String trimTrailingZeros(const String& value);

String toStringNoZero(int64_t value);

String doubleToString(const double& value,
                      unsigned int  decimalPlaces     = 2,
                      bool          trimTrailingZeros = false);

bool  doubleToValidString(String& str,
                        const double& value,
                        unsigned int  decimalPlaces     = 2,
                        bool          trimTrailingZeros = false);
  
String floatToString(const float& value,
                      unsigned int  decimalPlaces     = 2,
                      bool          trimTrailingZeros = false);


/********************************************************************************************\
   Handling HEX strings
 \*********************************************************************************************/

// Convert max. 8 hex decimals to unsigned long
unsigned long hexToUL(const String& input_c,
                      size_t        nrHexDecimals);

unsigned long hexToUL(const String& input_c);

unsigned long hexToUL(const String& input_c,
                      size_t        startpos,
                      size_t        nrHexDecimals);

// Convert max. 16 hex decimals to unsigned long long
unsigned long long hexToULL(const String& input_c,
                            size_t        nrHexDecimals); 

unsigned long long hexToULL(const String& input_c);

unsigned long long hexToULL(const String& input_c,
                            size_t        startpos,
                            size_t        nrHexDecimals);

void appendHexChar(uint8_t data, String& string);

// Binary data to HEX
// Returned string length will be twice the size of the data array.
String formatToHex_array(const uint8_t* data, size_t size);
String formatToHex_wordarray(const uint16_t* data, size_t size);

String formatULLtoHex(const uint64_t& value,
                   const __FlashStringHelper * prefix,
                   unsigned int minimal_hex_digits);

String formatULLtoHex(const uint64_t& value,
                   const __FlashStringHelper * prefix);

String formatULLtoHex(const uint64_t& value, unsigned int minimal_hex_digits = 0);

String formatULLtoHex_no_prefix(const uint64_t& value, unsigned int minimal_hex_digits = 0);

String formatULLtoHex_decimal(const uint64_t& value);


String formatToHex(unsigned long value,
                   const __FlashStringHelper * prefix,
                   unsigned int minimal_hex_digits);

String formatToHex(unsigned long value,
                   const __FlashStringHelper * prefix);

String formatToHex(unsigned long value, unsigned int minimal_hex_digits = 0);

String formatToHex_no_prefix(unsigned long value, unsigned int minimal_hex_digits = 0);

String formatHumanReadable(uint64_t value,
                           uint32_t factor);

String formatHumanReadable(uint64_t value,
                           uint32_t factor,
                           int           NrDecimals);

String formatToHex_decimal(unsigned long value);

String formatToHex_decimal(unsigned long value,
                           unsigned long factor);

int intFromHexChar(char a);

String stringFromHexArray(const String& arr);

#endif // ifndef HELPERS_STRINGCONVERTER_NUMERICAL_H

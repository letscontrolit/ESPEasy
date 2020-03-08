#ifndef RN2XX3_HELPER_H
#define RN2XX3_HELPER_H

#include "Arduino.h"

class rn2xx3_helper {
public:

  static bool valid_hex_char(char c);
  static bool valid_char(char c);


  static bool isHexStr(const String& string);
  static bool isHexStr_of_length(const String& str,
                                 size_t        length);


  /*
   * Decode a HEX string to an ASCII string. Useful to decode a
   * string received from the RN2xx3.
   */
  static String base16decode(const String&);

  /*
   * Encode an ASCII string to a HEX string as needed when passed
   * to the RN2xx3 module.
   */
  static String base16encode(const String& input_c);

  /*
   * Encode binary data to a HEX string as needed when passed
   * to the RN2xx3 module.
   */
  static String base16encode(const byte *data, uint8_t size);
};


#endif // RN2XX3_HELPER_H

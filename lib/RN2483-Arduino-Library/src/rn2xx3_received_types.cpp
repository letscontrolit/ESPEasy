#include "rn2xx3_received_types.h"

RN2xx3_received_types::received_t RN2xx3_received_types::determineReceivedDataType(const String& receivedData) {
  // Uncrustify must not be used on macros, so turn it off.
  // *INDENT-OFF*
  #define MATCH_STRING(S) if (receivedData.startsWith(F(#S))) return (RN2xx3_received_types::S); 
  // Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
 // *INDENT-ON*

  if (receivedData.length() != 0) {
    switch (receivedData[0]) {
      case 'a':
        MATCH_STRING(accepted);
        break;
      case 'b':
        MATCH_STRING(busy);
        break;
      case 'd':
        MATCH_STRING(denied);
        break;
      case 'f':
        MATCH_STRING(frame_counter_err_rejoin_needed);
        break;
      case 'i':
        MATCH_STRING(invalid_data_len);
        MATCH_STRING(invalid_param);
        break;
      case 'k':
        MATCH_STRING(keys_not_init);
        break;
      case 'm':
        MATCH_STRING(mac_err);
        MATCH_STRING(mac_paused);
        MATCH_STRING(mac_rx);
        MATCH_STRING(mac_tx_ok);
        break;
      case 'n':
        MATCH_STRING(no_free_ch);
        MATCH_STRING(not_joined);
        break;
      case 'o':
        MATCH_STRING(ok);
        break;
      case 'r':
        MATCH_STRING(radio_err);
        MATCH_STRING(radio_rx);
        MATCH_STRING(radio_tx_ok);
        break;
      case 's':
        MATCH_STRING(silent);
        break;
    }
  }
  #undef MATCH_STRING
  return RN2xx3_received_types::UNKNOWN;
}

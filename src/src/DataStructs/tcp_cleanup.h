#ifndef DATASTRUCTS_TCP_CLEANUP_H
#define DATASTRUCTS_TCP_CLEANUP_H


#include "../../ESPEasy_common.h"

// clean up tcp connections that are in TIME_WAIT status, to conserve memory
// In future versions of WiFiClient it should be possible to call abort(), but
// this feature is not in all upstream versions yet.
// See https://github.com/esp8266/Arduino/issues/1923
// and https://github.com/letscontrolit/ESPEasy/issues/253
#if defined(ESP8266)
  #include <md5.h>
#endif
#if defined(ESP8266)

struct tcp_pcb;
extern struct tcp_pcb* tcp_tw_pcbs;
extern "C" void tcp_abort (struct tcp_pcb* pcb);

void tcpCleanup();
#endif



#endif // DATASTRUCTS_TCP_CLEANUP_H

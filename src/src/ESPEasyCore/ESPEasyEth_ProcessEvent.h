#ifndef ESPEASYCORE_ESPEASYETH_PROCESSEVENT_H
#define ESPEASYCORE_ESPEASYETH_PROCESSEVENT_H

#include "../../ESPEasy_common.h"

#if FEATURE_ETHERNET
void handle_unprocessedEthEvents();

void check_Eth_DNS_valid();

void processEthernetConnected();
void processEthernetDisconnected();
void processEthernetGotIP();
#endif // if FEATURE_ETHERNET

#endif // ifndef ESPEASYCORE_ESPEASYETH_PROCESSEVENT_H

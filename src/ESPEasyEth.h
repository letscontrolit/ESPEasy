#ifdef HAS_ETHERNET
#ifndef ESPEASY_ETH_H
#define ESPEASY_ETH_H

#include "ESPEasy_common.h"

bool ethUseStaticIP();
void ethSetupStaticIPconfig();
bool ethCheckSettings();
bool ethPrepare();
void ethPrintSettings();
bool ETHConnectRelaxed();
bool ETHConnected();
uint8_t * ETHMacAddress(uint8_t* mac);

#endif // ESPEASY_ETH_H
#endif
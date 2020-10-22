#ifndef ESPEASY_ETH_H
#define ESPEASY_ETH_H

#include "../../ESPEasy_common.h"

#ifdef HAS_ETHERNET


bool     ethUseStaticIP();
void     ethSetupStaticIPconfig();
bool     ethCheckSettings();
bool     ethPrepare();
void     ethPrintSettings();
bool     ETHConnectRelaxed();
bool     ETHConnected();
uint8_t* ETHMacAddress(uint8_t *mac);

#endif // ifdef HAS_ETHERNET
#endif // ifndef ESPEASY_ETH_H

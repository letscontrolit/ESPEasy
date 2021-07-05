#ifndef ESPEASY_ETH_H
#define ESPEASY_ETH_H

#include "../../ESPEasy_common.h"

#ifdef HAS_ETHERNET

#include "../DataStructs/MAC_address.h"

bool     ethUseStaticIP();
void     ethSetupStaticIPconfig();
bool     ethCheckSettings();
bool     ethPrepare();
void     ethPrintSettings();
bool     ETHConnectRelaxed();
bool     ETHConnected();
MAC_address ETHMacAddress();

#endif // ifdef HAS_ETHERNET
#endif // ifndef ESPEASY_ETH_H

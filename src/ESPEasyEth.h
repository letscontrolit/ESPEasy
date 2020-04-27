#ifndef ESPEASY_ETH_H
#define ESPEASY_ETH_H

#include "ESPEasy_common.h"

bool ethUseStaticIP();
void ethSetupStaticIPconfig();
bool ethCheckSettings();
bool ethPrepare();
String ethGetDebugClockModeStr();
String ethGetDebugEthWifiModeStr();
void ethPrintSettings();
void ETHConnectRelaxed();
bool ETHConnected();

#endif // ESPEASY_ETH_H
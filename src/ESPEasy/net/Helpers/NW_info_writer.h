#pragma once

#include "../../../ESPEasy_common.h"

#include "../../../src/Helpers/KeyValueWriter.h"

#include "../DataTypes/NetworkIndex.h"

#if defined(USES_NW003) || defined(USES_NW004)
#include "../DataTypes/EthernetParameters.h"
#include <ETH.h>
#endif

namespace ESPEasy {
namespace net {

#ifdef ESP32
bool write_NetworkAdapterFlags(ESPEasy::net::networkIndex_t networkindex,
                               KeyValueWriter              *writer);
#ifndef LIMIT_BUILD_SIZE
bool write_NetworkAdapterPort(ESPEasy::net::networkIndex_t networkindex,
                              KeyValueWriter              *writer);
#endif

bool write_IP_config(ESPEasy::net::networkIndex_t networkindex,
                     KeyValueWriter              *writer);
#endif // ifdef ESP32

bool write_NetworkConnectionInfo(ESPEasy::net::networkIndex_t networkindex,
                                 KeyValueWriter              *writer);

#if defined(USES_NW003) || defined(USES_NW004)

bool write_Eth_Show_Connected(const ETHClass& eth,
                              KeyValueWriter *writer);

bool write_Eth_HW_Address(const ESPEasy::net::EthPhyType_t phyType,
    const ETHClass* eth,
                          KeyValueWriter *writer);

#endif // if defined(USES_NW003) || defined(USES_NW004)

bool write_NetworkPort(const __FlashStringHelper*labels[], const int pins[], size_t nrElements, KeyValueWriter *writer);

} // namespace net
} // namespace ESPEasy

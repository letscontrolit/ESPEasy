#include "../PluginStructs/P089_data_struct.h"

#ifdef USES_P089


# include "../Helpers/Networking.h"
# include "../Helpers/_Plugin_init.h"

# include "../../ESPEasy/net/ESPEasyNetwork.h"

# ifdef ESP32
P089_data_struct::P089_data_struct() {
  if (nullptr == P089_ping_service) {
    P089_ping_service = new (std::nothrow) P089_ping_service_struct();
    addLog(LOG_LEVEL_INFO, F("PING : Starting ping service."));
  }

  if ((nullptr != P089_ping_service) && P089_ping_service->isInitialized()) {
    addLog(LOG_LEVEL_INFO, F("PING : Increment task instance counter."));
    P089_ping_service->increment();
  }
}

P089_data_struct::~P089_data_struct() {
  if (nullptr != P089_ping_service) {
    addLog(LOG_LEVEL_INFO, F("PING : Decrement task instance counter."));

    if (0 == P089_ping_service->decrement()) {
      addLog(LOG_LEVEL_INFO, F("PING : Stopping ping service."));
      delete P089_ping_service;
      P089_ping_service = nullptr;
    }
  }
}

bool P089_data_struct::send_ping(struct EventStruct *event) {
  /* This ping lost for sure */
  if (!isInitialized() || !ESPEasy::net::NetworkConnected()) {
    return true;
  }

  if ((nullptr == P089_ping_service) ||
      !P089_ping_service->isInitialized()) {
    return true; // Service not available
  }

  if (P089_ping_service->getPingResult(event->TaskIndex, _ping_request)) {
    // Get requested result

    if (_ping_request.status == P089_request_status::Result) {
      if (_ping_request.result) {
        addLog(LOG_LEVEL_INFO, strformat(F("PING : Successfully pinged %s (%s) count: %d avg: %.03f ms for task: %d"),
                                         _ping_request.hostname.c_str(),
                                         _ping_request.ip.toString().c_str(),
                                         _ping_request.count,
                                         _ping_request.avgTime,
                                         event->TaskIndex + 1));
      } else {
        addLog(LOG_LEVEL_ERROR, strformat(F("PING : Error pinging %s (%s) for task: %d"),
                                          _ping_request.hostname.c_str(),
                                          _ping_request.ip.toString().c_str(),
                                          event->TaskIndex + 1));
      }
      UserVar.setFloat(event->TaskIndex, 1, _ping_request.avgTime); // Set always, even when not specifically enabled

      _ping_request.status = P089_request_status::Finished;         // Ready for new work
      return !_ping_request.result;                                 // Inverted result to report back!
    }
  }

  if ((_ping_request.status > P089_request_status::Request) && (_ping_request.status < P089_request_status::Result)) {
    // addLog(LOG_LEVEL_INFO, F("PING : Still working."));
    return false; // Busy, but not a failure
  }

  // New request

  IPAddress ip;
  char hostname[PLUGIN_089_HOSTNAME_SIZE]{};

  LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&hostname, PLUGIN_089_HOSTNAME_SIZE);

  /* This one lost as well, DNS dead? */
  if (!resolveHostByName(hostname, ip)) {
    return true;
  }

  int16_t pingCount = P089_PING_COUNT;

  if ((pingCount < 1) || (pingCount > P089_MAX_PING_COUNT)) {
    pingCount = 5;
  }

  _ping_request.status   = P089_request_status::Request;
  _ping_request.count    = pingCount;
  _ping_request.ip       = ip;
  _ping_request.hostname = hostname; // Bring all data to the request
  const bool result = !P089_ping_service->addPingRequest(event->TaskIndex, _ping_request);
  P089_ping_service->loop();         // Kick-off if not already working
  return result;
}

bool P089_data_struct::loop() {
  if ((nullptr != P089_ping_service) && P089_ping_service->isInitialized()) {
    return P089_ping_service->loop();
  }
  return false; // We did nothing
}

# endif // ifdef ESP32

# ifdef ESP8266
P089_data_struct::P089_data_struct() {
  destIPAddress.addr = 0;
  idseq              = 0;

  if (nullptr == P089_data) {
    P089_data = new (std::nothrow) P089_icmp_pcb();

    if (P089_data != nullptr) {
      P089_data->m_IcmpPCB = raw_new(IP_PROTO_ICMP);
      raw_recv(P089_data->m_IcmpPCB, PingReceiver, nullptr);
      raw_bind(P089_data->m_IcmpPCB, IP_ADDR_ANY);
    }
  } else {
    P089_data->instances++;
  }
}

P089_data_struct::~P089_data_struct() {
  if (P089_data != nullptr) {
    P089_data->instances--;

    if (P089_data->instances == 0) {
      raw_remove(P089_data->m_IcmpPCB);
      delete P089_data;
      P089_data = nullptr;
    }
  }
}

bool P089_data_struct::send_ping(struct EventStruct *event) {
  bool is_failure = false;
  IPAddress ip;

  // Do we have unanswered pings? If we are sending new one, this means old one is lost
  if (destIPAddress.addr != 0) {
    is_failure = true;
  }

  /* This ping lost for sure */
  if (!ESPEasy::net::NetworkConnected()) {
    return true;
  }

  char hostname[PLUGIN_089_HOSTNAME_SIZE];

  LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&hostname, PLUGIN_089_HOSTNAME_SIZE);

  /* This one lost as well, DNS dead? */
  if (!resolveHostByName(hostname, ip)) {
    return true;
  }
  destIPAddress.addr = ip;

  /* Generate random ID & seq */
  idseq = HwRandom();
  u16_t ping_len            = ICMP_PAYLOAD_LEN + sizeof(struct icmp_echo_hdr);
  struct pbuf *packetBuffer = pbuf_alloc(PBUF_IP, ping_len, PBUF_RAM);

  /* Lost for sure, TODO: Might be good to log such failures, this means we are short on ram? */
  if (packetBuffer == nullptr) {
    return true;
  }

  struct icmp_echo_hdr *echoRequestHeader = (struct icmp_echo_hdr *)packetBuffer->payload;

  ICMPH_TYPE_SET(echoRequestHeader, ICMP_ECHO);
  ICMPH_CODE_SET(echoRequestHeader, 0);
  echoRequestHeader->chksum = 0;
  echoRequestHeader->id     = (uint16_t)((idseq & 0xffff0000) >> 16);
  echoRequestHeader->seqno  = (uint16_t)(idseq & 0xffff);
  size_t icmpHeaderLen = sizeof(struct icmp_echo_hdr);
  size_t icmpDataLen   = ping_len - icmpHeaderLen;
  char   dataByte      = 0x61;

  for (size_t i = 0; i < icmpDataLen; i++) {
    ((char *)echoRequestHeader)[icmpHeaderLen + i] = dataByte;
    ++dataByte;

    if (dataByte > 0x77) // 'w' character
    {
      dataByte = 0x61;
    }
  }
  echoRequestHeader->chksum = inet_chksum(echoRequestHeader, ping_len);
  ip_addr_t destIPAddress;

  destIPAddress.addr = ip;
  raw_sendto(P089_data->m_IcmpPCB, packetBuffer, &destIPAddress);

  pbuf_free(packetBuffer);

  return is_failure;
}

uint8_t PingReceiver(void *origin, struct raw_pcb *pcb, struct pbuf *packetBuffer, const ip_addr_t *addr)
{
  if ((packetBuffer == nullptr) || (addr == nullptr)) {
    return 0;
  }

  if (packetBuffer->len < sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr) + ICMP_PAYLOAD_LEN) {
    return 0;
  }

  // TODO: Check some ipv4 header values?
  // struct ip_hdr * ip = (struct ip_hdr *)packetBuffer->payload;

  if (pbuf_header(packetBuffer, -PBUF_IP_HLEN) != 0) {
    return 0;
  }

  // After the IPv4 header, we can access the icmp echo header
  struct icmp_echo_hdr *icmp_hdr = (struct icmp_echo_hdr *)packetBuffer->payload;

  // Is it echo reply?
  if (icmp_hdr->type != 0) {
    pbuf_header(packetBuffer, PBUF_IP_HLEN);
    return 0;
  }

  bool is_found = false;

  for (taskIndex_t index = 0; index < TASKS_MAX; index++) {
    deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(index);

    // Match all ping plugin instances and check them
    constexpr pluginID_t PLUGIN_ID_P089_PING(PLUGIN_ID_089);

    if (getPluginID_from_DeviceIndex(deviceIndex) == PLUGIN_ID_P089_PING) {
      P089_data_struct *P089_taskdata = static_cast<P089_data_struct *>(getPluginTaskData(index));

      if ((P089_taskdata != nullptr) && (icmp_hdr->id == (uint16_t)((P089_taskdata->idseq & 0xffff0000) >> 16)) &&
          (icmp_hdr->seqno == (uint16_t)(P089_taskdata->idseq & 0xffff))) {
        UserVar.setFloat(index, 0, 0); // Reset "fails", we got reply
        P089_taskdata->idseq              = 0;
        P089_taskdata->destIPAddress.addr = 0;
        is_found                          = true;
      }
    }
  }

  if (!is_found) {
    pbuf_header(packetBuffer, PBUF_IP_HLEN);
    return 0;
  }

  // Everything fine, release the kraken, ehm, buffer
  pbuf_free(packetBuffer);
  return 1;
}

# endif // ifdef ESP8266
#endif // ifdef USES_P089

#include "../PluginStructs/P089_data_struct.h"

#if defined(USES_P089) && defined(ESP8266)


#include "../Helpers/Networking.h"

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
  if (!NetworkConnected()) {
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
  idseq = random(UINT32_MAX);
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
    if (validDeviceIndex(deviceIndex) && (DeviceIndex_to_Plugin_id[deviceIndex] == PLUGIN_ID_089)) {
      P089_data_struct *P089_taskdata = static_cast<P089_data_struct *>(getPluginTaskData(index));

      if ((P089_taskdata != nullptr) && (icmp_hdr->id == (uint16_t)((P089_taskdata->idseq & 0xffff0000) >> 16)) &&
          (icmp_hdr->seqno == (uint16_t)(P089_taskdata->idseq & 0xffff))) {
        UserVar[index * VARS_PER_TASK]    = 0; // Reset "fails", we got reply
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

#endif // if defined(USES_P089) && defined(ESP8266)

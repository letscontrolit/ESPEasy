#ifdef USES_P088
//#######################################################################################################
//#################### Plugin 088 ICMP Ping probing ##############
//#######################################################################################################
/*
   Ping tests for hostnames and ips
   written by https://github.com/nuclearcat
   Useful to detect strange wifi failures (when it stays connected but not able to reach any ip)
   and to test other devices for reachability (this is why SendDataOption is enabled)
   Maintainer: Denys Fedoryshchenko, denys AT nuclearcat.com
*/

extern "C"
{
#include <lwip/raw.h>
#include <lwip/icmp.h> // needed for icmp packet definitions
#include <lwip/inet_chksum.h> // needed for inet_chksum()
#include <lwip/sys.h> // needed for sys_now()
#include <lwip/netif.h>
//#include "ESP8266WiFi.h" // needed for WiFi.hostByName()
}

#define PLUGIN_088
#define PLUGIN_ID_088             88
#define PLUGIN_NAME_088           "Communication - Ping"
#define PLUGIN_VALUENAME1_088     "Fails"
#define PLUGIN_088_HOSTNAME_SIZE  64
#define PLUGIN_088_MAX_INSTANCES  8

/* Most efficient data packing if we order from place elements from largest to smallest */
struct P088_data_struct {
  ip_addr_t destIPAddress[TASKS_MAX];
  uint32_t idseq[TASKS_MAX];
  struct raw_pcb *m_IcmpPCB = NULL;
  uint8_t instances;
};

struct P088_data_struct *P088_data = NULL;

boolean Plugin_088(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
  case PLUGIN_DEVICE_ADD:
  {
    Device[++deviceCount].Number = PLUGIN_ID_088;
    Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
    Device[deviceCount].VType = DEVICE_TYPE_SINGLE;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].ValueCount = 1;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].DecimalsOnly = true;
    Device[deviceCount].FormulaOption = false;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    break;
  }

  case PLUGIN_GET_DEVICENAME:
  {
    //return the device name
    string = F(PLUGIN_NAME_088);
    break;
  }
  case PLUGIN_GET_DEVICEVALUENAMES:
  {
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_088));
    break;
  }

  case PLUGIN_WEBFORM_LOAD:
  {
    char hostname[PLUGIN_088_HOSTNAME_SIZE];
    LoadCustomTaskSettings(event->TaskIndex, (byte*)&hostname, PLUGIN_088_HOSTNAME_SIZE);
    addFormTextBox(String(F("Hostname")), F("p088_ping_host"), hostname, PLUGIN_088_HOSTNAME_SIZE - 2);
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE:
  {
    char hostname[PLUGIN_088_HOSTNAME_SIZE];
    strncpy(hostname,  WebServer.arg(F("p088_ping_host")).c_str() , sizeof(hostname));
    SaveCustomTaskSettings(event->TaskIndex, (byte*)&hostname, PLUGIN_088_HOSTNAME_SIZE);
    success = true;
    break;
  }

  case PLUGIN_INIT:
  {
    if (!P088_data) {
      P088_data = new P088_data_struct();
      P088_data->m_IcmpPCB = raw_new(IP_PROTO_ICMP);
      raw_recv(P088_data->m_IcmpPCB, PingReceiver, NULL);
      raw_bind(P088_data->m_IcmpPCB, IP_ADDR_ANY);
      for (int i = 0; i < PLUGIN_088_MAX_INSTANCES; i++) {
        memset(&P088_data->destIPAddress[i], 0x0, sizeof(P088_data->destIPAddress[0]));
        memset(&P088_data->idseq[i], 0x0, sizeof(P088_data->idseq[0]));
      }
      P088_data->instances = 1;
    } else {
      P088_data->instances++;
    }
    UserVar[event->BaseVarIndex] = 0;
    success = true;
    break;
  }

  case PLUGIN_EXIT:
  {
    P088_data->instances--;
    if (P088_data->instances == 0) {
      raw_remove(P088_data->m_IcmpPCB);
      delete P088_data;
    }
    break;
  }

  case PLUGIN_READ:
  {
    char hostname[PLUGIN_088_HOSTNAME_SIZE];
    IPAddress ip;

    // Does we have unanswered pings?
    if (P088_data->destIPAddress[event->TaskIndex].addr != 0) {
      UserVar[event->BaseVarIndex]++;
    }

    success = true;
    if (!WiFiConnected()) {
      break;
    }
    LoadCustomTaskSettings(event->TaskIndex, (byte*)&hostname, PLUGIN_088_HOSTNAME_SIZE);

    if (WiFi.hostByName(hostname, ip) == false) {
      break;
    }

    P088_data->destIPAddress[event->TaskIndex].addr = ip;

    /* Generate random ID & seq */
    P088_data->idseq[event->TaskIndex] = random(UINT32_MAX);
    u16_t ping_len = 32 + sizeof(struct icmp_echo_hdr);
    struct pbuf *packetBuffer = pbuf_alloc(PBUF_IP, ping_len, PBUF_RAM);
    if (packetBuffer == NULL) {
      break;
    }

    struct icmp_echo_hdr * echoRequestHeader = (struct icmp_echo_hdr *)packetBuffer->payload;
    ICMPH_TYPE_SET(echoRequestHeader, ICMP_ECHO);
    ICMPH_CODE_SET(echoRequestHeader, 0);
    echoRequestHeader->chksum = 0;
    echoRequestHeader->id = (uint16_t)((P088_data->idseq[event->TaskIndex] & 0xffff0000) >> 16 );
    echoRequestHeader->seqno = (uint16_t)(P088_data->idseq[event->TaskIndex] & 0xffff);
    size_t icmpHeaderLen = sizeof(struct icmp_echo_hdr);
    size_t icmpDataLen = ping_len - icmpHeaderLen;
    char dataByte = 0x61;
    for (size_t i = 0; i < icmpDataLen; i++) {
      ((char*)echoRequestHeader)[icmpHeaderLen + i] = dataByte;
      ++dataByte;
      if (dataByte > 0x77) // 'w' character
      {
        dataByte = 0x61;
      }
    }
    echoRequestHeader->chksum = inet_chksum(echoRequestHeader, ping_len);
    ip_addr_t destIPAddress;
    destIPAddress.addr = ip;
    raw_sendto(P088_data->m_IcmpPCB, packetBuffer, &destIPAddress);

    pbuf_free(packetBuffer);

    break;
  }

  case PLUGIN_WRITE:
  {
    String command = parseString(string, 1);
    if (command == F("pingset"))
    {
      String taskName = parseString(string, 2);
      int8_t taskIndex = getTaskIndexByName(taskName);
      if (taskIndex != -1 && taskIndex == event->TaskIndex) {
        success = true;
        String param1 = parseString(string, 3);
        int val_new;
        if (validIntFromString(param1, val_new)) {
          // Avoid overflow and weird values
          if (val_new > -1024 && val_new < 1024) {
            UserVar[event->BaseVarIndex] = val_new;
          }
        }
      }
    }
    break;
  }
  }
  return success;
}

uint8_t PingReceiver (void *origin, struct raw_pcb *pcb, struct pbuf *packetBuffer, const ip_addr_t *addr)
{
  if (packetBuffer == nullptr || addr == nullptr)
    return 0;
  // Save IPv4 header structure to read ttl value
  struct ip_hdr * ip = (struct ip_hdr *)packetBuffer->payload;
  if (ip == nullptr)
  {
    return 0;
  }

  if (pbuf_header(packetBuffer, -PBUF_IP_HLEN) != 0)
  {
    return 0;
  }

  // After the IPv4 header, one can access the icmp echo header
  struct icmp_echo_hdr * icmp_hdr = (struct icmp_echo_hdr *)packetBuffer->payload;
  if (icmp_hdr == nullptr)
  {
    // Restore original position of ->payload pointer
    pbuf_header(packetBuffer, PBUF_IP_HLEN);
    return 0;
  }

  uint8_t index;

  for (index = 0; index < TASKS_MAX; index++) {
    // Does IP match?
    if (P088_data->destIPAddress[index].addr == addr->addr) {
      // Verify also seqno and id
      if (icmp_hdr->id == (uint16_t)((P088_data->idseq[index] & 0xffff0000) >> 16 ) &&
          icmp_hdr->seqno == (uint16_t)(P088_data->idseq[index] & 0xffff) ) {

        P088_data->destIPAddress[index].addr = 0;
        P088_data->idseq[index] = 0;
        // FIXME? Do we need atomic/synchronization? I think on ESP32 - definitely
        UserVar[index * VARS_PER_TASK] = 0; // Reset fails, we got reply
        break;
      }
    }
  }

  // Not found, because we reached the end and not breaked in middle
  if (index == TASKS_MAX) {
    pbuf_header(packetBuffer, PBUF_IP_HLEN);
    return 0;
  }

  // Everything fine, release the kraken, ehm, buffer
  pbuf_free(packetBuffer);
  return 1;
}

#endif
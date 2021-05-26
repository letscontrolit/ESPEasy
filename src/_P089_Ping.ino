#include "_Plugin_Helper.h"
#if defined(USES_P089) && defined(ESP8266)
//#######################################################################################################
//#################### Plugin 089 ICMP Ping probing ##############
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
}

#include "src/ESPEasyCore/ESPEasyNetwork.h"


#define PLUGIN_089
#define PLUGIN_ID_089             89
#define PLUGIN_NAME_089           "Communication - Ping"
#define PLUGIN_VALUENAME1_089     "Fails"
#define PLUGIN_089_HOSTNAME_SIZE  64
#define PLUGIN_089_MAX_INSTANCES  8

#define ICMP_PAYLOAD_LEN          32

struct P089_icmp_pcb {
  P089_icmp_pcb() : m_IcmpPCB(nullptr), instances(1) {}
  struct raw_pcb *m_IcmpPCB;
  uint8_t instances; /* Sort of refcount */
};

struct P089_icmp_pcb *P089_data = nullptr;

class P089_data_struct: public PluginTaskData_base {
public:
  P089_data_struct() {
    destIPAddress.addr = 0;
    idseq = 0;
    if (nullptr == P089_data) {
      P089_data = new (std::nothrow) P089_icmp_pcb();
      if (P089_data != nullptr) {
        P089_data->m_IcmpPCB = raw_new(IP_PROTO_ICMP);
        raw_recv(P089_data->m_IcmpPCB, PingReceiver, NULL);
        raw_bind(P089_data->m_IcmpPCB, IP_ADDR_ANY);
      }
    } else {
      P089_data->instances++;
    }
  }

  ~P089_data_struct() {
      if (P089_data != nullptr) {
        P089_data->instances--;
        if (P089_data->instances == 0) {
          raw_remove(P089_data->m_IcmpPCB);
          delete P089_data;
          P089_data = nullptr;
        }
      }
  }

  ip_addr_t destIPAddress;
  uint32_t idseq;

  bool send_ping(struct EventStruct *event) {
    bool is_failure = false;
    IPAddress ip;

    // Do we have unanswered pings? If we are sending new one, this means old one is lost
    if (destIPAddress.addr != 0)
      is_failure = true;

    /* This ping lost for sure */
    if (!NetworkConnected()) {
      return true;
    }

    char hostname[PLUGIN_089_HOSTNAME_SIZE];
    LoadCustomTaskSettings(event->TaskIndex, (byte*)&hostname, PLUGIN_089_HOSTNAME_SIZE);

    /* This one lost as well, DNS dead? */
    if (WiFi.hostByName(hostname, ip) == false) {
      return true;
    }
    destIPAddress.addr = ip;

    /* Generate random ID & seq */
    idseq = random(UINT32_MAX);
    u16_t ping_len = ICMP_PAYLOAD_LEN + sizeof(struct icmp_echo_hdr);
    struct pbuf *packetBuffer = pbuf_alloc(PBUF_IP, ping_len, PBUF_RAM);
    /* Lost for sure, TODO: Might be good to log such failures, this means we are short on ram? */
    if (packetBuffer == NULL) {
      return true;
    }

    struct icmp_echo_hdr * echoRequestHeader = (struct icmp_echo_hdr *)packetBuffer->payload;
    ICMPH_TYPE_SET(echoRequestHeader, ICMP_ECHO);
    ICMPH_CODE_SET(echoRequestHeader, 0);
    echoRequestHeader->chksum = 0;
    echoRequestHeader->id = (uint16_t)((idseq & 0xffff0000) >> 16 );
    echoRequestHeader->seqno = (uint16_t)(idseq & 0xffff);
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
    raw_sendto(P089_data->m_IcmpPCB, packetBuffer, &destIPAddress);

    pbuf_free(packetBuffer);

    return is_failure;
  }
};

boolean Plugin_089(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
  case PLUGIN_DEVICE_ADD:
  {
    Device[++deviceCount].Number = PLUGIN_ID_089;
    Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
    Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;
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
    string = F(PLUGIN_NAME_089);
    break;
  }
  case PLUGIN_GET_DEVICEVALUENAMES:
  {
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_089));
    break;
  }

  case PLUGIN_WEBFORM_LOAD:
  {
    char hostname[PLUGIN_089_HOSTNAME_SIZE];
    LoadCustomTaskSettings(event->TaskIndex, (byte*)&hostname, PLUGIN_089_HOSTNAME_SIZE);
    addFormTextBox(F("Hostname"), F("p089_ping_host"), hostname, PLUGIN_089_HOSTNAME_SIZE - 2);
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE:
  {
    char hostname[PLUGIN_089_HOSTNAME_SIZE];
    // Reset "Fails" if settings updated
    UserVar[event->BaseVarIndex] = 0;
    strncpy(hostname,  webArg(F("p089_ping_host")).c_str() , sizeof(hostname));
    SaveCustomTaskSettings(event->TaskIndex, (byte*)&hostname, PLUGIN_089_HOSTNAME_SIZE);
    success = true;
    break;
  }

  case PLUGIN_INIT:
  {
    initPluginTaskData(event->TaskIndex, new (std::nothrow) P089_data_struct());
    UserVar[event->BaseVarIndex] = 0;
    success = true;
    break;
  }

  case PLUGIN_READ:
  {
    P089_data_struct *P089_taskdata =
      static_cast<P089_data_struct *>(getPluginTaskData(event->TaskIndex));
    if (nullptr == P089_taskdata)
      break;

    if (P089_taskdata->send_ping(event))
      UserVar[event->BaseVarIndex]++;

    success = true;
    break;
  }

  case PLUGIN_WRITE:
  {
    String command = parseString(string, 1);
    if (command == F("pingset"))
    {
      String taskName = parseString(string, 2);
      taskIndex_t taskIndex = findTaskIndexByName(taskName);
      if (taskIndex != TASKS_MAX && taskIndex == event->TaskIndex) {
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

  if (packetBuffer->len < sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr) + ICMP_PAYLOAD_LEN)
    return 0;

  // TODO: Check some ipv4 header values?
  // struct ip_hdr * ip = (struct ip_hdr *)packetBuffer->payload;

  if (pbuf_header(packetBuffer, -PBUF_IP_HLEN) != 0)
    return 0;

  // After the IPv4 header, we can access the icmp echo header
  struct icmp_echo_hdr * icmp_hdr = (struct icmp_echo_hdr *)packetBuffer->payload;

  // Is it echo reply?
  if (icmp_hdr->type != 0) {
    pbuf_header(packetBuffer, PBUF_IP_HLEN);
    return 0;
  }

  bool is_found = false;
  for (taskIndex_t index = 0; index < TASKS_MAX; index++) {
    deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(index);
    // Match all ping plugin instances and check them
    if (validDeviceIndex(deviceIndex) && DeviceIndex_to_Plugin_id[deviceIndex] == PLUGIN_ID_089) {
      P089_data_struct *P089_taskdata = static_cast<P089_data_struct *>(getPluginTaskData(index));
      if (P089_taskdata != nullptr && icmp_hdr->id == (uint16_t)((P089_taskdata->idseq & 0xffff0000) >> 16 ) &&
          icmp_hdr->seqno == (uint16_t)(P089_taskdata->idseq & 0xffff) ) {
        UserVar[index * VARS_PER_TASK] = 0; // Reset "fails", we got reply
        P089_taskdata->idseq = 0;
        P089_taskdata->destIPAddress.addr = 0;
        is_found = true;
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

#endif

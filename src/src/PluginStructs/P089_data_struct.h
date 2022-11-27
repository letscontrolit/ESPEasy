#ifndef PLUGINSTRUCTS_P089_DATA_STRUCT_H
#define PLUGINSTRUCTS_P089_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#if defined(USES_P089) && defined(ESP8266)


# define PLUGIN_ID_089             89

# define PLUGIN_089_HOSTNAME_SIZE  64
# define PLUGIN_089_MAX_INSTANCES  8

# define ICMP_PAYLOAD_LEN          32

extern "C"
{
# include <lwip/raw.h>
# include <lwip/icmp.h>        // needed for icmp packet definitions
# include <lwip/inet_chksum.h> // needed for inet_chksum()
# include <lwip/sys.h>         // needed for sys_now()
# include <lwip/netif.h>
}

# include "src/ESPEasyCore/ESPEasyNetwork.h"


struct P089_icmp_pcb {
  P089_icmp_pcb() = default;
  struct raw_pcb *m_IcmpPCB = nullptr;
  uint8_t         instances = 1; /* Sort of refcount */
};

class P089_data_struct : public PluginTaskData_base {
public:

  P089_data_struct();

  virtual ~P089_data_struct();

  bool send_ping(struct EventStruct *event);

  struct P089_icmp_pcb *P089_data = nullptr;
  ip_addr_t destIPAddress;
  uint32_t idseq;
};

// Callback function
uint8_t PingReceiver(void            *origin,
                     struct raw_pcb  *pcb,
                     struct pbuf     *packetBuffer,
                     const ip_addr_t *addr);


#endif // if defined(USES_P089) && defined(ESP8266)
#endif // ifndef PLUGINSTRUCTS_P089_DATA_STRUCT_H

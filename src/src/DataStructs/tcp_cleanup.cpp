#include "../DataStructs/tcp_cleanup.h"



#if defined(ESP8266)

struct tcp_pcb;
extern struct tcp_pcb* tcp_tw_pcbs;
extern "C" void tcp_abort (struct tcp_pcb* pcb);

void tcpCleanup()
{

     while(tcp_tw_pcbs!=NULL)
    {
      tcp_abort(tcp_tw_pcbs);
    }

 }
#endif
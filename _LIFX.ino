
typedef struct __attribute__((packed)) {
  // Framce
  uint16_t msgsize;
  uint16_t protocol;
  uint32_t source;    // 32 bits/uint32, unique ID set by client. If zero, broadcast reply
  
  // Frame address
  uint8_t target_mac[8];  // 64 bits/uint64, either single MAC address or all zeroes for broadcast.
  uint8_t reserved1[6];
  uint8_t ctrl;           // 6 first bits are reserved, next bit ack_required, next bit res_required (response message)
  uint8_t seq_num;        // Message sequence number (will be provided in response if response is requested)

  // Protcol Header
  uint64_t reserved2;
  uint16_t packet_type;   // Message type determines the payload being used
  uint16_t reserved3;
  
} lx_header_t;

typedef struct __attribute__((packed)) {
  uint8_t   rfu;
  uint16_t  hue;
  uint16_t  saturation;
  uint16_t  brightness;
  uint16_t  kelvin;
  uint32_t  duration;
} setcolor_102_t;

typedef struct __attribute__((packed)) {
  uint16_t  level;
  uint32_t  duration;
} setpower_117_t;


void getPktSetBulbPower2(uint8_t * data, uint16_t state) {
  lx_header_t * tmp;
 
  tmp = (lx_header_t *)data;

  memset(data, 0, 38);
  tmp->msgsize = 38;

  tmp->protocol = 0x1400;  // PROTOCOL_COMMAND
  tmp->packet_type = 0x15; // Device message : set power state (21 dec)
  memcpy(&tmp->target_mac, Settings.LifxMAC, 6);
  memcpy(&tmp->source, "didi", 4);
  
  // The payload
  memcpy(&data[36], &state, 2);

  print_msg_serial(data, 38);
}


uint16_t getPktSetBulbPower(uint8_t * data, int state, uint32_t duration) {
  lx_header_t * tmp;
  setpower_117_t * tmp2;

  tmp = (lx_header_t *)data;

  memset(data, 0, 42);
  
  tmp->msgsize = 42; // 36 (header) + 6 (setpower payload)

  tmp->protocol = 0x1400;  // PROTOCOL_COMMAND
  tmp->packet_type = 117; // Light message : SetPower - 117 (0x75)
  memcpy(&tmp->target_mac, Settings.LifxMAC, 6);
  memcpy(&tmp->source, "didi", 4);
  
  // The payload : https://lan.developer.lifx.com/docs/light-messages#section-setpower-117
  tmp2 = (setpower_117_t *)&data[36];
  tmp2->level = state;
  tmp2->duration = duration;

  print_msg_serial(data, tmp->msgsize);
  
  return tmp->msgsize;
}


uint16_t getPktSetBulbColor(uint8_t * data, uint16_t h, uint16_t s, uint16_t b, uint16_t k, uint32_t duration) {
  lx_header_t * tmp;
  setcolor_102_t * tmp2;

  tmp = (lx_header_t *)data;

  memset(data, 0, 49);

  tmp->msgsize = 49; // 36 (header) + 13 (setcolor payload)
  
  tmp->protocol = 0x1400;  // PROTOCOL_COMMAND
  tmp->packet_type = 102; // Light message : SetColor - 102 (0x66)
  memcpy(&tmp->target_mac, Settings.LifxMAC, 6);
  memcpy(&tmp->source, "didi", 4);
  
  // The payload HSBK : https://lan.developer.lifx.com/docs/light-messages#section-hsbk
  tmp2 = (setcolor_102_t *)&data[36];
  tmp2->duration = duration;
  tmp2->hue = h;
  tmp2->saturation = s;
  tmp2->brightness = b;
  tmp2->kelvin = k;

  print_msg_serial(data, tmp->msgsize);
  
  return tmp->msgsize;
}

void print_msg_serial(uint8_t * data, uint16_t len) {
   char str[10];
   for (int i=0 ; i<len; i++) {
      sprintf_P(str, PSTR("%02x "), data[i]);
      if ( (i+1) % 8 == 0 )
        Serial.println(str);
      else
        Serial.print(str);
   }
   Serial.println("");
}


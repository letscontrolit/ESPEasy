#ifdef SWSERIALDEBUG
#define TX_DELAY                112
#define XMIT_START_ADJUSTMENT     4

uint8_t _transmitBitMask;
volatile uint8_t *_transmitPortRegister;

void SoftwareSerial_print(char* string)
{
  byte x=0;
  while (string[x] != 0)
  {
    SoftwareSerial_write(string[x]);
    x++;
  }
}

void SoftwareSerial_println(char* string)
{
  SoftwareSerial_print(string);
  SoftwareSerial_write(13);
  SoftwareSerial_write(10);
}

void SoftwareSerial_init(byte transmitPin)
{
  SoftwareSerial_setTX(transmitPin);
  SoftwareSerial_tunedDelay(TX_DELAY); // if we were low this establishes the end
}

inline void SoftwareSerial_tunedDelay(uint16_t delay) { 
  uint8_t tmp=0;

  asm volatile("sbiw    %0, 0x01 \n\t"
    "ldi %1, 0xFF \n\t"
    "cpi %A0, 0xFF \n\t"
    "cpc %B0, %1 \n\t"
    "brne .-10 \n\t"
    : "+r" (delay), "+a" (tmp)
    : "0" (delay)
    );
}

void SoftwareSerial_tx_pin_write(uint8_t pin_state)
{
  if (pin_state == LOW)
    *_transmitPortRegister &= ~_transmitBitMask;
  else
    *_transmitPortRegister |= _transmitBitMask;
}

void SoftwareSerial_setTX(uint8_t tx)
{
  pinMode(tx, OUTPUT);
  digitalWrite(tx, HIGH);
  _transmitBitMask = digitalPinToBitMask(tx);
  uint8_t port = digitalPinToPort(tx);
  _transmitPortRegister = portOutputRegister(port);
}

size_t SoftwareSerial_write(uint8_t b)
{
  // DISABLED: conflicts with I2C
  //uint8_t oldSREG = SREG;
  //cli();  // turn off interrupts for a clean txmit

  // Write the start bit
  SoftwareSerial_tx_pin_write(LOW);
  SoftwareSerial_tunedDelay(TX_DELAY + XMIT_START_ADJUSTMENT);

  // Write each of the 8 bits
  for (byte mask = 0x01; mask; mask <<= 1)
  {
    if (b & mask) // choose bit
      SoftwareSerial_tx_pin_write(HIGH); // send 1
    else
      SoftwareSerial_tx_pin_write(LOW); // send 0
  
    SoftwareSerial_tunedDelay(TX_DELAY);
  }
  SoftwareSerial_tx_pin_write(HIGH); // restore pin to natural state

  // DISABLED: conflicts with I2C
  // SREG = oldSREG; // turn interrupts back on
  
  SoftwareSerial_tunedDelay(TX_DELAY);
  
  return 1;
}
#endif

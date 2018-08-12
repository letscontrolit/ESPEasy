#include <Arduino.h>
#include <HeatpumpIR.h>

#ifndef ESP8266
IRSenderPWM irSender(9);       // IR led on Arduino digital pin 9, using Arduino PWM
//IRSenderBlaster irSender(3); // IR led on Arduino digital pin 3, using IR Blaster (generates the 38 kHz carrier)
#else
IRSenderBitBang irSender(D1);  // IR led on Wemos D1 mini pin 'D1'
#endif


// This sketch can be used to send 'raw' IR signals, like this:
// Hh001101011010111100000100101001010100000000000111000000001111111011010100000001000100101000
//
// Adjust these timings based on the heatpump / A/C model
#define IR_ONE_SPACE    2080
#define IR_ZERO_SPACE   780
#define IR_BIT_MARK     426
#define IR_PAUSE_SPACE  30000
#define IR_HEADER_MARK  6680
#define IR_HEADER_SPACE 3140


void setup()
{
  Serial.begin(9600);
  delay(500);

  Serial.println(F("Starting..."));
}

void loop()
{
  char symbols[1024];
  char *symbolsPtr = symbols;
  int currentpulse=0;
  memset(symbols, 0, sizeof(symbols));

  Serial.println(F("Ready to send symbols."));

  while ((currentpulse = Serial.readBytesUntil('\n', symbols, sizeof(symbols)-1)) == 0)
  {}

  sendRaw(symbols);
}

void sendRaw(char *symbols)
{
  irSender.space(0);
  irSender.setFrequency(38);

  while (char symbol = *symbols++)
  {
    switch (symbol)
    {
      case '1':
        irSender.space(IR_ONE_SPACE);
        irSender.mark(IR_BIT_MARK);
        break;
      case '0':
        irSender.space(IR_ZERO_SPACE);
        irSender.mark(IR_BIT_MARK);
        break;
      case 'W':
        irSender.mark(IR_PAUSE_SPACE);
        break;
      case 'H':
        irSender.mark(IR_HEADER_MARK);
        break;
      case 'h':
        irSender.mark(IR_HEADER_SPACE);
        break;
    }
  }

  irSender.space(0);
}

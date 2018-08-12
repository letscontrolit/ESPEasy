#include <Arduino.h>
#include <HeatpumpIR.h>       // https://github.com/ToniA/arduino-heatpumpir
#include <Button.h>           // http://playground.arduino.cc/Code/Button

/*

This is a project I did for my my friend's summer cottage (he was previously using the MideaRelayControl sketch). 
He has an alarm system with some extra relays which can be controlled via GSM SMS messages. With two relays
we can have four heatpump states:
* relay 1 OFF and relay 2 in any state: heatpump OFF
* relay 1 ON and relay 2 OFF: heatpump ON, normal heating state
* relay 1 ON and relay 2 ON: heatpump ON, maintenance heating (FP mode on Midea's remote)

The IR led is driven by a transistor amplifier. I used 2 ohms as the resistor. I might sound
very small, but also the led operates on very short bursts. For example Panasonic uses 2 ohm
resistor for the IR led on its remotes.
*/

#define IR_HEADER_MARK   3500
#define IR_HEADER_SPACE  1800
#define IR_BIT_MARK       420
#define IR_ONE_SPACE     1350
#define IR_ZERO_SPACE     470
#define IR_PAUSE_SPACE  10000

// Heatpump states
const byte heatpumpOff         = 0;
const byte heatpumpNormal      = 1;
const byte heatpumpMaintenance = 2;

Button relay1 = Button(11, INPUT_PULLUP); // Heatpump ON-OFF state
Button relay2 = Button(12, INPUT_PULLUP); // FP mode (maintenance heating at 8 degrees C) ON-OFF state

IRSenderPWM irSender(9);     // IR led on Duemilanove digital pin 9, using Arduino PWM

byte heatpumpState = heatpumpOff;

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
        irSender.space(IR_PAUSE_SPACE);
        break;
      case 'H':
        irSender.mark(IR_HEADER_MARK);
        break;
      case 'h':
        irSender.space(IR_HEADER_SPACE);
        irSender.mark(IR_BIT_MARK);
        break;
    }
  }

  irSender.space(0);
}

void setup(){
  Serial.begin(9600);
  delay(500);
  Serial.println(F("Starting..."));
}

void loop(){

  if (relay1.isPressed()) {
    // Normal heating operation
    if (!relay2.isPressed() && heatpumpState != heatpumpNormal) {
      Serial.println(F("Normal heating operation"));

      Serial.println(F("* Sending power OFF"));
      sendRaw("Hh0100000000000100000001110010000000000000000000000000000001100000WHh01000000000001000000011100100000000000000001001000101000000000011000111000000000000000000111000000000111000000000000000010010001000000000000000001010011");
      delay(15 * 1000); // 15 seconds, to allow full shutdown

      Serial.println(F("* Sending normal heat command"));
      sendRaw("Hh0100000000000100000001110010000000000000000000000000000001100000WHh01000000000001000000011100100000000000001001001001010100000000011000010100000000000000000111000000000111000000000000000010010001000000000000000010001000");
      heatpumpState = heatpumpNormal;

    // Maintenance heating operation
    } else if (relay2.isPressed() && heatpumpState != heatpumpMaintenance) {
      Serial.println(F("Maintenance heating operation"));

      Serial.println(F("* Sending power OFF"));
      sendRaw("Hh0100000000000100000001110010000000000000000000000000000001100000WHh01000000000001000000011100100000000000000001001000101000000000011000111000000000000000000111000000000111000000000000000010010001000000000000000001010011");
      delay(15 * 1000); // 15 seconds, to allow full shutdown

      Serial.println(F("* Switching to maintenance heating"));
      sendRaw("Hh0100000000000100000001110010000000000000000000000000000001100000WHh01000000000001000000011100100000000000001001001000101000000000011000111000000000000000000111000000000111000000000000000010010001000000000000000011010011");

      heatpumpState = heatpumpMaintenance;
    }
  // Power OFF
  } else if (!relay1.isPressed() && heatpumpState != heatpumpOff) {
    heatpumpState = heatpumpOff;
    Serial.println(F("Sending power OFF"));
    sendRaw("Hh0100000000000100000001110010000000000000000000000000000001100000WHh01000000000001000000011100100000000000000001001000101000000000011000111000000000000000000111000000000111000000000000000010010001000000000000000001010011");
  }

  delay(500); // 0.5 seconds delay
}

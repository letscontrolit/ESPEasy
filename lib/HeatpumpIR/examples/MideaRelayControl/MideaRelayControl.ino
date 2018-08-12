#include <Arduino.h>
#include <MideaHeatpumpIR.h>       // https://github.com/ToniA/arduino-heatpumpir
#include <Button.h>                // http://playground.arduino.cc/Code/Button

/*

This is a project I did for my my friend's summer cottage. He has an alarm system
with some extra relays which can be controlled via GSM SMS messages. With two relays
we can have four heatpump states:
* relay 1 OFF and relay 2 in any state: heatpump OFF
* relay 1 ON and relay 2 OFF: heatpump ON, normal heating state
* relay 1 ON and relay 2 ON: heatpump ON, maintenance heating (FP mode on Midea's remote)

The heatpump is 'Ultimate 12 Pro Plus Inverter', sold by ultimatemarket.com:
Midea MSR1U-12HRDN1-QRC4W + MOB-12HFN1-QRC4W, with remote control RG51I20/BGE

This same sketch would also control the older 'Ultimate Pro Plus 13FP':
Midea MSR1-12HRN1-QC2 + MOA1-12HN1-QC2 heatpump control, with remote control RG51M1/E

... and very likely many other Midea heatpumps...


The Midea 'FP', or 'maintenance mode', is not really a mode even if the HeatpumpIR library
interface makes it look like a mode. Instead it's a switch between the 'normal' and 'FP'
mode, so to be absolutely sure about the operating mode, we need to turn the heatpump OFF
first, and then turn it on (goes into 'normal' mode), and finally switch into 'FP'.

The IR led is driven by a transistor amplifier. I used 2 ohms as the resistor. I might sound
very small, but also the led operates on very short bursts. For example Panasonic uses 2 ohm
resistor for the IR led on its remotes.
*/

// Heatpump states
const byte heatpumpOff         = 0;
const byte heatpumpNormal      = 1;
const byte heatpumpMaintenance = 2;

Button relay1 = Button(11, INPUT_PULLUP); // Heatpump ON-OFF state
Button relay2 = Button(12, INPUT_PULLUP); // FP mode (maintenance heating at 8 degrees C) ON-OFF state

IRSenderPWM irSender(9);     // IR led on Duemilanove digital pin 9, using Arduino PWM
//IRSenderBlaster irSender(9); // IR led on Duemilanove digital pin 9, using IR Blaster (generates the 38 kHz carrier)

byte heatpumpState = heatpumpOff;

HeatpumpIR *heatpumpIR = new MideaHeatpumpIR();

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
      heatpumpIR->send(irSender, POWER_OFF, MODE_HEAT, FAN_2, 22, VDIR_UP, HDIR_AUTO);
      delay(15 * 1000); // 15 seconds, to allow full shutdown

      Serial.println(F("* Sending normal heat command"));
      heatpumpIR->send(irSender, POWER_ON, MODE_HEAT, FAN_2, 22, VDIR_UP, HDIR_AUTO);
      heatpumpState = heatpumpNormal;

    // Maintenance heating operation
    } else if (relay2.isPressed() && heatpumpState != heatpumpMaintenance) {
      Serial.println(F("Maintenance heating operation"));

      Serial.println(F("* Sending power OFF"));
      heatpumpIR->send(irSender, POWER_OFF, MODE_HEAT, FAN_2, 22, VDIR_UP, HDIR_AUTO);
      delay(15 * 1000); // 15 seconds, to allow full shutdown

      Serial.println(F("* Sending normal heat command"));
      heatpumpIR->send(irSender, POWER_ON, MODE_HEAT, FAN_2, 22, VDIR_UP, HDIR_AUTO);
      delay(5 * 1000); // 15 seconds, to allow full start before switching to maintenance

      Serial.println(F("* Switching to maintenance heating"));
      heatpumpIR->send(irSender, POWER_ON, MODE_MAINT, FAN_3, 10, VDIR_UP, HDIR_AUTO);

      heatpumpState = heatpumpMaintenance;
    }
  // Power OFF
  } else if (!relay1.isPressed() && heatpumpState != heatpumpOff) {
    heatpumpState = heatpumpOff;
    Serial.println(F("Sending power OFF"));
    heatpumpIR->send(irSender, POWER_OFF, MODE_HEAT, FAN_2, 22, VDIR_UP, HDIR_AUTO);
  }

  delay(500); // 0.5 seconds delay
}

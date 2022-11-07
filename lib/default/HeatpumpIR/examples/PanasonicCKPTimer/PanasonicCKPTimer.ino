#include <Arduino.h>

#include <PanasonicCKPHeatpumpIR.h>
#include <Timer.h> // https://github.com/JChristensen/Timer

/*
    This schema demonstrates how to control the Panasonic CKP power state change. The CKP does not have discrete
    'ON' and 'OFF' commands, but only a state switch. So, if the initial power state is now known, switching the
    state does not help much.
    
    Luckily this can be implemented by using the timer:
    * The 'send' command will send all the settings AND program the timer so that the pump will turn ON in a minute
    * The 'sendPanasonicCKPCancelTimer' will cancel the timer
    
    The 'turn OFF' must be implemented the same way.
    
    Of course you can choose to not turn off the timer, but that means that the heatpump will attempt to switch ON
    (or OFF) every day at the same time, as the timer will still be active.
*/


IRSenderPWM irSender(3);     // IR led on Duemilanove digital pin 3, using Arduino PWM
//IRSenderBlaster irSender(3); // IR led on Duemilanove digital pin 3, using IR Blaster (generates the 38 kHz carrier)

PanasonicCKPHeatpumpIR *heatpumpIR;

Timer timer;

void setup()
{
  Serial.begin(9600);
  delay(500);

  heatpumpIR = new PanasonicCKPHeatpumpIR();

  Serial.println("Turning the Panasonic CKP heatpump ON by using the timer");
  heatpumpIR->send(irSender, POWER_ON, MODE_HEAT, FAN_2, 24, VDIR_UP, HDIR_AUTO);
  Serial.println("The heatpump should have beeped, and the TIMER led should be ON");

  timer.after(60000, panasonicIsOn); // Called after 1 minute
  timer.after(120000, panasonicCancelTimer); // Called after 2 minutes

}

void loop()
{
  timer.update();
}

void panasonicIsOn()
{
  Serial.println("The heatpump should should turn ON by now, the TIMER led is still ON");
}

void panasonicCancelTimer()
{
  heatpumpIR->sendPanasonicCKPCancelTimer(irSender);
  Serial.println("The TIMER led should now be OFF");
}
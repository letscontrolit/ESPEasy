#include <Arduino.h>
#include <HisenseHeatpumpIR.h>

IRSenderPWM irSender(3);     // IR led on Duemilanove digital pin 3, using Arduino PWM
//IRSenderBlaster irSender(3); // IR led on Duemilanove digital pin 3, using IR Blaster (generates the 38 kHz carrier)

HisenseHeatpumpIR *heatpumpIR;

void setup()
{
  Serial.begin(9600);
  delay(500);
  heatpumpIR = new HisenseHeatpumpIR();
  Serial.println(F("Starting"));
}

void loop()
{
  const char* buf;

  Serial.print(F("Sending IR to "));
    // Print the model
  buf = heatpumpIR->model();
  // 'model' is a PROGMEM pointer, so need to write a byte at a time
  while (char modelChar = pgm_read_byte(buf++))
  {
    Serial.print(modelChar);
  }
  Serial.print(F(", info: "));
 // Print the info
  buf = heatpumpIR->info();
  // 'info' is a PROGMEM pointer, so need to write a byte at a time
  while (char infoChar = pgm_read_byte(buf++))
  {
    Serial.print(infoChar);
  }
  Serial.println();

  heatpumpIR->send(irSender, POWER_ON, MODE_HEAT, FAN_3, 30, VDIR_AUTO, HDIR_AUTO);
  delay(1500);
  heatpumpIR->send(irSender, POWER_ON, MODE_DRY,  FAN_2, 20, VDIR_AUTO, HDIR_AUTO);
  delay(1500);
  heatpumpIR->send(irSender, POWER_ON, MODE_FAN, FAN_AUTO, 18, VDIR_AUTO, HDIR_AUTO);
  delay(1500);
  heatpumpIR->send(irSender, POWER_ON, MODE_COOL, FAN_1, 18, VDIR_AUTO, HDIR_AUTO);
  delay(2500);
  heatpumpIR->send(irSender, POWER_OFF, MODE_COOL, FAN_1, 18, VDIR_AUTO, HDIR_AUTO);
  delay(1500);
}
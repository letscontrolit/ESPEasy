#include <Arduino.h>
#include <CarrierHeatpumpIR.h>

IRSenderPWM irSender(9);     // IR led on Duemilanove digital pin 3, using Arduino PWM
//IRSenderBlaster irSender(3); // IR led on Duemilanove digital pin 3, using IR Blaster (generates the 38 kHz carrier)

CarrierNQVHeatpumpIR *heatpumpIR;

int redLED = 6;
int orangeLED = 5;
int greenLED = 4;
int blueLED = 3;

void setup()
{
  Serial.begin(9600);
  pinMode(redLED, OUTPUT);
  pinMode(orangeLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  delay(500);
  heatpumpIR = new CarrierNQVHeatpumpIR();
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

  digitalWrite(orangeLED,HIGH);
  delay(4000);
  heatpumpIR->send(irSender, POWER_ON, MODE_HEAT, FAN_AUTO, 24, VDIR_AUTO, HDIR_AUTO);
  digitalWrite(orangeLED,LOW);

  digitalWrite(redLED, HIGH);

  // don't loop()
  for(;;)
    ;
}

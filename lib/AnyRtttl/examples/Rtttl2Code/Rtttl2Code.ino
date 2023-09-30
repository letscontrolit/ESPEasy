#include <anyrtttl.h>
#include <binrtttl.h>
#include <pitches.h>

//project's constants
#define BUZZER_PIN 8
const char * tetris = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";

//*******************************************************************************************************************
//  The following replacement functions prints the function call & parameters to the serial port.
//*******************************************************************************************************************
void serialTone(uint8_t pin, unsigned int frequency, unsigned long duration) {
  Serial.print("tone(");
  Serial.print(pin);
  Serial.print(",");
  Serial.print(frequency);
  Serial.print(",");
  Serial.print(duration);
  Serial.println(");");
}

void serialNoTone(uint8_t pin) {
  Serial.print("noTone(");
  Serial.print(pin);
  Serial.println(");");
}

void serialDelay(unsigned long duration) {
  Serial.print("delay(");
  Serial.print(duration);
  Serial.println(");");
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();

  //Use custom functions
  anyrtttl::setToneFunction(&serialTone);
  anyrtttl::setNoToneFunction(&serialNoTone);
  anyrtttl::setDelayFunction(&serialDelay);
}

void loop() {
  anyrtttl::blocking::play(BUZZER_PIN, tetris);

  while(true)
  {
  }
}

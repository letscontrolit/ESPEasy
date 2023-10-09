#include <anyrtttl.h>
#include <binrtttl.h>
#include <pitches.h>

//project's constants
#define BUZZER_PIN 8
const char tetris[] PROGMEM = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";
const char arkanoid[] PROGMEM = "Arkanoid:d=4,o=5,b=140:8g6,16p,16g.6,2a#6,32p,8a6,8g6,8f6,8a6,2g6";
const char mario[] PROGMEM = "mario:d=4,o=5,b=140:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6";
// James Bond theme defined in inline code below (also stored in flash memory) 

// tone() and noTone() are not implemented for Arduino core for the ESP32
// See https://github.com/espressif/arduino-esp32/issues/980
// and https://github.com/espressif/arduino-esp32/issues/1720
void esp32NoTone(uint8_t pin) {
  // don't care about the given pin
  ledcWrite(0, 0); // channel, volume
}

void esp32Tone(uint8_t pin, unsigned int frq, unsigned long duration) {
  // don't care about the given pin or the duration
  ledcWriteTone(0, frq); // channel, freq
  ledcWrite(0, 255); // channel, volume
}

void esp32ToneSetup(uint8_t pin) {
  ledcSetup(0, 1000, 10); // resolution always seems to be 10bit, no matter what is given
  ledcAttachPin(pin, 0);
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  // setup AnyRtttl for ESP32
  esp32ToneSetup(BUZZER_PIN);
  anyrtttl::setToneFunction(&esp32Tone);
  anyrtttl::setNoToneFunction(&esp32NoTone);

  Serial.begin(115200);

  Serial.println("ready");
}

void loop() {
  anyrtttl::blocking::playProgMem(BUZZER_PIN, tetris);
  delay(1000);

  anyrtttl::blocking::play_P(BUZZER_PIN, arkanoid);
  delay(1000);

#if defined(ESP8266) || defined(ESP32)
  anyrtttl::blocking::play(BUZZER_PIN, FPSTR(mario));
  delay(1000);
#endif

  anyrtttl::blocking::play(BUZZER_PIN, F("Bond:d=4,o=5,b=80:32p,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d#6,16d#6,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d6,16c#6,16c#7,c.7,16g#6,16f#6,g#.6"));
  delay(1000);

  while(true)
  {
  }
}

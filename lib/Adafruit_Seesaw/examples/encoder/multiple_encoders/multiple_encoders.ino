/* Demo with 128x64 OLED display and multiple I2C encoders wired up. The sketch will auto-
 * detect up to 4 encoder on the first 4 addresses. Twisting will display text on OLED
 * and change neopixel color.
 * set USE_OLED to true t
 */

#define USE_OLED false  // set to false to skip the OLED, true to use it!


#include "Adafruit_seesaw.h"
#include <seesaw_neopixel.h>

#if USE_OLED
  #include <Adafruit_SH110X.h>
  #include <Fonts/FreeSans9pt7b.h>
  Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
#endif

#define SS_SWITCH        24      // this is the pin on the encoder connected to switch
#define SS_NEOPIX        6       // this is the pin on the encoder connected to neopixel

#define SEESAW_BASE_ADDR          0x36  // I2C address, starts with 0x36


// create 4 encoders!
Adafruit_seesaw encoders[4];
// create 4 encoder pixels
seesaw_NeoPixel encoder_pixels[4] = {
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800)};

int32_t encoder_positions[] = {0, 0, 0, 0};
bool found_encoders[] = {false, false, false, false};

void setup() {
  Serial.begin(115200);

  // wait for serial port to open
  while (!Serial) delay(10);

  Serial.println("128x64 OLED + seesaw Encoders test");

#if USE_OLED
  display.begin(0x3C, true); // Address 0x3C default
  Serial.println("OLED begun");
  display.display();
  delay(500); // Pause for half second
  display.setRotation(1);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(SH110X_WHITE);
#endif

  Serial.println("Looking for seesaws!");

  for (uint8_t enc=0; enc<sizeof(found_encoders); enc++) {
    // See if we can find encoders on this address
    if (! encoders[enc].begin(SEESAW_BASE_ADDR + enc) ||
        ! encoder_pixels[enc].begin(SEESAW_BASE_ADDR + enc)) {
      Serial.print("Couldn't find encoder #");
      Serial.println(enc);
    } else {
      Serial.print("Found encoder + pixel #");
      Serial.println(enc);

      uint32_t version = ((encoders[enc].getVersion() >> 16) & 0xFFFF);
      if (version != 4991){
        Serial.print("Wrong firmware loaded? ");
        Serial.println(version);
        while(1) delay(10);
      }
      Serial.println("Found Product 4991");

      // use a pin for the built in encoder switch
      encoders[enc].pinMode(SS_SWITCH, INPUT_PULLUP);

      // get starting position
      encoder_positions[enc] = encoders[enc].getEncoderPosition();

      Serial.println("Turning on interrupts");
      delay(10);
      encoders[enc].setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
      encoders[enc].enableEncoderInterrupt();

      // set not so bright!
      encoder_pixels[enc].setBrightness(30);
      encoder_pixels[enc].show();

      found_encoders[enc] = true;
    }
  }

  Serial.println("Encoders started");
}

void loop() {
#if USE_OLED
  display.clearDisplay();
  uint16_t display_line = 1;
#endif

  for (uint8_t enc=0; enc<sizeof(found_encoders); enc++) {
     if (found_encoders[enc] == false) continue;

     int32_t new_position = encoders[enc].getEncoderPosition();
     // did we move around?
     if (encoder_positions[enc] != new_position) {
       Serial.print("Encoder #");
       Serial.print(enc);
       Serial.print(" -> ");
       Serial.println(new_position);         // display new position
       encoder_positions[enc] = new_position;

       // change the neopixel color, mulitply the new positiion by 4 to speed it up
       encoder_pixels[enc].setPixelColor(0, Wheel((new_position*4) & 0xFF));
       encoder_pixels[enc].show();
     }

#if USE_OLED
     // draw the display
     display.setCursor(0, 20*display_line++);
     display.print("Enc #");
     display.print(enc);
     display.print(" : ");
     display.print(encoder_positions[enc]);
#endif

     if (! encoders[enc].digitalRead(SS_SWITCH)) {
        Serial.print("Encoder #");
        Serial.print(enc);
        Serial.println(" pressed");
#if USE_OLED
        display.print(" P");
#endif
     }
  }

#if USE_OLED
  display.display();
#endif

  // don't overwhelm serial port
  yield();
  delay(10);
}


uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return seesaw_NeoPixel::Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return seesaw_NeoPixel::Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return seesaw_NeoPixel::Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

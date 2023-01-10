#include "Adafruit_NeoKey_1x4.h"
#include "seesaw_neopixel.h"

#define Y_DIM 2 //number of rows of keys
#define X_DIM 4 //number of columns of keys

// create a matrix of NeoKey 1x4's 
// this example is just two, one on top of another to make a 2x4 grid
Adafruit_NeoKey_1x4 nk_array[Y_DIM][X_DIM/4] = {
  { Adafruit_NeoKey_1x4(0x30) }, 
  { Adafruit_NeoKey_1x4(0x31) },
};

// pass this matrix to the multi-neokey object
Adafruit_MultiNeoKey1x4 neokey((Adafruit_NeoKey_1x4 *)nk_array, Y_DIM, X_DIM/4);


void setup() {
  Serial.begin(115200);
  while (! Serial) delay(10);
   
  if (! neokey.begin()) {  // start matrix
    Serial.println("Could not start NeoKeys, check wiring?");
    while(1) delay(10);
  }
  
  Serial.println("NeoKey started!");

  // Pulse all the LEDs on to show we're working
  for (uint16_t i=0; i< X_DIM*Y_DIM; i++) {
    neokey.setPixelColor(i, 0x808080); // make each LED white
    neokey.show();
    delay(50);
  }
  for (uint16_t i=0; i< X_DIM*Y_DIM; i++) {
    neokey.setPixelColor(i, 0x000000);
    neokey.show();
    delay(50);
  }

  // activate all keys and set callbacks
  for(int y=0; y<Y_DIM; y++){
    for(int x=0; x<X_DIM; x++){
      neokey.registerCallback(x, y, blink);
    }
  }
}

void loop() {
  neokey.read();

  delay(10);    // don't print too fast
}


//define a callback for key presses
NeoKey1x4Callback blink(keyEvent evt) {
  uint8_t key = evt.bit.NUM;
  
  if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
    Serial.print("Key press ");
    Serial.println(key);
    neokey.setPixelColor(key, Wheel(map(key, 0, X_DIM*Y_DIM, 0, 255)));
    
  } else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
    Serial.print("Key release ");
    Serial.println(key);

    neokey.setPixelColor(key, 0);
  }

  // Turn on/off the neopixels!
  neokey.show();
  return 0;
}

/******************************************/

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return seesaw_NeoPixel::Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return seesaw_NeoPixel::Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return seesaw_NeoPixel::Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  return 0;
}

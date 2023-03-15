/****************************************************************************
 * This example was developed by the Hackerspace San Salvador to demonstrate
 * the simultaneous use of the NeoPixel library and the Bluetooth SoftDevice.
 * To compile this example you'll need to add support for the NRF52 based
 * following the instructions at:
 *  https://github.com/sandeepmistry/arduino-nRF5
 * Or adding the following URL to the board manager URLs on Arduino preferences:
 *  https://sandeepmistry.github.io/arduino-nRF5/package_nRF5_boards_index.json
 * Then you can install the BLEPeripheral library avaiable at:
 *  https://github.com/sandeepmistry/arduino-BLEPeripheral
 * To test it, compile this example and use the UART module from the nRF
 * Toolbox App for Android. Edit the interface and send the characters
 * 'a' to 'i' to switch the animation.
 * There is a no delay because this example does not block the threads execution
 * so the change will be shown immediately and will not need to wait for the current 
 * animation to end.
 * For more info write us at: info _at- teubi.co
 */
#include <SPI.h>
#include <BLEPeripheral.h>
#include "BLESerial.h"
#include <Adafruit_NeoPixel.h>

#define PIN 15 // Pin where NeoPixels are connected

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(64, PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

// define pins (varies per shield/board)
#define BLE_REQ   10
#define BLE_RDY   2
#define BLE_RST   9

// create ble serial instance, see pinouts above
BLESerial BLESerial(BLE_REQ, BLE_RDY, BLE_RST);

uint8_t current_state = 0;
uint8_t rgb_values[3];

void setup() {
  Serial.begin(115200);
  Serial.println("Hello World!");
  // custom services and characteristics can be added as well
  BLESerial.setLocalName("UART_HS");
  BLESerial.begin();

  strip.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();  // Turn OFF all pixels ASAP

  //pinMode(PIN, OUTPUT);
  //digitalWrite(PIN, LOW);

  current_state = 'a';
}

void loop() {
  while(BLESerial.available()) {
    uint8_t character = BLESerial.read();
    switch(character) {
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'g':
      case 'h':
        current_state = character;
        break;
    };
  }
  switch(current_state) {
    case 'a':
      colorWipe(strip.Color(255,   0,   0), 20);    // Red
      break;
    case 'b':
      colorWipe(strip.Color(  0, 255,   0), 20);    // Green
      break;
    case 'c':
      colorWipe(strip.Color(  0,   0, 255), 20);    // Blue
      break;
    case 'd':
      theaterChase(strip.Color(255,   0,   0), 20); // Red
      break;
    case 'e':
      theaterChase(strip.Color(  0, 255,   0), 20); // Green
      break;
    case 'f':
      theaterChase(strip.Color(255,   0, 255), 20); // Cyan
      break;
    case 'g':
      rainbow(10);
      break;
    case 'h':
      theaterChaseRainbow(20);
      break;
  }
}

// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time
  strip.setPixelColor(pixelCurrent, color); //  Set pixel's color (in RAM)
  strip.show();                             //  Update strip to match
  pixelCurrent++;                           //  Advance current pixel
  if(pixelCurrent >= pixelNumber)           //  Loop the pattern from the first LED
    pixelCurrent = 0;
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time
  for(int i = 0; i < pixelNumber; i++) {
    strip.setPixelColor(i + pixelQueue, color); //  Set pixel's color (in RAM)
  }
  strip.show();                             //  Update strip to match
  for(int i=0; i < pixelNumber; i+3) {
    strip.setPixelColor(i + pixelQueue, strip.Color(0, 0, 0)); //  Set pixel's color (in RAM)
  }
  pixelQueue++;                             //  Advance current pixel
  if(pixelQueue >= 3)
    pixelQueue = 0;                         //  Loop the pattern from the first LED
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(uint8_t wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   
  for(uint16_t i=0; i < pixelNumber; i++) {
    strip.setPixelColor(i, Wheel((i + pixelCycle) & 255)); //  Update delay time  
  }
  strip.show();                             //  Update strip to match
  pixelCycle++;                             //  Advance current cycle
  if(pixelCycle >= 256)
    pixelCycle = 0;                         //  Loop the cycle back to the begining
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time  
  for(int i=0; i < pixelNumber; i+3) {
    strip.setPixelColor(i + pixelQueue, Wheel((i + pixelCycle) % 255)); //  Update delay time  
  }
  strip.show();
  for(int i=0; i < pixelNumber; i+3) {
    strip.setPixelColor(i + pixelQueue, strip.Color(0, 0, 0)); //  Update delay time  
  }      
  pixelQueue++;                           //  Advance current queue  
  pixelCycle++;                           //  Advance current cycle
  if(pixelQueue >= 3)
    pixelQueue = 0;                       //  Loop
  if(pixelCycle >= 256)
    pixelCycle = 0;                       //  Loop
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

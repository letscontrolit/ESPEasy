/* Small game for Adafruit NeoTrellis
 * Works fine with smaller chips like Nano
*/

#include "Adafruit_NeoTrellis.h"

Adafruit_NeoTrellis trellis;
#define RED 0xFF00000
#define GREEN 0x00FF000

/*Defines the maximum difficulty (16 patterns in a row)*/
#define MAX_DIFFICULTY 16

int game_sequence[MAX_DIFFICULTY]  = {};
int current_difficulty = 2;
int cur = 0;

//define a callback for key presses
// Release event will trigger the game check
TrellisCallback blink(keyEvent evt){
  // Check is the pad pressed?
  if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
    trellis.pixels.setPixelColor(evt.bit.NUM, Wheel(map(evt.bit.NUM, 0, trellis.pixels.numPixels(), 0, 255))); //on rising
  } else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
  // or is the pad released?
    trellis.pixels.setPixelColor(evt.bit.NUM, 0); //off falling
    // Check if the pressed button it correct
    if (game_sequence[cur] == evt.bit.NUM){
      flash(GREEN);
      cur++;
      if (cur == current_difficulty){
        flash(GREEN);
        flash(GREEN);
        flash(GREEN);
        cur = 0;
        restart_game();
      }
    } else{
      flash(RED);
      cur = 0;
      show_solution(game_sequence, current_difficulty);
  
    }
    }

  // Turn on/off the neopixels!
  trellis.pixels.show();
  return 0;
}

/*Increse difficulty and restart the game*/
void restart_game(){
  if (current_difficulty <= MAX_DIFFICULTY){
    current_difficulty++;  
  }
  start_game(current_difficulty);
}
/*
 * Flash all leds for a short time
 */
void flash(uint32_t color){
  
  for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
    trellis.pixels.setPixelColor(i, color);
  }
  trellis.pixels.show();
  delay(200);
  for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
  
    trellis.pixels.setPixelColor(i, 0x000000);
  }
  trellis.pixels.show();

}

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  //while(!Serial);
  
  if (!trellis.begin()) {
    Serial.println("Could not start trellis, check wiring?");
    while(1) delay(1);
  } else {
    Serial.println("NeoPixel Trellis started");
  }

  //activate all keys and set callbacks
  for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++){
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
    trellis.registerCallback(i, blink);
  }

  //do a little animation to show we're on
  for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
    trellis.pixels.setPixelColor(i, Wheel(map(i, 0, trellis.pixels.numPixels(), 0, 255)));
    trellis.pixels.show();
    delay(50);
  }
  delay(50);
  for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
  
    trellis.pixels.setPixelColor(i, 0x000000);
    trellis.pixels.show();
    delay(50);
  }
  trellis.pixels.show();
  
  delay(1000);
  start_game(current_difficulty);
  
}

void start_game(int level){
  for (int x = 0; x <= level; x++){
    int led = random(trellis.pixels.numPixels());
    game_sequence[x] = led;
  }
  show_solution(game_sequence, level);
}

void show_solution(int solution[], int level){
  for (int x=0; x < level; x++){
    int led = solution[x];
    trellis.pixels.setPixelColor(led, Wheel(map(led, 0, trellis.pixels.numPixels(), 0, 255)));
    trellis.pixels.show();
    delay(500);
    trellis.pixels.setPixelColor(led, 0x000000);
    trellis.pixels.show();
    delay(50);
  }

}

void loop() {
  trellis.read();  // interrupt management does all the work! :)
  delay(20); //the trellis has a resolution of around 60hz
}


/******************************************/

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return trellis.pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return trellis.pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return trellis.pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  return 0;
}

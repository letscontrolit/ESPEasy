#include <anyrtttl.h>
#include <binrtttl.h>
#include <pitches.h>

//project's constants
#define BUZZER_PIN 8
const char * tetris = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";

unsigned long playStart = 0;
bool firstPass = true;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
}

void loop() {
  if (firstPass) {
    anyrtttl::nonblocking::begin(BUZZER_PIN, tetris);
    
    //remember when we started playing the song
    playStart = millis();

    firstPass = false;
  }

  //if we are playing something
  if ( anyrtttl::nonblocking::isPlaying() ) {
    
    //does the melody been playing for more than 5 seconds ?
    if ( millis() - playStart > 5000 )
    {
      anyrtttl::nonblocking::stop();      
    }
  }
  
  //if anything available for playing, play it
  anyrtttl::nonblocking::play();
  
}

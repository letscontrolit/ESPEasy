#include <anyrtttl.h>
#include <binrtttl.h>
#include <pitches.h>

//project's constants
#define BUZZER_PIN 8
const char * tetris = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";
const char * arkanoid = "Arkanoid:d=4,o=5,b=140:8g6,16p,16g.6,2a#6,32p,8a6,8g6,8f6,8a6,2g6";
const char * mario = "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6";
byte songIndex = 0; //which song to play when the previous one finishes

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
}

void loop() {
  // If we are not playing something 
  if ( !anyrtttl::nonblocking::isPlaying() )
  {
    // Play a song based on songIndex.
    if (songIndex == 0)
      anyrtttl::nonblocking::begin(BUZZER_PIN, tetris);
    else if (songIndex == 1)
      anyrtttl::nonblocking::begin(BUZZER_PIN, arkanoid);
    else if (songIndex == 2)
      anyrtttl::nonblocking::begin(BUZZER_PIN, mario);

    //Set songIndex ready for next song
    songIndex++;
  }
  else
  {
    anyrtttl::nonblocking::play();
  }
}

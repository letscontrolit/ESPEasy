/*
  Audio Spectrum.

  This example shows how to set the audio sampling rate and read
  audio spectrum data from a compatible Seesaw device.
*/

#include <seesaw_spectrum.h>

seesaw_Audio_Spectrum ss;

// The setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  
  while (!Serial) delay(10);   // wait until serial port is opened
  Serial.println("A");
  
  if (!ss.begin()) {
    Serial.println("seesaw not found!");
    while(1) delay(10);
  }
  Serial.println("B");

  // Configure audio sampling rate, which determines the peak
  // frequency of the spectrum output. There are 32 possible values
  // (0-31), where lower numbers = higher frequency.
  // The corresponding frequency for each setting will depend on the
  // F_CPU frequency on the Seesaw device, which has not yet been
  // determined. 10 or 20 MHz would be ideal, but others may happen,
  // so specific numbers are not documented here yet.
  // If 10 or 20 MHz, value of 12 here maps to 6250 Hz:
  ss.setRate(12);
}

// The loop routine runs over and over again forever:
void loop() {
  ss.getData(); // Pull audio spectrum data from device
  // Print contents of each of the 64 spectrum bins...
  for (uint8_t i=0; i<64; i++) {
    Serial.print(ss.getLevel(i));
    Serial.write(' ');
  }
  Serial.println();
}

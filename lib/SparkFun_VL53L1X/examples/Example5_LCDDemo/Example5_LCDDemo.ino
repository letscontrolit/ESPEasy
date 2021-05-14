/*
  Reading distance from the laser based VL53L1X
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 4th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SparkFun labored with love to create this code. Feel like supporting open source hardware? 
  Buy a board from SparkFun! https://www.sparkfun.com/products/14667

  Reading distance and outputting the distance and speed to a SerLCD.
  This is based on the Speed Trap project: https://www.youtube.com/watch?v=uC9CkhJiIaQ

  Are you getting weird readings? Make sure the vacuum tape has been removed.

*/
#include <Wire.h>
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X
#include <SoftwareSerial.h>

//Optional interrupt and shutdown pins.
#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3

SFEVL53L1X distanceSensor;
//Uncomment the following line to use the optional shutdown and interrupt pins.
//SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);

#if defined(ESP8266)
SoftwareSerial lcd(10, 9); // RX, TX
#elif defined(ARDUINO_ARCH_APOLLO3)
SoftwareSerial lcd(14, 15); // use RX1, TX1 of Apollo boards
#else
SoftwareSerial lcd(10, A3); // RX, TX
#endif

//Store distance readings to get rolling average
#define HISTORY_SIZE 8
int history[HISTORY_SIZE];
byte historySpot;

long lastReading = 0;
long lastDistance = 0;
float newDistance;
int maxDistance = 0;

const byte numberOfDeltas = 8;
float deltas[numberOfDeltas];
byte deltaSpot = 0; //Keeps track of where we are within the deltas array

//This controls how quickly the display updates
//Too quickly and it gets twitchy. Too slow and it doesn't seem like it's responding.
#define LOOPTIME 50

int maxMPH = 0;          //Keeps track of what the latest fastest speed is
long maxMPH_timeout = 0; //Forget the max speed after some length of time

#define maxMPH_remember 3000 //After this number of ms the system will forget the max speed

boolean readingValid = false;
long validCount = 0;

void setup(void)
{
  Wire.begin();
  Wire.setClock(400000);

  Serial.begin(115200);
  Serial.println("VL53L1X Qwiic Test");

  lcd.begin(9600);

  lcd.write(254); //Move cursor to beginning of first line
  lcd.write(128);
  lcd.print("Distance: 3426  ");
  lcd.print("12 mph          ");

  if (distanceSensor.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1)
      ;
  }
  Serial.println("Sensor online!");

  for (int x = 0; x < HISTORY_SIZE; x++)
    history[x] = 0;
}

void loop(void)
{

  //Write configuration block of 135 bytes to setup a measurement
  distanceSensor.startRanging();
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  int distanceMM = distanceSensor.getDistance();
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();

  lastReading = millis();

  history[historySpot] = distanceMM;
  if (historySpot++ == HISTORY_SIZE)
    historySpot = 0;

  long avgDistance = 0;
  for (int x = 0; x < HISTORY_SIZE; x++)
    avgDistance += history[x];

  avgDistance /= HISTORY_SIZE;

  //Every loop let's get a reading
  newDistance = distanceMM / 10; //Go get distance in cm

  int deltaDistance = lastDistance - newDistance;
  lastDistance = newDistance;

  //Scan delta array to see if this new delta is sane or not
  boolean safeDelta = true;
  for (int x = 0; x < numberOfDeltas; x++)
  {
    //We don't want to register jumps greater than 30cm in 50ms
    //But if we're less than 1000cm then maybe
    //30 works well
    if (abs(deltaDistance - deltas[x]) > 40)
      safeDelta = false;
  }

  //Insert this new delta into the array
  if (safeDelta)
  {
    deltas[deltaSpot++] = deltaDistance;
    if (deltaSpot > numberOfDeltas)
      deltaSpot = 0; //Wrap this variable
  }

  //Get average of the current deltas array
  float avgDeltas = 0.0;
  for (byte x = 0; x < numberOfDeltas; x++)
    avgDeltas += (float)deltas[x];
  avgDeltas /= numberOfDeltas;

  //22.36936 comes from a big coversion from cm per 50ms to mile per hour
  float instantMPH = 22.36936 * (float)avgDeltas / (float)LOOPTIME;

  instantMPH = abs(instantMPH); //We want to measure as you walk away

  instantMPH = ceil(instantMPH); //Round up to the next number. This is helpful if we're not displaying decimals.

  if (instantMPH > maxMPH)
  {
    maxMPH = instantMPH;
    maxMPH_timeout = millis();
  }

  if (millis() - maxMPH_timeout > maxMPH_remember)
  {
    maxMPH = 0;
  }

  int signalRate = distanceSensor.getSignalRate();
  if (signalRate < 10)
  {
    readingValid = false;
    validCount = 0;
  }
  else
  {
    validCount++;
    if (avgDistance > maxDistance)
      maxDistance = avgDistance;
  }

  if (validCount > 10)
    readingValid = true;

  if (readingValid == false)
  {
    instantMPH = 0;
    avgDistance = 0;
  }

  //Convert mm per millisecond to miles per hour
  //float mph = distanceDelta * 2.236936 / (float)timeDelta;
  //mph *= -1; //Flip sign as we will be traveling towards sensor, decreasing the distance

  lcd.write(254); //Move cursor
  lcd.write(138);
  lcd.print("    ");
  lcd.write(254); //Move cursor
  lcd.write(138);

  if (readingValid == true)
    lcd.print(avgDistance);
  else
    lcd.print(maxDistance);

  lcd.write(254); //Move cursor
  lcd.write(192);
  lcd.print(instantMPH, 0);
  lcd.print(" mph  ");

  delay(25);
}

/*  ********************************************* 
 *  SparkFun_BabyBlynkMonitorThing
 *  Baby Monitor Thing Project Example Code 
 *  Blog Post: https://www.sparkfun.com/news/2185 
 *  
 *  Utilizing 
 *    Sparkfun's ADXL345 Library
 *    Blynk Library is licensed under MIT license
 *    ESP8266WiFi Library
 *    BlynkSimpleEsp8266 Library
 *  
 *  E.Robert @ SparkFun Electronics
 *  Created: Sep 12, 2016
 *  Updated: Sep 13, 2016
 *  
 *  Development Environment Specifics:
 *  Arduino 1.6.11
 *  Blynk App
 *  
 *  Blynk is a platform with iOS and Android apps to control
 *  Arduino, Raspberry Pi and the likes over the Internet.
 *  You can easily build graphic interfaces for all your
 *  projects by simply dragging and dropping widgets.
 *
 *   Downloads, docs, tutorials: http://www.blynk.cc
 *   Blynk community:            http://community.blynk.cc
 *   Social networks:            http://www.fb.com/blynkapp
 *                               http://twitter.com/blynk_app
 *  
 *  Hardware Specifications:
 *  SparkFun Triple Axis Accelerometer ADXL345
 *  SparkFun ESP8266 Thing
 *  *********************************************/

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SparkFun_ADXL345.h>
#include <SimpleTimer.h>

/*********** COMMUNICATION SELECTION ***********/
/*                                             */
//ADXL345 adxl = ADXL345(10);           // USE FOR SPI COMMUNICATION, ADXL345(CS_PIN);
ADXL345 adxl = ADXL345();             // USE FOR I2C COMMUNICATION

/****************** VARIABLES ******************/
/*                                             */
int gotUpFlag = 0;                        // Flags first occurance
int wentDownFlag = 0;                     // Flags first occurance
int gotUp = 0;                            // Variable for number of times baby up
unsigned long babyMovingStartTime = 0;    // Will store time when baby starts moving
unsigned long babySleepingStartTime = 0;  // Will store time when baby starts sleeping
unsigned long babySleepingEndTime = 0;    // Will store time when baby wakes up
long TimeLimit = 180000;                  // Notification in 3 minutes when awake
double minutesTimeS = 0;                  // For minute conversion
double minutesTimeA = 0;                  // For minute conversion


/******************** BLYNK ********************/
/*      Communication with your BLYNK app      */
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "b6873bbab8fa449dbd0c4d8bfa3b38ca";
#define XValue                  V0
#define YValue                  V1
#define ZValue                  V2
#define VIRTUAL_LCD             V3
#define babyMoving              V4
#define awakeTime               V5
#define asleepTime              V6
#define awakeLED                V7
#define asleepLED               V8
WidgetLCD lcd(VIRTUAL_LCD);

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "INSERT NETWORK HERE";
char pass[] = "INSERT PASSWORD HERE";

SimpleTimer timer;

/*********** REFRESH APPLICATION NAME **********/
/*      Communication with your BLYNK app      */
void refreshTime()
{
  long uptime = millis() / 60000L;

  // Output the following every minute:
  lcd.clear();                              // Clear LCD Screen on Blynk
  lcd.print(0, 0, "  BABY BLYNK   ");     // Outputs Application Name
  lcd.print(0, 1, " MONITOR THING ");
}

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);            // Give us access!

  while (Blynk.connect() == false) {        // Be patient. 
                                            // Wait for Blynk to come online   
  }

  // Notify immediately on startup
  Blynk.notify("Device Started");           // Notification to smartphone

  // Setup a function to be called every minute
  timer.setInterval(60000L, refreshTime);

  adxl.powerOn();                     // Power on the ADXL345

  adxl.setRangeSetting(8);            // Give the range settings
                                      // Accepted values are 2g, 4g, 8g or 16g
                                      // Higher Values = Wider Measurement Range
                                      // Lower Values = Greater Sensitivity

  // Set values to zero
  gotUpFlag = 0;
  wentDownFlag = 0;
  gotUp = 0;
  Blynk.virtualWrite(awakeTime, 0);
  Blynk.virtualWrite(asleepTime, 0);
  Blynk.virtualWrite(awakeLED, LOW);
  Blynk.virtualWrite(asleepLED, LOW);

  // Print a splash screen:
  lcd.clear();
  lcd.print(0, 0, "  BABY MONITOR   ");
  lcd.print(0, 1, "     THING       ");
}

/******************* MAIN CODE *****************/
/*                                             */
void loop()
{ 
  Blynk.run();
  timer.run();

  // ADXL345 Accelerometer Readings
  int x,y,z;   
  adxl.readAccel(&x, &y, &z);         // Read the accelerometer values in variables x,y,z

  // Write the values to Blynk:
  Blynk.virtualWrite(XValue, x);
  Blynk.virtualWrite(YValue, y);
  Blynk.virtualWrite(ZValue, z);

  // Monitoring Up and Down Time
  if (y >= 50 && y <= 200) {
    if (wentDownFlag == 1){
      babySleepingEndTime = millis();   // Stopped sleeping time            
    }
    Blynk.virtualWrite(awakeLED, 1023);  // Awake LED lit
    Blynk.virtualWrite(asleepLED, 0); // Asleep LED out
    
    if (gotUpFlag == 0) {             // If first time baby has gotten up  
      babyMovingStartTime = millis(); // Baby moving start time
      gotUpFlag = 1;                  
      gotUp = gotUp + 1;              // Count the number of times the baby gets up
                                      //  in the middle of the night
    } else {
      checkBaby();
    }
  } else if (y <= 30) {
    Blynk.virtualWrite(awakeLED, 0);  // Awake LED out
    Blynk.virtualWrite(asleepLED, 1023); // Asleep LED lit
    
    // Print to VIRTUAL_LCD:
    lcd.clear();
    lcd.print(0, 0, "  BABY SLEEPING  ");
    lcd.print(0, 1, "    SHHH!!!      ");
    
    if (wentDownFlag == 0) {
      babySleepingStartTime = millis(); // Baby sleeping start time 
      wentDownFlag = 1;
    } else {
      babySleepingEndTime = millis();   // Stopped sleeping time
    }
    
    if (gotUpFlag == 1) {
      wentDownFlag = 0;                 // Reset flag if baby went back down   
      gotUpFlag = 0;                   
    }
    sleepTime();                          // Time asleep
  } else {
    // do nothing                     
  }

  // Write number of times baby has gotten up to Blynk
  Blynk.virtualWrite(babyMoving, gotUp);
}

/***************** BABY IS AWAKE ***************/
/*             Time to get them yet?           */
void checkBaby() {
  long currentTime = millis();        // Current time
  long upTime = currentTime - babyMovingStartTime; // Time baby awake
  minutesTimeA = upTime * 1.6667E-5;  // Time conversion to minutes

  // Print to VIRTUAL_LCD:
  lcd.clear();                        
  lcd.print(0, 0, "  BABY MOVING   ");
  lcd.print(0, 1, "    AROUND      ");

  // Check to see if baby has been awake for a while 
  if (upTime >= TimeLimit) {
    Blynk.notify("BABY IS AWAKE!");   // Notification to smartphone
  } else {
    // do nothing
  }

  // Baby awake time
  Blynk.virtualWrite(awakeTime, minutesTimeA);
}

/***************** BABY IS Asleep ***************/
/*                But for how long?             */
void sleepTime(){
  // Calculat down time in millis and minutes
  long downTime = babySleepingEndTime - babySleepingStartTime;
  minutesTimeS = downTime * 1.6667E-5;

  // Baby sleeping time
  Blynk.virtualWrite(asleepTime, minutesTimeS);
}


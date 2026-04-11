#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <Adafruit_SSD1306.h>

// Connect to the two encoder outputs!
#define ENCODER_A   12
#define ENCODER_B   11

// These let us convert ticks-to-RPM
#define GEARING     20
#define ENCODERMULT 12

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// And connect a DC motor to port M1
Adafruit_DCMotor *myMotor = AFMS.getMotor(1);
// We'll display the speed/direction on the OLED
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

volatile float RPM = 0;
volatile uint32_t lastA = 0;
volatile bool motordir = FORWARD;

void interruptA() {
  motordir = digitalRead(ENCODER_B);

  digitalWrite(LED_BUILTIN, HIGH);
  uint32_t currA = micros();
  if (lastA < currA) {
    // did not wrap around
    float rev = currA - lastA;  // us
    rev = 1.0 / rev;            // rev per us
    rev *= 1000000;             // rev per sec
    rev *= 60;                  // rev per min
    rev /= GEARING;             // account for gear ratio
    rev /= ENCODERMULT;         // account for multiple ticks per rotation
    RPM = rev;
  }
  lastA = currA;
  digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(ENCODER_A, INPUT_PULLUP);
  attachInterrupt(ENCODER_A, interruptA, RISING);

  delay(100);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  //while (!Serial) delay(1);

  Serial.println("MMMMotor party!");
  if (!AFMS.begin()) {         // create with the default frequency 1.6KHz
  // if (!AFMS.begin(1000)) {  // OR with a different frequency, say 1KHz
    Serial.println("Could not find Motor Shield. Check wiring.");
    while (1);
  }
  Serial.println("Motor Shield found.");  Serial.println("Begun");
  // turn on motor M1
  myMotor->setSpeed(0);
}

void printRPM() {
    display.clearDisplay();
    display.setCursor(0,0);

    Serial.print("Direction: ");
    if (motordir) {
      display.println("Forward");
      Serial.println("forward @ ");
    } else {
      display.println("Backward");
      Serial.println("backward @ ");
    }
    display.print((int)RPM); display.println(" RPM");
    Serial.print((int)RPM); Serial.println(" RPM");
    display.display();
}

int i;
void loop() {
  delay(50);
  myMotor->run(FORWARD);
  for (i=0; i<255; i++) {
    myMotor->setSpeed(i);
    delay(20);
    printRPM();
  }

  for (i=255; i!=0; i--) {
    myMotor->setSpeed(i);
    delay(20);
    printRPM();
  }

  myMotor->run(BACKWARD);
  for (i=0; i<255; i++) {
    myMotor->setSpeed(i);
    delay(20);
    printRPM();
  }

  for (i=255; i!=0; i--) {
    myMotor->setSpeed(i);
    delay(20);
    printRPM();
  }
}

#include <SPI.h>
#include <Wire.h>
#include <SC16IS752.h>

#ifdef __AVR__
 #define GPIO 0
 #define CS 6
#elif ESP8266 // ESP8266
 #define GPIO 4
 #define CS 15
#endif

SC16IS752 spiuart = SC16IS752(SC16IS750_PROTOCOL_SPI,CS); 

//Remove A0, A1 resistors which set the I2C address
//Remove SCL pull up resistors if you are using Duemilanove
//Pin 6 should be connected to CS of the module.

void setup() 
{
    Serial.begin(115200);
    Serial.println("Start testing");
    // UART to Serial Bridge Initialization
    spiuart.begin(SC16IS752_DEFAULT_SPEED, SC16IS752_DEFAULT_SPEED); //baudrate setting
    if (spiuart.ping()!=1) {
        Serial.println("Device not found");
        while(1);
    } else {
        Serial.println("Device found");
    }

    spiuart.pinMode(GPIO, OUTPUT);
    spiuart.digitalWrite(GPIO, LOW);
}

void loop() 
{
    spiuart.digitalWrite(GPIO, HIGH);
    delay(1000);
    spiuart.digitalWrite(GPIO, LOW);
    delay(1000);
}

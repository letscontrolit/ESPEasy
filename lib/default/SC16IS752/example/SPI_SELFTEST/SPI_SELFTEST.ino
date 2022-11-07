#include <SPI.h>
#include <Wire.h>
#include <SC16IS752.h>

#ifdef __AVR__
 #define CS 6
#elif ESP8266 // ESP8266
 #define CS 15
#endif

SC16IS752 spiuart = SC16IS752(SC16IS750_PROTOCOL_SPI,CS); 

//Connect TX(A-CH) and RX(B-CH) with a wire
//Connect TX(B-CH) and RX(A-CH) with a wire
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
    Serial.println("Start serial communication");
}

void loop() 
{
    spiuart.write(SC16IS752_CHANNEL_A, 0x55);
    delay(10);
    if (spiuart.available(SC16IS752_CHANNEL_B)==0) {
        Serial.println("Please connnect TX(Channel A) and RX(Channel B) with a wire and reset your Arduino");
        while(1);
    }        
    if (spiuart.read(SC16IS752_CHANNEL_B)!=0x55) {
        Serial.println("Please connnect TX(Channel A) and RX(Channel B) with a wire and reset your Arduino");
        while(1);
    }   
    delay(200);
    
    spiuart.write(SC16IS752_CHANNEL_B, 0xAA);
    delay(10);
    if (spiuart.available(SC16IS752_CHANNEL_A)==0) {
        Serial.println("Please connnect TX(Channel B) and RX(Channel A) with a wire and reset your Arduino");
        while(1);
    }  
    if (spiuart.read(SC16IS752_CHANNEL_A)!=0xAA) {
        Serial.println("Please connnect TX(Channel B) and RX(Channel A) with a wire and reset your Arduino");
        while(1);
    }   
    
    delay(200);
}

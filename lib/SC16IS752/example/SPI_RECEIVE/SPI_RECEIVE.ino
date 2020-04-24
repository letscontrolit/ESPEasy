#include <SPI.h>
#include <Wire.h>
#include <SC16IS752.h>

#ifdef __AVR__
 #define CS 6
#elif ESP8266 // ESP8266
 #define CS 15
#endif

SC16IS752 spiuart = SC16IS752(SC16IS750_PROTOCOL_SPI,CS); 

#define baudrate_A 9600
#define baudrate_B 38400

void setup() 
{
  Serial.begin(115200);
  Serial.println("Start testing");
  // UART to Serial Bridge Initialization
  spiuart.begin(baudrate_A, baudrate_B); //baudrate setting
  if (spiuart.ping()!=1) {
    Serial.println("Device not found");
    while(1);
  } else {
    Serial.println("Device found");
  }
  Serial.println("start serial communication");
  Serial.print("baudrate(channel A) = ");
  Serial.println(baudrate_A);
  Serial.print("baudrate(channel B) = ");
  Serial.println(baudrate_B);
}

void loop() 
{
  static char buffer_A[64] = {0};
  static int index_A = 0;
  static char buffer_B[64] = {0};
  static int index_B = 0;

  if (spiuart.available(SC16IS752_CHANNEL_A) > 0){
    // read the incoming byte:
    char c = spiuart.read(SC16IS752_CHANNEL_A);

#if 0
    Serial.print("c=");
    if (c < 0x20) {
      Serial.print(" ");
    } else {
      Serial.print(c);
    }
    Serial.print(" 0x");
    Serial.println(c,HEX);
#endif

    if (c == 0x0d) {
      
    } else if (c == 0x0a) {
      Serial.print("Channel A=[");
      Serial.print(buffer_A);
      Serial.println("]");
      index_A = 0;
    } else {
      buffer_A[index_A++] = c;
      buffer_A[index_A] = 0;
    }
  }


  if (spiuart.available(SC16IS752_CHANNEL_B) > 0){
    // read the incoming byte:
    char c = spiuart.read(SC16IS752_CHANNEL_B);

#if 0
    Serial.print("c=");
    if (c < 0x20) {
      Serial.print(" ");
    } else {
      Serial.print(c);
    }
    Serial.print(" 0x");
    Serial.println(c,HEX);
#endif

    if (c == 0x0d) {
      
    } else if (c == 0x0a) {
      Serial.print("Channel B=[");
      Serial.print(buffer_B);
      Serial.println("]");
      index_B = 0;
    } else {
      buffer_B[index_B++] = c;
      buffer_B[index_B] = 0;
    }
  }


}

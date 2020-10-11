#include <SPI.h>
#include <Wire.h>
#include <SC16IS752.h>

#define GPIO 0

SC16IS752 i2cuart = SC16IS752(SC16IS750_PROTOCOL_I2C,SC16IS750_ADDRESS_AA);

void setup() 
{
    Serial.begin(115200);
    Serial.println("Start testing");
    // UART to Serial Bridge Initialization
    i2cuart.begin(SC16IS752_DEFAULT_SPEED, SC16IS752_DEFAULT_SPEED); //baudrate setting
    if (i2cuart.ping()!=1) {
        Serial.println("Device not found");
        while(1);
    } else {
        Serial.println("Device found");
    }

    i2cuart.pinMode(GPIO, OUTPUT);
    i2cuart.digitalWrite(GPIO, LOW);
}

void loop() 
{
	  i2cuart.digitalWrite(GPIO, HIGH);
	  delay(1000);
	  i2cuart.digitalWrite(GPIO, LOW);
	  delay(1000);
}

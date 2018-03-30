
#include <Arduino.h>
#include <output_export.h>



void output_start(unsigned int baudrate)
{
    Serial.begin(115200);
}

void output_char(int c)
{
    Serial.write(c);
}

void output_flush(void)
{
    Serial.flush();
}

void output_complete(void)
{
   Serial.end();
}


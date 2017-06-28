// SDS011 dust sensor example
// -----------------------------
//
// By R. Zschiegner (rz@madavi.de).
// April 2016

#include <SDS011.h>

float p10,p25;
int error;

SDS011 my_sds;

void setup() {
	my_sds.begin(D1,D2);
	Serial.begin(9600);
}

void loop() {
	error = my_sds.read(&p25,&p10);
	if (! error) {
		Serial.println("P2.5: "+String(p25));
		Serial.println("P10:  "+String(p10));
	}
	delay(100);
}

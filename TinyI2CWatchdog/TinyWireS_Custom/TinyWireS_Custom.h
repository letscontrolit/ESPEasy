/*
  TinyWireS.h - a wrapper class for Don Blake's usiTwiSlave routines.
  Provides TWI/I2C Slave functionality on ATtiny processers in Arduino environment.
  1/23/2011 BroHogan -  brohoganx10 at gmail dot com

  Major credit and thanks to Don Blake for his usiTwiSlave code which makes this possible
  http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=51467&start=all&postdays=0&postorder=asc
  (Changed #define USI_START_COND_INT  USISIF (was USICIF) in usiTwiSlave.h)

  NOTE! - It's very important to use pullups on the SDA & SCL lines! More so than with the Wire lib.
  Current Rx & Tx buffers set at 32 bytes - see usiTwiSlave.h
 
 USAGE is modeled after the standard Wire library . . .
  Put in setup():
	TinyWireS.begin(I2C_SLAVE_ADDR);                 // initialize I2C lib & setup slave's address (7 bit - same as Wire)

  To Receive:
    someByte = TinyWireS.available(){                // returns the number of bytes in the received buffer
    someByte = TinyWireS.receive(){                  // returns the next byte in the received buffer

  To Send:
	TinyWireS.send(uint8_t data){                    // sends a requested byte to master
	
  TODO:	(by others!)
	- onReceive and onRequest handlers are not implimented.
	- merge this class with TinyWireM for master & slave support in one library
	
  This library is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2.1 of the License, or any later version.
  This program is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef TinyWireS_h
#define TinyWireS_h

#include <inttypes.h>


class USI_TWI_S
{
  private:
	//static uint8_t USI_BytesAvail;
	
  public:
 	USI_TWI_S();
    void begin(uint8_t I2C_SLAVE_ADDR);
    void send(uint8_t data);
    uint8_t available();
    uint8_t receive();
    void onReceive( void (*)(uint8_t) );
    void onRequest( void (*)(void) );

    // mvdbro, 2016-01-01, avoid hang on bus error with SDA sustained low and SCL sustained high
    uint16_t initCount(void);

};

void TinyWireS_stop_check();
// Implement a delay loop that checks for the stop bit (basically direct copy of the stock arduino implementation from wiring.c)
void tws_delay(unsigned long);

extern USI_TWI_S TinyWireS;

#endif


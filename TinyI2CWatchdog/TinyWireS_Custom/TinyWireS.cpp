/*
  TinyWireS.cpp - a wrapper class for Don Blake's usiTwiSlave routines.
  Provides TWI/I2C Slave functionality on ATtiny processers in Arduino environment.
  1/23/2011 BroHogan -  brohoganx10 at gmail dot com

  **** See TinyWireS.h for Credits and Usage information ****

  This library is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2.1 of the License, or any later version.
  This program is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

extern "C" {
  #include <inttypes.h>
  #include "usiTwiSlave.h"
  #include <avr/interrupt.h>
  }

#include "TinyWireS_Custom.h"
#include "Arduino.h"

// Constructors ////////////////////////////////////////////////////////////////

USI_TWI_S::USI_TWI_S(){
}


// Public Methods //////////////////////////////////////////////////////////////

// mvdbro, 2016-01-01, avoid hang on bus error with SDA sustained low and SCL sustained high
uint16_t USI_TWI_S::initCount(void)
{
  return twi_initCount();
}

void USI_TWI_S::begin(uint8_t slaveAddr){ // initialize I2C lib
  usiTwiSlaveInit(slaveAddr); 
}

void USI_TWI_S::send(uint8_t data){  // send it back to master
  usiTwiTransmitByte(data);
}

uint8_t USI_TWI_S::available(){ // the bytes available that haven't been read yet
  return usiTwiAmountDataInReceiveBuffer(); 
  //return usiTwiDataInReceiveBuffer(); // This is wrong as far as the Wire API is concerned since it returns boolean and not amount
}
 
uint8_t USI_TWI_S::receive(){ // returns the bytes received one at a time
  return usiTwiReceiveByte(); 
}

// sets function called on slave write
void USI_TWI_S::onReceive( void (*function)(uint8_t) )
{
  usi_onReceiverPtr = function;
}

// sets function called on slave read
void USI_TWI_S::onRequest( void (*function)(void) )
{
  usi_onRequestPtr = function;
}

void TinyWireS_stop_check()
{
    if (!usi_onReceiverPtr)
    {
        // no onReceive callback, nothing to do...
        return;
    }
    if (!(USISR & ( 1 << USIPF )))
    {
        // Stop not detected
        return;
    }
    uint8_t amount = usiTwiAmountDataInReceiveBuffer();
    if (amount == 0)
    {
        // no data in buffer
        return;
    }
    usi_onReceiverPtr(amount);
}

// Implement a delay loop that checks for the stop bit (basically direct copy of the stock arduino implementation from wiring.c)
void tws_delay(unsigned long ms)
{
    uint16_t start = (uint16_t)micros();
    while (ms > 0)
    {
        TinyWireS_stop_check();
        if (((uint16_t)micros() - start) >= 1000)
        {
            ms--;
            start += 1000;
        }
    }
}

// Preinstantiate Objects //////////////////////////////////////////////////////

USI_TWI_S TinyWireS = USI_TWI_S();


/*--------------------------------------------------------------------
Arduino library to ...

Written by Jochen Krapf,
contributions by ... and other members of the open
source community.

-------------------------------------------------------------------------
This file is part of the MechInputs library.

MechInputs is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

MechInputs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with MechInputs.  If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------*/

#ifndef QEIX4_H
#define QEIX4_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

class QEIx4
{
public:

	/** constructor of QEIx4 object
	*/
	QEIx4();

	/** destructor of QEIx4 object
	*/
	~QEIx4();

	void begin(int16_t pinA, int16_t pinB, int16_t pinI=-1, uint8_t mode=4);

	/** Gets the actual counter value.
	*
	* @return        Actual counter value
	*/
	long read();

	/** Gets the actual counter value as long operator.
	*
	* @return        Actual counter value as long operator
	*/
	operator long() {   // int-Operator
		return read();
	}

	/** Sets the counter value at actual encoder position to given value.
	*
	* @param        Counter value
	*/
	void write(long counter) {
		_counter = counter;
	}

	/** Sets the counter value at actual encoder position to given value as assign operator.
	*
	* @param        Counter value
	*/
	int operator= (long counter) {   // Assign-Operator
		write(counter);
		return counter;
	}

  void setLimit(long limitMin, long limitMax)
  {
    _limitMin = limitMin;
    _limitMax = limitMax;
  }

	/** Sets the flag for zeroing on next high on index pin while AB lines triggers next counting. The trigger calls tha callback function in which the counter can be set to zero or the actual counter can be latched in for later offset calculation
	*
	* @param        Flag for triggering. Set to 1 for call the attached callback. It is reseted after this call
	*/
	void setIndexTrigger(bool bIndexTrigger=true) {
    if (_pinI < 0)
      bIndexTrigger = false;
		_bIndexTrigger = bIndexTrigger;
	}

  bool hasChanged(){
    return _bHasChanged;
  }

  void loop();

protected:

  /** Polls the state machine and updates the counter value.
	*/
	void processStateMachine();

  /** Entry point for arduino interrupts - route to class instances
  */
  static void ISR();

protected:

  volatile long _counter;
  volatile bool _bHasChanged;
  volatile bool _bIndexTrigger;

  int16_t _pinA;
  int16_t _pinB;
  int16_t _pinI;
  long _limitMin;
  long _limitMax;
	uint16_t _state;
	uint16_t _eMode;

private:
	static uint16_t __stateLUT[32];
  static QEIx4* __instance[4];
  uint16_t _eventMask;
};

#endif // QEIX4_H

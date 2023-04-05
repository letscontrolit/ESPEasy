Introduction
------------

The ADS1113, ADS1114 and ADS1115 are 16 bit ADC chips from Texas Instruments.

These can be used to measure upto 4 single-ended or upto 2 differential inputs.

ADS1114 and ADS1115 support a programmable gain to set a range for the analog voltage to be measured.

.. note::
  
  The analog input should never exceed VDD + 0.3V.

ADS1115 can select which analog input to measure. This can also be a differential voltage between 2 pins.


Device Settings
---------------

Gain
^^^^

The ADC111x ADCs have a programmable gain.
This can be used to set the sensitivity and thus the full scale (FS) range of the ADC.

* 2/3x gain (FS=6.144V)
* 1x gain (FS=4.096V)
* 2x gain (FS=2.048V)
* 4x gain (FS=1.024V)
* 8x gain (FS=0.512V)
* 16x gain (FS=0.256V)

.. note::

  Never supply an analog voltage exceeding the Vdd of the chip.

Input Multiplexer
^^^^^^^^^^^^^^^^^

The ADS1115 does have 4 analog input pins.

When measuring "Single-Ended", the voltage is measured against GND.
In "differential" mode, the voltage between given pins is measured.

* AIN0 - AIN1 (Differential)
* AIN0 - AIN3 (Differential)
* AIN1 - AIN3 (Differential)
* AIN2 - AIN3 (Differential)
* AIN0 - GND (Single-Ended)
* AIN1 - GND (Single-Ended)
* AIN2 - GND (Single-Ended)
* AIN3 - GND (Single-Ended)


Convert to Volt
^^^^^^^^^^^^^^^

(Added: 2023/03/31)

When this option is checked, the measured value will be converted to Volt, based on the selected gain.

# MechInputs Library (https://github.com/jkDesignDE/MechInputs)

Arduino library for reading and debouncing mechanical inputs Like buttons, switches and encoders.

After downloading, rename folder to 'MechInputs' and install in Arduino Libraries folder. Restart Arduino IDE, then open File->Sketchbook->Library->MechInputs->* sketches.

# QEI (quadrature encoder interface) library, for decoding AB signals from a rotary encoder

Use cases:
 - Rotary encoder in closed loop motor regulation
 - Hand wheel
 - Input device for motion control (MoCo)


A class to decode pulses on a rotary encoder with AB signals (quadrature encoder).
It uses all 4 edges of the AB signals to increase the counter resolution 4 times of cycles per rotation/revolution (CPR) (e.g. an encoder with 500 CPR get 2000 counts per rotation)

In opposite to most common QEI implementation this is resistant to jitter and chatter on AB signals and motor vibrations.
Whes reaching the next position the edge that triggerd this position (state) is ignored to aboid oscillating up/down counts.

It can also be used in polling mode i.g. in idle routines if interrupts are not desired.
At this mode be sure that the sampling frequency is heigher than the maximum rotation speed (expeced counts per second)

The internal state machine is based on a look up table (LUT) to minimize interrupt retention time and get all necessary flags at once.

The library is designed to support closed loop speed- and motion-controller for also slow and smooth motions like movie camera motion control.



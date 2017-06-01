#!/usr/bin/env python

import serial
import sys
import time

p=serial.Serial(sys.argv[1])

p.setDTR(0)
time.sleep(0.1)
p.setDTR(1)


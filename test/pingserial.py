#!/usr/bin/env python
# from __future__ import print_function

import serial
import sys
import time

#wait until ESPEasy is booted and ready

s=serial.Serial(port=sys.argv[1], baudrate=115200, timeout=1, write_timeout=1)

s.reset_input_buffer();
sys.stdout.write("Waiting for serial response from ESPEASY: ")
sys.stdout.flush()

while 1:
        s.write("\n");
        a=True
        while a!="":
            a=s.readline()
            if a=="Unknown command!\r\n":
                print("OK")
                s.reset_input_buffer();
                sys.exit(0)
            else:
                sys.stdout.write(".")
                sys.stdout.flush()

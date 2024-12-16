#!/bin/bash

sudo modprobe usbserial
sudo modprobe ch341
sudo modprobe cp210x
sudo modprobe ftdi_sio


sudo chmod 666 /dev/ttyACM*
sudo chmod 666 /dev/ttyUSB*
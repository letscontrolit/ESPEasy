# -*- coding:utf-8 -*-
'''!
  @file  output_sin.py
  @brief  Use DAC to output sine wave.
  @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license  The MIT License (MIT)
  @author  [tangjie](jie.tang@dfrobot.com)
  @version  V1.0
  @date  2022-03-07
  @url  https://github.com/DFRobot/DFRobot_GP8403
'''
from __future__ import print_function
import sys
import os
import time

sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
from DFRobot_GP8403 import *

DAC = DFRobot_GP8403(0x58)  
while DAC.begin() != 0:
    print("init error")
    time.sleep(1)
print("init succeed")
  
#Set output range  
DAC.set_DAC_outrange(OUTPUT_RANGE_5V)
while True:
  '''!
      @brief output sine wave from channel 0
      @param amp Set sine wave amplitude Vp
      @param freq Set sine wave frequency f
      @param offset Set sine wave DC offset Voffset
      @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
    '''
  DAC.output_sin(2500, 10, 2500, 0)
  
  

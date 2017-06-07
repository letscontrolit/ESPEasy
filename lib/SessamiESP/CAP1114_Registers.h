/*************************************************** 
 Begin Date : 2nd August 2016
 Description : Register Header file for CAP1114
 Original Source : Adafruit_CAP1188.h Link: https://github.com/adafruit/Adafruit_CAP1188_Library/archive/master.zip
 Datasheet Link : http://ww1.microchip.com/downloads/en/DeviceDoc/CAP1114.pdf
 Modifier : Sagar Naikwadi
 ****************************************************/
// Referred from Adafruit contributors for CAP1118
/*************************************************** 
 This is a library for the CAP1188 8-Channel Capacitive Sensor

 Designed specifically to work with the CAP1188 breakout from Adafruit
 ----> https://www.adafruit.com/products/1602

 These sensors use I2C/SPI to communicate, 2+ pins are required to interface
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, all text above must be included in any redistribution
 ****************************************************/
#ifndef _CAP1114_REGISTERSH_
#define _CAP1114_REGISTERSH_

namespace CAP1114 {
	
//CAP1114 Register initialization
#define CAP1114_MAIN 0x00 //Primary Power State => x|DEACT|SLEEP|DSLEEP|x|x|PWR_LED|INT
#define CAP1114_MAIN_INT 0x01 //Detect Interrupt Occured
#define CAP1114_SENINPUTSTATUS1 0x03 //Touch Sensor Detection 1 => UP(CS8>>CS9>>CS10)|DOWN(CS14>>CS13>>CS12)|CS6|CS5|CS4|CS3|CS2|CS1
#define CAP1114_SENINPUTSTATUS2 0x04 //Touch Sensor Detection 2 => CS14|CS13|CS12|CS11|CS10|CS9|CS8|CS7
#define CAP1114_REV 0x05 //Build Revision Register => x|x|x|Build4|Build3|Build2|Build1|Build0
#define CAP1114_SLIDERPOSITION 0x06 //Slider Position Detection => x|POS[6:0]
/*TOUCH POSITION | POS[6:0]SETTINGS
 CS8 | 02h
 CS9 | 12h
 CS10 | 22h
 CS11 | 32h
 CS12 | 42h
 CS13 | 52h
 CS14 | 62h*/
#define CAP1114_VENDORID 0x08 //Vendor ID
#define CAP1114_VOLUMETRICSTEP 0x09 //Step Size for Slide detected in UP/DOWN => x|x|x|x|VOL_STEP[3:0]
#define CAP1114_NOISESTATUS1 0x0A //
#define CAP1114_NOISESTATUS2 0x0B //
#define CAP1114_LIDCLOSURESTATUS1 0x0C //Lid Closure Detection => x|S7_LID|S6_LID|S5_LID|S4_LID|S3_LID|S2_LID|S1_LID
#define CAP1114_LIDCLOSURESTATUS1 0x0D //Lid Closure Detection => x|S14_LID|S13_LID|S12_LID|S11_LID|S10_LID|S9_LID|S8_LID
#define CAP1114_GPIOSTATUS 0x0E //GPIO Input Status => GPIO_8|GPIO_7|GPIO_6|GPIO_5|GPIO_4|GPIO_3|GPIO_2|GPIO_1
#define CAP1114_GROUPSTATUS 0x0F //Group Status => LID|MULT|RESET|x|UP|DOWN|TAP|PH
#define CAP1114_SENSORDELTACOUNT1 0x10 //Sensor Sensitivity Set against Threshold => Sign|64|32|16|8|4|2|1
#define CAP1114_SENSORDELTACOUNT2 0x11 //Value represented in 2s compliment number
#define CAP1114_SENSORDELTACOUNT3 0x12 //Positive value capped at 7F 
#define CAP1114_SENSORDELTACOUNT4 0x13 //Negative value capped at FF
#define CAP1114_SENSORDELTACOUNT5 0x14
#define CAP1114_SENSORDELTACOUNT6 0x15
#define CAP1114_SENSORDELTACOUNT7 0x16
#define CAP1114_SENSORDELTACOUNT8 0x17
#define CAP1114_SENSORDELTACOUNT9 0x18
#define CAP1114_SENSORDELTACOUNT10 0x19
#define CAP1114_SENSORDELTACOUNT11 0x1A
#define CAP1114_SENSORDELTACOUNT12 0x1B
#define CAP1114_SENSORDELTACOUNT13 0x1C
#define CAP1114_SENSORDELTACOUNT14 0x1D
#define CAP1114_BUTTONQUEUECONTROL 0x1E //Consecutive Samples a Single Sensor Output is above Threshold before touch detected.
/* QUEUE_B[2:0] | NUMBER OF CONSECUTIVE READINGS > THRESHOLD 
 2 1 0
 0 0 0 | 1
 0 0 1 | 1
 0 1 0 | 2
 0 1 1 | 3 (default)
 1 0 0 | 4
 1 0 1 | 5
 1 1 0 | 6
 1 1 1 | 7*/
#define CAP1114_DATASENSITIVITY 0x1F //Sensitivity Control of All Button Channels => x|DELTA_SENSE[2:0]|BASE_SHIFT[3:0]
/* DELTA_SENSE[2:0] - Controls the sensitivity of a touch detection
 DELTA_SENSE[2:0] | SENSITIVITY MULTIPLIER
 2 1 0
 0 0 0 | 128x (most sensitive)
 0 0 1 | 64x
 0 1 0 | 32x (default)
 0 1 1 | 16x
 1 0 0 | 8x
 1 0 1 | 4x
 1 1 0 | 2x
 1 1 1 | 1x - (least sensitive)
 BASE_SHIFT [3:0] - Controls the scaling and data presentation*/
/*BASE_SHIFT[3:0] | DATA SCALING FACTOR 
 3 2 1 0
 0 0 0 0 | 1x
 0 0 0 1 | 2x
 0 0 1 0 | 4x
 0 0 1 1 | 8x
 0 1 0 0 | 16x
 0 1 0 1 | 32x
 0 1 1 0 | 64x
 0 1 1 1 | 128x
 1 0 0 0 | 256x
 All others | 256x(default = 1111b)*/
#define CAP1114_CONFIGREG 0x20 //General Global Functionality => 
#define CAP1114_SENSORENABLE 0x21 //Determines the capacitive Touch sensor input in sample cycle => GP_EN|S7_EN|S6_EN|S5_EN|S4_EN|S3_EN|S2_EN|S1_EN
#define CAP1114_BUTTONCONFIG 0x22 //Control timings with capacitive sensor ungrouped 1 till 7 => MAX_DUR_B[3:0]|RPT_RATE_B[3:0] => Refer Datasheet
#define CAP1114_GROUPCONFIG1 0x23 //Control timings with capacitive sensors grouped 8 till 14 => RPT_RATE_PH[3:0]|M_PRESS[3:0] => Refer Datasheet
#define CAP1114_GROUPCONFIG2 0x24 //Control timings with capacitive sensors grouped 8 till 14 => MAX_DUR_G[3:0]|RPT_RATE_SL[3:0] => Refer Datasheet
#define CAP1114_CALIBRATIONENABLE 0x25 //Control auto re-calibration of sensor => G_CEN|S7_CEN|S6_CEN|S5_CEN|S4_CEN|S3_CEN|S2_CEN|S1_CEN
#define CAP1114_CALIBRATIONACTIVATE 0x26 //Sensor Calibration Activate => G_CAL|S7_CAL|S6_CAL|S5_CAL|S4_CAL|S3_CAL|S2_CAL|S1_CAL
#define CAP1114_GROUPCALIBRATIONACTIVATE 0x46 //Group sensor Calibration Activate => - |S14_CAL|S13_CAL|S12_CAL|S11_CAL|S10_CAL|S9_CAL|S8_CAL
#define CAP1114_INT_ENABLE_REG1 0X27 //Detect Interrupt => SLIDE/TAP/PRESSNHOLD|CS7|CS6|CS5|CS4|CS3|CS2|CS1
#define CAP1114_INT_ENABLE_REG2 0X28 //Detect Interrupt => GPIO8|GPIO7|GPIO6|GPIO5|GPIO4|GPIO3|GPIO2|GPIO1
#define CAP1114_SLEEPCONTROL 0x29 //Sleep State Sensor Sampled => GR_SLEEP|S7_SLEEP|S6_SLEEP|S5_SLEEP|S4_SLEEP|S3_SLEEP|S2_SLEEP|S1_SLEEP
#define CAP1114_MTBLK 0x2A ////Detect if multiple touch has been observed => MULT_BLK_EN|x|x|x|B_MULT_T[1:0]|G_MULT_T[1:0]
/*B_MULT_T[1:0] | NUMBER OF SIMULTANEOUS TOUCHES 
 1 0
 0 0 | 1 (default)
 0 1 | 2
 1 0 | 3
 1 1 | 4*/
/*G_MULT_T[1:0] | NUMBER OF SIMULTANEOUS TOUCHES 
 1 0
 0 0 | 2
 0 1 | 3
 1 0 | 4 (default)
 1 1 | 1*/

//The  Sensor  LED  Linking  Register  controls  whether  
//a  Capacitive  Touch  Sensor  is  linked  to  an  LED output or not
//UP/DOWN|CS7-LED7|CS6-LED6|CS5-LED5|CS4-LED4|CS3-LED3|CS2-LED2|CS1-LED1
#define CAP1114_LEDLINK 0x80
//To detect the Product ID register 
#define CAP1114_PRODID 0xFD
//To detect the Manufacturing ID register
#define CAP1114_MANUID 0xFE
//Proximity sensor detection at CS1 input
#define CAP1114_PROXIMITYCTRL 0x42
//The LED Polarity Registers control the logical polarity 
//of the LED outputs.
//LED8_POL|LED7_POL|LED6_POL|LED5_POL|LED4_POL|LED3_POL|LED2_POL|LED1_POL
#define CAP1114_LEDPOL1 0x75
//--|--|--|--|--|LED11_POL|LED10_POL|LED9_POL
#define CAP1114_LEDPOL2 0x76
//Set the Direction Register for GPIO/LED 0=Input 1=Output
//LED8|LED7|LED6|LED5|LED4|LED3|LED2|LED1
#define CAP1114_LEDDIRECTION 0X70
#define CAP1114_LEDOUTPUTTYPE 0x71
//Set the Output for LED/GPIO
//LED8|LED7|LED6|LED5|LED4|LED3|LED2|LED1
#define CAP1114_LEDOUTPUT1 0x73
//--|--|--|--|--|LED11|LED10|LED9
#define CAP1114_LEDOUTPUT2 0x74
//To set the GPIO Input Register which reflects the state of LED/GPIO pins
//GPIO8|GPIO7|GPIO6|GPIO5|GPIO4|GPIO3|GPIO2|GPIO1
#define CAP1114_GPIOINPUTREG 0X72
//LED Behavior Registers
#define CAP1114_LEDBEHAVIOR1 0x81
#define CAP1114_LEDBEHAVIOR2 0x82
#define CAP1114_LEDBEHAVIOR3 0x83
//LED Pulse and Breathe Duty Cycle Registers
#define CAP1114_LEDDUTYCYCLE1 0x90
#define CAP1114_LEDDUTYCYCLE2 0x91
#define CAP1114_LEDDUTYCYCLE3 0x92
#define CAP1114_LEDDUTYCYCLE4 0x93

#define CAP1114_VELCONFIG 0x3E

}

#endif

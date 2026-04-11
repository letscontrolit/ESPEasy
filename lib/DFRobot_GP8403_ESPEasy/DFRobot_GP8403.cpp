/*!
 * @file DFRobot_GP8403.cpp
 * @brief This is a method implementation file for the DAC module.
 * @copyright	Copyright (c) 2021 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author [TangJie](jie.tang@dfrobot.com)
 * @version V1.0
 * @date 2022-2-18
 * @url https://github.com/DFRobot/DFRobot_GP8403
 */
#include "DFRobot_GP8403.h"
#ifdef GP8403_SINE_WAVE_ENABLED
const PROGMEM uint16_t DACLookup_FullSine_5Bit[32] =
{
  2048, 2447, 2831, 3185, 3495, 3750, 3939, 4056,
  4095, 4056, 3939, 3750, 3495, 3185, 2831, 2447,
  2048, 1648, 1264,  910,  600,  345,  156,   39,
     0,   39,  156,  345,  600,  910, 1264, 1648
};

const PROGMEM uint16_t DACLookup_FullSine_6Bit[64] =
{
  2048, 2248, 2447, 2642, 2831, 3013, 3185, 3346,
  3495, 3630, 3750, 3853, 3939, 4007, 4056, 4085,
  4095, 4085, 4056, 4007, 3939, 3853, 3750, 3630,
  3495, 3346, 3185, 3013, 2831, 2642, 2447, 2248,
  2048, 1847, 1648, 1453, 1264, 1082,  910,  749,
   600,  465,  345,  242,  156,   88,   39,   10,
     0,   10,   39,   88,  156,  242,  345,  465,
   600,  749,  910, 1082, 1264, 1453, 1648, 1847
};

const PROGMEM uint16_t DACLookup_FullSine_7Bit[128] =
{
  2048, 2148, 2248, 2348, 2447, 2545, 2642, 2737,
  2831, 2923, 3013, 3100, 3185, 3267, 3346, 3423,
  3495, 3565, 3630, 3692, 3750, 3804, 3853, 3898,
  3939, 3975, 4007, 4034, 4056, 4073, 4085, 4093,
  4095, 4093, 4085, 4073, 4056, 4034, 4007, 3975,
  3939, 3898, 3853, 3804, 3750, 3692, 3630, 3565,
  3495, 3423, 3346, 3267, 3185, 3100, 3013, 2923,
  2831, 2737, 2642, 2545, 2447, 2348, 2248, 2148,
  2048, 1947, 1847, 1747, 1648, 1550, 1453, 1358,
  1264, 1172, 1082,  995,  910,  828,  749,  672,
   600,  530,  465,  403,  345,  291,  242,  197,
   156,  120,   88,   61,   39,   22,   10,    2,
     0,    2,   10,   22,   39,   61,   88,  120,
   156,  197,  242,  291,  345,  403,  465,  530,
   600,  672,  749,  828,  910,  995, 1082, 1172,
  1264, 1358, 1453, 1550, 1648, 1747, 1847, 1947
};

const PROGMEM uint16_t DACLookup_FullSine_8Bit[256] =
{
  2048, 2098, 2148, 2198, 2248, 2298, 2348, 2398,
  2447, 2496, 2545, 2594, 2642, 2690, 2737, 2784,
  2831, 2877, 2923, 2968, 3013, 3057, 3100, 3143,
  3185, 3226, 3267, 3307, 3346, 3385, 3423, 3459,
  3495, 3530, 3565, 3598, 3630, 3662, 3692, 3722,
  3750, 3777, 3804, 3829, 3853, 3876, 3898, 3919,
  3939, 3958, 3975, 3992, 4007, 4021, 4034, 4045,
  4056, 4065, 4073, 4080, 4085, 4089, 4093, 4094,
  4095, 4094, 4093, 4089, 4085, 4080, 4073, 4065,
  4056, 4045, 4034, 4021, 4007, 3992, 3975, 3958,
  3939, 3919, 3898, 3876, 3853, 3829, 3804, 3777,
  3750, 3722, 3692, 3662, 3630, 3598, 3565, 3530,
  3495, 3459, 3423, 3385, 3346, 3307, 3267, 3226,
  3185, 3143, 3100, 3057, 3013, 2968, 2923, 2877,
  2831, 2784, 2737, 2690, 2642, 2594, 2545, 2496,
  2447, 2398, 2348, 2298, 2248, 2198, 2148, 2098,
  2048, 1997, 1947, 1897, 1847, 1797, 1747, 1697,
  1648, 1599, 1550, 1501, 1453, 1405, 1358, 1311,
  1264, 1218, 1172, 1127, 1082, 1038,  995,  952,
   910,  869,  828,  788,  749,  710,  672,  636,
   600,  565,  530,  497,  465,  433,  403,  373,
   345,  318,  291,  266,  242,  219,  197,  176,
   156,  137,  120,  103,   88,   74,   61,   50,
    39,   30,   22,   15,   10,    6,    2,    1,
     0,    1,    2,    6,   10,   15,   22,   30,
    39,   50,   61,   74,   88,  103,  120,  137,
   156,  176,  197,  219,  242,  266,  291,  318,
   345,  373,  403,  433,  465,  497,  530,  565,
   600,  636,  672,  710,  749,  788,  828,  869,
   910,  952,  995, 1038, 1082, 1127, 1172, 1218,
  1264, 1311, 1358, 1405, 1453, 1501, 1550, 1599,
  1648, 1697, 1747, 1797, 1847, 1897, 1947, 1997
};

const PROGMEM uint16_t DACLookup_FullSine_9Bit[512] =
{
  2048, 2073, 2098, 2123, 2148, 2174, 2199, 2224,
  2249, 2274, 2299, 2324, 2349, 2373, 2398, 2423,
  2448, 2472, 2497, 2521, 2546, 2570, 2594, 2618,
  2643, 2667, 2690, 2714, 2738, 2762, 2785, 2808,
  2832, 2855, 2878, 2901, 2924, 2946, 2969, 2991,
  3013, 3036, 3057, 3079, 3101, 3122, 3144, 3165,
  3186, 3207, 3227, 3248, 3268, 3288, 3308, 3328,
  3347, 3367, 3386, 3405, 3423, 3442, 3460, 3478,
  3496, 3514, 3531, 3548, 3565, 3582, 3599, 3615,
  3631, 3647, 3663, 3678, 3693, 3708, 3722, 3737,
  3751, 3765, 3778, 3792, 3805, 3817, 3830, 3842,
  3854, 3866, 3877, 3888, 3899, 3910, 3920, 3930,
  3940, 3950, 3959, 3968, 3976, 3985, 3993, 4000,
  4008, 4015, 4022, 4028, 4035, 4041, 4046, 4052,
  4057, 4061, 4066, 4070, 4074, 4077, 4081, 4084,
  4086, 4088, 4090, 4092, 4094, 4095, 4095, 4095,
  4095, 4095, 4095, 4095, 4094, 4092, 4090, 4088,
  4086, 4084, 4081, 4077, 4074, 4070, 4066, 4061,
  4057, 4052, 4046, 4041, 4035, 4028, 4022, 4015,
  4008, 4000, 3993, 3985, 3976, 3968, 3959, 3950,
  3940, 3930, 3920, 3910, 3899, 3888, 3877, 3866,
  3854, 3842, 3830, 3817, 3805, 3792, 3778, 3765,
  3751, 3737, 3722, 3708, 3693, 3678, 3663, 3647,
  3631, 3615, 3599, 3582, 3565, 3548, 3531, 3514,
  3496, 3478, 3460, 3442, 3423, 3405, 3386, 3367,
  3347, 3328, 3308, 3288, 3268, 3248, 3227, 3207,
  3186, 3165, 3144, 3122, 3101, 3079, 3057, 3036,
  3013, 2991, 2969, 2946, 2924, 2901, 2878, 2855,
  2832, 2808, 2785, 2762, 2738, 2714, 2690, 2667,
  2643, 2618, 2594, 2570, 2546, 2521, 2497, 2472,
  2448, 2423, 2398, 2373, 2349, 2324, 2299, 2274,
  2249, 2224, 2199, 2174, 2148, 2123, 2098, 2073,
  2048, 2023, 1998, 1973, 1948, 1922, 1897, 1872,
  1847, 1822, 1797, 1772, 1747, 1723, 1698, 1673,
  1648, 1624, 1599, 1575, 1550, 1526, 1502, 1478,
  1453, 1429, 1406, 1382, 1358, 1334, 1311, 1288,
  1264, 1241, 1218, 1195, 1172, 1150, 1127, 1105,
  1083, 1060, 1039, 1017,  995,  974,  952,  931,
   910,  889,  869,  848,  828,  808,  788,  768,
   749,  729,  710,  691,  673,  654,  636,  618,
   600,  582,  565,  548,  531,  514,  497,  481,
   465,  449,  433,  418,  403,  388,  374,  359,
   345,  331,  318,  304,  291,  279,  266,  254,
   242,  230,  219,  208,  197,  186,  176,  166,
   156,  146,  137,  128,  120,  111,  103,   96,
    88,   81,   74,   68,   61,   55,   50,   44,
    39,   35,   30,   26,   22,   19,   15,   12,
    10,    8,    6,    4,    2,    1,    1,    0,
     0,    0,    1,    1,    2,    4,    6,    8,
    10,   12,   15,   19,   22,   26,   30,   35,
    39,   44,   50,   55,   61,   68,   74,   81,
    88,   96,  103,  111,  120,  128,  137,  146,
   156,  166,  176,  186,  197,  208,  219,  230,
   242,  254,  266,  279,  291,  304,  318,  331,
   345,  359,  374,  388,  403,  418,  433,  449,
   465,  481,  497,  514,  531,  548,  565,  582,
   600,  618,  636,  654,  673,  691,  710,  729,
   749,  768,  788,  808,  828,  848,  869,  889,
   910,  931,  952,  974,  995, 1017, 1039, 1060,
  1083, 1105, 1127, 1150, 1172, 1195, 1218, 1241,
  1264, 1288, 1311, 1334, 1358, 1382, 1406, 1429,
  1453, 1478, 1502, 1526, 1550, 1575, 1599, 1624,
  1648, 1673, 1698, 1723, 1747, 1772, 1797, 1822,
  1847, 1872, 1897, 1922, 1948, 1973, 1998, 2023
};
#endif // ifdef GP8403_SINE_WAVE_ENABLED

#ifdef GP8403_STORE_ENABLED
#define GP8302_STORE_TIMING_HEAD            0x02  ///< Store function timing start head
#define GP8302_STORE_TIMING_ADDR            0x10  ///< The first address for entering store timing
#define GP8302_STORE_TIMING_CMD1            0x03  ///< The command 1 to enter store timing
#define GP8302_STORE_TIMING_CMD2            0x00  ///< The command 2 to enter store timing
#define GP8302_STORE_TIMING_DELAY           10    ///< Store procedure interval delay time: 10ms, more than 7ms
#define GP8302_STORE_TIMING_DELAY           10    ///< Store procedure interval delay time: 10ms, more than 7ms
#define I2C_CYCLE_TOTAL                     5     ///< Total I2C communication cycle
#define I2C_CYCLE_BEFORE                    1     ///< The first half cycle 2 of the total I2C communication cycle
#define I2C_CYCLE_AFTER                     2     ///< The second half cycle 3 of the total I2C communication cycle
#endif // ifdef GP8403_STORE_ENABLED

DFRobot_GP8403::DFRobot_GP8403(TwoWire *pWire,uint8_t addr)
{
  _pWire = pWire;
	_addr  = addr;
}

uint8_t DFRobot_GP8403::begin(void)
{
  // _pWire->begin();           // I2C bus is already initialized correctly by ESPEasy core
  // _pWire->setClock(400000);
  _pWire->beginTransmission(_addr);
  _pWire->write(OUTPUT_RANGE);
  if(_pWire->endTransmission() != 0)
    return 1;
  return 0;
}

void DFRobot_GP8403::setDACOutRange(eOutPutRange_t range)
{
	if(range == eOutPutRange_t::eOutputRange5V)
	{
		voltage = 5000;
	}else{
		voltage = 10000;
	}
  writeReg(OUTPUT_RANGE,&range,1);
}

void DFRobot_GP8403::setDACOutVoltage(uint16_t data, uint8_t channel)
{
  uint16_t dataTransmission = (uint16_t)(((float)data / voltage) * 4095);
	DBG(dataTransmission);
	dataTransmission = dataTransmission << 4;
  sendData(dataTransmission,channel);
}

#ifdef GP8403_SINE_WAVE_ENABLED
void DFRobot_GP8403::outputSin(uint16_t amp, uint16_t freq, uint16_t offset,uint8_t channel)
{
  uint64_t starttime;
  uint64_t stoptime;
  uint64_t looptime;
  uint64_t frame;
  uint16_t num=512;
  int16_t data = 0;
  #ifdef TWBR
    uint8_t twbrback = TWBR;
    TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz
  #endif
  if(freq < 8){
    num = 512;
  }else if( 8 <= freq && freq <= 16){
    num = 256;
  }else if(16 < freq && freq < 33){
    num = 128;
  }else if(33 <= freq && freq <= 68 ){
    num = 64;
  }else{
    num = 32;
  }
  if(freq > 100){
    freq = 100;
  }
  frame = 1000000/(freq*num);
  for(uint16_t i=0;i<num;i++){   
    starttime = micros();
    switch(num){
      case 512:
        data = ((int16_t)(pgm_read_word(&(DACLookup_FullSine_9Bit[i])) - 2047)*(amp/(float)voltage))*2;
        break;
      case 256:
        data = ((int16_t)(pgm_read_word(&(DACLookup_FullSine_8Bit[i])) - 2047)*(amp/(float)voltage))*2;
        break;
      case 128:
        data = ((int16_t)(pgm_read_word(&(DACLookup_FullSine_7Bit[i])) - 2047)*(amp/(float)voltage))*2;
        break;
      case 64:
        data = ((int16_t)(pgm_read_word(&(DACLookup_FullSine_6Bit[i])) - 2047)*(amp/(float)voltage))*2;
        break;
      case 32:
        data = ((int16_t)(pgm_read_word(&(DACLookup_FullSine_5Bit[i])) - 2047)*(amp/(float)voltage))*2;
        break;
      default:
        data = ((int16_t)(pgm_read_word(&(DACLookup_FullSine_5Bit[i])) - 2047)*(amp/(float)voltage))*2;
        break;
      }
      data = data+(offset*(4096/(float)voltage));
      if(data <= 0){
        data=0;
      }
      if(data >= 4095){
        data=4095;
      }
       
    data = data << 4;
    sendData(data,channel);
    stoptime = micros();
    looptime = stoptime-starttime;
    while(looptime <= frame){
      stoptime = micros();
      looptime = stoptime-starttime;
    }
  }
  #ifdef TWBR
    TWBR = twbrback;
  #endif
}
#endif // ifdef GP8403_SINE_WAVE_ENABLED

#ifdef GP8403_TRIANGLE_WAVE_ENABLED
void DFRobot_GP8403::outputTriangle(uint16_t amp, uint16_t freq, uint16_t offset, int8_t dutyCycle,uint8_t channel)
{
  uint64_t starttime;
  uint64_t stoptime;
  uint64_t looptime;
  uint64_t frame;
  uint16_t num = 64;
  uint16_t up_num;
  uint16_t down_num;
  uint16_t maxV;
  maxV=amp*(4096/(float)voltage);
  if(freq > 100){
    num = 16;
  }else if(50 <= freq && freq <= 100){
    num = 32;
  }else{
    num = 64;
  }
  frame = 1000000/(freq*num*2);
  if(dutyCycle>100){
    dutyCycle = 100;
  }
  if(dutyCycle<0){
    dutyCycle=0;
  }
  up_num = (2*num)*((float)dutyCycle/100);
  down_num = ((2*num) - up_num);
#ifdef TWBR
  uint8_t twbrback = TWBR;
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz
#endif
  uint16_t counter;
  int16_t enterV;
    
  for (counter = 0; counter < (maxV-(maxV/up_num)-1); counter+=(maxV/up_num)){
    starttime = micros();
    enterV=counter+(offset*(4096/(float)voltage));
    if(enterV > 4095){
      enterV = 4095;
    }else if(enterV < 0){
      enterV = 0;
    }
    enterV = enterV << 4;
    sendData(enterV,channel);
    stoptime = micros();
    looptime = stoptime-starttime;
    while(looptime <= frame){
      stoptime = micros();
      looptime = stoptime-starttime;
    }
  }
  for (counter = maxV-1; counter > (maxV/down_num); counter-=(maxV/down_num)){
    starttime = micros();
    enterV=counter+(offset*(4096/(float)voltage));
    if(enterV > 4095){
      enterV = 4095;
    }else if(enterV < 0){
      enterV = 0;
    }
    enterV = enterV << 4;
    sendData(enterV,channel);
    stoptime = micros();
    looptime = stoptime-starttime;
    while(looptime <= frame){
      stoptime = micros();
      looptime = stoptime-starttime;
    }
  }
#ifdef TWBR
    TWBR = twbrback;
#endif
}
#endif // ifdef GP8403_TRIANGLE_WAVE_ENABLED

#ifdef GP8403_SQUARE_WAVE_ENABLED
void DFRobot_GP8403::outputSquare(uint16_t amp, uint16_t freq, uint16_t offset, int8_t dutyCycle, uint8_t channel)
{
  uint64_t starttime;
  uint64_t stoptime;
  uint64_t looptime;
  uint64_t frame;
  uint16_t num = 64;
  uint16_t up_num;
  uint16_t down_num;
  uint16_t data;
  data=amp*(4096/(float)voltage);
  if(freq > 100){
    num = 16;
  }else if(50 <= freq && freq <= 100){
    num = 32;
  }else{
    num = 64;
  }
  frame = 1000000/(freq*num*2);
  if(dutyCycle>100){
    dutyCycle = 100;
  }
  if(dutyCycle<0){
    dutyCycle=0;
  }
  up_num = (2*num)*((float)dutyCycle/100);//64
  down_num = ((2*num) - up_num);//64
#ifdef TWBR
  uint8_t twbrback = TWBR;
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz
#endif
  uint16_t counter;
  int16_t enterV;
    
  for (counter = 0; counter < up_num; counter++){
    starttime = micros();
    enterV=data+(offset*(4096/(float)voltage));
    if(enterV > 4095){
      enterV = 4095;
    }else if(enterV < 0){
      enterV = 0;
    }
    enterV = enterV << 4;
    sendData(enterV,channel);
    stoptime = micros();
    looptime = stoptime-starttime;
    while(looptime <= frame){
      stoptime = micros();
      looptime = stoptime-starttime;
    }
  }
  for (counter=0;counter < down_num; counter++){
    starttime = micros();
    enterV=data-(offset*(4096/(float)voltage));
    if(enterV > 4095){
      enterV = 4095;
    }else if(enterV < 0){
      enterV = 0;
    }
    enterV = enterV << 4;
    sendData(enterV,channel);
    stoptime = micros();
    looptime = stoptime-starttime;
    while(looptime <= frame){
      stoptime = micros();
      looptime = stoptime-starttime;
    }
  }
#ifdef TWBR
    TWBR = twbrback;
#endif

}
#endif // ifdef GP8403_SQUARE_WAVE_ENABLED

void DFRobot_GP8403::sendData(uint16_t data, uint8_t channel)
{
  if(channel == 0){
    _pWire->beginTransmission(_addr);
    _pWire->write(GP8302_CONFIG_CURRENT_REG);
    _pWire->write(data & 0xff);
    _pWire->write((data >>8) & 0xff);
    _pWire->endTransmission();
    DBG(channel);
  }else if(channel == 1){
		_pWire->beginTransmission(_addr);
    _pWire->write(GP8302_CONFIG_CURRENT_REG<<1);
    _pWire->write(data & 0xff);
    _pWire->write((data >>8) & 0xff);
    _pWire->endTransmission();
    DBG(channel);
  }else{
    _pWire->beginTransmission(_addr);
    _pWire->write(GP8302_CONFIG_CURRENT_REG);
    _pWire->write(data & 0xff);
    _pWire->write((data >>8) & 0xff);
    _pWire->write(data & 0xff);
    _pWire->write((data >>8) & 0xff);
    _pWire->endTransmission();
    DBG(channel);
  }
}

#ifdef GP8403_STORE_ENABLED
void DFRobot_GP8403::store(){
  #if defined(ESP32)
    _pWire->~TwoWire();
  #elif !defined(ESP8266)
    _pWire->end();
  #endif
  pinMode(_scl, OUTPUT);
  pinMode(_sda, OUTPUT);
  digitalWrite(_scl, HIGH);
  digitalWrite(_sda, HIGH);
  startSignal();
  sendByte(GP8302_STORE_TIMING_HEAD, 0, 3, false);
  stopSignal();
  startSignal();
  sendByte(GP8302_STORE_TIMING_ADDR);
  sendByte(GP8302_STORE_TIMING_CMD1);
  stopSignal();
  
  startSignal();
  sendByte(_addr<<1, 1);
  sendByte(GP8302_STORE_TIMING_CMD2, 1);
  sendByte(GP8302_STORE_TIMING_CMD2, 1);
  sendByte(GP8302_STORE_TIMING_CMD2, 1);
  sendByte(GP8302_STORE_TIMING_CMD2, 1);
  sendByte(GP8302_STORE_TIMING_CMD2, 1);
  sendByte(GP8302_STORE_TIMING_CMD2, 1);
  sendByte(GP8302_STORE_TIMING_CMD2, 1);
  sendByte(GP8302_STORE_TIMING_CMD2, 1);
  stopSignal();

  delay(GP8302_STORE_TIMING_DELAY);

  startSignal();
  sendByte(GP8302_STORE_TIMING_HEAD, 0, 3, false);
  stopSignal();
  startSignal();
  sendByte(GP8302_STORE_TIMING_ADDR);
  sendByte(GP8302_STORE_TIMING_CMD2);
  stopSignal();
  _pWire->begin();
}

void DFRobot_GP8403::startSignal(void){
  digitalWrite(_scl,HIGH);
  digitalWrite(_sda,HIGH);
  delayMicroseconds(I2C_CYCLE_BEFORE);
  digitalWrite(_sda,LOW);
  delayMicroseconds(I2C_CYCLE_AFTER);
  digitalWrite(_scl,LOW);
  delayMicroseconds(I2C_CYCLE_TOTAL);
}

void DFRobot_GP8403::stopSignal(void){
  digitalWrite(_sda,LOW);
  delayMicroseconds(I2C_CYCLE_BEFORE);
  digitalWrite(_scl,HIGH);
  delayMicroseconds(I2C_CYCLE_TOTAL);
  digitalWrite(_sda,HIGH);
  delayMicroseconds(I2C_CYCLE_TOTAL);
}

uint8_t DFRobot_GP8403::sendByte(uint8_t data, uint8_t ack, uint8_t bits, bool flag){
  for(int i=bits-1; i>=0;i--){  
    if(data & (1<<i)){
      digitalWrite(_sda,HIGH);//Change the status of sda level during scl low level, and it lasts for some time
    }else{
      digitalWrite(_sda,LOW);
    }
    delayMicroseconds(I2C_CYCLE_BEFORE); 
    digitalWrite(_scl,HIGH);
    delayMicroseconds(I2C_CYCLE_TOTAL);
    //while(digitalRead(_scl) == 0){
        //delayMicroseconds(1);
    //}
    digitalWrite(_scl,LOW);
    delayMicroseconds(I2C_CYCLE_AFTER); 
  }
  if(flag) return recvAck(ack);
  else {
    digitalWrite(_sda,LOW);//
    digitalWrite(_scl,HIGH);//
    return 0;
  }
}

uint8_t DFRobot_GP8403::recvAck(uint8_t ack){
  uint8_t ack_=0;
  uint16_t errorTime = 0;
  pinMode(_sda,INPUT_PULLUP);
  digitalWrite(_sda,HIGH);
  delayMicroseconds(I2C_CYCLE_BEFORE);
  digitalWrite(_scl,HIGH);
  delayMicroseconds(I2C_CYCLE_AFTER);
  while(digitalRead(_sda) != ack){
      delayMicroseconds(1);
      errorTime++;
      if(errorTime > 250) break;
  }
  ack_=digitalRead(_sda);
  delayMicroseconds(I2C_CYCLE_BEFORE);
  digitalWrite(_scl,LOW);
  delayMicroseconds(I2C_CYCLE_AFTER);
  pinMode(_sda,OUTPUT);
  return ack_;
}
#endif // ifdef GP8403_STORE_ENABLED

void DFRobot_GP8403::writeReg(uint8_t reg, void *pBuf, size_t size)
{
  uint8_t *_pBuf = (uint8_t*)pBuf;
  _pWire->beginTransmission(_addr);
  _pWire->write(reg);

  for(size_t i = 0; i < size; i++){
    _pWire->write(_pBuf[i]);
  }
  _pWire->endTransmission();
}

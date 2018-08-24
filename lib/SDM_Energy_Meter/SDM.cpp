/* Library for reading SDM 120/220/230/630 Modbus Energy meters.
*  Reading via Hardware or Software Serial library & rs232<->rs485 converter
*  2016-2018 Reaper7 (tested on wemos d1 mini->ESP8266 with Arduino 1.9.0-beta & 2.4.1 esp8266 core)
*  crc calculation by Jaime Garc�a (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino/)
*/
//------------------------------------------------------------------------------
#include "SDM.h"
//------------------------------------------------------------------------------
#ifdef USE_HARDWARESERIAL
SDM::SDM(HardwareSerial& serial, long baud, int dere_pin, int config, bool swapuart) : sdmSer(serial) {
  this->_baud = baud;
  this->_config = config;
  this->_dere_pin = dere_pin;
  this->_swapuart = swapuart;
}
#else
SDM::SDM(ESPeasySoftwareSerial& serial, long baud, int dere_pin) : sdmSer(serial) {
  this->_baud = baud;
  this->_dere_pin = dere_pin;
}
#endif

SDM::~SDM() {
}

void SDM::begin(void) {
#ifdef USE_HARDWARESERIAL
  #ifdef ESP8266
    sdmSer.begin(_baud, (SerialConfig)_config);
  #else
    sdmSer.begin(_baud, _config);
  #endif
#else
  sdmSer.begin(_baud);
#endif
#ifdef USE_HARDWARESERIAL
  #ifdef ESP8266
    if (_swapuart)
      sdmSer.swap();
  #endif
#endif
  if (_dere_pin != NOT_A_PIN)	                                                  //set output pin mode for DE/RE pin when used (for control MAX485)
    pinMode(_dere_pin, OUTPUT);
}

float SDM::readVal(uint16_t reg, uint8_t node) {
  uint16_t temp;
  unsigned long resptime;
  uint8_t sdmarr[FRAMESIZE] = {node, SDM_B_02, 0, 0, SDM_B_05, SDM_B_06, 0, 0, 0};
  float res = NAN;
  uint16_t readErr = SDM_ERR_NO_ERROR;

  sdmarr[2] = highByte(reg);
  sdmarr[3] = lowByte(reg);

  temp = calculateCRC(sdmarr, FRAMESIZE - 3);                                   //calculate out crc only from first 6 bytes

  sdmarr[6] = lowByte(temp);
  sdmarr[7] = highByte(temp);

#ifndef USE_HARDWARESERIAL
  sdmSer.listen();                                                              //enable softserial rx interrupt
#endif

  while (sdmSer.available() > 0)  {                                             //read serial if any old data is available
    sdmSer.read();
  }

  if (_dere_pin != NOT_A_PIN)                                                   //transmit to SDM  -> DE Enable, /RE Disable (for control MAX485)
    digitalWrite(_dere_pin, HIGH);

  delay(2);                                                                     //fix for issue (nan reading) by sjfaustino: https://github.com/reaper7/SDM_Energy_Meter/issues/7#issuecomment-272111524

  sdmSer.write(sdmarr, FRAMESIZE - 1);                                          //send 8 bytes

  sdmSer.flush();                                                               //clear out tx buffer

  if (_dere_pin != NOT_A_PIN)                                                   //receive from SDM -> DE Disable, /RE Enable (for control MAX485)
    digitalWrite(_dere_pin, LOW);

  resptime = millis() + MAX_MILLIS_TO_WAIT;

  while (sdmSer.available() < FRAMESIZE) {
    if (resptime < millis()) {
      readErr = SDM_ERR_TIMEOUT;                                                //err debug (4)
      break;
    }
    yield();
  }

  if (readErr == SDM_ERR_NO_ERROR) {                                            //if no timeout...

    if(sdmSer.available() == FRAMESIZE) {

      for(int n=0; n<FRAMESIZE; n++) {
        sdmarr[n] = sdmSer.read();
      }

      if (sdmarr[0] == node && sdmarr[1] == SDM_B_02 && sdmarr[2] == SDM_REPLY_BYTE_COUNT) {

        if ((calculateCRC(sdmarr, FRAMESIZE - 2)) == ((sdmarr[8] << 8) | sdmarr[7])) {  //calculate crc from first 7 bytes and compare with received crc (bytes 7 & 8)
          ((uint8_t*)&res)[3]= sdmarr[3];
          ((uint8_t*)&res)[2]= sdmarr[4];
          ((uint8_t*)&res)[1]= sdmarr[5];
          ((uint8_t*)&res)[0]= sdmarr[6];
        } else {
          readErr = SDM_ERR_CRC_ERROR;                                          //err debug (1)
        }

      } else {
        readErr = SDM_ERR_WRONG_BYTES;                                          //err debug (2)
      }

    } else {
      readErr = SDM_ERR_NOT_ENOUGHT_BYTES;                                      //err debug (3)
    }

  }

  if (readErr != SDM_ERR_NO_ERROR) {                                            //if error then copy temp error value to global val and increment global error counter
    readingerrcode = readErr;
    readingerrcount++;
  }

#ifndef USE_HARDWARESERIAL
  sdmSer.end();                                                                 //disable softserial rx interrupt
#endif

  return (res);
}

uint16_t SDM::getErrCode(bool _clear) {
  uint16_t _tmp = readingerrcode;
  if (_clear == true)
    clearErrCode();
  return (_tmp);
}

uint16_t SDM::getErrCount(bool _clear) {
  uint16_t _tmp = readingerrcount;
  if (_clear == true)
    clearErrCount();
  return (_tmp);
}

void SDM::clearErrCode() {
  readingerrcode = SDM_ERR_NO_ERROR;
}

void SDM::clearErrCount() {
  readingerrcount = 0;
}

uint16_t SDM::calculateCRC(uint8_t *array, uint8_t num) {
  uint16_t _crc, _flag;
  _crc = 0xFFFF;
  for (uint8_t i = 0; i < num; i++) {
    _crc = _crc ^ array[i];
    for (uint8_t j = 8; j; j--) {
      _flag = _crc & 0x0001;
      _crc >>= 1;
      if (_flag)
        _crc ^= 0xA001;
    }
  }
  return _crc;
}

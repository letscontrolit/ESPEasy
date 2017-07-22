//#######################################################################################################
//#################################### Plugin 056: Dust Sensor SDS011 / SDS018 ##########################
//#######################################################################################################
/*
  Plugin is based upon SDS011 dust sensor PM2.5 and PM10 lib (https://github.com/ricki-z/SDS011.git) by R. Zschiegner (rz@madavi.de)
  This plug in is written by Jochen Krapf (jk@nerd2nerd.org)

  This plugin reads the particle concentration from SDS011 Sensor
  DevicePin1 - RX on ESP, TX on SDS
*/

#ifdef PLUGIN_BUILD_DEV

#define PLUGIN_056
#define PLUGIN_ID_056         56
#define PLUGIN_NAME_056       "Dust Sensor - SDS011/018/198 [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_056 "PM2.5"   // Dust <2.5µm in µg/m³
#define PLUGIN_VALUENAME2_056 "PM10"    // Dust <10µm in µg/m³   SDS198:<100µm in µg/m³
//#define PLUGIN_READ_TIMEOUT   3000

//#include <SDS011.h>   //https://github.com/ricki-z/SDS011.git
#include <SoftwareSerial.h>


#define SERIALBUFFER_SIZE 32
#define SERIALBUFFER_MASK 31

class SensorSerial : public SoftwareSerial
{
public:
  SensorSerial(int receivePin, int transmitPin = -1, bool inverse_logic = false, unsigned int buffSize = 64) : SoftwareSerial(receivePin, transmitPin, inverse_logic, buffSize)
  {
    //boolean sws = isValidGPIOpin(receivePin);
    _hw = receivePin < 0;
  }

  void begin(long speed)
  {
    if (_hw)
      Serial.begin(speed);
    else
      SoftwareSerial::begin(speed);
  };

  int peek()
  {
    if (_hw)
      return Serial.peek();
    else
      return SoftwareSerial::peek();
  };

  virtual size_t write(uint8_t byte)
  {
    if (_hw)
      return Serial.write(byte);
    else
      return SoftwareSerial::write(byte);
  };

  virtual int read()
  {
    if (_hw)
      return Serial.read();
    else
      return SoftwareSerial::read();
  };

  virtual int available()
  {
    if (_hw)
      return Serial.available();
    else
      return SoftwareSerial::available();
  };

  virtual void flush()
  {
    if (_hw)
      Serial.flush();
    else
      SoftwareSerial::flush();
  };

protected:
  boolean _hw;
};

class CSerialBuffer
{
public:
  CSerialBuffer()
  {
    _writeIndex = 0;
    _packetLength = 0;
    Clear();
  };

  void Clear ()
  {
    for (byte i=0; i<SERIALBUFFER_SIZE; i++)
      _buffer[i] = 0;
  };

  void AddData (byte b)
  {
    _buffer[_writeIndex] = b;
    _writeIndex++;
    _writeIndex &= SERIALBUFFER_MASK;
  };

  void SetPacketLength (byte len)
  {
    _packetLength = len;
  };

  byte& operator[] (byte x)
  {
    x += _writeIndex;
    x -= _packetLength;
    x &= SERIALBUFFER_MASK;
    return _buffer[x];
  }

private:
  byte _buffer[SERIALBUFFER_SIZE];
  byte _writeIndex;
  byte _packetLength;
};

class CjkSDS011
{
public:
  CjkSDS011(int16_t pinRX, int16_t pinTX) : _serial(pinRX, pinTX)
  {
    _sws = ! ( pinRX < 0 || pinRX == 3 );
    _pm2_5 = NAN;
    _pm10_ = NAN;
    _available = false;
    _pm2_5avr = 0;
    _pm10_avr = 0;
    _avr = 0;
    _data.SetPacketLength(10);

    _serial.begin(9600);
  };

  void Process()
  {
    while (_serial.available())
    {
    	_data.AddData(_serial.read());

      if (_data[0] == 0xAA && _data[9] == 0xAB && (_data[1] == 0xC0 || _data[1] == 0xCF))   // correct packet frame?
      {
        byte checksum = 0;
        for (byte i=2; i<= 7; i++)
        checksum += _data[i];
        if (checksum != _data[8])
          continue;

        if (_data[1] == 0xC0)   // SDS011 or SDS018?
        {
          _pm2_5 = (float)((_data[3] << 8) | _data[2]) * 0.1;
          _pm10_ = (float)((_data[5] << 8) | _data[4]) * 0.1;
          _available = true;
        }
        else if (_data[1] == 0xCF)   // SDS198?
        {
          _pm2_5 = (float)((_data[5] << 8) | _data[4]);
          _pm10_ = (float)((_data[3] << 8) | _data[2]);
          _available = true;
        }
        else
          continue;

        _pm2_5avr += _pm2_5;
        _pm10_avr += _pm10_;
        _avr++;
        return;
      }
    }
  };

  boolean available()
  {
    boolean ret = _available;
    _available = false;
    return ret;
  };

  float GetPM2_5() { return _pm2_5; };
  float GetPM10_() { return _pm10_; };

  void ReadAverage(float &pm25, float &pm10)
  {
    if (_avr)
    {
      pm25 = _pm2_5avr / _avr;
      pm10 = _pm10_avr / _avr;
      _pm2_5avr = 0;
      _pm10_avr = 0;
      _avr = 0;
    }
    else
    {
      pm25 = NAN;
      pm10 = NAN;
    }
  }

private:
  SensorSerial _serial;
  CSerialBuffer _data;
  float _pm2_5;
  float _pm10_;
  float _pm2_5avr;
  float _pm10_avr;
  uint16_t _avr;
  boolean _available;
  boolean _sws;
};


CjkSDS011 *Plugin_056_SDS = NULL;


boolean Plugin_056(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_056;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_056);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_056));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_056));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = F("GPIO &larr; TX");
        //event->String2 = F("GPIO &#8674; RX (optional)");
        break;
      }

    case PLUGIN_INIT:
      {
        if (Plugin_056_SDS)
          delete Plugin_056_SDS;
        Plugin_056_SDS = new CjkSDS011(Settings.TaskDevicePin1[event->TaskIndex], -1);
        addLog(LOG_LEVEL_INFO, F("SDS  : Init OK "));

        //delay first read, because hardware needs to initialize on cold boot
        //otherwise we get a weird value or read error
        //JK timerSensor[event->TaskIndex] = millis() + 15000;

        success = true;
        break;
      }

    case PLUGIN_EXIT:
      {
        if (Plugin_056_SDS)
          delete Plugin_056_SDS;
        addLog(LOG_LEVEL_INFO, F("SDS  : Exit"));
        break;
      }

/*    case PLUGIN_WRITE:
      {
        if (!Plugin_056_SDS)
          break;

        String command = parseString(string, 1);

        if (command == F("sdssleep"))
        {
          //JK Plugin_056_SDS->sleep();;
          addLog(LOG_LEVEL_INFO, F("SDS  : sleep"));
          success = true;
        }
        if (command == F("sdswakeup"))
        {
          //JK Plugin_056_SDS->wakeup();;
          addLog(LOG_LEVEL_INFO, F("SDS  : wake up"));
          success = true;
        }
        break;
      }
*/
    case PLUGIN_FIFTY_PER_SECOND:
      {
        if (!Plugin_056_SDS)
          break;

        Plugin_056_SDS->Process();

        if (Plugin_056_SDS->available())
        {
          String log = F("SDS  : act ");
          log += Plugin_056_SDS->GetPM2_5();
          log += F(" ");
          log += Plugin_056_SDS->GetPM10_();
          addLog(LOG_LEVEL_INFO, log);

          if (Settings.TaskDeviceTimer[event->TaskIndex] == 0)
          {
            UserVar[event->BaseVarIndex + 0] = Plugin_056_SDS->GetPM2_5();
            UserVar[event->BaseVarIndex + 1] = Plugin_056_SDS->GetPM10_();
            event->sensorType = SENSOR_TYPE_DUAL;
            sendData(event);
          }
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (!Plugin_056_SDS)
          break;

        float pm25, pm10;
        Plugin_056_SDS->ReadAverage(pm25, pm10);

        UserVar[event->BaseVarIndex + 0] = pm25;
        UserVar[event->BaseVarIndex + 1] = pm10;
        success = true;
        break;
      }
  }

  return success;
}

#endif   //PLUGIN_BUILD_DEV

/*
    Class to send IR signals using the Arduino PWM
*/
#ifndef IRSender_h
#define IRSender_h

#include <Arduino.h>

#if defined(ESP8266) || defined(ESP32)
#include <IRsend.h>  // From IRremoteESP8266 library
#include <stdint.h>
#endif

class IRSender
{
  protected:
    IRSender(uint8_t pin); // Cannot create generic IRSender instances

  public:
    virtual ~IRSender() = default;
    virtual void setFrequency(int frequency);
    void sendIRbyte(uint8_t sendByte, int bitMarkLength, int zeroSpaceLength, int oneSpaceLength);
    uint8_t bitReverse(uint8_t x);
    virtual void space(int spaceLength);
    virtual void mark(int markLength);

  protected:
    uint8_t _pin;
};


class IRSenderPWM : public IRSender
{
  public:
    IRSenderPWM(uint8_t pin);
    void setFrequency(int frequency);
    void space(int spaceLength);
    void mark(int markLength);
};


class IRSenderBlaster : public IRSender
{
  public:
    IRSenderBlaster(uint8_t pin);
    void setFrequency(int frequency);
    void space(int spaceLength);
    void mark(int markLength);
};

class IRSenderBitBang : public IRSender
{
  public:
    IRSenderBitBang(uint8_t pin);
    void setFrequency(int frequency);
    void space(int spaceLength);
    void mark(int markLength);

  protected:
    int _halfPeriodicTime;
};

#ifdef ESP32
class IRSenderESP32 : public IRSender
{
  public:
    IRSenderESP32(uint8_t pin, uint8_t pwmChannel);
    void setFrequency(int frequency);
    void space(int spaceLength);
    void mark(int markLength);

  protected:
    uint32_t _frequency;
    uint8_t _pwmChannel;
};
#endif

#if defined(ESP8266) || defined(ESP32)
class IRSenderIRremoteESP8266 : public IRSender
{
  public:
    IRSenderIRremoteESP8266(uint8_t pin);
    void setFrequency(int frequency);
    void space(int spaceLength);
    void mark(int markLength);

  private:
    IRsend _ir;
};

class IRSenderESP8266 : public IRSender
{
  public:
    IRSenderESP8266(uint8_t pin);
    void setFrequency(int frequency);
    void space(int spaceLength);
    void mark(int markLength);

  protected:
    uint32_t _halfPeriodicTime;
};

class IRSenderESP8266Alt : public IRSender
{
  public:
    IRSenderESP8266Alt(uint8_t pin);
    void setFrequency(int frequency);
    void space(int spaceLength);
    void mark(int markLength);

  protected:
    uint32_t _halfPeriodicTime;
};
#endif

#endif
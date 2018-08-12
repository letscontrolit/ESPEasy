/*
    Class to send IR signals using the Arduino PWM
*/
#ifndef IRSender_h
#define IRSender_h

#include <Arduino.h>

#ifdef ESP8266
#include <IRsend.h>  // From IRremoteESP8266 library
#endif

class IRSender
{
  protected:
    IRSender(uint8_t pin); // Cannot create generic IRSender instances

  public:
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

#ifdef ESP8266
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
#endif

#endif

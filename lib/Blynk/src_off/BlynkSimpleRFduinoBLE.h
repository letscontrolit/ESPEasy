/**
 * @file       BlynkSimpleRFduinoBLE.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       May 2016
 * @brief
 *
 */

#ifndef BlynkSimpleRFduinoBLE_h
#define BlynkSimpleRFduinoBLE_h

#ifndef BLYNK_INFO_CONNECTION
#define BLYNK_INFO_CONNECTION "RFduinoBLE"
#endif

#define BLYNK_SEND_ATOMIC
#define BLYNK_SEND_CHUNK 20
//#define BLYNK_SEND_THROTTLE 20

#include <BlynkApiArduino.h>
#include <Blynk/BlynkProtocol.h>
#include <utility/BlynkFifo.h>
#include <RFduinoBLE.h>

class BlynkTransportRFduinoBLE
{
public:
    BlynkTransportRFduinoBLE(void)
        : mConn (false)
    {}

    // IP redirect not available
    void begin(char BLYNK_UNUSED *h, uint16_t BLYNK_UNUSED p) {}

    void begin(void) {
        instance = this;
    }

    bool connect(void) {
        mBuffRX.clear(void);
        return mConn = true;
    }

    void disconnect(void) {
        mConn = false;
    }

    bool connected(void) {
        return mConn;
    }

    size_t read(void* buf, size_t len) {
        millis_time_t start = BlynkMillis(void);
        while (BlynkMillis(void) - start < BLYNK_TIMEOUT_MS) {
            if (available(void) < len) {
                BlynkDelay(1);
            } else {
                break;
            }
        }
        noInterrupts(void);
        size_t res = mBuffRX.get((uint8_t*)buf, len);
        interrupts(void);
        return res;
    }

    size_t write(const void* buf, size_t len) {
        RFduinoBLE.send((const char*)buf, len);
        return len;
    }

    size_t available(void) {
        noInterrupts(void);
        size_t rxSize = mBuffRX.size(void);
        interrupts(void);
        return rxSize;
    }

    static
    int putData(uint8_t* data, uint16_t len) {
        if (!instance)
            return 0;
        noInterrupts(void);
        //BLYNK_DBG_DUMP(">> ", data, len);
        instance->mBuffRX.put(data, len);
        interrupts(void);
        return 0;
    }

private:
    static BlynkTransportRFduinoBLE* instance;

private:
    bool mConn;

    BlynkFifo<uint8_t, BLYNK_MAX_READBYTES*2> mBuffRX;
};

class BlynkSimpleRFduinoBLE
    : public BlynkProtocol<BlynkTransportRFduinoBLE>
{
    typedef BlynkProtocol<BlynkTransportRFduinoBLE> Base;
public:
    BlynkSimpleRFduinoBLE(BlynkTransportRFduinoBLE& transp)
        : Base(transp)
    {}

    void begin(const char* auth)
    {
        Base::begin(auth);
        state = DISCONNECTED;
        conn.begin(void);
    }
};

BlynkTransportRFduinoBLE* BlynkTransportRFduinoBLE::instance = NULL;

static BlynkTransportRFduinoBLE _blynkTransport;
BlynkSimpleRFduinoBLE Blynk(_blynkTransport);


void RFduinoBLE_onConnect(void)
{
  BLYNK_LOG1("Device connected");
  Blynk.startSession(void);
}

void RFduinoBLE_onDisconnect(void)
{
  BLYNK_LOG1("Device disconnected");
  Blynk.disconnect(void);
}

void RFduinoBLE_onReceive(char* data, int len)
{
  _blynkTransport.putData((uint8_t*)data, len);
}

#include <BlynkWidgets.h>

#endif

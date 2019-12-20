/**
 * @file       BlynkSimpleEsp32_BLE.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Nov 2017
 * @brief
 *
 */

#ifndef BlynkSimpleEsp32_BLE_h
#define BlynkSimpleEsp32_BLE_h

#ifndef BLYNK_INFO_CONNECTION
#define BLYNK_INFO_CONNECTION "Esp32_BLE"
#endif

#define BLYNK_SEND_ATOMIC
#define BLYNK_SEND_CHUNK 20
//#define BLYNK_SEND_THROTTLE 20

#include <BlynkApiArduino.h>
#include <Blynk/BlynkProtocol.h>
#include <utility/BlynkFifo.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID           "713D0000-503E-4C75-BA94-3148F18D941E"
#define CHARACTERISTIC_UUID_RX "713D0003-503E-4C75-BA94-3148F18D941E"
#define CHARACTERISTIC_UUID_TX "713D0002-503E-4C75-BA94-3148F18D941E"

class BlynkTransportEsp32_BLE :
    public BLEServerCallbacks,
    public BLECharacteristicCallbacks
{

public:
    BlynkTransportEsp32_BLE(void)
        : mConn (false)
        , mName ("Blynk")
    {}

    void setDeviceName(const char* name) {
        mName = name;
    }

    // IP redirect not available
    void begin(char BLYNK_UNUSED *h, uint16_t BLYNK_UNUSED p) {}

    void begin(void) {
        // Create the BLE Device
        BLEDevice::init(mName);

        // Create the BLE Server
        pServer = BLEDevice::createServer(void);
        pServer->setCallbacks(this);

        // Create the BLE Service
        pService = pServer->createService(SERVICE_UUID);

        // Create a BLE Characteristic
        pCharacteristicTX = pService->createCharacteristic(
                            CHARACTERISTIC_UUID_TX,
                            BLECharacteristic::PROPERTY_NOTIFY
                          );

        pCharacteristicTX->addDescriptor(new BLE2902(void));

        pCharacteristicRX = pService->createCharacteristic(
                                              CHARACTERISTIC_UUID_RX,
                                              BLECharacteristic::PROPERTY_WRITE
                                            );

        pCharacteristicRX->setCallbacks(this);

        // Start the service
        pService->start(void);

        // Start advertising
        pServer->getAdvertising(void)->addServiceUUID(pService->getUUID(void));
        pServer->getAdvertising(void)->start(void);
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
                delay(1);
            } else {
                break;
            }
        }
        size_t res = mBuffRX.get((uint8_t*)buf, len);
        return res;
    }

    size_t write(const void* buf, size_t len) {
        pCharacteristicTX->setValue((uint8_t*)buf, len);
        pCharacteristicTX->notify(void);
        return len;
    }

    size_t available(void) {
        size_t rxSize = mBuffRX.size(void);
        return rxSize;
    }

private:

    void onConnect(BLEServer* pServer);
    void onDisconnect(BLEServer* pServer);

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue(void);

      if (rxValue.length(void) > 0) {
        uint8_t* data = (uint8_t*)rxValue.data(void);
        size_t len = rxValue.length(void);

        BLYNK_DBG_DUMP(">> ", data, len);
        mBuffRX.put(data, len);
      }
    }

private:
    bool mConn;
    const char* mName;

    BLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *pCharacteristicTX;
    BLECharacteristic *pCharacteristicRX;

    BlynkFifo<uint8_t, BLYNK_MAX_READBYTES*2> mBuffRX;
};

class BlynkEsp32_BLE
    : public BlynkProtocol<BlynkTransportEsp32_BLE>
{
    typedef BlynkProtocol<BlynkTransportEsp32_BLE> Base;
public:
    BlynkEsp32_BLE(BlynkTransportEsp32_BLE& transp)
        : Base(transp)

    {}

    void begin(const char* auth)
    {
        Base::begin(auth);
        state = DISCONNECTED;
        conn.begin(void);
    }

    void setDeviceName(const char* name) {
        conn.setDeviceName(name);
    }

};


static BlynkTransportEsp32_BLE _blynkTransportBLE;
BlynkEsp32_BLE Blynk(_blynkTransportBLE);

inline
void BlynkTransportEsp32_BLE::onConnect(BLEServer* pServer) {
  BLYNK_LOG1(BLYNK_F("BLE connect"));
  connect(void);
  Blynk.startSession(void);
};

inline
void BlynkTransportEsp32_BLE::onDisconnect(BLEServer* pServer) {
  BLYNK_LOG1(BLYNK_F("BLE disconnect"));
  Blynk.disconnect(void);
  disconnect(void);
}


#include <BlynkWidgets.h>

#endif

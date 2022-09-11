/**
 * @file
 *
 * EspNowBroadcast.ino demonstrates how to perform ESP-NOW pseudo broadcast with @c WifiEspNowBroadcast .
 * You need two or more ESP8266 or ESP32 devices to run this example.
 *
 * All devices should run the same program.
 * You may need to modify the PIN numbers so that you can observe the effect.
 *
 * With the program running on several devices:
 * @li Press the button to transmit a message.
 * @li When a device receives a message, it will toggle its LED between "on" and "off" states.
 */

#include <WifiEspNowBroadcast.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

/**
 * @brief PIN number of a button.
 *
 * The default `0` is the "flash" button on NodeMCU, Witty Cloud, Heltec WiFi_Kit_32, etc.
 */
static const int BUTTON_PIN = 0;

/**
 * @brief PIN number of an LED.
 *
 * The default `2` is the blue LED on ESP-12F.
 */
static const int LED_PIN = 2;

int ledState = HIGH;

void
processRx(const uint8_t mac[WIFIESPNOW_ALEN], const uint8_t* buf, size_t count, void* arg)
{
  Serial.printf("Message from %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3],
                mac[4], mac[5]);
  for (size_t i = 0; i < count; ++i) {
    Serial.print(static_cast<char>(buf[i]));
  }
  Serial.println();

  digitalWrite(LED_PIN, ledState);
  ledState = 1 - ledState;
}

void
setup()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.persistent(false);
  bool ok = WifiEspNowBroadcast.begin("ESPNOW", 3);
  if (!ok) {
    Serial.println("WifiEspNowBroadcast.begin() failed");
    ESP.restart();
  }
  // WifiEspNowBroadcast.begin() function sets WiFi to AP+STA mode.
  // The AP interface is also controlled by WifiEspNowBroadcast.
  // You may use the STA interface after calling WifiEspNowBroadcast.begin().
  // For best results, ensure all devices are using the same WiFi channel.

  WifiEspNowBroadcast.onReceive(processRx, nullptr);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);

  Serial.print("MAC address of this node is ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println("Press the button to send a message");
}

void
sendMessage()
{
  char msg[60];
  int len = snprintf(msg, sizeof(msg), "hello ESP-NOW from %s at %lu",
                     WiFi.softAPmacAddress().c_str(), millis());
  WifiEspNowBroadcast.send(reinterpret_cast<const uint8_t*>(msg), len);

  Serial.println("Sending message");
  Serial.println(msg);
  Serial.print("Recipients:");
  const int MAX_PEERS = 20;
  WifiEspNowPeerInfo peers[MAX_PEERS];
  int nPeers = std::min(WifiEspNow.listPeers(peers, MAX_PEERS), MAX_PEERS);
  for (int i = 0; i < nPeers; ++i) {
    Serial.printf(" %02X:%02X:%02X:%02X:%02X:%02X", peers[i].mac[0], peers[i].mac[1],
                  peers[i].mac[2], peers[i].mac[3], peers[i].mac[4], peers[i].mac[5]);
  }
  Serial.println();
}

void
loop()
{
  if (digitalRead(BUTTON_PIN) == LOW) { // button is pressed
    sendMessage();

    while (digitalRead(BUTTON_PIN) == LOW) // wait for button release
      ;
  }

  WifiEspNowBroadcast.loop();
  delay(10);
}

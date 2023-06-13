#pragma once

#include <Stream.h>
#include "ImprovTypes.h"

#ifdef ARDUINO
# include <Arduino.h>
#endif // ifdef ARDUINO

/**
 * Improv WiFi class
 *
 * ### Description
 *
 * Handles the Improv WiFi Serial protocol (https://www.improv-wifi.com/serial/)
 *
 * ### Example
 *
 * Simple example of using ImprovWiFi lib. A complete one can be seen in `examples/` folder.
 *
 * ```cpp
 * #include <ImprovWiFiLibrary.h>
 *
 * ImprovWiFi improvSerial(&Serial);
 *
 * void setup() {
 *   improvSerial.setDeviceInfo("My-Device-9a4c2b", "2.1.5", "My Device");
 * }
 *
 * void loop() {
 *   improvSerial.handleSerial();
 * }
 * ```
 *
 */
class ImprovWiFi {
private:

  ImprovTypes::ImprovWiFiParamsStruct improvWiFiParams;

  uint8_t _buffer[128]{};
  uint8_t _position = 0;

  Stream *_serial = nullptr;

  void           sendDeviceUrl(ImprovTypes::Command cmd);
  bool           onCommandCallback(ImprovTypes::ImprovCommand cmd);
  void           onErrorCallback(ImprovTypes::Error err);
  void           setState(ImprovTypes::State state);
  void           sendResponse(const std::vector<uint8_t>& response);
  void           setError(ImprovTypes::Error error);

  void           send(uint8_t                     improvType,
                      const std::vector<uint8_t>& response);
  static uint8_t computeChecksum(const uint8_t *data,
                                 size_t         length);

  void           getAvailableWifiNetworks();
  inline void    replaceAll(std::string      & str,
                            const std::string& from,
                            const std::string& to);

  // improv SDK
  ImprovTypes::ParseState    parseImprovSerial(size_t         position,
                                               uint8_t        byte,
                                               const uint8_t *buffer);
  ImprovTypes::ImprovCommand parseImprovData(const std::vector<uint8_t>& data,
                                             bool                        check_checksum = true);
  ImprovTypes::ImprovCommand parseImprovData(const uint8_t *data,
                                             size_t         length,
                                             bool           check_checksum = true);
  std::vector<uint8_t>       build_rpc_response(ImprovTypes::Command            command,
                                                const std::vector<std::string>& datum,
                                                bool                            add_checksum);

public:

  /**
   * ## Constructors
   **/

  /**
   * Create an instance of ImprovWiFi
   *
   * # Parameters
   *
   * - `serial` - Pointer to stream object used to handle requests, for the most cases use `Serial`
   */
  ImprovWiFi(Stream *serial) : _serial(serial) {}

  ImprovWiFi() : _serial(nullptr) {}

  /**
   * ## Type definition
   */

  /**
   * Callback function called when any error occurs during the protocol handling or wifi connection.
   */
  typedef void (OnImprovError)(ImprovTypes::Error);

  /**
   * Callback function called when the attempt of wifi connection is successful. It informs the SSID and Password used to that, it's a
   *perfect time to save them for further use.
   */
  typedef void (OnImprovConnected)(const char *ssid,
                                   const char *password);

  /**
   * Callback function to customize the wifi connection if you needed. Optional.
   */
  typedef bool (CustomConnectWiFi)(const char *ssid,
                                   const char *password);

  /**
   * ## Methods
   **/

  /**
   * Check if a communication via serial is happening. Put this call on your loop().
   *
   */
  bool handleSerial();

  /**
   * Process read byte b.
   * When a reply is suitable, use serialForWrite to reply.
   * This way you can use peek bytes, or plug in IMPROV handling in your console code.
   *
   */
  ImprovTypes::ParseState handleSerial(uint8_t b,
                    Stream *serialForWrite);

  /**
   * Set details of your device.
   *
   * # Parameters
   *
   * - `firmwareName` - Firmware name
   * - `firmwareVersion` - Firmware version
   * - `deviceName` - Your device name
   * - `deviceUrl`- The local URL to access your device. A placeholder called {LOCAL_IPV4} is available to form elaboreted URLs. E.g.
   *`http://{LOCAL_IPV4}?name=Guest`.
   *   There is overloaded method without `deviceUrl`, in this case the URL will be the local IP.
   *
   */
  void setDeviceInfo(const char *firmwareName,
                     const char *firmwareVersion,
                     const char *deviceName,
                     const char *deviceUrl);
  void setDeviceInfo(const char *firmwareName,
                     const char *firmwareVersion,
                     const char *deviceName);
  void setDeviceChipInfo(const char *chipVariant);

  /**
   * Method to set the typedef OnImprovError callback.
   */
  void onImprovError(OnImprovError *errorCallback);

  /**
   * Method to set the typedef OnImprovConnected callback.
   */
  void onImprovConnected(OnImprovConnected *connectedCallback);

  /**
   * Method to set the typedef CustomConnectWiFi callback.
   */
  void setCustomTryConnectToWiFi(CustomConnectWiFi *connectWiFiCallBack);

  /**
   * Default method to connect in a WiFi network.
   * It waits `DELAY_MS_WAIT_WIFI_CONNECTION` milliseconds (default 500) during `MAX_ATTEMPTS_WIFI_CONNECTION` (default 20) until it get
   *connected. If it does not happen, an error `ERROR_UNABLE_TO_CONNECT` is thrown.
   *
   */
  bool tryConnectToWifi(const char *ssid,
                        const char *password);

  /**
   * Check if connection is established using `WiFi.status() == WL_CONNECTED`
   *
   */
  bool isConnected();

private:

  OnImprovError     *onImprovErrorCallback          = nullptr;
  OnImprovConnected *onImprovConnectedCallback      = nullptr;
  CustomConnectWiFi *customTryConnectToWiFiCallback = nullptr;
};

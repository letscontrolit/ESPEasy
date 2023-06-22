> This document was generated from file `ImprovWiFiLibrary.h` at 3/30/2023, 7:34:30 PM
<a name="line-11"></a>
# ImprovWiFi

```cpp
class ImprovWiFi /* line 37 */
```

Improv WiFi class

#### Description

Handles the Improv WiFi Serial protocol (https://www.improv-wifi.com/serial/)

#### Example

Simple example of using ImprovWiFi lib. A complete one can be seen in `examples/` folder.

```cpp
#include <ImprovWiFiLibrary.h>

ImprovWiFi improvSerial(&Serial);

void setup() {
  improvSerial.setDeviceInfo("My-Device-9a4c2b", "2.1.5", "My Device");
}

void loop() {
  improvSerial.handleSerial();
}
```


<a name="line-64"></a>
## Constructors

<a name="line-68"></a>
### 💡 ImprovWiFi(Stream *serial)

```cpp
ImprovWiFi(Stream *serial) /* line 75 */
```

Create an instance of ImprovWiFi

#### Parameters

- `serial` - Pointer to stream object used to handle requests, for the most cases use `Serial`

<a name="line-80"></a>
## Type definition

<a name="line-84"></a>
### 🔘 typedef void(OnImprovError)(ImprovTypes::Error)

```cpp
typedef void(OnImprovError)(ImprovTypes::Error) /* line 87 */
```

Callback function called when any error occurs during the protocol handling or wifi connection.

<a name="line-89"></a>
### 🔘 typedef void(OnImprovConnected)(const char *ssid, const char *password)

```cpp
typedef void(OnImprovConnected)(const char *ssid, const char *password) /* line 92 */
```

Callback function called when the attempt of wifi connection is successful. It informs the SSID and Password used to that, it's a perfect time to save them for further use.

<a name="line-94"></a>
### 🔘 typedef bool(CustomConnectWiFi)(const char *ssid, const char *password)

```cpp
typedef bool(CustomConnectWiFi)(const char *ssid, const char *password) /* line 97 */
```

Callback function to customize the wifi connection if you needed. Optional.

<a name="line-99"></a>
## Methods

<a name="line-103"></a>
### Ⓜ️ void handleSerial()

```cpp
void handleSerial() /* line 107 */
```

Check if a communication via serial is happening. Put this call on your loop().


<a name="line-109"></a>
### Ⓜ️ void setDeviceInfo(const char *firmwareName, const char *firmwareVersion, const char *deviceName, const char *deviceUrl)

```cpp
void setDeviceInfo(const char *firmwareName, const char *firmwareVersion, const char *deviceName, const char *deviceUrl) /* line 122 */
```

Set details of your device.

#### Parameters

- `firmwareName` - Firmware name
- `firmwareVersion` - Firmware version
- `deviceName` - Your device name
- `deviceUrl`- The local URL to access your device. A placeholder called {LOCAL_IPV4} is available to form elaboreted URLs. E.g. `http://{LOCAL_IPV4}?name=Guest`.
  There is overloaded method without `deviceUrl`, in this case the URL will be the local IP.


<a name="line-125"></a>
### Ⓜ️ void onImprovError(OnImprovError *errorCallback)

```cpp
void onImprovError(OnImprovError *errorCallback) /* line 128 */
```

Method to set the typedef OnImprovError callback.

<a name="line-130"></a>
### Ⓜ️ void onImprovConnected(OnImprovConnected *connectedCallback)

```cpp
void onImprovConnected(OnImprovConnected *connectedCallback) /* line 133 */
```

Method to set the typedef OnImprovConnected callback.

<a name="line-135"></a>
### Ⓜ️ void setCustomTryConnectToWiFi(CustomConnectWiFi *connectWiFiCallBack)

```cpp
void setCustomTryConnectToWiFi(CustomConnectWiFi *connectWiFiCallBack) /* line 138 */
```

Method to set the typedef CustomConnectWiFi callback.

<a name="line-140"></a>
### Ⓜ️ bool tryConnectToWifi(const char *ssid, const char *password)

```cpp
bool tryConnectToWifi(const char *ssid, const char *password) /* line 145 */
```

Default method to connect in a WiFi network.
It waits `DELAY_MS_WAIT_WIFI_CONNECTION` milliseconds (default 500) during `MAX_ATTEMPTS_WIFI_CONNECTION` (default 20) until it get connected. If it does not happen, an error `ERROR_UNABLE_TO_CONNECT` is thrown.


<a name="line-147"></a>
### Ⓜ️ bool isConnected()

```cpp
bool isConnected() /* line 151 */
```

Check if connection is established using `WiFi.status() == WL_CONNECTED`


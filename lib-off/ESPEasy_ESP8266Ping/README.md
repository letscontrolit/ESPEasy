# ESP8266Ping
Let the ESP8266 ping a remote machine.

With this library an ESP8266 can ping a remote machine and know if it's reachable.
It provide some basic measurements on ping messages (avg response time).

## Usage

First, include the library in your sketch along with WiFi library:

```Arduino
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
```

Next, simply call the `Ping.ping()` function

```Arduino
IPAddress ip (192, 168, 0, 1); // The remote ip to ping
bool ret = Ping.ping(ip);
```

`ret` will be true if the remote responded to pings, false if not reachable.
The library supports hostname too, just pass a string instead of the ip address:

```Arduino
bool ret = Ping.ping("www.google.com");
```

Additionally, the function accept a second integer parameter `count` that specify how many pings has to be sent:

```Arduino
bool ret = Ping.ping(ip_or_host, 10);
```

After `Ping.ping()` has been called, the average response time (in milliseconds) can be retrieved with

```Arduino
int avg_time_ms = Ping.averageTime();
```

# PZEM-004T v3.0
ESPeasy library for Peacefair PZEM-004T-10A and PZEM-004T-100A v3.0 Energy monitor.
(Based on Arduino communication library for Peacefair PZEM-004T-10A and PZEM-004T-100A v3.0 Energy monitor.)

***

This module is an upgraded version of the PZEM-004T with frequency and power factor measurement features available at the usual places. It communicates using a TTL interface over a Modbus-RTU like communication protocol but is incompatible with the older [@olehs](https://github.com/olehs) library found here: [https://github.com/olehs/PZEM004T](https://github.com/olehs/PZEM004T). I would like to thank [@olehs](https://github.com/olehs) for the great library which inspired me to write this one.


### Manufacturer (optimistic) specifications

| Function      | Measuring range    | Resolution      | Accuracy | TODO: Realistic specifications |
|---------------|--------------------|-----------------|----------|--------------------------------|
| Voltage       | 80~260V            | 0.1V            | 0.5%     |                                |
| Current       | 0\~10A or 0\~100A*   | 0.01A or 0.02A* | 0.5%     |                                |
| Active power  | 0\~2.3kW or 0\~23kW* | 0.1W            | 0.5%     |                                |
| Active energy | 0~9999.99kWh       | 1Wh             | 0.5%     |                                |
| Frequency     | 45~65Hz            | 0.1Hz           | 0.5%     |                                |
| Power factor  | 0.00~1.00          | 0.01            | 1%       |                                |

\* Using the external current transformer instead of the built in shunt

#### Other features
  * 247 unique programmable slave addresses
    * Enables multiple slaves to use the same Serial interface
  * Over power alarm
  * Energy counter reset
  * CRC16 checksum
  * Better, but not perfect mains isolation

### Example
```c++
#include <PZEM004Tv30.h>

PZEM004Tv30 pzem(&Serial3);

void setup() {
  Serial.begin(115200);

  Serial.print("Reset Energy");
  pzem.resetEnergy();


  Serial.print("Set address to 0x42");
  pzem.setAddress(0x42);
}

void loop() {
  float volt = pzem.voltage();
  Serial.print("Voltage: ");
  Serial.print(volt);
  Serial.println("V");

  float cur = pzem.current();
  Serial.print("Current: ");
  Serial.print(cur);
  Serial.println("A");

  float powe = pzem.power();
  Serial.print("Power: ");
  Serial.print(powe);
  Serial.println("W");

  float ener = pzem.energy();
  Serial.print("Energy: ");
  Serial.print(ener,3);
  Serial.println("kWh");

  float freq = pzem.frequency();
  Serial.print("Frequency: ");
  Serial.print(freq);
  Serial.println("Hz");

  float pf = pzem.pf();
  Serial.print("PF: ");
  Serial.println(pf);

  delay(1000);
}
```

***
Thank you to [@olehs](https://github.com/olehs) for inspiring this library.

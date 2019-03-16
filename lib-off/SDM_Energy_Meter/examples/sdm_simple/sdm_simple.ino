/*  WEMOS D1 Mini
                     ______________________________
                    |   L T L T L T L T L T L T    |
                    |                              |
                 RST|                             1|TX HSer
                  A0|                             3|RX HSer
                  D0|16                           5|D1
                  D5|14                           4|D2
                  D6|12                    10kPUP_0|D3
RX SSer/HSer swap D7|13                LED_10kPUP_2|D4
TX SSer/HSer swap D8|15                            |GND
                 3V3|__                            |5V
                       |                           |
                       |___________________________|
*/

#include <SoftwareSerial.h>                                                     //import SoftwareSerial library
#include <SDM.h>                                                                //import SDM library

SoftwareSerial swSerSDM(13, 15);                                                //config SoftwareSerial (rx->pin13 / tx->pin15)

SDM sdm(swSerSDM, 9600, NOT_A_PIN);                                             //config SDM

void setup() {
  Serial.begin(115200);                                                         //initialize serial
  sdm.begin();                                                                  //initialize SDM communication
}

void loop() {
  char bufout[10];
  sprintf(bufout, "%c[1;0H", 27);
  Serial.print(bufout);

  Serial.print("Voltage:   ");
  Serial.print(sdm.readVal(SDM220T_VOLTAGE), 2);                                //display voltage
  Serial.println("V");

  delay(50);

  Serial.print("Current:   ");
  Serial.print(sdm.readVal(SDM220T_CURRENT), 2);                                //display current  
  Serial.println("A");

  delay(50);

  Serial.print("Power:     ");
  Serial.print(sdm.readVal(SDM220T_POWER), 2);                                  //display power
  Serial.println("W");

  delay(50);

  Serial.print("Frequency: ");
  Serial.print(sdm.readVal(SDM220T_FREQUENCY), 2);                              //display frequency
  Serial.println("Hz");   

  delay(1000);                                                                  //wait a while before next loop
}

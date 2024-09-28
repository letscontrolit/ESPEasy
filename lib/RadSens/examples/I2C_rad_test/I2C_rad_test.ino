#include <Arduino.h>
#include <Wire.h>
#include "CG_RadSens.h"

CG_RadSens radSens(RS_DEFAULT_I2C_ADDRESS); /*Constructor of the class ClimateGuard_RadSens1v2,
                                                           sets the address parameter of I2C sensor.
                                                           Default address: 0x66.*/

void setup()
{
  Serial.begin(115200);
  Wire.begin(); // This function initializes the Wire library
  delay(1000);
while(!radSens.init()) /*Initializates function and sensor connection. Returns false if the sensor is not connected to the I2C bus.*/
{
  Serial.println("Sensor wiring error!");
  delay(1000);
}
  uint8_t sensorChipId = radSens.getChipId(); /*Returns chip id, default value: 0x7D.*/

  Serial.print("Chip id: 0x");
  Serial.println(sensorChipId, HEX);

  uint8_t firmWareVer = radSens.getFirmwareVersion(); /*Returns firmware version.*/

  Serial.print("Firmware version: ");
  Serial.println(firmWareVer);

  Serial.println("-------------------------------------");
  Serial.println("Set Sensitivity example:\n");

  uint16_t sensitivity = radSens.getSensitivity(); /*Rerutns the value coefficient used for calculating
                                                    the radiation intensity or 0 if sensor isn't connected.*/

  Serial.print("\t getSensitivity(): ");
  Serial.println(sensitivity);
  Serial.println("\t setSensitivity(55)... ");

  radSens.setSensitivity(55); /*Sets the value coefficient used for calculating
                                the radiation intensity*/

  sensitivity = radSens.getSensitivity();
  Serial.print("\t getSensitivity(): ");
  Serial.println(sensitivity);
  Serial.println("\t setSensitivity(105)... ");

  radSens.setSensitivity(105);

  Serial.print("\t getSensitivity(): ");
  Serial.println(radSens.getSensitivity());
  Serial.println("-------------------------------------");
  Serial.println("HW generator example:\n");

  bool hvGeneratorState = radSens.getHVGeneratorState(); /*Returns state of high-voltage voltage Converter.
                                                           If return true -> on
                                                           If return false -> off or sensor isn't conneted*/

  Serial.print("\n\t HV generator state: ");
  Serial.println(hvGeneratorState);
  Serial.println("\t setHVGeneratorState(false)... ");

  radSens.setHVGeneratorState(false); /*Set state of high-voltage voltage Converter.
                                        if setHVGeneratorState(true) -> turn on HV generator
                                        if setHVGeneratorState(false) -> turn off HV generator*/

  hvGeneratorState = radSens.getHVGeneratorState();
  Serial.print("\t HV generator state: ");
  Serial.println(hvGeneratorState);
  Serial.println("\t setHVGeneratorState(true)... ");

  radSens.setHVGeneratorState(true);

  hvGeneratorState = radSens.getHVGeneratorState();
  Serial.print("\t HV generator state: ");
  Serial.println(hvGeneratorState);
  Serial.println("-------------------------------------");
  Serial.println("LED indication control example:\n");

  bool ledState = radSens.getLedState(); /*Returns state of LED indicator.
                                                           If return true -> on
                                                           If return false -> off*/

  Serial.print("\n\t LED indication state: ");
  Serial.println(ledState);
  Serial.println("\t turn off LED indication... ");

  radSens.setLedState(false); /*Set state of LED indicator.
                                        if setHVGeneratorState(true) -> turn on LED indicator
                                        if setHVGeneratorState(false) -> turn off LED indicator*/
  ledState = radSens.getLedState();
  Serial.print("\t LED indication state: ");
  Serial.println(ledState);
  Serial.println("\t turn on led indication... ");

  radSens.setLedState(true);

  ledState = radSens.getLedState();
  Serial.print("\t LED indication state: ");
  Serial.print(ledState);
  Serial.println("\n-------------------------------------");
  delay(5000);
}

void loop()
{
  Serial.print("Rad intensy dyanmic: ");

  Serial.println(radSens.getRadIntensyDynamic()); /*Returns dynamic radiation intensity (recommended if measurement period T < 123 sec).*/

  Serial.print("Rad intensy static: ");

  Serial.println(radSens.getRadIntensyStatic()); /*Returns static radiation intensity (recommended if measurement period T = 500 sec).*/

  Serial.print("Number of pulses: ");

  Serial.println(radSens.getNumberOfPulses()); /*Returns the accumulated number of pulses registered by the 
                                                 module since the last I2C data reading.*/

  delay(2000);
}

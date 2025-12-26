/*
 Program to demonstrate using the interrupt pin of any INA2xx which supports that functionality in
 order to trigger readings in the background and using a timer on the Arduino to trigger data
 averaging and storing or displaying the computed values.

 The INA226 is set up to pull the alert pin down when a measurement is ready. The program has set
 the bus and shunt to the maximum conversion time of 8.244ms and then averaging to 8, so each
 measurement will take about 64ms. The interrupt vector "PCINT0_vect" is called and the readings
 from the INA226 are read and added to the averages.

 A timer interrupt is defined in the setup() method that triggers a call to the vector
 "TIMER1_COMPA_vect" once every second.  The average values collected in the "PCINT0_vect" call are
 then taken and stored in memory. As the amount of RAM is limited and the absolute readings are 2
 Bytes long while the delta values to the previous measurement are usually quite small, a variable
 length Huffmann coding has been implemented at a nibble (4 bit) level to provide a higher-density
 method of storing data. Each array is declared at 600 Bytes (one array for voltage measurements and
 one array for shunt voltage) and those 1200 Bytes total can store up to 18 minutes of per-second
 data, which would otherwise occupy over 4Kb memory. The Huffmann encoding method is described in
 more detail in the interrupt code below.

 This example works on Atmel-Arduinos since it uses Atmel interrupts which are different on
 processors such as the ESP32. The value of ARRAY_BYTES is set at 1200, which works on Arduinos with
 2K or more of RAM, smaller processors would need to reduce this value in order to work correctly.
 The example is also coded for the INA226, as a chip with an ALERT pin is required for the program
 to work; additionally the hard-coded LSB values for the bus voltage and shunt voltage have been set
 to those used in the INA226.

 Detailed documentation can be found on the GitHub Wiki pages at
 https://github.com/Zanduino/INA/wiki

 This example is for a INA226 set up to measure a 5-Volt load with a 0.1Ω resistor in place, this is
 the same setup that can be found in the Adafruit INA226 breakout board.  The complex calibration
 options are done at runtime using the 2 parameters specified in the "begin()" call and the library
 has gone to great lengths to avoid the use of floating point to conserve space and minimize
 runtime.  This demo program uses floating point only to convert and display the data conveniently.
 The INA226 uses 15 bits of precision, and even though the current and watt information is returned
 using 32-bit integers the precision remains the same.

 The INA226 is set up to measure using the maximum conversion length (and maximum accuracy) and then
 average those readings 64 times. This results in readings taking 8.244ms x 64 = 527.616ms or just
 less than 2 times a second. The pin-change interrupt handler is called when a reading is finished
 and the INA226 pulls the pin down to ground, it resets the pin status and adds the readings to the
 global variables. The main program will do whatever processing it has to and every 5 seconds it
 will display the current averaged readings and reset them.

 The datasheet for the INA226 can be found at http://www.ti.com/lit/ds/symlink/INA226.pdf and it
 contains the information required in order to hook up the device. Unfortunately it comes as a VSSOP
 package but it can be soldered onto a breakout board for breadboard use. The INA226 is quite
 similar to the INA219 mentioned above, but it can take bus voltages of up to 36V (which I needed in
 order to monitor a 24V battery system which goes above 28V while charging and which is above the
 absolute limits of the INA219). It is also significantly more accurate than the INA219, plus has an
 alert pin.

 Interrupts on Arduinos can get a bit confusing, differentiating between external interrupts and pin
 change interrupts. The external interrupts are limited and which pins are available are different
 for each processor, see
 https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/ for
 additional information. Pin Change interrupts, on the other hand, can be assigned to most pins, but
 these interrupts are shared in groups of pins (call "ports") and when the interrupts are triggered
 they call one of 3 possible ISRs. This program makes use of PCINT0_vect and the interrupt is set to
 pin 8. The tests were done on an Arduino UNO and Arduino Micro using this pin

 Sometimes the INA devices will do a soft/hard reset on voltage spikes (despite using decoupling
 capacitors) and since the "PCINT0_vect" is called only when the ALERT pin is pulled low and the
 default mode of the INA226 upon reset is "off, this would result in the program never collecting
 statistics. For this reason the TIMER1 is used as a watchdog timer, triggering an interrupt every
 second. If no measurements are detected then the INA226 is manually reset and processing continues.

 GNU General Public License 3
 ============================
 This program is free software: you can redistribute it and/or modify it under the terms of the GNU
 General Public License as published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version. This program is distributed in the hope that it
 will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should
 have received a copy of the GNU General Public License along with this program (see
 https://github.com/Zanduino/INA/blob/master/LICENSE).  If not, see
 <http://www.gnu.org/licenses/>.

 Vers.  Date       Developer  Comments
 ====== ========== ========== ==============================================================
 1.0.1  2020-06-30 SV-Zanshin Issue #58 - clang-formatted document
 1.0.0  2018-10-13 SV-Zanshin Ready for publishing
 1.0.0  2018-10-03 SV-Zanshin Cloned and adapted example
*/
#include <INA.h>  // INA Library

#include "MB85_FRAM.h"  // I2C FRAM Library
/**************************************************************************************************
** Declare program Constants                                                                     **
**************************************************************************************************/
const uint8_t  INA_ALERT_PIN = 8;       // Pin 8.
const uint8_t  GREEN_LED_PIN = 13;      // Green LED (standard location)
const uint32_t SERIAL_SPEED  = 115200;  // Use fast serial speed
const uint16_t ARRAY_BYTES   = 1200;    // Bytes in data array
/**************************************************************************************************
** Declare global variables, structures and instantiate classes                                  **
**************************************************************************************************/
uint8_t           deviceNumber   = UINT8_MAX;  // Device Number to use in example
volatile uint64_t sumBusRaw      = 0;          // Sum of bus raw values
volatile int64_t  sumShuntRaw    = 0;          // Sum of shunt raw values
volatile uint8_t  readings       = 0;          // Number of measurements taken
uint8_t           chips_detected = 0;          // Number of I2C FRAM chips detected
volatile uint32_t framIndex      = 0;          // Index to the next free position
INA_Class         INA;                         // INA class instantiation
MB85_FRAM_Class   FRAM;                        // FRAM Memory class instantiation

void writeNibble(uint8_t dataArray[], const uint16_t nibblePos, const uint8_t nibbleData) {
  /************************************************************************************************
  ** Method "writeNibble()" will write the LSB 4 bits of "nibbleData" to the "dataArray" nibble  **
  ** offset "nibblePos", each index position is 4 bits.                                          **
  ************************************************************************************************/
  uint8_t writeByte = *(dataArray + (nibblePos / 2));     // Read the correct byte and select
  if (nibblePos & 1) {                                    // whether the LSB or MSB is to be set
    writeByte = (writeByte & 0xF0) | (nibbleData & 0xF);  // Keep MSB & set the LSB to value
  } else {
    writeByte = (nibbleData << 4) | (writeByte & 0xF);  // Keep LSB & set the MSB to value
  }                                                     // of if-then-else nibblePos is odd
  *(dataArray + (nibblePos / 2)) = writeByte;           // Write the new value to array
}  // of method "writeNibble()"
uint8_t readNibble(uint8_t dataArray[], const uint16_t nibblePos) {
  /************************************************************************************************
  ** Method "readNibble()" will read the nibble addressed by "nibblePos" into the write the 4    **
  ** LSB bits of the return value. Each index position of the virtual array is 4 bits.           **
  ************************************************************************************************/
  uint8_t returnVal = *(dataArray + (nibblePos / 2));  // Read the correct byte and select
  if (nibblePos & 1) {                                 // whether the LSB or MSB is to be returned
    returnVal = returnVal & 0xF;                       // Use the 4 LSB bits
  } else {
    returnVal = returnVal >> 4;  // Use the 4 MSB bits
  }                              // of if-then-else nibblePos is odd
  return returnVal;              // Return the computed nibble
}  // of method "readNibble()"
ISR(PCINT0_vect) {
  /************************************************************************************************
  ** Declare interrupt service routine for the pin-change interrupt on pin 8 which is set in the **
  ** setup() method                                                                              **
  ************************************************************************************************/
  static uint16_t tempsumBusRaw;    // Declare as static to only init 1
  static int16_t  tempsumShuntRaw;  // Declare as static to only init 1
  *digitalPinToPCMSK(INA_ALERT_PIN) &= ~bit(digitalPinToPCMSKbit(INA_ALERT_PIN));  // Disable PCMSK
  PCICR &= ~bit(digitalPinToPCICRbit(INA_ALERT_PIN));        // disable interrupt for the group
  digitalWrite(GREEN_LED_PIN, !digitalRead(GREEN_LED_PIN));  // Toggle LED to show we are working
  sei();                                                     // Enable interrupts for I2C calls
  tempsumBusRaw   = INA.getBusRaw(deviceNumber);             // Read the current value into temp
  tempsumShuntRaw = INA.getShuntRaw(deviceNumber);           // Read the current value into temp
  INA.waitForConversion(deviceNumber);                       // Resets interrupt flag and start
  cli();                                                     // Disable interrupts
  sumBusRaw += tempsumBusRaw;                                // copy value while ints disabled
  sumShuntRaw += tempsumShuntRaw;                            // copy value while ints disabled
  readings++;                                                // Increment the number of readings
  *digitalPinToPCMSK(INA_ALERT_PIN) |= bit(digitalPinToPCMSKbit(INA_ALERT_PIN));  // Enable PCMSK
  PCIFR |= bit(digitalPinToPCICRbit(INA_ALERT_PIN));  // clear any outstanding interrupt
  PCICR |= bit(digitalPinToPCICRbit(INA_ALERT_PIN));  // enable interrupt for the group
}  // of ISR handler for INT0 group of pins
void writeDataToArray(uint8_t dataArray[], uint16_t &nibbleIndex, const int16_t deltaData) {
  /************************************************************************************************
  ** Function "writeDataToArray()" writes 4LSB from "nibbleData" to the "nibbleIndex" nibble in  **
  *"dataArray". A Huffmann-like encoding with variable length is used to write the delta values  **
  ** to the appropriate array. Each array "index" element is one nibble (4 bits) and the MSB     **
  ** characters of the MSB nibble denote the record type. If the MSB is "B0" then it is a        **
  ** 1-nibble long value, if the 2 MSB are "B10" then it is a 2 nibble value, if "B110" then 3   **
  ** nibbles, "B1110" denotes 4 and "B1111" denotes 5 nibbles. See the table below:              **
  **                                                                                             **
  ** 24-Bit representation  Data Bits    Value Range                                             **
  ** =====================  ============ ===============                                         **
  **  ----------------0xxx   3 bits data    -4  to     3                                         **
  **  ------------10xxxxxx   6 bits data    -16 to    15                                         **
  **  --------110xxxxxxxxx   9 bits data   -256 to   255                                         **
  **  ----1110xxxxxxxxxxxx  12 bits data  -2048 to  2047                                         **
  **  1111xxxxxxxxxxxxxxxx  16 bits data -32768 to 32767                                         **
  ************************************************************************************************/

  if (deltaData >= -4 && deltaData <= 3)  // 1N, format 0xxx
  {
    writeNibble(dataArray, nibbleIndex++, deltaData & B111);  // Write 1N to array
  } else {
    if (deltaData >= -16 && deltaData <= 15)  // 2N, format 10xxxxxx
    {
      writeNibble(dataArray, nibbleIndex++, ((deltaData >> 4) & B11) | B1000);  // write MSB
      writeNibble(dataArray, nibbleIndex++, deltaData);                         // write LSB
    } else {
      if (deltaData >= -256 && deltaData <= 255)  // 3N, format 110xxxxxxxxx
      {
        writeNibble(dataArray, nibbleIndex++, ((deltaData >> 8) & 1) | B1100);  // Set 3MSB 9th bit
        writeNibble(dataArray, nibbleIndex++, deltaData >> 4 & B1111);  // write 4 MSB bits byte 1
        writeNibble(dataArray, nibbleIndex++, deltaData);               // write 4 LSB bits byte 1
      } else {
        if (deltaData >= -2048 && deltaData <= 2047)  // 4N, format 1110xxxxxxxxxxxx
        {
          writeNibble(dataArray, nibbleIndex++, B1110);            // Header nibble
          writeNibble(dataArray, nibbleIndex++, deltaData >> 8);   // next nibble
          writeNibble(dataArray, nibbleIndex++, deltaData >> 4);   // next nibble
          writeNibble(dataArray, nibbleIndex++, deltaData);        // LSB nibble
        } else {                                                   // 5N, fmt 1111xxxxxxxxxxxxxxxx
          writeNibble(dataArray, nibbleIndex++, B1111);            // Header nibble
          writeNibble(dataArray, nibbleIndex++, deltaData >> 12);  // MSB nibble
          writeNibble(dataArray, nibbleIndex++, deltaData >> 8);   // next nibble
          writeNibble(dataArray, nibbleIndex++, deltaData >> 4);   // next nibble
          writeNibble(dataArray, nibbleIndex++, deltaData);        // LSB nibble
        }  // if-then-else value fits in 4 or 5 nibbles
      }    // if-then-else value fits in 3 nibbles
    }      // if-then-else value fits in 2 nibbles
  }        // if-then-else value fits in 1 nibble
}  // of method "WriteDataToArray()"
int16_t readDataFromArray(uint8_t dataArray[], uint16_t &nibbleIndex) {
  /************************************************************************************************
  ** Function "readDataToArray()" returns a 2-Byte signed integer from "dataArray" starting at   **
  ** "nibbleIndex" and expanding the Array's internal Huffmann-encoding values. See the descrip- **
  ** tion of writeDataToArray() for details                                                      **
  ************************************************************************************************/

  int16_t outValue    = 0;                                     // Declare return variable
  uint8_t controlBits = readNibble(dataArray, nibbleIndex++);  // Read the header nibble
  if (controlBits >> 3 == 0)  // ----------------0xxx   3 bits data - 4  to     3
  {
    outValue = controlBits & B111;                   // mask High Bit
    if (outValue >> 2 & B1) { outValue |= 0xFFF8; }  // If it is a negative number
  } else {
    if (controlBits >> 2 == B10)  // ------------10xxxxxx   6 bits data - 16 to    15
    {
      outValue = (controlBits & B11) << 4;               // mask 2 High Bits
      outValue |= readNibble(dataArray, nibbleIndex++);  // move in 4 LSB
      if (outValue >> 5 & B1) { outValue |= 0xFFE0; }    // If it is a negative number
    } else {
      if (controlBits >> 1 == B110)  // --------110xxxxxxxxx   9 bits data - 256 to   255
      {
        outValue = (controlBits & B1) << 8;                     // mask 2 High Bits
        outValue |= readNibble(dataArray, nibbleIndex++) << 4;  // move in 4 middle bits
        outValue |= readNibble(dataArray, nibbleIndex++);       // move in 4 LSB
        if (outValue >> 8 & B1) { outValue |= 0xFE00; }         // If it is a negative number
      } else {
        if (controlBits == B1110)  // ----1110xxxxxxxxxxxx  12 bits data - 2048 to  2047
        {
          outValue = readNibble(dataArray, nibbleIndex++) << 8;   // move in 4 high bits
          outValue |= readNibble(dataArray, nibbleIndex++) << 4;  // move in 4 middle bits
          outValue |= readNibble(dataArray, nibbleIndex++);       // move in 4 low bits
          if (outValue >> 11 & B1) { outValue |= 0xF000; }        // If it is a negative number
        } else {
          if (controlBits == B1111)  // 1111xxxxxxxxxxxxxxxx  16 bits data - 16384 to 16383
          {
            outValue = readNibble(dataArray, nibbleIndex++) << 12;  // move in 4 high bits
            outValue |= readNibble(dataArray, nibbleIndex++) << 8;  // move in 4 middle bits
            outValue |= readNibble(dataArray, nibbleIndex++) << 4;  // move in 4 middle bits
            outValue |= readNibble(dataArray, nibbleIndex++);       // move in 4 low bits
          }                                                         // if-then 5 nibbles
        }                                                           // if-then-else 4 nibbles
      }                                                             // if-then-else 3 nibbles
    }                                                               // if-then-else 2 nibbles
  }                                                                 // if-then-else 1 nibble
  return (outValue);
}  // of method "readDataFromArray()"

ISR(TIMER1_COMPA_vect) {
  /**********************************************************************************************
  ** Declare interrupt service routine for TIMER1, which is set to trigger once every second   **
  **********************************************************************************************/
  static int16_t  deltaBus, deltaShunt;              // Difference value from last
  static uint16_t arrayNibbleIndex = 0;              // Array index in Nibbles
  static int16_t  lastBusRaw       = 0;              // Value from last reading
  static int16_t  lastShuntRaw     = 0;              // Value from last reading
  static int16_t  baseBusRaw       = 0;              // Base value for delta readings
  static int16_t  baseShuntRaw     = 0;              // Base value for delta readings
  static uint16_t arrayReadings    = 0;              // Number of readings in array
  static uint8_t  dataArray[ARRAY_BYTES];            // Array for bus and shunt readings
  if (arrayNibbleIndex == 0 && millis() < 3000) {    // Skip first 3 seconds
    baseBusRaw   = (int16_t)(sumBusRaw / readings);  // after startup to allow settings
    lastBusRaw   = baseBusRaw;                       // to settle
    baseShuntRaw = (int16_t)(sumShuntRaw / readings);
    lastShuntRaw = baseShuntRaw;
    readings     = 0;  // then skip readings to let the
    sumBusRaw    = 0;  // sensor settle down
    sumShuntRaw  = 0;  // Reset values
    return;
  }  // of if-then first second after startup
  deltaBus   = ((int16_t)(sumBusRaw / readings) - lastBusRaw);      // Compute the delta bus
  deltaShunt = ((int16_t)(sumShuntRaw / readings) - lastShuntRaw);  // Compute the delta shunt
  writeDataToArray(dataArray, arrayNibbleIndex, deltaBus);          // Add bus reading to array
  writeDataToArray(dataArray, arrayNibbleIndex, deltaShunt);        // Add shunt reading to array
  arrayReadings++;                                                  // increment the counter
  lastBusRaw   = sumBusRaw / readings;                              // Reset values
  lastShuntRaw = sumShuntRaw / readings;                            // Reset values
  readings     = 0;                                                 // Reset values
  sumBusRaw    = 0;                                                 // Reset values
  sumShuntRaw  = 0;                                                 // Reset values
  /*****************************************************************************************************************
  ** Once the array could fill up on the next reading (2x max reading of 5 nibbles) then it is
  *time to flush the  **
  ** the accumulated readings. **
  *****************************************************************************************************************/
  if ((arrayNibbleIndex + 10) / 2 >= ARRAY_BYTES)  //                                 //
  {                                                //                                 //
    int16_t  busValue        = 0;                  // Contains current bus value      //
    int16_t  shuntValue      = 0;                  // Contains current shunt value    //
    uint16_t workNibbleIndex = 0;                  // Index into array for reading    //
    /***************************************************************************************************************
    ** If there is a FRAM memory board attached, then copy the array contents to it **
    ***************************************************************************************************************/
    if (chips_detected > 0)  // Only execute if there is memory //
    {                        //                                 //
      if ((framIndex + sizeof(dataArray) <
           FRAM.totalBytes()))               // Only write when space available //
      {                                      //                                 //
        cli();                               // Enable interrupts temporarily   //
        Serial.print(millis() / 1000 / 60);  //                                 //
        Serial.print(" ");                   //                                 //
        Serial.print(F("Writing "));         //                                 //
        Serial.print(sizeof(dataArray));     //                                 //
        Serial.print(" Bytes to memory @");  //                                 //
        Serial.print(framIndex);             //                                 //
        Serial.print(".\n");                 //                                 //
        sei();                               // Disable interrupts again        //
        FRAM.write(framIndex, dataArray);    // Write the whole array to FRAM   //
        framIndex += sizeof(dataArray);      // set index to new location       //
      }  // of if-then there is space in the EEPROM                             // //
    }    // of if-then we have at least one EEPROM attached to the I2C bus        // //
    for (uint16_t readingNo = 1; readingNo <= arrayReadings;
         readingNo++)  // Process every reading in array  //
    {                  //                                 //
      busValue = readDataFromArray(dataArray, workNibbleIndex);  // Get next bus value from array //
      baseBusRaw += busValue;  // apply delta value to bus base   //
      shuntValue =
          readDataFromArray(dataArray, workNibbleIndex);  // Get shunt next value from array //
      baseShuntRaw += shuntValue;                         // apply delta value to shunt base //
      /*************************************************************************************************************
      ** Insert code here to save data to static RAM or to a SD-Card or elsewhere **
      *************************************************************************************************************/

      cli();  // Enable interrupts temporarily   //
      Serial.print(millis() / 1000);
      Serial.print(" ");
      Serial.print(readingNo);
      Serial.print(" ");
      Serial.print(baseBusRaw * 0.00125, 4);
      Serial.print("V ");
      Serial.print(0.0025 * baseShuntRaw);
      Serial.println("mA");
      sei();  // Disable interrupts again        //

    }  // of for-next each array reading                                        // //
    arrayNibbleIndex = 0;  // reset                           //
    arrayReadings    = 0;  // reset                           //
  }  // of if-then the internal array is full                                   // //
}  // of ISR "TIMER1_COMPA_vect"                                                // //

/*******************************************************************************************************************
** Method Setup(). This is an Arduino IDE method which is called first upon initial boot or
*restart. It is only   **
** called one time and all of the variables and other initialization calls are done here prior to
*entering the    **
** main loop for data measurement. **
*******************************************************************************************************************/
void setup()                             //                                  //
{                                        //                                  //
  pinMode(GREEN_LED_PIN, OUTPUT);        // Define the green LED as an output//
  digitalWrite(GREEN_LED_PIN, true);     // Turn on the LED                  //
  pinMode(INA_ALERT_PIN, INPUT_PULLUP);  // Declare pin with pull-up resistor//
  *digitalPinToPCMSK(INA_ALERT_PIN) |=
      bit(digitalPinToPCMSKbit(INA_ALERT_PIN));       // Enable PCMSK pin                 //
  PCIFR |= bit(digitalPinToPCICRbit(INA_ALERT_PIN));  // clear any outstanding interrupt  //
  PCICR |= bit(digitalPinToPCICRbit(INA_ALERT_PIN));  // enable interrupt for the group   //
  Serial.begin(SERIAL_SPEED);                         // Start serial communications      //
#ifdef __AVR_ATmega32U4__                             // If this is a 32U4 processor,     //
  delay(2000);                                        // wait 3 seconds for serial port   //
#endif                                                // interface to initialize          //
  Serial.print(
      F("\n\nINA Data Logging with interrupts V1.0.3\n"));  // Display program information      //
  uint8_t devicesFound = 0;                                 // Number of INA2xx found on I2C    //
  while (deviceNumber == UINT8_MAX)                         // Loop until we find devices       //
  {                                                         //                                  //
    devicesFound = INA.begin(1, 100000);                    // ±1Amps maximum for 0.1Ω resistor //
    for (uint8_t i = 0; i < devicesFound; i++)              // the first INA226 device found    //
    {                                                       // Change "INA226" to "INA260" or   //
                                                            // whichever INA2xx to measure      //
      if (strcmp(INA.getDeviceName(i), "INA226") == 0)      // Set deviceNumber appropriately   //
      {                                                     //                                  //
        deviceNumber = i;                                   //                                  //
        INA.reset(deviceNumber);                            // Reset device to default settings //
        break;                                              //                                  //
      }  // of if-then we have found an INA226                                 // //
    }    // of for-next loop through all devices found                           // //
    if (deviceNumber == UINT8_MAX)                        // Try again if no device found     //
    {                                                     //                                  //
      Serial.print(F("No INA226 found. Waiting 5s.\n"));  //                                  //
      delay(5000);                                        //                                  //
    }  // of if-then no INA226 found                                           // //
  }    // of if-then no device found                                             // //
  Serial.print(F("Found INA226 at device number "));    //                                  //
  Serial.println(deviceNumber);                         //                                  //
  Serial.println();                                     //                                  //
  INA.setAveraging(64, deviceNumber);                   // Average each reading 64 times    //
  INA.setAveraging(8, deviceNumber);                    // Average each reading 4 times     //
  INA.setBusConversion(82440, deviceNumber);            // Maximum conversion time 8.244ms  //
  INA.setShuntConversion(82440, deviceNumber);          // Maximum conversion time 8.244ms  //
  INA.setMode(INA_MODE_CONTINUOUS_BOTH, deviceNumber);  // Bus/shunt measured continuously  //
  INA.AlertOnConversion(true, deviceNumber);            // Make alert pin go low on finish  //
  chips_detected = FRAM.begin();                        // return number of memories        //
  if (chips_detected > 0) {                             //                                  //
    Serial.print(F("Found "));                          //                                  //
    Serial.print(chips_detected);                       //                                  //
    Serial.print(F(" FRAM with a total of "));          //                                  //
    uint32_t totalMemory = 0;                           //                                  //
    for (uint8_t i = 0; i < chips_detected; i++) {      //                                  //
      totalMemory += FRAM.memSize(i);                   // Add memory of chip to total      //
    }  // of for-next each memory                                              // //
    Serial.print(totalMemory / 1024);  //                                  //
    Serial.println(F("KB memory."));   //                                  //
  }                // if-then we have found a FRAM memory                                    // //
  cli();           // disable interrupts while setting //
  TCCR1A = 0;      // TCCR1A register reset            //
  TCCR1B = 0;      // TCCR1B register reset            //
  TCNT1  = 0;      // initialize counter               //
  OCR1A  = 15624;  // ((16*10^6) / (1*1024)) - 1       //
  TCCR1B |= (1 << WGM12);               // Enable CTC mode                  //
  TCCR1B |= (1 << CS12) | (1 << CS10);  // CS10 & CS12 for 1024 prescaler   //
  TIMSK1 |= (1 << OCIE1A);              // Enable timer compare interrupt   //
  sei();                                // re-enable interrupts             //
}  // of method setup()                                                        // //

/*******************************************************************************************************************
** This is the main program for the Arduino IDE, it is called in an infinite loop. The INA226
*measurements are    **
** triggered by the interrupt handler each time a conversion is ready, and another interrupt is
*triggered every   **
** second to store the collected readings. Thus the main program is free to do other tasks. **
*******************************************************************************************************************/
void loop()  //                                  //
{            //                                  //
  delay(10000);
}  // of method loop //----------------------------------//

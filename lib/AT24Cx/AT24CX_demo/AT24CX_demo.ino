/*
*
* Read and write demo of the AT24CX library
* Written by Christian Paul, 2014-11-24
* 
* 
*/

// include libraries
#include <Wire.h>
#include <AT24CX.h>

// EEPROM object
AT24CX mem;

// setup
void setup() {
  // serial init
  Serial.begin(115200);
  Serial.println("AT24CX read/write demo");
  Serial.println("----------------------");
}

// main loop
void loop() {
  // read and write byte
  Serial.println("Write 42 to address 12");
  mem.write(12, 42);
  Serial.println("Read byte from address 12 ...");
  byte b = mem.read(12);
  Serial.print("... read: ");
  Serial.println(b, DEC);
  Serial.println();
  
  // read and write integer
  Serial.println("Write 65000 to address 15");
  mem.writeInt(15, 65000);
  Serial.println("Read integer from address 15 ...");
  unsigned int i = mem.readInt(15);
  Serial.print("... read: ");
  Serial.println(i, DEC);
  Serial.println();

  // read and write long
  Serial.println("Write 3293732729 to address 20");
  mem.writeLong(20, 3293732729UL);
  Serial.println("Read long from address 20 ...");
  unsigned long l = mem.readLong(20);
  Serial.print("... read: ");
  Serial.println(l, DEC);
  Serial.println();

  // read and write long
  Serial.println("Write 1111111111 to address 31");
  mem.writeLong(31, 1111111111);
  Serial.println("Read long from address 31 ...");
  unsigned long l2 = mem.readLong(31);
  Serial.print("... read: ");
  Serial.println(l2, DEC);
  Serial.println();
  
  // read and write float
  Serial.println("Write 3.14 to address 40");
  mem.writeFloat(40, 3.14);
  Serial.println("Read float from address 40 ...");
  float f = mem.readFloat(40);
  Serial.print("... read: ");
  Serial.println(f, DEC);
  Serial.println();  

  // read and write double
  Serial.println("Write 3.14159265359 to address 50");
  mem.writeDouble(50, 3.14159265359);
  Serial.println("Read double from address 50 ...");
  double d = mem.readDouble(50);
  Serial.print("... read: ");
  Serial.println(d, DEC);
  Serial.println();
  
  // read and write char
  Serial.print("Write chars: '");
  char msg[] = "This is a message";
  Serial.print(msg);
  Serial.println("' to address 200");
  mem.writeChars(200, msg, sizeof(msg));
  Serial.println("Read chars from address 200 ...");
  char msg2[30];
  mem.readChars(200, msg2, sizeof(msg2));
  Serial.print("... read: '");
  Serial.print(msg2);
  Serial.println("'");
  Serial.println();

  // write array of bytes 
  Serial.println("Write array of 80 bytes at address 1000");
  byte xy[] = {0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,5,5,5,6,6,6,7,7,7,8,8,8,9,9,9,    // 10 x 3 = 30
              10,11,12,13,14,15,16,17,18,19,                                   //          10
              120,121,122,123,124,125,126,127,128,129,                         //          10
              130,131,132,133,134,135,136,137,138,139,                         //          10
              200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219};                   //          20
  mem.write(1000, (byte*)xy, sizeof(xy));

  // read bytes with multiple steps
  Serial.println("Read 80 single bytes starting at address 1000");
  for (int i=0; i<sizeof(xy); i++) {
    byte sb = mem.read(1000+i);
    Serial.print("[");
    Serial.print(1000+i);
    Serial.print("] = ");
    Serial.println(sb);
  } 
  Serial.println();

  // read bytes with one step
  Serial.println("Read 80 bytes with one operation at address 1000");
  byte z[80];
  memset(&z[0], 32, sizeof(z));
  mem.read(1000, z, sizeof(z));
  for (int i=0; i<sizeof(z); i++) {
    Serial.print("[");
    Serial.print(1000+i);
    Serial.print("] = ");
    Serial.println(z[i]);
  } 
  
  // stop
  while (1==1) {}
}

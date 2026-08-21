/**
 * This sketch shows examples on how to use all the features of this library
 * 
 * It can also be used as a test  to verify that you have your eprom configured
 * propery to your Arduino as it prints out the results so you can see if everything works
 */

#include <at24c256.h>

// Create a eprom object configured at address 0
// Sketch assumes that there is an eprom present at this address
AT24C256 eprom(AT24C_ADDRESS_0);
// Create another eprom object configured att address 2
// Sketch assumes that there is NO eprom present at this address
AT24C256 badEprom(AT24C_ADDRESS_2);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up");

  // Initialize the i2c library
  Wire.begin();

  /** Write and read an integer */
  int foo = 42;
  // Write the integer foo to the eprom starting at address 0
  eprom.put(0, foo);
  int foo_in;
  // Read the integer foo_in from eprom starting at address 0
  eprom.get(0, foo_in);
  Serial.println(foo_in);

  /** Write and read a double */
  double pi = 3.141593;
  // Write the double pi to the eprom starting at address 0
  eprom.put(0, pi);
  double pi_in;
  // Read the double pi_in from eprom starting at address 0
  eprom.get(0, pi_in);
  // Create a buffer and convert pi_in to a string to be able to print it
  char buffer[10];
  dtostrf(pi_in, 9, 6, buffer); // Converts a double to a string
  Serial.println(buffer);

  /** Write and read a struct */
  // Declare the struct "Point"
  struct Point {
    int x;
    int y;
  };
  Point point = {17, 42};
  // Write the struct point to the eprom starting at address 0
  eprom.put(0, point);
  Point point_in;
  // Read the struct point_in from eprom starting at address 0
  eprom.get(0, point_in);
  Serial.println(point_in.x);
  Serial.println(point_in.y);

  /** Write and read a single byte */
  // Write the value 77 to the eprom byte at address 0
  eprom.write(0, 77);
  // Read the value of the byte at address 0
  int value = eprom.read(0);
  Serial.println(value);

  /** Write and read a byte buffer */
  uint8_t out[15] = "Test of buffer";
  // Write the 15 bytes long buffer "out" to eprom starting at address 0
  eprom.writeBuffer(0, out, 15);
  uint8_t in[15];
  // Read 15 bytes from eprom starting at address 0 into the buffer "in".
  eprom.readBuffer(0, in, 15);
  Serial.println((char*)in);

  /** Error handling on write, no error */
  // Read a byte from address 0, this should not result in an error
  eprom.write(0, 77);
  // Get the last error code, it should be 0 since there was no error
  int lastError = eprom.getLastError();
  Serial.print("last status on write: ");
  Serial.println(lastError);

  /** Error handling on write, using a eprom address without eprom */
  // Write the value 77 to an eprom that is not connected - this will fail
  badEprom.write(0, 77);
  // Get the last error code, it should not be zero, but 2 which means that there were no response from the eprom
  int lastError2 = badEprom.getLastError();
  Serial.print("last error on write: ");
  Serial.println(lastError2);  

  /** Error handling on read, using a eprom address without eprom */
  // Read the value from address 0 from an eprom that is not connected (and print it)
  badEprom.read(0);
  // Get the last error code, it should not be zero, but 2 which means that there were no response from the eprom
  int lastError3 = badEprom.getLastError();
  Serial.print("last error on read: ");
  Serial.println(lastError3);  

  // Read the size of the eprom and print it
  Serial.println(eprom.length());

  /** Write and read a long byte buffer, >32 which is TwoWire's internal buffer size and 
    * >64 which is the at24c256 page size) */
  uint8_t out2[80] = "Writing a really long message, testing some of several buffer limits on the way";
  // Write the long buffer to the eprom starting at address 0 and check how many bytes were actually written
  int written = eprom.writeBuffer(0, out2, 80);
  Serial.print(written);
  Serial.print(" bytes written, last error on write: ");
  // Get the last error and print it
  Serial.println(eprom.getLastError());
  uint8_t in2[80];
  // Read 80 bytes from eprom starting at address 0 and store it in the in2 buffer. Check how many bytes were actually read
  int readBytes = eprom.readBuffer(0, in2, 80);
  Serial.print(readBytes);
  Serial.print(" bytes read, last error on read: ");
  // Get the last error and print it
  Serial.println(eprom.getLastError()); 
  in2[79] = '\0';
  Serial.println((char*)in2);
}

// This test program has no loop, it just runs once
void loop() {
}

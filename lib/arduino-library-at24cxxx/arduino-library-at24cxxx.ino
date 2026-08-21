/**
 * These are the test cases to verify the functions of the API. 
 * They are made to run on the target device with a at24c250 on address 0
 *
 * Because of the Arduino IDEs limitations in including and placing files
 * This file has to be placed here.
 */

#include <AUnit.h>
#include "src/at24c256.h" // trigger compilation of the source directory

AT24C256 eprom(AT24C_ADDRESS_0);
AT24C256 badEprom(AT24C_ADDRESS_7);

#define NO_ERROR (0)
#define ADDRESS_NACK (2)
#define AT24C256_LENGTH ((uint16_t)32768)

int memoryPosition = 0;
int8_t buffer[1024];

// Spread out the test values a bit in memory to spread out ware
int getNextMemoryPosition() {
  memoryPosition += 2;
  return memoryPosition;
}

test(writeAndReadByte) {
  int pos = getNextMemoryPosition();
  uint8_t in = 19;
  eprom.write(pos, in);
  assertEqual(eprom.getLastError(), NO_ERROR);
  uint8_t out = eprom.read(pos);
  assertEqual(eprom.getLastError(), NO_ERROR);
  assertEqual(in, out);
}

test(updateAndReadByte) {
  int pos = getNextMemoryPosition();
  uint8_t in = 42;
  eprom.update(pos, in);
  assertEqual(eprom.getLastError(), NO_ERROR);
  uint8_t out = eprom.read(pos);
  assertEqual(eprom.getLastError(), NO_ERROR);
  assertEqual(in, out);
}

test(putIntAndGetBack) {
  int pos = getNextMemoryPosition();
  int in = 17;
  eprom.put(pos, in);
  assertEqual(eprom.getLastError(), NO_ERROR);
  int out;
  eprom.get(pos, out);
  assertEqual(eprom.getLastError(), NO_ERROR);
  assertEqual(in, out);
}

test(putDoubleAndGetBack) {
  int pos = getNextMemoryPosition();
  double pi = 3.141593;
  eprom.put(pos, pi);
  assertEqual(eprom.getLastError(), NO_ERROR);
  double pi_in;
  eprom.get(pos, pi_in);
  assertEqual(eprom.getLastError(), NO_ERROR);
  assertEqual(pi_in, pi);
}

test(getLength) {
  assertEqual(eprom.length(), AT24C256_LENGTH);
}

test(badAddressErrorBusWorksAfter) {
  int pos = getNextMemoryPosition();
  uint8_t in = 19;
  assertEqual(badEprom.getLastError(), NO_ERROR);
  badEprom.write(pos, in);
  assertEqual(badEprom.getLastError(), ADDRESS_NACK);
  // Verify bus is still ok after error
  eprom.write(pos, in);
  assertEqual(eprom.getLastError(), NO_ERROR);
}


test(writeAndReadSmallBuffer) {
  int8_t out[15] = "Test of buffer";
  int lenw = eprom.writeBuffer(0, out, 15);
  assertEqual(lenw, 15);
  assertEqual(eprom.getLastError(), NO_ERROR);

  uint8_t in[15];
  int lenr = eprom.readBuffer(0, in, 15);
  assertEqual(lenr, 15);
  assertEqual(eprom.getLastError(), NO_ERROR);

  assertEqual((char*)in, (char*)out);
}

test(writeBufferAtBadAddress) {
  int8_t out[15] = "Test of buffer";
  int lenw = badEprom.writeBuffer(0, out, 15);
  assertEqual(lenw, 0);
  assertEqual(badEprom.getLastError(), ADDRESS_NACK);
}

test(writeAndRead256Buffer) {
  for (int i = 0; i < 255; i++) {
    buffer[i] = 'a' + (i % 26);
  }
  buffer[255] = 0;
  int lenw = eprom.writeBuffer(0, buffer, 256);
  assertEqual(lenw, 256);
  assertEqual(eprom.getLastError(), NO_ERROR);

  for (int i = 0; i < 255; i++) {
    buffer[i] = '0';
  }

  int lenr = eprom.readBuffer(0, buffer, 256);
  assertEqual(lenr, 256);
  assertEqual(eprom.getLastError(), NO_ERROR);

  for (int i = 0; i < 255; i++) {
    assertEqual(buffer[i], (uint8_t)('a' + (i % 26)));
  }
}

test(writeAndRead1024Buffer) {
  for (int i = 0; i < 1023; i++) {
    buffer[i] = 'a' + (i % 26);
  }
  buffer[1023] = 0;
  int lenw = eprom.writeBuffer(0, buffer, 1024);
  assertEqual(lenw, 1024);
  assertEqual(eprom.getLastError(), NO_ERROR);

  for (int i = 0; i < 1023; i++) {
    buffer[i] = '0';
  }

  int lenr = eprom.readBuffer(0, buffer, 1024);
  assertEqual(lenr, 1024);
  assertEqual(eprom.getLastError(), NO_ERROR);

  for (int i = 0; i < 1023; i++) {
    assertEqual(buffer[i], (uint8_t)('a' + (i % 26)));
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println(F("\n\nModule test for eeprom driver\n"));
}

void loop() {
  aunit::TestRunner::run();
}



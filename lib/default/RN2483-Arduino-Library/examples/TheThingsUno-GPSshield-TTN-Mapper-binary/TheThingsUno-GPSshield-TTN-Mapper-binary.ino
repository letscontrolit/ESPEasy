/*
 * Author: JP Meijers
 * Date: 2016-09-07
 *
 * This example program is meant for a The Things UNO board with an
 * "Adafruit Ultimate GPS+Logging Shield" on top of it. Make sure the switch on
 * the GPS shield is switched to the "Soft. Serial" position.
 *
 * Coordinates from the GPS is packed into a LoRaWan packet using binary
 * encoding, and then sent out on the air using the TTN UNO's RN2xx3 module.
 * This happens as fast as possible, while still keeping to the 1% duty cycle
 * rules enforced by the RN2483's built in LoRaWAN stack. Even though this is
 * allowed by the radio regulations of the 868MHz band, the fair use policy of
 * TTN may prohibit this.
 *
 * CHECK THE RULES BEFORE USING THIS PROGRAM!
 *
 * CHANGE ADDRESS!
 * Change the device address, network (session) key, and app (session) key to
 * the values that are registered via the TTN dashboard.
 * The appropriate line is "myLora.initABP(XXX);" or "myLora.initOTAA(XXX);"
 * When using ABP, it is advised to enable "relax frame count" on the dashboard.
 *
 * TO CONTRIBUTE TO TTN Mapper:
 * 1. Register a new Application and/or new device on the
 *     TTN Dashboard (https://console.thethingsnetwork.org).
 * 2. Copy the correct keys into the line "myLora.initABP(XXX);"
 *     or "myLora.initOTAA(XXX);" in this program.
 * 3. Comment out or remove the line: "while(!Serial); //wait for Serial to
       be available - remove this line after successful test run"
 * 4. Make sure packets are arriving on the TTN console when your node is
 *     powered and in reach of a gateway.
 * 5. Share your Application ID, Access Key and Device ID with
 *     contribute@ttnmapper.org so that your measurements can be automatically
 *     imported into TTN Mapper.
 *
 *
 * To decode the binary payload, you can use the following
 * javascript decoder function. It should work with the TTN console.
 *
function Decoder(bytes, port) {
  // Decode an uplink message from a buffer
  // (array) of bytes to an object of fields.
  var decoded = {};

  // if (port === 1) decoded.led = bytes[0];
  decoded.lat = ((bytes[0]<<16)>>>0) + ((bytes[1]<<8)>>>0) + bytes[2];
  decoded.lat = (decoded.lat / 16777215.0 * 180) - 90;

  decoded.lon = ((bytes[3]<<16)>>>0) + ((bytes[4]<<8)>>>0) + bytes[5];
  decoded.lon = (decoded.lon / 16777215.0 * 360) - 180;

  var altValue = ((bytes[6]<<8)>>>0) + bytes[7];
  var sign = bytes[6] & (1 << 7);
  if(sign)
  {
    decoded.alt = 0xFFFF0000 | altValue;
  }
  else
  {
    decoded.alt = altValue;
  }

  decoded.hdop = bytes[8] / 10.0;

  return decoded;
}
 *
 */
#include "TinyGPS++.h"
#include <SoftwareSerial.h>
#include <rn2xx3.h>

SoftwareSerial gpsSerial(8, 9); // RX, TX
TinyGPSPlus gps;
rn2xx3 myLora(Serial1);

unsigned long last_update = 0;
String toLog;
uint8_t txBuffer[9];
uint32_t LatitudeBinary, LongitudeBinary;
uint16_t altitudeGps;
uint8_t hdopGps;

#define PMTK_SET_NMEA_UPDATE_05HZ  "$PMTK220,2000*1C"
#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"
#define PMTK_SET_NMEA_OUTPUT_RMCGGA "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"

void setup() {
  //output LED pin
  pinMode(13, OUTPUT);
  led_on();

  Serial.begin(57600); //serial to computer
  gpsSerial.begin(9600); //serial to gps
  Serial1.begin(57600); //serial to RN2xx3

  // make sure usb serial connection is available,
  // or after 10s go on anyway for 'headless' use of the
  // node.
  while ((!Serial) && (millis() < 10000));

  Serial.println("TTN UNO + GPS shield startup");

  //set up RN2xx3
  initialize_radio();
  //transmit a startup message
  myLora.tx("TTN Mapper on TTN Uno with GPS");

  gpsSerial.println(F(PMTK_SET_NMEA_OUTPUT_RMCGGA));
  gpsSerial.println(F(PMTK_SET_NMEA_UPDATE_1HZ));   // 1 Hz update rate

  led_off();
  delay(1000);
}

void initialize_radio()
{
  delay(100); //wait for the RN2xx3's startup message
  Serial1.flush();

  //print out the HWEUI so that we can register it via ttnctl
  String hweui = myLora.hweui();
  while(hweui.length() != 16)
  {
    Serial.println("Communication with RN2xx3 unsuccessful. Power cycle the TTN UNO board.");
    delay(10000);
    hweui = myLora.hweui();
  }
  Serial.println("When using OTAA, register this DevEUI: ");
  Serial.println(hweui);
  Serial.println("RN2xx3 firmware version:");
  Serial.println(myLora.sysver());

  //configure your keys and join the network
  Serial.println("Trying to join TTN");
  bool join_result = false;

  //ABP: initABP(String addr, String AppSKey, String NwkSKey);
  join_result = myLora.initABP("02017201", "8D7FFEF938589D95AAD928C2E2E7E48F", "AE17E567AECC8787F749A62F5541D522");

  //OTAA: initOTAA(String AppEUI, String AppKey);
  //join_result = myLora.initOTAA("70B3D57ED00001A6", "A23C96EE13804963F8C2BD6285448198");

  while(!join_result)
  {
    Serial.println("Unable to join. Are your keys correct, and do you have TTN coverage?");
    delay(60000); //delay a minute before retry
    join_result = myLora.init();
  }
  Serial.println("Successfully joined TTN");

}

void loop() {
  while (gpsSerial.available()){
    gps.encode(gpsSerial.read());
  }

  if (gps.location.age() < 1000 && (millis() - last_update) >= 1000) {
    led_on();
    Serial.print("Interval: ");
    Serial.println(millis()-last_update);

    build_packet();

    Serial.println(toLog);
    myLora.txBytes(txBuffer, sizeof(txBuffer));
    Serial.println("TX done");

    led_off();
    last_update = millis();
  }


}

void build_packet()
{
  LatitudeBinary = ((gps.location.lat() + 90) / 180.0) * 16777215;
  LongitudeBinary = ((gps.location.lng() + 180) / 360.0) * 16777215;

  txBuffer[0] = ( LatitudeBinary >> 16 ) & 0xFF;
  txBuffer[1] = ( LatitudeBinary >> 8 ) & 0xFF;
  txBuffer[2] = LatitudeBinary & 0xFF;

  txBuffer[3] = ( LongitudeBinary >> 16 ) & 0xFF;
  txBuffer[4] = ( LongitudeBinary >> 8 ) & 0xFF;
  txBuffer[5] = LongitudeBinary & 0xFF;

  altitudeGps = gps.altitude.meters();
  txBuffer[6] = ( altitudeGps >> 8 ) & 0xFF;
  txBuffer[7] = altitudeGps & 0xFF;

  hdopGps = gps.hdop.value()/10;
  txBuffer[8] = hdopGps & 0xFF;

  toLog = "";
  for(size_t i = 0; i<sizeof(txBuffer); i++)
  {
    char buffer[3];
    sprintf(buffer, "%02x", txBuffer[i]);
    toLog = toLog + String(buffer);
  }
}

void led_on(){
  digitalWrite(13, 1);
}

void led_off(){
  digitalWrite(13, 0);
}

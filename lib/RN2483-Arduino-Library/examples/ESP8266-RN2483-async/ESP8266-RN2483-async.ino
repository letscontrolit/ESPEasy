/*
 * Author: G Noorlander (TD-er)
 * Date: 2020-02-17
 *
 * Perform initialization + transfer of some data in async mode.
 * This mode does allow to perform other tasks while waiting for a reply.
 * In Async mode we must call the async_loop() function frequently which does
 * process the state changes of the LoRa module.
 *
 * CHECK THE RULES BEFORE USING THIS PROGRAM!
 *
 * CHANGE ADDRESS!
 * Change the device address, network (session) key, and app (session) key to the values
 * that are registered via the TTN dashboard.
 * The appropriate line is "myLora.initABP(XXX);" or "myLora.initOTAA(XXX);"
 * When using ABP, it is advised to enable "relax frame count".
 *
 * Connect the RN2xx3 as follows:
 * RN2xx3 -- ESP8266
 * Uart TX -- GPIO4
 * Uart RX -- GPIO5
 * Reset -- GPIO15
 * Vcc -- 3.3V
 * Gnd -- Gnd
 *
 */
#include <rn2xx3.h>
#include <SoftwareSerial.h>

#define RESET 15
SoftwareSerial mySerial(4, 5); // RX, TX !! labels on relay board is swapped !!

// create an instance of the rn2xx3 library,
// giving the software UART as stream to use,
// and using LoRa WAN
rn2xx3 myLora(mySerial);

unsigned long _timer         = 0;
unsigned long _timeout       = 0;
unsigned long _start_command = 0;

bool _command_sent       = false;
int  _message_count      = 0;

void toggle_async_mode()
{
  bool new_async_mode_enabled = !myLora.getAsyncMode();
  myLora.setAsyncMode(new_async_mode_enabled);
  Serial.print(F("Async mode: "));
  Serial.println(new_async_mode_enabled ? F("enabled") : F("disabled"));
}

unsigned long time_passed_since(unsigned long start)
{
  return millis() - start;
}

// Set some timeout in msec from now.
void set_timeout(unsigned long timeout)
{
  _timeout = timeout;
  _timer   = millis();
}

// Check whether _timeout msec have passed since we called set_timeout
bool time_out_reached()
{
  return time_passed_since(_timer) >= _timeout;
}

void log_time_passed(unsigned long start)
{
  Serial.print(F("Time passed: "));
  Serial.print(String(time_passed_since(start)));
  Serial.println(F(" msec"));
}

// the setup routine runs once when you press reset:
void setup() {
  // LED pin is GPIO2 which is the ESP8266's built in LED
  pinMode(2, OUTPUT);
  led_on();

  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  mySerial.begin(57600);

  delay(1000); // wait for the arduino ide's serial console to open

  Serial.println("Startup");

  initialize_radio();

  // transmit a startup message
  myLora.tx("TTN Mapper on ESP8266 node");

  led_off();
  set_timeout(2000);
}

void initialize_radio()
{
  mySerial.flush();

  // reset RN2xx3
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW);
  delay(100);
  digitalWrite(RESET, HIGH);

  delay(100); // wait for the RN2xx3's startup message
  Serial.println(F("Boot message:"));
  Serial.println(mySerial.readBytesUntil('\n'));
  mySerial.flush();

  // check communication with radio
  String hweui = myLora.hweui();

  while (hweui.length() != 16)
  {
    Serial.println("Communication with RN2xx3 unsuccessful. Power cycle the board.");
    Serial.println(hweui);
    delay(10000);
    hweui = myLora.hweui();
  }

  // print out the HWEUI so that we can register it via ttnctl
  Serial.println("When using OTAA, register this DevEUI: ");
  Serial.println(hweui);
  Serial.println("RN2xx3 firmware version:");
  Serial.println(myLora.sysver());

  // configure your keys and join the network
  Serial.println("Trying to join TTN");
  bool join_result = false;

  // ABP: initABP(String addr, String AppSKey, String NwkSKey);
  join_result = myLora.initABP("02017201", "8D7FFEF938589D95AAD928C2E2E7E48F", "AE17E567AECC8787F749A62F5541D522");

  // OTAA: initOTAA(String AppEUI, String AppKey);
  // join_result = myLora.initOTAA("70B3D57ED00001A6", "A23C96EE13804963F8C2BD6285448198");

  while (!join_result)
  {
    Serial.println("Unable to join. Are your keys correct, and do you have TTN coverage?");
    delay(60000); // delay a minute before retry
    join_result = myLora.init();
  }
  Serial.println("Successfully joined TTN");
}

bool send_message(const String& message) {
  led_on();

  Serial.print("TXing: ");
  Serial.println(message);

  unsigned long start = millis();

  RN2xx3_datatypes::TX_return_type result = myLora.tx(message);
  log_time_passed(start);

  bool success = false;

  switch (result) {
    case RN2xx3_datatypes::TX_return_type::TX_FAIL:

      // A TX command may fail for various reasons.
      // Possible reasons:
      // - not_joined
      // - previous command has not yet finished (async mode)
      // - Message is too long
      // - Module does not reply within timeout period
      // - other reasons
      Serial.print(F("TX failed: "));
      Serial.println(myLora.getLastError());
      break;
    case RN2xx3_datatypes::TX_return_type::TX_SUCCESS:

      // Async mode: Command is accepted
      // Default mode: No error received from module after RX2 window has passed.
      Serial.println(F("TX Success"));
      success = true;
      break;
    case RN2xx3_datatypes::TX_return_type::TX_WITH_RX:

      // Async mode: Received RX is not yet known, will have to check later.
      // Default mode: TX Success and something received in RX2 window.
      Serial.print(F("TX Success, RX received: "));
      Serial.println(myLora.getRx());
      success = true;
      break;

      // No default: here, so the compiler will warn us
      // if the enum has new cases which we do not yet handle.
  }
  led_off();
  return success;
}

// To make it clear this is just one of the many tasks we can do,
// this is split in a separate function.
void loop_handle_LoRa_command() {
  if (!time_out_reached()) {
    // It is not our time yet to do stuff.
    return;
  }

  if (!_command_sent) {
    // Have not sent a TX command, try sending one.
    String message = F("Hello World! (");
    message += _message_count;
    message += ')';

    if (send_message(message)) {
      ++_message_count;
      _start_command = millis();
      _command_sent  = true;
    } else {
      // Failed to send a message, wait for 1 second.
      set_timeout(1000);
    }
  } else {
    // A command has been sent, check if it has completed.
    if (!myLora.command_finished()) {
      // We still wait for the command to end....
      Serial.print('.');
      set_timeout(50); // Only print a dot every 50 msec.
    } else {
      Serial.println(F("Command finished."));

      // Log how long it took
      log_time_passed(_start_command);
      _command_sent = false;

      if (_message_count % 2 == 0) {
        // Every 2 messages, toggle Async mode.
        toggle_async_mode();
      }

      // To not overload the network, wait for 10 seconds before we send a new message.
      set_timeout(10000);
    }
  }
}

// the loop routine runs over and over again forever:
void loop() {
  if (myLora.getAsyncMode()) {
    myLora.async_loop();
  }
  loop_handle_LoRa_command();
}

void led_on()
{
  digitalWrite(2, 1);
}

void led_off()
{
  digitalWrite(2, 0);
}

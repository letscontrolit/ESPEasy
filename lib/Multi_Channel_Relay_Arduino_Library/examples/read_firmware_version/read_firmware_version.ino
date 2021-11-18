#include <multi_channel_relay.h>

#define USE_8_CHANNELS (1)

Multi_Channel_Relay relay;

void setup() {
    DEBUG_PRINT.begin(9600);
    while (!DEBUG_PRINT);

    // Set I2C address and start relay
    relay.begin(0x11);

    /* Read firmware  version */
    DEBUG_PRINT.print("firmware version: ");
    DEBUG_PRINT.print("0x");
    DEBUG_PRINT.print(relay.getFirmwareVersion(), HEX);
    DEBUG_PRINT.println();
}

void loop() {

}

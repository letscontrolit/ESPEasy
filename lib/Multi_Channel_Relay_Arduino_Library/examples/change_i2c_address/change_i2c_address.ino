/**
    When running this sketch please make sure there is only one
    Multi Channel Relay module connected to your board, otherwise
    it may not work to change module address, or cause other
    unpredictable issue.


*/

#include <multi_channel_relay.h>


int new_i2c_address = 0x21;

Multi_Channel_Relay relay;

void setup() {
    uint8_t old_address = 0;
    uint8_t retry = 0;

    DEBUG_PRINT.begin(9600);
    while (!DEBUG_PRINT);

    // // Set I2C address by default and start relay
    relay.begin();

    /* Scan I2C device detect device address */
    while ((retry++ < 10) && (old_address == 0x00)) {
        old_address = relay.scanI2CDevice();
        delay(100);
    }

    if ((0x00 == old_address) || (0xff == old_address)) {
        while (1);
    }

    relay.changeI2CAddress(old_address, new_i2c_address);  /* Set I2C address and save to Flash */
    DEBUG_PRINT.print("Address 0x");
    DEBUG_PRINT.print(new_i2c_address, HEX);
    DEBUG_PRINT.println(" has been saved to flash.");
}

void loop() {

}

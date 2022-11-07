#include <multi_channel_relay.h>

/**
    channle: 8 7 6 5 4 3 2 1
    state: 0b00000000 -> 0x00  (all off)
    state: 0b11111111 -> 0xff  (all on)
*/

Multi_Channel_Relay relay;

void setup() {

    DEBUG_PRINT.begin(9600);
    while (!DEBUG_PRINT);

    // Set I2C address and start relay
    relay.begin(0x11);

    /* Begin Controlling Relay */
    DEBUG_PRINT.println("Channel 1 on");
    relay.turn_on_channel(1);
    delay(500);
    DEBUG_PRINT.println("Channel 2 on");
    relay.turn_off_channel(1);
    relay.turn_on_channel(2);
    delay(500);
    DEBUG_PRINT.println("Channel 3 on");
    relay.turn_off_channel(2);
    relay.turn_on_channel(3);
    delay(500);
    DEBUG_PRINT.println("Channel 4 on");
    relay.turn_off_channel(3);
    relay.turn_on_channel(4);
    delay(500);
    DEBUG_PRINT.println("Channel 5 on");
    relay.turn_off_channel(4);
    relay.turn_on_channel(5);
    delay(500);
    DEBUG_PRINT.println("Channel 6 on");
    relay.turn_off_channel(5);
    relay.turn_on_channel(6);
    delay(500);
    DEBUG_PRINT.println("Channel 7 on");
    relay.turn_off_channel(6);
    relay.turn_on_channel(7);
    delay(500);
    DEBUG_PRINT.println("Channel 8 on");
    relay.turn_off_channel(7);
    relay.turn_on_channel(8);
    delay(500);
    relay.turn_off_channel(8);

    relay.channelCtrl(CHANNLE1_BIT |
                      CHANNLE2_BIT |
                      CHANNLE3_BIT |
                      CHANNLE4_BIT |
                      CHANNLE5_BIT |
                      CHANNLE6_BIT |
                      CHANNLE7_BIT |
                      CHANNLE8_BIT);
    DEBUG_PRINT.print("Turn all channels on, State: ");
    DEBUG_PRINT.println(relay.getChannelState(), BIN);

    delay(2000);

    relay.channelCtrl(CHANNLE1_BIT |
                      CHANNLE3_BIT |
                      CHANNLE5_BIT |
                      CHANNLE7_BIT);
    DEBUG_PRINT.print("Turn 1 3 5 7 channels on, State: ");
    DEBUG_PRINT.println(relay.getChannelState(), BIN);

    delay(2000);

    relay.channelCtrl(CHANNLE2_BIT |
                      CHANNLE4_BIT |
                      CHANNLE6_BIT |
                      CHANNLE8_BIT);
    DEBUG_PRINT.print("Turn 2 4 6 8 channels on, State: ");
    DEBUG_PRINT.println(relay.getChannelState(), BIN);

    delay(2000);


    relay.channelCtrl(0);
    DEBUG_PRINT.print("Turn off all channels, State: ");
    DEBUG_PRINT.println(relay.getChannelState(), BIN);

    delay(2000);
}

void loop() {


}

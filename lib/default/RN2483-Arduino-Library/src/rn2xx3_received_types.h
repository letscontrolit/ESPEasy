
#ifndef RN2XX3_RECEIVED_TYPES_H
#define RN2XX3_RECEIVED_TYPES_H

#include "Arduino.h"

// This class only decodes the possible replies of the RN2483/RN2903
// It allows to convert a received string into a single enum value
class RN2xx3_received_types {
public:

  enum received_t {
    accepted,                        // successful join
    busy,                            //  if MAC state is not in an Idle state
    denied,                          //  if the join procedure was unsuccessful (the module attempted to join the network, but was
                                     // rejected);
    frame_counter_err_rejoin_needed, //  if the frame counter rolled over
    invalid_data_len,                //  if application payload length is greater than the maximum application payload length corresponding
                                     // to the current data rate
                                     // (after first uplink transmission)  if application payload length is greater than the maximum
                                     // application payload length corresponding to the current data rate. This can occur after an earlier
                                     // uplink attempt if retransmission back-off has reduced the data rate
    invalid_param,                   //  if parameters (e.g. <type> <portno> <data>) are not valid
    keys_not_init,                   // if the keys corresponding to the Join mode (otaa or abp) were not configured
    mac_err,                         // (after first uplink transmission)  if transmission was unsuccessful, ACK not received back from the
                                     // server
    mac_paused,                      //  if MAC was paused and not resumed back
    mac_rx,                          // (after first uplink transmission)  if transmission was successful, <portno>: port number, from 1 to
                                     // 223; <data>: hexadecimal value that was received from the server;
    mac_tx_ok,                       // (after first uplink transmission)  if uplink transmission was successful and no downlink data was
                                     // received back from the server
    no_free_ch,                      //  if all channels are busy
    not_joined,                      // if the network is not joined
    ok,                              // if parameters and configurations are valid and the packet was forwarded to the radio transceiver for
                                     // transmission
    radio_err,                       // radio rx :  if reception was not successful, reception time-out occurred
                                     // radio tx :  if transmission was unsuccessful (interrupted by radio Watchdog Timer time-out)
    radio_rx,                        // radio_rx <data> – if reception was successful, <data>: hexadecimal value that was received;
    radio_tx_ok,                     //  if transmission was successful
    silent,                          //  if the module is in a Silent Immediately state
    UNKNOWN
  };

  static received_t determineReceivedDataType(const String& receivedData);
};

    #endif // RN2XX3_RECEIVED_TYPES_H

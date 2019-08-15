/*
 * A library for controlling a Microchip RN2xx3 LoRa radio.
 *
 * @Author JP Meijers
 * @Author Nicolas Schteinschraber
 * @Date 18/12/2015
 *
 */

#ifndef rn2xx3_h
#define rn2xx3_h

#include "Arduino.h"

enum RN2xx3_t {
  RN_NA = 0, // Not set
  RN2903 = 2903,
  RN2483 = 2483
};

enum FREQ_PLAN {
  SINGLE_CHANNEL_EU = 0,
  TTN_EU,
  TTN_US,
  DEFAULT_EU
};

enum TX_RETURN_TYPE {
  TX_FAIL = 0,    // The transmission failed.
                  // If you sent a confirmed message and it is not acked,
                  // this will be the returned value.

  TX_SUCCESS = 1, // The transmission was successful.
                  // Also the case when a confirmed message was acked.

  TX_WITH_RX = 2  // A downlink message was received after the transmission.
                  // This also implies that a confirmed message is acked.
};

class rn2xx3
{
  public:

    /*
     * A simplified constructor taking only a Stream ({Software/Hardware}Serial) object.
     * The serial port should already be initialised when initialising this library.
     */
    rn2xx3(Stream& serial);

    /*
     * Transmit the correct sequence to the rn2xx3 to trigger its autobauding feature.
     * After this operation the rn2xx3 should communicate at the same baud rate than us.
     */
    bool autobaud();

    /*
     * Get the hardware EUI of the radio, so that we can register it on The Things Network
     * and obtain the correct AppKey.
     * You have to have a working serial connection to the radio before calling this function.
     * In other words you have to at least call autobaud() some time before this function.
     */
    String hweui();

    /*
     * Returns the AppSKey or AppKey used when initializing the radio.
     * In the case of ABP this function will return the App Session Key.
     * In the case of OTAA this function will return the App Key.
     */
    String appkey();

    /*
     * In the case of OTAA this function will return the Application EUI used
     * to initialize the radio.
     */
    String appeui();

    /*
     * In the case of OTAA this function will return the Device EUI used to
     * initialize the radio. This is not necessarily the same as the Hardware EUI.
     * To obtain the Hardware EUI, use the hweui() function.
     */
    String deveui();

    /*
     * Get the RN2xx3's hardware and firmware version number. This is also used
     * to detect if the module is either an RN2483 or an RN2903.
     */
    String sysver();

    bool setSF(uint8_t sf);

    /*
     * Initialise the RN2xx3 and join the LoRa network (if applicable).
     * This function can only be called after calling initABP() or initOTAA().
     * The sole purpose of this function is to re-initialise the radio if it
     * is in an unknown state.
     */
    bool init();

    /*
     * Initialise the RN2xx3 and join a network using personalization.
     *
     * addr: The device address as a HEX string.
     *       Example "0203FFEE"
     * AppSKey: Application Session Key as a HEX string.
     *          Example "8D7FFEF938589D95AAD928C2E2E7E48F"
     * NwkSKey: Network Session Key as a HEX string.
     *          Example "AE17E567AECC8787F749A62F5541D522"
     */
    bool initABP(const String& addr, const String& AppSKey, const String& NwkSKey);

    //TODO: initABP(uint8_t * addr, uint8_t * AppSKey, uint8_t * NwkSKey)

    /*
     * Initialise the RN2xx3 and join a network using over the air activation.
     *
     * AppEUI: Application EUI as a HEX string.
     *         Example "70B3D57ED00001A6"
     * AppKey: Application key as a HEX string.
     *         Example "A23C96EE13804963F8C2BD6285448198"
     * DevEUI: Device EUI as a HEX string.
     *         Example "0011223344556677"
     * If the DevEUI parameter is omitted, the Hardware EUI from module will be used
     * If no keys, or invalid length keys, are provided, no keys
     * will be configured. If the module is already configured with some keys
     * they will be used. Otherwise the join will fail and this function
     * will return false.
     */
    bool initOTAA(const String& AppEUI="", const String& AppKey="", const String& DevEUI="");

    /*
     * Initialise the RN2xx3 and join a network using over the air activation,
     * using byte arrays. This is useful when storing the keys in eeprom or flash
     * and reading them out in runtime.
     *
     * AppEUI: Application EUI as a uint8_t buffer
     * AppKey: Application key as a uint8_t buffer
     * DevEui: Device EUI as a uint8_t buffer (optional - set to 0 to use Hardware EUI)
     */
     bool initOTAA(uint8_t * AppEUI, uint8_t * AppKey, uint8_t * DevEui);

    /*
     * Transmit the provided data. The data is hex-encoded by this library,
     * so plain text can be provided.
     * This function is an alias for txUncnf().
     *
     * Parameter is an ascii text string.
     */
    TX_RETURN_TYPE tx(const String& , uint8_t port = 1);

    /*
     * Transmit raw byte encoded data via LoRa WAN.
     * This method expects a raw byte array as first parameter.
     * The second parameter is the count of the bytes to send.
     */
    TX_RETURN_TYPE txBytes(const byte*, uint8_t size, uint8_t port = 1);

    TX_RETURN_TYPE txHexBytes(const String&, uint8_t port = 1);

    /*
     * Do a confirmed transmission via LoRa WAN.
     *
     * Parameter is an ascii text string.
     */
    TX_RETURN_TYPE txCnf(const String&, uint8_t port = 1);

    /*
     * Do an unconfirmed transmission via LoRa WAN.
     *
     * Parameter is an ascii text string.
     */
    TX_RETURN_TYPE txUncnf(const String&, uint8_t port = 1);

    /*
     * Transmit the provided data using the provided command.
     *
     * String - the tx command to send
                can only be one of "mac tx cnf 1 " or "mac tx uncnf 1 "
     * String - an ascii text string if bool is true. A HEX string if bool is false.
     * bool - should the data string be hex encoded or not
     */
    TX_RETURN_TYPE txCommand(const String&, const String&, bool, uint8_t port = 1);

    /*
     * Change the datarate at which the RN2xx3 transmits.
     * A value of between 0 and 5 can be specified,
     * as is defined in the LoRaWan specs.
     * This can be overwritten by the network when using OTAA.
     * So to force a datarate, call this function after initOTAA().
     */
    bool setDR(int dr);

    /*
     * Put the RN2xx3 to sleep for a specified timeframe.
     * The RN2xx3 accepts values from 100 to 4294967296.
     * Rumour has it that you need to do a autobaud() after the module wakes up again.
     */
    void sleep(long msec);

    /*
     * Send a raw command to the RN2xx3 module.
     * Returns the raw string as received back from the RN2xx3.
     * If the RN2xx3 replies with multiple line, only the first line will be returned.
     */
    String sendRawCommand(const String& command);

    /*
     * Returns the module type either RN2903 or RN2483, or NA.
     */
    RN2xx3_t moduleType();

    /*
     * Set the active channels to use.
     * Returns true if setting the channels is possible.
     * Returns false if you are trying to use the wrong channels on the wrong module type.
     */
    bool setFrequencyPlan(FREQ_PLAN);

    /*
     * Returns the last downlink message HEX string.
     */
    String getRx();

    /*
     * Get the RN2xx3's SNR of the last received packet. Helpful to debug link quality.
     */
    int getSNR();

    /*
     * Get the RN2xx3's voltage measurement on the Vdd in mVolt
     * 0–3600 (decimal value from 0 to 3600)
     */
    int getVbat();

    /*
     * Return the current data rate formatted like sf7bw125
     * Firmware 1.0.1 returns always "sf9"
     */
    String getDataRate();

    /*
     * Return radio Received Signal Strength Indication (rssi) value
     * for the last received frame.
     * Supported since firmware 1.0.5
     */
    int getRSSI();

    /*
     * Encode an ASCII string to a HEX string as needed when passed
     * to the RN2xx3 module.
     */
    String base16encode(const String&);

    /*
     * Decode a HEX string to an ASCII string. Useful to decode a
     * string received from the RN2xx3.
     */
    String base16decode(const String&);

    /*
     * Almost all commands can return "invalid_param"
     * The last command resulting in such an error can be retrieved.
     * Reading this will clear the error.
     */
    String getLastErrorInvalidParam();

    String peekLastErrorInvalidParam();

    bool hasJoined() const { return Status.Joined; }

    bool useOTAA() const { return _otaa; }

    // Get the current frame counter values for downlink and uplink
    bool getFrameCounters(uint32_t &dnctr, uint32_t &upctr);

    // Set frame counter values for downlink and uplink
    // E.g. to restore them after a reboot or reset of the module.
    bool setFrameCounters(uint32_t dnctr, uint32_t upctr);

    // At init() the module is assumed to be joined, which is also checked against the 
    // _otaa flag.
    // Allow to set the last used join mode to help prevent unneeded join requests.
    void setLastUsedJoinMode(bool isOTAA) { _otaa = isOTAA; }
    
    struct Status_t {
      Status_t() { decode(0); }
      Status_t(uint32_t value) { decode(value); }

      enum MacState_t {
        Idle = 0,                  // Idle (transmissions are possible)
        TransmissionOccurring = 1, // Transmission occurring
        PreOpenReceiveWindow1 = 2, // Before the opening of Receive window 1
        ReceiveWindow1Open = 3,    // Receive window 1 is open
        BetwReceiveWindow1_2 = 4,  // Between Receive window 1 and Receive window 2
        ReceiveWindow2Open = 5,    // Receive window 2 is open
        RetransDelay = 6,          // Retransmission delay - used for ADR_ACK delay, FSK can occur
        APB_delay = 7,             //APB_delay
        Class_C_RX2_1_open = 8, // Class C RX2 1 open
        Class_C_RX2_2_open = 9  // Class C RX2 2 open
      } MacState;

      // Joined does not seem to be updated in the status bits.
      // Assume joined at first unless a transmit command returns "not_joined".
      // This will prevent a lot of unneeded join requests.
      bool Joined = true;    
      bool AutoReply;
      bool ADR;
      bool SilentImmediately; // indicates the device has been silenced by the network. To enable: "mac forceENABLE"
      bool MacPause;          // Temporary disable the LoRaWAN protocol interpreter. (e.g. to change radio settings)
      bool RxDone;
      bool LinkCheck;
      bool ChannelsUpdated;
      bool OutputPowerUpdated;
      bool NbRepUpdated;  // NbRep is the number of repetitions for unconfirmed packets
      bool PrescalerUpdated;
      bool SecondReceiveWindowParamUpdated;
      bool RXtimingSetupUpdated;
      bool RejoinNeeded;
      bool Multicast;

      bool decode(uint32_t value) {
        _rawstatus = value;

        MacState = static_cast<MacState_t>(value & 0xF);
        value = value >> 4;
        Joined                          = Joined | (value & 1); value = value >> 1;
        AutoReply                       = (value & 1); value = value >> 1;
        ADR                             = (value & 1); value = value >> 1;
        SilentImmediately               = (value & 1); value = value >> 1;
        MacPause                        = (value & 1); value = value >> 1;
        RxDone                          = (value & 1); value = value >> 1;
        LinkCheck                       = (value & 1); value = value >> 1;
        ChannelsUpdated                 = ChannelsUpdated                 | (value & 1); value = value >> 1;
        OutputPowerUpdated              = OutputPowerUpdated              | (value & 1); value = value >> 1;
        NbRepUpdated                    = NbRepUpdated                    | (value & 1); value = value >> 1;
        PrescalerUpdated                = PrescalerUpdated                | (value & 1); value = value >> 1;
        SecondReceiveWindowParamUpdated = SecondReceiveWindowParamUpdated | (value & 1); value = value >> 1;
        RXtimingSetupUpdated            = RXtimingSetupUpdated            | (value & 1); value = value >> 1;
        RejoinNeeded                    = (value & 1); value = value >> 1;
        Multicast                       = (value & 1); value = value >> 1;


        /*
        The following bits are cleared after issuing a “mac get status” command:
        - 11 (Channels updated)
        - 12 (Output power updated)
        - 13 (NbRep updated)
        - 14 (Prescaler updated)
        - 15 (Second Receive window parameters updated)
        - 16 (RX timing setup updated) 

        So we must keep track of them to see if they were updated since the last time they were saved to the 
        */

        _saveSettingsNeeded = 
          _saveSettingsNeeded ||
          ChannelsUpdated || 
          OutputPowerUpdated || 
          NbRepUpdated || 
          PrescalerUpdated || 
          SecondReceiveWindowParamUpdated || 
          RXtimingSetupUpdated;
        return _saveSettingsNeeded;
      }

      bool saveSettingsNeeded() const { return _saveSettingsNeeded; }

      bool clearSaveSettingsNeeded() {
        bool ret = _saveSettingsNeeded;

        _saveSettingsNeeded = false;
        ChannelsUpdated = false;
        OutputPowerUpdated = false;
        NbRepUpdated = false;
        PrescalerUpdated = false;
        SecondReceiveWindowParamUpdated = false;
        RXtimingSetupUpdated = false;

        return ret;
      }

      uint32_t getRawStatus() const { return _rawstatus; };


    private:
      uint32_t _rawstatus = 0;
      bool _saveSettingsNeeded = false;
    } Status;

  private:
    Stream& _serial;

    RN2xx3_t _moduleType = RN_NA;

    //Flags to switch code paths. Default is to use OTAA.
    bool _otaa = true;

    FREQ_PLAN _fp = TTN_EU;
    uint8_t _sf = 7;

    uint32_t rxdelay1 = 1000;
    uint32_t rxdelay2 = 2000;

    //The default address to use on TTN if no address is defined.
    //This one falls in the "testing" address space.
    String _devAddr = "03FFBEEF";

    // if you want to use another DevEUI than the hardware one
    // use this deveui for LoRa WAN
    String _deveui = "0011223344556677";

    //the appeui to use for LoRa WAN
    String _appeui = "0";

    //the nwkskey to use for LoRa WAN
    String _nwkskey = "0";

    //the appskey/appkey to use for LoRa WAN
    String _appskey = "0";

    // The downlink messenge
    String _rxMessenge = "";

    String _lastErrorInvalidParam = "";

    /*
     * Auto configure for either RN2903 or RN2483 module
     */
    RN2xx3_t configureModuleType();

    bool resetModule();

    void sendEncoded(const String&);

    enum received_t {
      accepted,
      busy,
      denied,
      frame_counter_err_rejoin_needed,
      invalid_data_len,
      invalid_param,
      keys_not_init,
      mac_err,
      mac_paused,
      mac_rx,
      mac_tx_ok,
      no_free_ch,
      not_joined,
      ok,
      radio_err,
      radio_tx_ok,
      silent,
      UNKNOWN
    };

    static received_t determineReceivedDataType(const String& receivedData);

    int readIntValue(const String& command);

    bool readUIntMacGet(const String& param, uint32_t &value);


    // All "mac set ..." commands return either "ok" or "invalid_param"
    bool sendMacSet(const String& param, const String& value);
    bool sendMacSetEnabled(const String& param, bool enabled);
    bool sendMacSetCh(const String& param, unsigned int channel, const String& value);
    bool sendMacSetCh(const String& param, unsigned int channel, uint32_t value);
    bool setChannelDutyCycle(unsigned int channel, unsigned int dutyCycle);
    bool setChannelFrequency(unsigned int channel, uint32_t frequency);
    bool setChannelDataRateRange(unsigned int channel, unsigned int minRange, unsigned int maxRange);

    // Set channel enabled/disabled.
    // Frequency, data range, duty cycle must be issued prior to enabling the status of that channel
    bool setChannelEnabled(unsigned int channel, bool enabled);
    
    bool set2ndRecvWindow(unsigned int dataRate, uint32_t frequency);
    bool setAdaptiveDataRate(bool enabled);
    bool setAutomaticReply(bool enabled);
    bool setTXoutputPower(int pwridx);

    // Read the internal status of the module
    // @retval true when update was successful
    bool updateStatus();
    bool saveUpdatedStatus();

    // Set the serial timeout for standard transactions. (not waiting for a packet acknowledgement)
    void setSerialTimeout();
    // Set serial timeout to wait for 2nd receive window (RX2)
    void setSerialTimeoutRX2();

    void clearSerialBuffer();

    static bool isHexStr(const String& string);

};

#endif

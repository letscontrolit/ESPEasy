#ifdef USES_P035
//#######################################################################################################
//#################################### Plugin 035: Output IR ############################################
//#######################################################################################################

#ifdef ESP8266  // Needed for precompile issues.
#include <IRremoteESP8266.h>
#endif
#include <IRsend.h>
#include <IRutils.h>

IRsend *Plugin_035_irSender=nullptr;


#define PLUGIN_035
#define PLUGIN_ID_035         35
#define PLUGIN_NAME_035       "Communication - IR Transmit"
#define STATE_SIZE_MAX        53U
#define PRONTO_MIN_LENGTH     6U

#define from_32hex(c) ((((c) | ('A' ^ 'a')) - '0') % 39)

#define P35_Ntimings 250 //Defines the ammount of timings that can be stored. Used in RAW and RAW2 encodings

boolean Plugin_035(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_035;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].SendDataOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_035);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output("LED");
        break;
      }
          case PLUGIN_WEBFORM_LOAD:
      {
        addRowLabel(F("Command"));
        addHtml(F("IRSENT,[PROTOCOL],[DATA],[BITS optional],[REPEATS optional]<BR>BITS and REPEATS are optional and default to 0"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        int irPin = CONFIG_PIN1;
        if (Plugin_035_irSender == 0 && irPin != -1)
        {
          addLog(LOG_LEVEL_INFO, F("INIT: IR TX"));
          Plugin_035_irSender = new IRsend(irPin);
          Plugin_035_irSender->begin(); // Start the sender
        }
        if (Plugin_035_irSender != 0 && irPin == -1)
        {
          addLog(LOG_LEVEL_INFO, F("INIT: IR TX Removed"));
          delete Plugin_035_irSender;
          Plugin_035_irSender = 0;
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        uint64_t IrCode=0;
        uint16_t  IrBits=0;

        String cmdCode = string;
        int argIndex = cmdCode.indexOf(',');
        if (argIndex) cmdCode = cmdCode.substring(0, argIndex);

        if (cmdCode.equalsIgnoreCase(F("IRSEND")) && Plugin_035_irSender != 0)
        {
          success = true;
          #ifdef PLUGIN_016
          if (irReceiver != 0) irReceiver->disableIRIn(); // Stop the receiver
          #endif

          String IrType;
          String IrType_orig;
          String TmpStr1;
          if (GetArgv(string.c_str(), TmpStr1, 2)) {
            IrType = TmpStr1;
            IrType_orig = TmpStr1;
            IrType.toLowerCase();
          }

          if (IrType.equals(F("raw")) || IrType.equals(F("raw2"))) {
            String IrRaw;
            uint16_t IrHz=0;
            unsigned int IrPLen=0;
            unsigned int IrBLen=0;

            if (GetArgv(string.c_str(), TmpStr1, 3)) IrRaw  = TmpStr1;                    //Get the "Base32" encoded/compressed Ir signal
            if (GetArgv(string.c_str(), TmpStr1, 4)) IrHz   = str2int(TmpStr1.c_str());   //Get the base freguency of the signal (allways 38)
            if (GetArgv(string.c_str(), TmpStr1, 5)) IrPLen = str2int(TmpStr1.c_str());   //Get the Pulse Length in ms
            if (GetArgv(string.c_str(), TmpStr1, 6)) IrBLen = str2int(TmpStr1.c_str());   //Get the Blank Pulse Length in ms

            printWebString += IrType_orig + String(F(": Base32Hex RAW Code: ")) + IrRaw + String(F("<BR>kHz: "))+ IrHz + String(F("<BR>Pulse Len: ")) + IrPLen + String(F("<BR>Blank Len: ")) + IrBLen + String(F("<BR>"));

            uint16_t idx = 0;                   //If this goes above the buf.size then the esp will throw a 28 EXCCAUSE
            uint16_t *buf;
            buf =  new uint16_t[P35_Ntimings]; //The Raw Timings that we can buffer.
            if (buf == nullptr) {              // error assigning memory.
            success = false;
            return success;
            }
            
            if (IrType.equals(F("raw"))) {
                unsigned int c0 = 0; //count consecutives 0s
                unsigned int c1 = 0; //count consecutives 1s

                //printWebString += F("Interpreted RAW Code: ");  //print the number of 1s and 0s just for debugging/info purposes
                //Loop throught every char in RAW string
                for(unsigned int i = 0; i < IrRaw.length(); i++)
                {
                  //Get the decimal value from base32 table
                  //See: https://en.wikipedia.org/wiki/Base32#base32hex
                  char c = from_32hex(IrRaw[i]);

                  //Loop through 5 LSB (bits 16, 8, 4, 2, 1)
                  for (unsigned int shft = 1; shft < 6; shft++)
                  {
                    //if bit is 1 (5th position - 00010000 = 16)
                    if ((c & 16) != 0) {
                      //add 1 to counter c1
                      c1++;
                      //if we already have any 0s in counting (the previous
                      //bit was 0)
                      if (c0 > 0) {
                        //add the total ms into the buffer (number of 0s multiplied
                        //by defined blank length ms)
                        buf[idx++] = c0 * IrBLen;
                        //print the number of 0s just for debuging/info purpouses
                        //for (uint t = 0; t < c0; t++)
                          //printWebString += '0';
                      }
                      //So, as we receive a "1", and processed the counted 0s
                      //sending them as a ms timing into the buffer, we clear
                      //the 0s counter
                      c0 = 0;
                    } else {
                      //So, bit is 0

                      //On first call, ignore 0s (suppress left-most 0s)
                      if (c0+c1 != 0) {
                        //add 1 to counter c0
                        c0++;
                        //if we already have any 1s in counting (the previous
                        //bit was 1)
                        if (c1 > 0) {
                          //add the total ms into the buffer (number of 1s
                          //multiplied by defined pulse length ms)
                          buf[idx++] = c1 * IrPLen;
                          //print the number of 1s just for debugging/info purposes
//                          for (uint t = 0; t < c1; t++)
//                            printWebString += '1';
                        }
                        //So, as we receive a "0", and processed the counted 1s
                        //sending them as a ms timing into the buffer, we clear
                        //the 1s counter
                        c1 = 0;
                      }
                    }
                    //shift to left the "c" variable to process the next bit that is
                    //in 5th position (00010000 = 16)
                    c <<= 1;
                  }
                }

                //Finally, we need to process the last counted bit that we were
                //processing

                //If we have pendings 0s
                if (c0 > 0) {
                  buf[idx++] = c0 * IrBLen;
                  //for (uint t = 0; t < c0; t++)
                    //printWebString += '0';
                }
                //If we have pendings 1s
                if (c1 > 0) {
                  buf[idx++] = c1 * IrPLen;
                  //for (uint t = 0; t < c1; t++)
                    //printWebString += '1';
                }

                //printWebString += F("<BR>");

            } else {        // RAW2
                for (unsigned int i = 0, total = IrRaw.length(), gotRep = 0, rep = 0; i < total;) {
                   char c = IrRaw[i++];
                   if (c == '*') {
                       if (i+2 >= total || idx + (rep = from_32hex(IrRaw[i++])) * 2 > sizeof(buf[0])*P35_Ntimings){
                         delete[] buf;
                         buf = nullptr;
                         return addErrorTrue();
                       }
                       gotRep = 2;
                   } else {
                       if ((c == '^' && i+1 >= total) || idx >= sizeof(buf[0])*P35_Ntimings){
                         delete[] buf;
                         buf = nullptr;
                         return addErrorTrue();
                       }

                       uint16_t irLen = (idx & 1)? IrBLen : IrPLen;
                       if (c == '^') {
                           buf[idx++] = (from_32hex(IrRaw[i]) * 32 + from_32hex(IrRaw[i+1])) * irLen;
                           i += 2;
                       } else
                           buf[idx++] = from_32hex(c) * irLen;

                       if (--gotRep == 0) {
                           while (--rep) {
                               buf[idx] = buf[idx-2];
                               buf[idx+1] = buf[idx-1];
                               idx += 2;
                           }
                       }
                   }
                }
            } //End RAW2

            Plugin_035_irSender->sendRaw(buf, idx, IrHz);
            delete[] buf;
            buf = nullptr;
            //String line = "";
            //for (int i = 0; i < idx; i++)
            //    line += uint64ToString(buf[i], 10) + ",";
            //serialPrintln(line);

            //sprintf_P(log, PSTR("IR Params1: Hz:%u - PLen: %u - BLen: %u"), IrHz, IrPLen, IrBLen);
            //addLog(LOG_LEVEL_INFO, log);
            //sprintf_P(log, PSTR("IR Params2: RAW Code:%s"), IrRaw.c_str());
            //addLog(LOG_LEVEL_INFO, log);
          } else {
           uint16_t  IrRepeat=0;
          //  unsigned long IrSecondCode=0UL;
            String ircodestr;
            if (GetArgv(string.c_str(), TmpStr1, 2)) {
              IrType = TmpStr1;
              IrType_orig = TmpStr1;
              IrType.toLowerCase(); // To lower case to inprove compare speed
            }
            if (GetArgv(string.c_str(), ircodestr, 3)) {
              IrCode = strtoull(ircodestr.c_str(), NULL, 16);
            }

            if (GetArgv(string.c_str(), TmpStr1, 4)) IrBits = str2int(TmpStr1.c_str());   //not needed any more as I can tell... Left for reverse compitability so we won't brake old forum posts etc..
            IrBits = 0;                                                                   //Leave it to 0 for default protocol bits
            if (GetArgv(string.c_str(), TmpStr1, 5)) IrRepeat = str2int(TmpStr1.c_str()); // Nr. of times the message is to be repeated

            if (IrType.equals(F("aiwa_rc_t501")))       sendIRCode(AIWA_RC_T501,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("carrier_ac")))         sendIRCode(CARRIER_AC,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("coolix")))             sendIRCode(COOLIX,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("denon")))              sendIRCode(DENON,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("dish")))               sendIRCode(DISH,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("gicable")))            sendIRCode(GICABLE,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("jvc")))                sendIRCode(JVC,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("lasertag")))           sendIRCode(LASERTAG,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("legopf")))             sendIRCode(LEGOPF,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("lg")))                 sendIRCode(LG,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("lg2")))                sendIRCode(LG2,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("lutron")))             sendIRCode(LUTRON,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("magiquest")))          sendIRCode(MAGIQUEST,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("midea")))              sendIRCode(MIDEA,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("mitsubishi")))         sendIRCode(MITSUBISHI,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("mitsubishi2")))        sendIRCode(MITSUBISHI2,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("nikai")))              sendIRCode(NIKAI,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("nec")))                sendIRCode(NEC,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("panasonic")))          sendIRCode(PANASONIC,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("pioneer")))            sendIRCode(PIONEER,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("rc5x")))               sendIRCode(RC5X,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("rc5")))                sendIRCode(RC5,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("rc6")))                sendIRCode(RC6,IrCode,IrBits,IrRepeat);	
            if (IrType.equals(F("rcmm")))               sendIRCode(RCMM,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("samsung")))            sendIRCode(SAMSUNG,IrCode,IrBits,IrRepeat);
			      if (IrType.equals(F("samsung36")))          sendIRCode(SAMSUNG36,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("sanyo_lc7461")))       sendIRCode(SANYO_LC7461,IrCode,IrBits,IrRepeat);
			      if (IrType.equals(F("sharp")))              sendIRCode(SHARP,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("sherwood")))           sendIRCode(SHERWOOD,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("sony")))               sendIRCode(SONY,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("teco")))               sendIRCode(TECO,IrCode,IrBits,IrRepeat);
			      if (IrType.equals(F("vestel_ac")))          sendIRCode(VESTEL_AC,IrCode,IrBits,IrRepeat);
            if (IrType.equals(F("whynter")))            sendIRCode(WHYNTER,IrCode,IrBits,IrRepeat);

            //if (IrType.equals(F("raw")))              parseStringAndSendRaw(Plugin_035_irSender, code_str);   //too big String is needed for this, 
                                                                                                                //also conflicts with the keyword RAW (for the encoding) and RAW as in the library meanning of the timmings information. 
            if (IrType.equals(F("gc")))                 parseStringAndSendGC(ircodestr);                        //Needs testing
            if (IrType.equals(F("pronto")))             parseStringAndSendPronto(ircodestr, 0);                 //Needs testing
            if (IrType.equals(F("mitsubishi_ac")))      parseStringAndSendAirCon(MITSUBISHI_AC, ircodestr);
            if (IrType.equals(F("fujitsu_ac")))         parseStringAndSendAirCon(FUJITSU_AC, ircodestr);
            if (IrType.equals(F("kelvinator")))         parseStringAndSendAirCon(KELVINATOR, ircodestr);
            if (IrType.equals(F("daikin")))             parseStringAndSendAirCon(DAIKIN, ircodestr);
            if (IrType.equals(F("daikin2")))            parseStringAndSendAirCon(DAIKIN2, ircodestr);
            if (IrType.equals(F("gree")))               parseStringAndSendAirCon(GREE, ircodestr);	
            if (IrType.equals(F("argo")))               parseStringAndSendAirCon(ARGO, ircodestr);
            if (IrType.equals(F("trotec")))             parseStringAndSendAirCon(TROTEC, ircodestr);
            if (IrType.equals(F("toshiba_ac")))         parseStringAndSendAirCon(TOSHIBA_AC, ircodestr);
            if (IrType.equals(F("haier_ac")))           parseStringAndSendAirCon(HAIER_AC, ircodestr);
            if (IrType.equals(F("haier_ac_yrw02")))     parseStringAndSendAirCon(HAIER_AC_YRW02, ircodestr);
            if (IrType.equals(F("hitachi_ac")))         parseStringAndSendAirCon(HITACHI_AC, ircodestr);
            if (IrType.equals(F("hitachi_ac1")))        parseStringAndSendAirCon(HITACHI_AC1, ircodestr);
            if (IrType.equals(F("hitachi_ac2")))        parseStringAndSendAirCon(HITACHI_AC2, ircodestr);
            if (IrType.equals(F("electra_ac")))         parseStringAndSendAirCon(ELECTRA_AC, ircodestr);
            if (IrType.equals(F("panasonic_ac")))       parseStringAndSendAirCon(PANASONIC_AC, ircodestr);
            if (IrType.equals(F("samsung_ac")))         parseStringAndSendAirCon(SAMSUNG_AC, ircodestr);
            if (IrType.equals(F("whirlpool_ac")))       parseStringAndSendAirCon(WHIRLPOOL_AC, ircodestr);
            if (IrType.equals(F("mwm")))                parseStringAndSendAirCon(MWM, ircodestr);
            if (IrType.equals(F("tcl112ac")))           parseStringAndSendAirCon(TCL112AC, ircodestr);
          }

          addLog(LOG_LEVEL_INFO, String(F("IRTX: IR Code Sent: ")) + IrType_orig);
          if (printToWeb)
          {
            printWebString += String(F("IRTX: IR Code Sent: "))+ IrType_orig + String(F("<BR>"));
          }

          #ifdef PLUGIN_016
          if (irReceiver != 0) irReceiver->enableIRIn(); // Start the receiver
          #endif
        }
        break;
      }
  }
  return success;
}

boolean addErrorTrue() {
    addLog(LOG_LEVEL_ERROR, F("RAW2: Invalid encoding!"));
    return true;
}

// A lot of the following code has been taken directly (with permission) from the IRMQTTServer.ino example code
// of the IRremoteESP8266 library. (https://github.com/markszabo/IRremoteESP8266)



// Transmit the given IR message.
//
// Args:
//   irsend:   A pointer to a IRsend object to transmit via.
//   ir_type:  enum of the protocol to be sent.
//   code:     Numeric payload of the IR message. Most protocols use this.
//   code_str: The unparsed code to be sent. Used by complex protocol encodings.
//   bits:     Nr. of bits in the protocol. 0 means use the protocol's default.
//   repeat:   Nr. of times the message is to be repeated. (Not all protcols.)
// Returns:
//   bool: Successfully sent or not.
bool sendIRCode(int const ir_type,
                uint64_t const code, uint16_t bits,
                uint16_t repeat) {
  // Create a pseudo-lock so we don't try to send two codes at the same time.

  bool success = true;  // Assume success.

  // send the IR message.
  switch (ir_type) {
#if SEND_RC5
    case RC5:  // 1
      if (bits == 0)
        bits = kRC5Bits;
      Plugin_035_irSender->sendRC5(code, bits, repeat);
      break;
#endif
#if SEND_RC6
    case RC6:  // 2
      if (bits == 0)
        bits = kRC6Mode0Bits;
      Plugin_035_irSender->sendRC6(code, bits, repeat);
      break;
#endif
#if SEND_NEC
    case NEC:  // 3
      if (bits == 0)
        bits = kNECBits;
      Plugin_035_irSender->sendNEC(code, bits, repeat);
      break;
#endif
#if SEND_SONY
    case SONY:  // 4
      if (bits == 0)
        bits = kSony12Bits;
      repeat = std::max(repeat, kSonyMinRepeat);
      Plugin_035_irSender->sendSony(code, bits, repeat);
      break;
#endif
#if SEND_PANASONIC
    case PANASONIC:  // 5
      if (bits == 0)
        bits = kPanasonicBits;
      Plugin_035_irSender->sendPanasonic64(code, bits, repeat);
      break;
#endif
#if SEND_JVC
    case JVC:  // 6
      if (bits == 0)
        bits = kJvcBits;
      Plugin_035_irSender->sendJVC(code, bits, repeat);
      break;
#endif
#if SEND_SAMSUNG
    case SAMSUNG:  // 7
      if (bits == 0)
        bits = kSamsungBits;
      Plugin_035_irSender->sendSAMSUNG(code, bits, repeat);
      break;
#endif
#if SEND_SAMSUNG36
    case SAMSUNG36:  // 56
      if (bits == 0)
        bits = kSamsung36Bits;
      Plugin_035_irSender->sendSamsung36(code, bits, repeat);
      break;
#endif
#if SEND_WHYNTER
    case WHYNTER:  // 8
      if (bits == 0)
        bits = kWhynterBits;
      Plugin_035_irSender->sendWhynter(code, bits, repeat);
      break;
#endif
#if SEND_AIWA_RC_T501
    case AIWA_RC_T501:  // 9
      if (bits == 0)
        bits = kAiwaRcT501Bits;
      repeat = std::max(repeat, kAiwaRcT501MinRepeats);
      Plugin_035_irSender->sendAiwaRCT501(code, bits, repeat);
      break;
#endif
#if SEND_LG
    case LG:  // 10
      if (bits == 0)
        bits = kLgBits;
      Plugin_035_irSender->sendLG(code, bits, repeat);
      break;
#endif
#if SEND_MITSUBISHI
    case MITSUBISHI:  // 12
      if (bits == 0)
        bits = kMitsubishiBits;
      repeat = std::max(repeat, kMitsubishiMinRepeat);
      Plugin_035_irSender->sendMitsubishi(code, bits, repeat);
      break;
#endif
#if SEND_DISH
    case DISH:  // 13
      if (bits == 0)
        bits = kDishBits;
      repeat = std::max(repeat, kDishMinRepeat);
      Plugin_035_irSender->sendDISH(code, bits, repeat);
      break;
#endif
#if SEND_SHARP
    case SHARP:  // 14
      if (bits == 0)
        bits = kSharpBits;
      Plugin_035_irSender->sendSharpRaw(code, bits, repeat);
      break;
#endif
#if SEND_COOLIX
    case COOLIX:  // 15
      if (bits == 0)
        bits = kCoolixBits;
      Plugin_035_irSender->sendCOOLIX(code, bits, repeat);
      break;
#endif
#if SEND_DENON
    case DENON:  // 17
      if (bits == 0)
        bits = DENON_BITS;
      Plugin_035_irSender->sendDenon(code, bits, repeat);
      break;
#endif
#if SEND_SHERWOOD
    case SHERWOOD:  // 19
      if (bits == 0)
        bits = kSherwoodBits;
      repeat = std::max(repeat, kSherwoodMinRepeat);
      Plugin_035_irSender->sendSherwood(code, bits, repeat);
      break;
#endif
#if SEND_RCMM
    case RCMM:  // 21
      if (bits == 0)
        bits = kRCMMBits;
      Plugin_035_irSender->sendRCMM(code, bits, repeat);
      break;
#endif
#if SEND_SANYO
    case SANYO_LC7461:  // 22
      if (bits == 0)
        bits = kSanyoLC7461Bits;
      Plugin_035_irSender->sendSanyoLC7461(code, bits, repeat);
      break;
#endif
#if SEND_RC5
    case RC5X:  // 23
      if (bits == 0)
        bits = kRC5XBits;
      Plugin_035_irSender->sendRC5(code, bits, repeat);
      break;
#endif
#if SEND_NIKAI
    case NIKAI:  // 29
      if (bits == 0)
        bits = kNikaiBits;
      Plugin_035_irSender->sendNikai(code, bits, repeat);
      break;
#endif
#if SEND_MIDEA
    case MIDEA:  // 34
      if (bits == 0)
        bits = kMideaBits;
      Plugin_035_irSender->sendMidea(code, bits, repeat);
      break;
#endif
#if SEND_MAGIQUEST
    case MAGIQUEST:  // 35
      if (bits == 0)
        bits = kMagiquestBits;
      Plugin_035_irSender->sendMagiQuest(code, bits, repeat);
      break;
#endif
#if SEND_LASERTAG
    case LASERTAG:  // 36
      if (bits == 0)
        bits = kLasertagBits;
      Plugin_035_irSender->sendLasertag(code, bits, repeat);
      break;
#endif
#if SEND_CARRIER_AC
    case CARRIER_AC:  // 37
      if (bits == 0)
        bits = kCarrierAcBits;
      Plugin_035_irSender->sendCarrierAC(code, bits, repeat);
      break;
#endif
#if SEND_MITSUBISHI2
    case MITSUBISHI2:  // 39
      if (bits == 0)
        bits = kMitsubishiBits;
      repeat = std::max(repeat, kMitsubishiMinRepeat);
      Plugin_035_irSender->sendMitsubishi2(code, bits, repeat);
      break;
#endif
#if SEND_GICABLE
    case GICABLE:  // 43
      if (bits == 0)
        bits = kGicableBits;
      repeat = std::max(repeat, kGicableMinRepeat);
      Plugin_035_irSender->sendGICable(code, bits, repeat);
      break;
#endif
#if SEND_LUTRON
    case LUTRON:  // 47
      if (bits == 0)
        bits = kLutronBits;
      Plugin_035_irSender->sendLutron(code, bits, repeat);
      break;
#endif
#if SEND_PIONEER
    case PIONEER:  // 50
      if (bits == 0)
        bits = kPioneerBits;
      Plugin_035_irSender->sendPioneer(code, bits, repeat);
      break;
#endif
#if SEND_LG
    case LG2:  // 51
      if (bits == 0)
        bits = kLgBits;
      Plugin_035_irSender->sendLG2(code, bits, repeat);
      break;
#endif
#if SEND_VESTEL_AC
    case VESTEL_AC:  // 54
      if (bits == 0)
        bits = kVestelAcBits;
      Plugin_035_irSender->sendVestelAc(code, bits, repeat);
      break;
#endif
#if SEND_TECO
    case TECO:  // 55
      if (bits == 0)
        bits = kTecoBits;
      Plugin_035_irSender->sendTeco(code, bits, repeat);
      break;
#endif
#if SEND_LEGOPF
    case LEGOPF:  // 58
      if (bits == 0)
        bits = kLegoPfBits;
      Plugin_035_irSender->sendLegoPf(code, bits, repeat);
      break;
#endif
    default:
      // If we got here, we didn't know how to send it.
      success = false;
  }
  return success;
}


// Parse an Air Conditioner A/C Hex String/code and send it.
// Args:
//   irType: Nr. of the protocol we need to send.
//   str: A hexadecimal string containing the state to be sent.
bool  parseStringAndSendAirCon(const uint16_t irType, const String& str) {
 uint8_t strOffset = 0;
  uint8_t state[kStateSizeMax] = {0};  // All array elements are set to 0.
  uint16_t stateSize = 0;

  if (str.startsWith("0x") || str.startsWith("0X"))
    strOffset = 2;
  // Calculate how many hexadecimal characters there are.
  uint16_t inputLength = str.length() - strOffset;
  if (inputLength == 0) {
    //debug("Zero length AirCon code encountered. Ignored.");
    return false;  // No input. Abort.
  }

  switch (irType) {  // Get the correct state size for the protocol.
    case KELVINATOR:
      stateSize = kKelvinatorStateLength;
      break;
    case TOSHIBA_AC:
      stateSize = kToshibaACStateLength;
      break;
    case DAIKIN:
      stateSize = kDaikinStateLength;
      break;
    case DAIKIN2:
      stateSize = kDaikin2StateLength;
      break;
    case ELECTRA_AC:
      stateSize = kElectraAcStateLength;
      break;
    case MITSUBISHI_AC:
      stateSize = kMitsubishiACStateLength;
      break;
    case PANASONIC_AC:
      stateSize = kPanasonicAcStateLength;
      break;
    case TROTEC:
      stateSize = kTrotecStateLength;
      break;
    case ARGO:
      stateSize = kArgoStateLength;
      break;
    case GREE:
      stateSize = kGreeStateLength;
      break;
    case FUJITSU_AC:
      // Fujitsu has four distinct & different size states, so make a best guess
      // which one we are being presented with based on the number of
      // hexadecimal digits provided. i.e. Zero-pad if you need to to get
      // the correct length/byte size.
      stateSize = inputLength / 2;  // Every two hex chars is a byte.
      // Use at least the minimum size.
      stateSize = std::max(stateSize,
                           (uint16_t) (kFujitsuAcStateLengthShort - 1));
      // If we think it isn't a "short" message.
      if (stateSize > kFujitsuAcStateLengthShort)
        // Then it has to be at least the smaller version of the "normal" size.
        stateSize = std::max(stateSize, (uint16_t) (kFujitsuAcStateLength - 1));
      // Lastly, it should never exceed the maximum "normal" size.
      stateSize = std::min(stateSize, kFujitsuAcStateLength);
      break;
    case HAIER_AC:
      stateSize = kHaierACStateLength;
      break;
    case HAIER_AC_YRW02:
      stateSize = kHaierACYRW02StateLength;
      break;
    case HITACHI_AC:
      stateSize = kHitachiAcStateLength;
      break;
    case HITACHI_AC1:
      stateSize = kHitachiAc1StateLength;
      break;
    case HITACHI_AC2:
      stateSize = kHitachiAc2StateLength;
      break;
    case WHIRLPOOL_AC:
      stateSize = kWhirlpoolAcStateLength;
      break;
    case SAMSUNG_AC:
      // Samsung has two distinct & different size states, so make a best guess
      // which one we are being presented with based on the number of
      // hexadecimal digits provided. i.e. Zero-pad if you need to to get
      // the correct length/byte size.
      stateSize = inputLength / 2;  // Every two hex chars is a byte.
      // Use at least the minimum size.
      stateSize = std::max(stateSize, (uint16_t) (kSamsungAcStateLength));
      // If we think it isn't a "normal" message.
      if (stateSize > kSamsungAcStateLength)
        // Then it probably the extended size.
        stateSize = std::max(stateSize,
                             (uint16_t) (kSamsungAcExtendedStateLength));
      // Lastly, it should never exceed the maximum "extended" size.
      stateSize = std::min(stateSize, kSamsungAcExtendedStateLength);
      break;
    case MWM:
      // MWM has variable size states, so make a best guess
      // which one we are being presented with based on the number of
      // hexadecimal digits provided. i.e. Zero-pad if you need to to get
      // the correct length/byte size.
      stateSize = inputLength / 2;  // Every two hex chars is a byte.
      // Use at least the minimum size.
      stateSize = std::max(stateSize, (uint16_t) 3);
      // Cap the maximum size.
      stateSize = std::min(stateSize, kStateSizeMax);
      break;
    case TCL112AC:
      stateSize = kTcl112AcStateLength;
      break;
    default:  // Not a protocol we expected. Abort.
      //debug("Unexpected AirCon protocol detected. Ignoring.");
      return false;
  }
  if (inputLength > stateSize * 2) {
    //debug("AirCon code to large for the given protocol.");
    return false;
  }

  // Ptr to the least significant byte of the resulting state for this protocol.
  uint8_t *statePtr = &state[stateSize - 1];

  // Convert the string into a state array of the correct length.
  for (uint16_t i = 0; i < inputLength; i++) {
    // Grab the next least sigificant hexadecimal digit from the string.
    uint8_t c = tolower(str[inputLength + strOffset - i - 1]);
    if (isxdigit(c)) {
      if (isdigit(c))
        c -= '0';
      else
        c = c - 'a' + 10;
    } else {
      //debug("Aborting! Non-hexadecimal char found in AirCon state: " + str);
      return false;
    }
    if (i % 2 == 1) {  // Odd: Upper half of the byte.
      *statePtr += (c << 4);
      statePtr--;  // Advance up to the next least significant byte of state.
    } else {  // Even: Lower half of the byte.
      *statePtr = c;
    }
  }

  // Make the appropriate call for the protocol type.
  switch (irType) {
#if SEND_KELVINATOR
    case KELVINATOR:
      Plugin_035_irSender->sendKelvinator(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_TOSHIBA_AC
    case TOSHIBA_AC:
      Plugin_035_irSender->sendToshibaAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_DAIKIN
    case DAIKIN:
      Plugin_035_irSender->sendDaikin(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_DAIKIN2
    case DAIKIN2:
      Plugin_035_irSender->sendDaikin2(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_MITSUBISHI_AC
    case MITSUBISHI_AC:
      Plugin_035_irSender->sendMitsubishiAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_TROTEC
    case TROTEC:
      Plugin_035_irSender->sendTrotec(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_ARGO
    case ARGO:
      Plugin_035_irSender->sendArgo(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_GREE
    case GREE:
      Plugin_035_irSender->sendGree(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_FUJITSU_AC
    case FUJITSU_AC:
      Plugin_035_irSender->sendFujitsuAC(reinterpret_cast<uint8_t *>(state), stateSize);
      break;
#endif
#if SEND_HAIER_AC
    case HAIER_AC:
      Plugin_035_irSender->sendHaierAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_HAIER_AC_YRW02
    case HAIER_AC_YRW02:
      Plugin_035_irSender->sendHaierACYRW02(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_HITACHI_AC
    case HITACHI_AC:
      Plugin_035_irSender->sendHitachiAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_HITACHI_AC1
    case HITACHI_AC1:
      Plugin_035_irSender->sendHitachiAC1(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_HITACHI_AC2
    case HITACHI_AC2:
      Plugin_035_irSender->sendHitachiAC2(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_WHIRLPOOL_AC
    case WHIRLPOOL_AC:
      Plugin_035_irSender->sendWhirlpoolAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_SAMSUNG_AC
    case SAMSUNG_AC:
      Plugin_035_irSender->sendSamsungAC(reinterpret_cast<uint8_t *>(state), stateSize);
      break;
#endif
#if SEND_ELECTRA_AC
    case ELECTRA_AC:
      Plugin_035_irSender->sendElectraAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_PANASONIC_AC
    case PANASONIC_AC:
      Plugin_035_irSender->sendPanasonicAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_MWM
    case MWM:
      Plugin_035_irSender->sendMWM(reinterpret_cast<uint8_t *>(state), stateSize);
      break;
#endif
#if SEND_TCL112AC
    case TCL112AC:
      Plugin_035_irSender->sendTcl112Ac(reinterpret_cast<uint8_t *>(state));
      break;
#endif
    default:
      //debug("Unexpected AirCon type in send request. Not sent.");
      return false;
  }
  return true;  // We were successful as far as we can tell.
}


#if SEND_PRONTO
// Parse a Pronto Hex String/code and send it.
// Args:
//   str: A comma-separated String of nr. of repeats, then hexadecimal numbers.
//        e.g. "R1,0000,0067,0000,0015,0060,0018,0018,0018,0030,0018,0030,0018,
//              0030,0018,0018,0018,0030,0018,0018,0018,0018,0018,0030,0018,
//              0018,0018,0030,0018,0030,0018,0030,0018,0018,0018,0018,0018,
//              0030,0018,0018,0018,0018,0018,0030,0018,0018,03f6"
//              or
//              "0000,0067,0000,0015,0060,0018". i.e. without the Repeat value
//        Requires at least PRONTO_MIN_LENGTH comma-separated values.
//        sendPronto() only supports raw pronto code types, thus so does this.
//   repeats:  Nr. of times the message is to be repeated.
//             This value is ignored if an embeddd repeat is found in str.
void parseStringAndSendPronto(const String& str, uint16_t repeats) {
  uint16_t count;
  uint16_t *code_array;
  int16_t index = -1;
  uint16_t start_from = 0;

  // Find out how many items there are in the string.
  count = countValuesInStr(str, ',');

  // Check if we have the optional embedded repeats value in the code string.
  if (str.startsWith("R") || str.startsWith("r")) {
    // Grab the first value from the string, as it is the nr. of repeats.
    index = str.indexOf(',', start_from);
    repeats = str.substring(start_from + 1, index).toInt();  // Skip the 'R'.
    start_from = index + 1;
    count--;  // We don't count the repeats value as part of the code array.
  }

  // We need at least PRONTO_MIN_LENGTH values for the code part.
  if (count < PRONTO_MIN_LENGTH) return;

  // Now we know how many there are, allocate the memory to store them all.
  code_array =  reinterpret_cast<uint16_t*>(malloc(count * sizeof(uint16_t)));

  // Rest of the string are values for the code array.
  // Now convert the hex strings to integers and place them in code_array.
  count = 0;
  do {
    index = str.indexOf(',', start_from);
    // Convert the hexadecimal value string to an unsigned integer.
    code_array[count] = strtoul(str.substring(start_from, index).c_str(),
                                NULL, 16);
    start_from = index + 1;
    count++;
  } while (index != -1);

  Plugin_035_irSender->sendPronto(code_array, count, repeats);  // All done. Send it.
  free(code_array);  // Free up the memory allocated.
}
#endif  // SEND_PRONTO
//#if SEND_RAW
//// Parse an IRremote Raw Hex String/code and send it.
//// Args:
////   str: A comma-separated String containing the freq and raw IR data.
////        e.g. "38000,9000,4500,600,1450,600,900,650,1500,..."
////        Requires at least two comma-separated values.
////        First value is the transmission frequency in Hz or kHz.
//void parseStringAndSendRaw(const String& str) {
//  uint16_t count;
//  uint16_t freq = 38000;  // Default to 38kHz.
//  uint16_t *raw_array;
//
//  // Find out how many items there are in the string.
//  count = countValuesInStr(str, ',');
//
//  // We expect the frequency as the first comma separated value, so we need at
//  // least two values. If not, bail out.
//  if (count < 2) return;
//  count--;  // We don't count the frequency value as part of the raw array.
//
//  // Now we know how many there are, allocate the memory to store them all.
//  raw_array = newCodeArray(count);
//
//  // Grab the first value from the string, as it is the frequency.
//  int16_t index = str.indexOf(',', 0);
//  freq = str.substring(0, index).toInt();
//  uint16_t start_from = index + 1;
//  // Rest of the string are values for the raw array.
//  // Now convert the strings to integers and place them in raw_array.
//  count = 0;
//  do {
//    index = str.indexOf(',', start_from);
//    raw_array[count] = str.substring(start_from, index).toInt();
//    start_from = index + 1;
//    count++;
//  } while (index != -1);
//
//  Plugin_035_irSender->sendRaw(raw_array, count, freq);  // All done. Send it.
//  free(raw_array);  // Free up the memory allocated.
//}
//#endif  // SEND_RAW
#if SEND_GLOBALCACHE
// Parse a GlobalCache String/code and send it.
// Args:
//   str: A GlobalCache formatted String of comma separated numbers.
//        e.g. "38000,1,1,170,170,20,63,20,63,20,63,20,20,20,20,20,20,20,20,20,
//              20,20,63,20,63,20,63,20,20,20,20,20,20,20,20,20,20,20,20,20,63,
//              20,20,20,20,20,20,20,20,20,20,20,20,20,63,20,20,20,63,20,63,20,
//              63,20,63,20,63,20,63,20,1798"
//        Note: The leading "1:1,1," of normal GC codes should be removed.
void parseStringAndSendGC(const String& str) {
  uint16_t count;
  uint16_t *code_array;
  String tmp_str;

  // Remove the leading "1:1,1," if present.
  if (str.startsWith("1:1,1,"))
    tmp_str = str.substring(6);
  else
    tmp_str = str;

  // Find out how many items there are in the string.
  count = countValuesInStr(tmp_str, ',');

  // Now we know how many there are, allocate the memory to store them all.
  code_array = reinterpret_cast<uint16_t*>(malloc(count * sizeof(uint16_t)));

  // Now convert the strings to integers and place them in code_array.
  count = 0;
  uint16_t start_from = 0;
  int16_t index = -1;
  do {
    index = tmp_str.indexOf(',', start_from);
    code_array[count] = tmp_str.substring(start_from, index).toInt();
    start_from = index + 1;
    count++;
  } while (index != -1);

  Plugin_035_irSender->sendGC(code_array, count);  // All done. Send it.
  free(code_array);  // Free up the memory allocated.
}
#endif  // SEND_GLOBALCACHE

// Count how many values are in the String.
// Args:
//   str:  String containing the values.
//   sep:  Character that separates the values.
// Returns:
//   The number of values found in the String.
uint16_t countValuesInStr(const String& str, char sep) {
  int16_t index = -1;
  uint16_t count = 1;
  do {
    index = str.indexOf(sep, index + 1);
    count++;
  } while (index != -1);
  return count;
}
// Dynamically allocate an array of uint16_t's.
// Args:
//   size:  Nr. of uint16_t's need to be in the new array.
// Returns:
//   A Ptr to the new array. Restarts the ESP8266 if it fails.
//uint16_t * newCodeArray(const uint16_t size) {
//  uint16_t *result;
//
//  result = reinterpret_cast<uint16_t*>(malloc(size * sizeof(uint16_t)));
//  // Check we malloc'ed successfully.
//  if (result == NULL) {  // malloc failed, so give up.
//    Serial.printf("\nCan't allocate %d bytes. (%d bytes free)\n",
//                  size * sizeof(uint16_t), ESP.getFreeHeap());
//    serialPrintln("Giving up & forcing a reboot.");
//    ESP.restart();  // Reboot.
//    delay(500);  // Wait for the restart to happen.
//    return result;  // Should never get here, but just in case.
//  }
//  return result;
//}
#endif // USES_P035

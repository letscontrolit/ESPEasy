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
        addHtml(F("IRSEND,[PROTOCOL],[DATA],[BITS optional],[REPEATS optional]<BR>BITS and REPEATS are optional and default to 0"));

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

        if ((cmdCode.equalsIgnoreCase(F("IRSEND")) || cmdCode.equalsIgnoreCase(F("IRSENDAC"))) && Plugin_035_irSender != 0)
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
          } else if (cmdCode.equalsIgnoreCase(F("IRSEND"))) {
           uint16_t  IrRepeat=0;
          //  unsigned long IrSecondCode=0UL;
            String ircodestr;
            if (GetArgv(string.c_str(), TmpStr1, 2)) {
              IrType = TmpStr1;
              IrType_orig = TmpStr1;
              IrType.toUpperCase(); //strToDecodeType assumes all capital letters
            }
            if (GetArgv(string.c_str(), ircodestr, 3)) {
              IrCode = strtoull(ircodestr.c_str(), NULL, 16);
            }

            if (GetArgv(string.c_str(), TmpStr1, 4)) IrBits = str2int(TmpStr1.c_str());   //not needed any more as I can tell... Left for reverse compitability so we won't brake old forum posts etc..
            IrBits = 0;                                                                   //Leave it to 0 for default protocol bits
            if (GetArgv(string.c_str(), TmpStr1, 5)) IrRepeat = str2int(TmpStr1.c_str()); // Nr. of times the message is to be repeated

            sendIRCode(strToDecodeType(IrType.c_str()),IrCode,ircodestr.c_str(),IrBits,IrRepeat);

          }
           if (cmdCode.equalsIgnoreCase(F("IRSENDAC"))) { //Preliminary-test code for the standardized AC commands.
             
              // decode_type_t vendor, 
              // uint16_t model,          //The specific model of A/C if applicable.
              // bool on,                 //POWER ON or OFF
              // stdAc::opmode_t mode,    //What operating mode should the unit perform? e.g. Cool, Heat etc.
              // float degrees,           //What temperature should the unit be set to?
              // bool celsius,            //Use degreees Celsius, otherwise Fahrenheit. 
              // stdAc::fanspeed_t fan,   //Fan Speed setting 
              // stdAc::swingv_t swingv,  //Vertical swing setting 
              // stdAc::swingh_t swingh,  //Horizontal Swing setting 
              // bool quiet,              //Quiet setting ON or OFF
              // bool turbo,              //Turbo setting ON or OFF
              // bool econo,              //Economy setting ON or OFF
              // bool light,              //Light setting ON or OFF
              // bool filter,             //Filter setting ON or OFF
              // bool clean,              //Clean setting ON or OFF
              // bool beep,               //Beep setting ON or OFF
              // int16_t sleep = -1,      //Nr. of mins of sleep mode, or use sleep mode. (<= 0 means off.)
              // int16_t clock            //Nr. of mins past midnight to set the clock to. (< 0 means off.)

              //Plugin_035_irACSender = new IRac(irPin);
             // ac.sendAc(GREE,);
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
      } //PLUGIN_WRITE END
  } // SWITCH END
  return success;
} // Plugin_035 END

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
                uint64_t const code, char const * code_str, uint16_t bits,
                uint16_t repeat) {
  bool success = true;  // Assume success.
  IRsend *irsend = Plugin_035_irSender;

 // send the IR message.
  switch (ir_type) {
#if SEND_RC5
    case RC5:  // 1
      if (bits == 0)
        bits = kRC5Bits;
      irsend->sendRC5(code, bits, repeat);
      break;
#endif
#if SEND_RC6
    case RC6:  // 2
      if (bits == 0)
        bits = kRC6Mode0Bits;
      irsend->sendRC6(code, bits, repeat);
      break;
#endif
#if SEND_NEC
    case NEC:  // 3
      if (bits == 0)
        bits = kNECBits;
      irsend->sendNEC(code, bits, repeat);
      break;
#endif
#if SEND_SONY
    case SONY:  // 4
      if (bits == 0)
        bits = kSony12Bits;
      repeat = std::max(repeat, kSonyMinRepeat);
      irsend->sendSony(code, bits, repeat);
      break;
#endif
#if SEND_PANASONIC
    case PANASONIC:  // 5
      if (bits == 0)
        bits = kPanasonicBits;
      irsend->sendPanasonic64(code, bits, repeat);
      break;
#endif
#if SEND_JVC
    case JVC:  // 6
      if (bits == 0)
        bits = kJvcBits;
      irsend->sendJVC(code, bits, repeat);
      break;
#endif
#if SEND_SAMSUNG
    case SAMSUNG:  // 7
      if (bits == 0)
        bits = kSamsungBits;
      irsend->sendSAMSUNG(code, bits, repeat);
      break;
#endif
#if SEND_SAMSUNG36
    case SAMSUNG36:  // 56
      if (bits == 0)
        bits = kSamsung36Bits;
      irsend->sendSamsung36(code, bits, repeat);
      break;
#endif
#if SEND_WHYNTER
    case WHYNTER:  // 8
      if (bits == 0)
        bits = kWhynterBits;
      irsend->sendWhynter(code, bits, repeat);
      break;
#endif
#if SEND_AIWA_RC_T501
    case AIWA_RC_T501:  // 9
      if (bits == 0)
        bits = kAiwaRcT501Bits;
      repeat = std::max(repeat, kAiwaRcT501MinRepeats);
      irsend->sendAiwaRCT501(code, bits, repeat);
      break;
#endif
#if SEND_LG
    case LG:  // 10
      if (bits == 0)
        bits = kLgBits;
      irsend->sendLG(code, bits, repeat);
      break;
#endif
#if SEND_MITSUBISHI
    case MITSUBISHI:  // 12
      if (bits == 0)
        bits = kMitsubishiBits;
      repeat = std::max(repeat, kMitsubishiMinRepeat);
      irsend->sendMitsubishi(code, bits, repeat);
      break;
#endif
#if SEND_DISH
    case DISH:  // 13
      if (bits == 0)
        bits = kDishBits;
      repeat = std::max(repeat, kDishMinRepeat);
      irsend->sendDISH(code, bits, repeat);
      break;
#endif
#if SEND_SHARP
    case SHARP:  // 14
      if (bits == 0)
        bits = kSharpBits;
      irsend->sendSharpRaw(code, bits, repeat);
      break;
#endif
#if SEND_COOLIX
    case COOLIX:  // 15
      if (bits == 0)
        bits = kCoolixBits;
      irsend->sendCOOLIX(code, bits, repeat);
      break;
#endif
    case DAIKIN:  // 16
    case DAIKIN2:  // 53
    case KELVINATOR:  // 18
    case MITSUBISHI_AC:  // 20
    case GREE:  // 24
    case ARGO:  // 27
    case TROTEC:  // 28
    case TOSHIBA_AC:  // 32
    case FUJITSU_AC:  // 33
    case HAIER_AC:  // 38
    case HAIER_AC_YRW02:  // 44
    case HITACHI_AC:  // 40
    case HITACHI_AC1:  // 41
    case HITACHI_AC2:  // 42
    case WHIRLPOOL_AC:  // 45
    case SAMSUNG_AC:  // 46
    case ELECTRA_AC:  // 48
    case PANASONIC_AC:  // 49
    case MWM:  // 52
      success = parseStringAndSendAirCon(ir_type, code_str);
      break;
#if SEND_DENON
    case DENON:  // 17
      if (bits == 0)
        bits = DENON_BITS;
      irsend->sendDenon(code, bits, repeat);
      break;
#endif
#if SEND_SHERWOOD
    case SHERWOOD:  // 19
      if (bits == 0)
        bits = kSherwoodBits;
      repeat = std::max(repeat, kSherwoodMinRepeat);
      irsend->sendSherwood(code, bits, repeat);
      break;
#endif
#if SEND_RCMM
    case RCMM:  // 21
      if (bits == 0)
        bits = kRCMMBits;
      irsend->sendRCMM(code, bits, repeat);
      break;
#endif
#if SEND_SANYO
    case SANYO_LC7461:  // 22
      if (bits == 0)
        bits = kSanyoLC7461Bits;
      irsend->sendSanyoLC7461(code, bits, repeat);
      break;
#endif
#if SEND_RC5
    case RC5X:  // 23
      if (bits == 0)
        bits = kRC5XBits;
      irsend->sendRC5(code, bits, repeat);
      break;
#endif
#if SEND_PRONTO
    case PRONTO:  // 25
     // success = parseStringAndSendPronto(irsend, code_str, repeat);
      break;
#endif
#if SEND_NIKAI
    case NIKAI:  // 29
      if (bits == 0)
        bits = kNikaiBits;
      irsend->sendNikai(code, bits, repeat);
      break;
#endif
#if SEND_RAW
    case RAW:  // 30
      //success = parseStringAndSendRaw(irsend, code_str);
      break;
#endif
#if SEND_GLOBALCACHE
    case GLOBALCACHE:  // 31
     // success = parseStringAndSendGC(irsend, code_str);
      break;
#endif
#if SEND_MIDEA
    case MIDEA:  // 34
      if (bits == 0)
        bits = kMideaBits;
      irsend->sendMidea(code, bits, repeat);
      break;
#endif
#if SEND_MAGIQUEST
    case MAGIQUEST:  // 35
      if (bits == 0)
        bits = kMagiquestBits;
      irsend->sendMagiQuest(code, bits, repeat);
      break;
#endif
#if SEND_LASERTAG
    case LASERTAG:  // 36
      if (bits == 0)
        bits = kLasertagBits;
      irsend->sendLasertag(code, bits, repeat);
      break;
#endif
#if SEND_CARRIER_AC
    case CARRIER_AC:  // 37
      if (bits == 0)
        bits = kCarrierAcBits;
      irsend->sendCarrierAC(code, bits, repeat);
      break;
#endif
#if SEND_MITSUBISHI2
    case MITSUBISHI2:  // 39
      if (bits == 0)
        bits = kMitsubishiBits;
      repeat = std::max(repeat, kMitsubishiMinRepeat);
      irsend->sendMitsubishi2(code, bits, repeat);
      break;
#endif
#if SEND_GICABLE
    case GICABLE:  // 43
      if (bits == 0)
        bits = kGicableBits;
      repeat = std::max(repeat, kGicableMinRepeat);
      irsend->sendGICable(code, bits, repeat);
      break;
#endif
#if SEND_LUTRON
    case LUTRON:  // 47
      if (bits == 0)
        bits = kLutronBits;
      irsend->sendLutron(code, bits, repeat);
      break;
#endif
#if SEND_PIONEER
    case PIONEER:  // 50
      if (bits == 0)
        bits = kPioneerBits;
      irsend->sendPioneer(code, bits, repeat);
      break;
#endif
#if SEND_LG
    case LG2:  // 51
      if (bits == 0)
        bits = kLgBits;
      irsend->sendLG2(code, bits, repeat);
      break;
#endif
#if SEND_VESTEL_AC
    case VESTEL_AC:  // 54
      if (bits == 0)
        bits = kVestelAcBits;
      irsend->sendVestelAc(code, bits, repeat);
      break;
#endif
#if SEND_TECO
    case TECO:  // 55
      if (bits == 0)
        bits = kTecoBits;
      irsend->sendTeco(code, bits, repeat);
      break;
#endif
#if SEND_LEGOPF
    case LEGOPF:  // 58
      if (bits == 0)
        bits = kLegoPfBits;
      irsend->sendLegoPf(code, bits, repeat);
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
bool  parseStringAndSendAirCon(const uint16_t irType, const String str) {
IRsend *irsend = Plugin_035_irSender;
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
    case MITSUBISHI_HEAVY_88:
      stateSize = kMitsubishiHeavy88StateLength;
      break;
    case MITSUBISHI_HEAVY_152:
      stateSize = kMitsubishiHeavy152StateLength;
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
     // debug("Aborting! Non-hexadecimal char found in AirCon state:");
     // debug(str.c_str());
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
      irsend->sendKelvinator(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_TOSHIBA_AC
    case TOSHIBA_AC:
      irsend->sendToshibaAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_DAIKIN
    case DAIKIN:
      irsend->sendDaikin(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_DAIKIN2
    case DAIKIN2:
      irsend->sendDaikin2(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_MITSUBISHI_AC
    case MITSUBISHI_AC:
      irsend->sendMitsubishiAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_MITSUBISHIHEAVY
    case MITSUBISHI_HEAVY_88:  // 59
      irsend->sendMitsubishiHeavy88(reinterpret_cast<uint8_t *>(state));
      break;
    case MITSUBISHI_HEAVY_152:  // 60
      irsend->sendMitsubishiHeavy152(reinterpret_cast<uint8_t *>(state));
      break;
#endif  // SEND_MITSUBISHIHEAVY
#if SEND_TROTEC
    case TROTEC:
      irsend->sendTrotec(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_ARGO
    case ARGO:
      irsend->sendArgo(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_GREE
    case GREE:
      irsend->sendGree(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_FUJITSU_AC
    case FUJITSU_AC:
      irsend->sendFujitsuAC(reinterpret_cast<uint8_t *>(state), stateSize);
      break;
#endif
#if SEND_HAIER_AC
    case HAIER_AC:
      irsend->sendHaierAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_HAIER_AC_YRW02
    case HAIER_AC_YRW02:
      irsend->sendHaierACYRW02(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_HITACHI_AC
    case HITACHI_AC:
      irsend->sendHitachiAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_HITACHI_AC1
    case HITACHI_AC1:
      irsend->sendHitachiAC1(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_HITACHI_AC2
    case HITACHI_AC2:
      irsend->sendHitachiAC2(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_WHIRLPOOL_AC
    case WHIRLPOOL_AC:
      irsend->sendWhirlpoolAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_SAMSUNG_AC
    case SAMSUNG_AC:
      irsend->sendSamsungAC(reinterpret_cast<uint8_t *>(state), stateSize);
      break;
#endif
#if SEND_ELECTRA_AC
    case ELECTRA_AC:
      irsend->sendElectraAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_PANASONIC_AC
    case PANASONIC_AC:
      irsend->sendPanasonicAC(reinterpret_cast<uint8_t *>(state));
      break;
#endif
#if SEND_MWM
    case MWM:
      irsend->sendMWM(reinterpret_cast<uint8_t *>(state), stateSize);
      break;
#endif
#if SEND_TCL112AC
    case TCL112AC:
      irsend->sendTcl112Ac(reinterpret_cast<uint8_t *>(state));
      break;
#endif
    default:
      //debug("Unexpected AirCon type in send request. Not sent.");
      return false;
  }
  return true;  // We were successful as far as we can tell.
}

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

#ifdef USES_P035
//#######################################################################################################
//#################################### Plugin 035: Output IR ############################################
//#######################################################################################################

#ifdef ESP8266  // Needed for precompile issues.
#include <IRremoteESP8266.h>
#endif
#include <IRsend.h>
#include <IRutils.h>

IRsend *Plugin_035_irSender;


#define PLUGIN_035
#define PLUGIN_ID_035         35
#define PLUGIN_NAME_035       "Communication - IR Transmit"
#define STATE_SIZE_MAX        53U
#define PRONTO_MIN_LENGTH     6U

#define from_32hex(c) ((((c) | ('A' ^ 'a')) - '0') % 39)

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

    case PLUGIN_INIT:
      {
        int irPin = Settings.TaskDevicePin1[event->TaskIndex];
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
        unsigned int IrBits=0;
        //char log[120];

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

            if (GetArgv(string.c_str(), TmpStr1, 3)) IrRaw  = TmpStr1;
            if (GetArgv(string.c_str(), TmpStr1, 4)) IrHz   = str2int(TmpStr1.c_str());
            if (GetArgv(string.c_str(), TmpStr1, 5)) IrPLen = str2int(TmpStr1.c_str());
            if (GetArgv(string.c_str(), TmpStr1, 6)) IrBLen = str2int(TmpStr1.c_str());

            printWebString += F("<a href='https://en.wikipedia.org/wiki/Base32#base32hex'>Base32Hex</a> RAW Code: ");
            printWebString += IrRaw;
            printWebString += F("<BR>");

            printWebString += F("kHz: ");
            printWebString += IrHz;
            printWebString += F("<BR>");

            printWebString += F("Pulse Len: ");
            printWebString += IrPLen;
            printWebString += F("<BR>");

            printWebString += F("Blank Len: ");
            printWebString += IrBLen;
            printWebString += F("<BR>");

            uint16_t buf[200];
            uint16_t idx = 0;
            if (IrType.equals(F("raw"))) {
                unsigned int c0 = 0; //count consecutives 0s
                unsigned int c1 = 0; //count consecutives 1s

                printWebString += F("Interpreted RAW Code: ");
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
                        for (uint t = 0; t < c0; t++)
                          printWebString += '0';
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
                          for (uint t = 0; t < c1; t++)
                            printWebString += '1';
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
                  for (uint t = 0; t < c0; t++)
                    printWebString += '0';
                }
                //If we have pendings 1s
                if (c1 > 0) {
                  buf[idx++] = c1 * IrPLen;
                  for (uint t = 0; t < c1; t++)
                    printWebString += '1';
                }

                printWebString += F("<BR>");

            } else {        // RAW2
                for (unsigned int i = 0, total = IrRaw.length(), gotRep = 0, rep = 0; i < total;) {
                   char c = IrRaw[i++];
                   if (c == '*') {
                       if (i+2 >= total || idx + (rep = from_32hex(IrRaw[i++])) * 2 > sizeof(buf))
                           return addErrorTrue("Invalid RAW2 B32 encoding!");
                       gotRep = 2;
                   } else {
                       if ((c == '^' && i+1 >= total) || idx == sizeof(buf))
                           return addErrorTrue("Invalid RAW2 B32 encoding!");

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
            }

            Plugin_035_irSender->sendRaw(buf, idx, IrHz);
            //String line = "";
            //for (int i = 0; i < idx; i++)
            //    line += uint64ToString(buf[i], 10) + ",";
            //serialPrintln(line);

            //sprintf_P(log, PSTR("IR Params1: Hz:%u - PLen: %u - BLen: %u"), IrHz, IrPLen, IrBLen);
            //addLog(LOG_LEVEL_INFO, log);
            //sprintf_P(log, PSTR("IR Params2: RAW Code:%s"), IrRaw.c_str());
            //addLog(LOG_LEVEL_INFO, log);
          } else {
           // unsigned int IrRepeat=0;
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
            //if (GetArgv(string.c_str(), TmpStr1, 4)) IrBits = str2int(TmpStr1); //not needed any more... leave it for reverce compatibility or remove it and break existing instalations?
            //if (GetArgv(string.c_str(), TmpStr1, 5)) IrRepeat = str2int(TmpStr1); // Ir repeat is usfull in some circonstances, have to see how to add it and have it be revese compatible as well.
            //if (GetArgv(string.c_str(), TmpStr1, 6)) IrSecondCode = strtoul(TmpStr1, NULL, 16);

            //Comented out need char[] for input Needs fixing
            if (IrType.equals(F("nec")))                Plugin_035_irSender->sendNEC(IrCode);
            if (IrType.equals(F("sony")))               Plugin_035_irSender->sendSony(IrCode);
            if (IrType.equals(F("sherwood")))           Plugin_035_irSender->sendSherwood(IrCode);
            if (IrType.equals(F("samsung")))            Plugin_035_irSender->sendSAMSUNG(IrCode);
            if (IrType.equals(F("lg")))                 Plugin_035_irSender->sendLG(IrCode);
            if (IrType.equals(F("lg2")))                Plugin_035_irSender->sendLG2(IrCode);
            if (IrType.equals(F("sharpraw")))           Plugin_035_irSender->sendSharpRaw(IrBits);
            if (IrType.equals(F("jvc")))                Plugin_035_irSender->sendJVC(IrCode);
            if (IrType.equals(F("denon")))              Plugin_035_irSender->sendDenon(IrCode);
            if (IrType.equals(F("sanyolc7461")))        Plugin_035_irSender->sendSanyoLC7461(IrCode);
            if (IrType.equals(F("dish")))               Plugin_035_irSender->sendDISH(IrCode);
            if (IrType.equals(F("panasonic64")))        Plugin_035_irSender->sendPanasonic64(IrCode);
            if (IrType.equals(F("panasonic")))          Plugin_035_irSender->sendPanasonic64(IrCode);
            if (IrType.equals(F("rc5")))                Plugin_035_irSender->sendRC5(IrCode);
            if (IrType.equals(F("rc5x")))               Plugin_035_irSender->sendRC5(IrCode);
            if (IrType.equals(F("rc6")))                Plugin_035_irSender->sendRC6(IrCode);
            if (IrType.equals(F("rcmm")))               Plugin_035_irSender->sendRCMM(IrCode);
            if (IrType.equals(F("coolix")))             Plugin_035_irSender->sendCOOLIX(IrCode);
            if (IrType.equals(F("whynter")))            Plugin_035_irSender->sendWhynter(IrCode);
            if (IrType.equals(F("mitsubishi")))         Plugin_035_irSender->sendMitsubishi(IrCode);
            if (IrType.equals(F("mitsubishi2")))        Plugin_035_irSender->sendMitsubishi2(IrCode);
            if (IrType.equals(F("gc")))                 parseStringAndSendGC(ircodestr);              //Needs testing
            if (IrType.equals(F("aiwa_rc_t501")))       Plugin_035_irSender->sendAiwaRCT501(IrCode);
            if (IrType.equals(F("pronto")))             parseStringAndSendPronto(ircodestr, 0);       //Needs testing
            if (IrType.equals(F("nikai")))              Plugin_035_irSender->sendNikai(IrCode);
            if (IrType.equals(F("midea")))              Plugin_035_irSender->sendMidea(IrCode);
            if (IrType.equals(F("magiquest")))          Plugin_035_irSender->sendMagiQuest(IrCode);
            if (IrType.equals(F("lasertag")))           Plugin_035_irSender->sendLasertag(IrCode);
            if (IrType.equals(F("carrier_ac")))         Plugin_035_irSender->sendCarrierAC(IrCode);
            if (IrType.equals(F("gicable")))            Plugin_035_irSender->sendGICable(IrCode);
            if (IrType.equals(F("pioneer")))            Plugin_035_irSender->sendPioneer(IrCode);
            if (IrType.equals(F("lutron")))             Plugin_035_irSender->sendLutron(IrCode);

            if (IrType.equals(F("mitsubishi_ac")))      parseStringAndSendAirCon(MITSUBISHI_AC, ircodestr);
            if (IrType.equals(F("fujitsu_ac")))         parseStringAndSendAirCon(FUJITSU_AC, ircodestr);
            if (IrType.equals(F("kelvinator")))         parseStringAndSendAirCon(KELVINATOR, ircodestr);
            if (IrType.equals(F("daikin")))             parseStringAndSendAirCon(DAIKIN, ircodestr);
            if (IrType.equals(F("gree")))               parseStringAndSendAirCon(GREE, ircodestr);
            if (IrType.equals(F("argo")))               parseStringAndSendAirCon(ARGO, ircodestr);
            if (IrType.equals(F("trotec")))             parseStringAndSendAirCon(TROTEC, ircodestr);
            if (IrType.equals(F("toshiba_ac")))         parseStringAndSendAirCon(TOSHIBA_AC, ircodestr);
            if (IrType.equals(F("haier_ac")))           parseStringAndSendAirCon(HAIER_AC, ircodestr);
            if (IrType.equals(F("hitachi_ac")))         parseStringAndSendAirCon(HITACHI_AC, ircodestr);
            if (IrType.equals(F("hitachi_ac1")))        parseStringAndSendAirCon(HITACHI_AC1, ircodestr);
            if (IrType.equals(F("hitachi_ac2")))        parseStringAndSendAirCon(HITACHI_AC2, ircodestr);
            if (IrType.equals(F("electra_ac")))         parseStringAndSendAirCon(ELECTRA_AC, ircodestr);
            if (IrType.equals(F("panasonic_ac")))       parseStringAndSendAirCon(PANASONIC_AC, ircodestr);
            if (IrType.equals(F("haier_ac_yrw02")))     parseStringAndSendAirCon(HAIER_AC_YRW02, ircodestr);
            if (IrType.equals(F("samsung_ac")))         parseStringAndSendAirCon(SAMSUNG_AC, ircodestr);
            if (IrType.equals(F("whirlpool_ac")))       parseStringAndSendAirCon(WHIRLPOOL_AC, ircodestr);
            if (IrType.equals(F("mwm")))                parseStringAndSendAirCon(MWM, ircodestr);
           // NEC (non-strict)?
          }

          addLog(LOG_LEVEL_INFO, (String("IRTX :IR Code Sent: ") + IrType).c_str());
          if (printToWeb)
          {
            printWebString += F("IR Code Sent ");
            printWebString += IrType_orig;
            printWebString += F("<BR>");
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

boolean addErrorTrue(const char *str) {
    addLog(LOG_LEVEL_ERROR, str);
    return true;
}

// A lot of the following code has been taken directly (with permission) from the IRMQTTServer.ino example code
// of the IRremoteESP8266 library. (https://github.com/markszabo/IRremoteESP8266)
// Parse an Air Conditioner A/C Hex String/code and send it.
// Args:
//   irType: Nr. of the protocol we need to send.
//   str: A hexadecimal string containing the state to be sent.
bool  parseStringAndSendAirCon(const uint16_t irType, const String& str) {
  uint8_t strOffset = 0;
  uint8_t state[STATE_SIZE_MAX] = {0};  // All array elements are set to 0.
  uint16_t stateSize = 0;

  if (str.startsWith("0x") || str.startsWith("0X"))
    strOffset = 2;
  // Calculate how many hexadecimal characters there are.
  uint16_t inputLength = str.length() - strOffset;
  if (inputLength == 0) {
 //   debug("Zero length AirCon code encountered. Ignored.");
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
    default:  // Not a protocol we expected. Abort.
   //   debug("Unexpected AirCon protocol detected. Ignoring.");
      return false;
  }
  if (inputLength > stateSize * 2) {
   // debug("AirCon code to large for the given protocol.");
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
    //  debug("Aborting! Non-hexadecimal char found in AirCon state: " + str);
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
#if MITSUBISHI_AC
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

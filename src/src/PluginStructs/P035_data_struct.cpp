#include "../PluginStructs/P035_data_struct.h"

#ifdef USES_P035

// **************************************************************************/
// Constructor
// **************************************************************************/
P035_data_struct::P035_data_struct(int8_t gpioPin)
  : _gpioPin(gpioPin) {}

// **************************************************************************/
// Destructor
// **************************************************************************/
P035_data_struct::~P035_data_struct() {
  if (Plugin_035_irSender != nullptr) {
    delete Plugin_035_irSender;
    Plugin_035_irSender = nullptr;
  }
  # ifdef P016_P035_Extended_AC

  if (Plugin_035_commonAc != nullptr) {
    delete Plugin_035_commonAc;
    Plugin_035_commonAc = nullptr;
  }
  # endif // ifdef P016_P035_Extended_AC
}

bool P035_data_struct::plugin_init(struct EventStruct *event) {
  bool success = false;

  if ((Plugin_035_irSender == nullptr) && validGpio(_gpioPin)) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, F("INIT: IR TX"));
      addLog(LOG_LEVEL_INFO, F("IR lib Version: " _IRREMOTEESP8266_VERSION_STR));
      # ifdef P035_DEBUG_LOG
      addLog(LOG_LEVEL_INFO, String(F("Supported Protocols by IRSEND: ")) + listProtocols());
      # endif // ifdef P035_DEBUG_LOG
    }
    Plugin_035_irSender = new (std::nothrow) IRsend(_gpioPin);

    if (Plugin_035_irSender != nullptr) {
      Plugin_035_irSender->begin(); // Start the sender
      success = true;
    }
  }

  // if ((Plugin_035_irSender != nullptr) && (_gpioPin == -1)) { // This can never be true because of the validGpio() check above
  //   addLog(LOG_LEVEL_INFO, F("INIT: IR TX Removed"));
  //   delete Plugin_035_irSender;
  //   Plugin_035_irSender = nullptr;
  //   success             = false;
  // }

  # ifdef P016_P035_Extended_AC

  if (success && (Plugin_035_commonAc == nullptr) && validGpio(_gpioPin)) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, F("INIT AC: IR TX"));
      #  ifdef P035_DEBUG_LOG
      addLog(LOG_LEVEL_INFO, String(F("Supported Protocols by IRSENDAC: ")) + listACProtocols());
      #  endif // ifdef P035_DEBUG_LOG
    }
    Plugin_035_commonAc = new (std::nothrow) IRac(_gpioPin);
  }

  // if ((Plugin_035_commonAc != nullptr) && (_gpioPin == -1)) { // This can never be true because of the validGpio() check above
  //   addLog(LOG_LEVEL_INFO, F("INIT AC: IR TX Removed"));
  //   delete Plugin_035_commonAc;
  //   Plugin_035_commonAc = nullptr;
  //   success             = false;
  // }
  # endif // ifdef P016_P035_Extended_AC
  return success;
}

bool P035_data_struct::plugin_exit(struct EventStruct *event) {
  if (Plugin_035_irSender != nullptr) {
    delete Plugin_035_irSender;
    Plugin_035_irSender = nullptr;
  }
  # ifdef P016_P035_Extended_AC

  if (Plugin_035_commonAc != nullptr) {
    delete Plugin_035_commonAc;
    Plugin_035_commonAc = nullptr;
  }
  # endif // ifdef P016_P035_Extended_AC
  return true;
}

bool P035_data_struct::plugin_write(struct EventStruct *event, const String& string) {
  bool success = false;

  String cmdCode = parseString(string, 1);

  if (cmdCode.equalsIgnoreCase(F("IRSEND")) && (Plugin_035_irSender != nullptr)) {
    success = true;
    enableIR_RX(false);

    # ifdef P016_P035_USE_RAW_RAW2
    handleRawRaw2Encoding(string);
    # endif // P016_P035_USE_RAW_RAW2

    handleIRremote(string);
  }

  # ifdef P016_P035_Extended_AC
  else if (cmdCode.equalsIgnoreCase(F("IRSENDAC")) && (Plugin_035_commonAc != nullptr)) {
    success = true;
    enableIR_RX(false);
    handle_AC_IRremote(parseStringToEnd(string, 2));
  }
  # endif // P016_P035_Extended_AC

  enableIR_RX(true);

  return success;
}

bool P035_data_struct::handleIRremote(const String& cmd) {
  String IrType;
  String TmpStr1;

  uint64_t IrCode   = 0;
  uint16_t IrBits   = 0;
  uint16_t IrRepeat = 0;
  String   ircodestr;

  StaticJsonDocument<200> docTemp;
  DeserializationError    error = deserializeJson(docTemp, parseStringToEnd(cmd, 2));

  if (!error) {                                  // If the command is in JSON format
    IrType    =  docTemp[F("protocol")].as<String>();
    ircodestr = docTemp[F("data")].as<String>(); // JSON does not support hex values, thus we use command representation
    IrCode    = strtoull(ircodestr.c_str(), nullptr, 16);
    IrBits    = docTemp[F("bits")] | 0;
    IrRepeat  = docTemp[F("repeats")] | 0;
  } else { // If the command is NOT in JSON format (legacy)
    IrType = parseString(cmd, 2);

    if (IrType.length() > 0) {
      ircodestr = parseString(cmd, 3);

      if (ircodestr.length() > 0) {
        IrCode = strtoull(ircodestr.c_str(), nullptr, 16);
      }
      IrBits   = parseString(cmd, 4).toInt(); // Number of bits to be sent. USE 0 for default protocol bits
      IrRepeat = parseString(cmd, 5).toInt(); // Nr. of times the message is to be repeated
    }
  }

  bool IRsent = sendIRCode(strToDecodeType(IrType.c_str()), IrCode, ircodestr.c_str(), IrBits, IrRepeat); // Send the IR command

  if (IRsent) {
    printToLog(IrType, ircodestr, IrBits, IrRepeat);
  }
  return IRsent;
}

# ifdef P016_P035_Extended_AC
bool P035_data_struct::handle_AC_IRremote(const String& irData) {
  StaticJsonDocument<JSON_OBJECT_SIZE(18) + 190> doc;
  DeserializationError error = deserializeJson(doc, irData); // Deserialize the JSON document

  if (error) {                                               // Test if parsing succeeds.
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, String(F("IRTX: Deserialize Json failed: ")) + error.c_str());
    }
    return false; // do not continue with sending the signal.
  }
  String sprotocol = doc[F("protocol")];

  st.protocol = strToDecodeType(sprotocol.c_str());

  if (!IRac::isProtocolSupported(st.protocol)) { // Check if we support the protocol
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, String(F("IRTX: Protocol not supported:")) + sprotocol);
    }
    return false; // do not continue with sending of the signal.
  }

  String tempstr;

  tempstr  = doc[F("model")].as<String>();
  st.model = IRac::strToModel(tempstr.c_str(), -1);                             // The specific model of A/C if applicable. //strToModel();.
                                                                                // Defaults to -1 (unknown) if missing from JSON
  tempstr    = doc[F("power")].as<String>();
  st.power   = IRac::strToBool(tempstr.c_str(), false);                         // POWER ON or OFF. Defaults to false if missing from JSON
  st.degrees = doc[F("temp")] | 22.0f;                                          // What temperature should the unit be set to?. Defaults to
                                                                                // 22c if missing from JSON
  tempstr    = doc[F("use_celsius")].as<String>();
  st.celsius = IRac::strToBool(tempstr.c_str(), true);                          // Use degreees Celsius, otherwise Fahrenheit. Defaults to
                                                                                // true if missing from JSON
  tempstr = doc[F("mode")].as<String>();
  st.mode = IRac::strToOpmode(tempstr.c_str(), stdAc::opmode_t::kAuto);         // What operating mode should the unit perform? e.g. Cool.
                                                                                // Defaults to auto if missing from JSON
  tempstr     = doc[F("fanspeed")].as<String>();
  st.fanspeed = IRac::strToFanspeed(tempstr.c_str(), stdAc::fanspeed_t::kAuto); // Fan Speed setting. Defaults to auto if missing from JSON
  tempstr     = doc[F("swingv")].as<String>();
  st.swingv   = IRac::strToSwingV(tempstr.c_str(), stdAc::swingv_t::kAuto);     // Vertical swing setting. Defaults to auto if missing from
                                                                                // JSON
  tempstr   = doc[F("swingh")].as<String>();
  st.swingh = IRac::strToSwingH(tempstr.c_str(), stdAc::swingh_t::kAuto);       // Horizontal Swing setting. Defaults to auto if missing
                                                                                // from JSON
  tempstr  = doc[F("quiet")].as<String>();
  st.quiet = IRac::strToBool(tempstr.c_str(), false);                           // Quiet setting ON or OFF. Defaults to false if missing
                                                                                // from JSON
  tempstr  = doc[F("turbo")].as<String>();
  st.turbo = IRac::strToBool(tempstr.c_str(), false);                           // Turbo setting ON or OFF. Defaults to false if missing
                                                                                // from JSON
  tempstr  = doc[F("econo")].as<String>();
  st.econo = IRac::strToBool(tempstr.c_str(), false);                           // Economy setting ON or OFF. Defaults to false if missing
                                                                                // from JSON
  tempstr  = doc[F("light")].as<String>();
  st.light = IRac::strToBool(tempstr.c_str(), true);                            // Light setting ON or OFF. Defaults to true if missing from
                                                                                // JSON
  tempstr   = doc[F("filter")].as<String>();
  st.filter = IRac::strToBool(tempstr.c_str(), false);                          // Filter setting ON or OFF. Defaults to false if missing
                                                                                // from JSON
  tempstr  = doc[F("clean")].as<String>();
  st.clean = IRac::strToBool(tempstr.c_str(), false);                           // Clean setting ON or OFF. Defaults to false if missing
                                                                                // from JSON
  tempstr = doc[F("beep")].as<String>();
  st.beep = IRac::strToBool(tempstr.c_str(), false);                            // Beep setting ON or OFF. Defaults to false if missing from
                                                                                // JSON
  st.sleep = doc[F("sleep")] | -1;                                              // Nr. of mins of sleep mode, or use sleep mode. (<= 0 means
                                                                                // off.). Defaults to -1 if missing from JSON
  st.clock = doc[F("clock")] | -1;                                              // Nr. of mins past midnight to set the clock to. (< 0 means
                                                                                // off.). Defaults to -1 if missing from JSON

  // Send the IR command
  bool IRsent = Plugin_035_commonAc->sendAc(st, &prev);

  if (IRsent) {
    printToLog(typeToString(st.protocol), irData, 0, 0);
  }
  return IRsent;
}

# endif // ifdef P016_P035_Extended_AC


bool P035_data_struct::handleRawRaw2Encoding(const String& cmd) {
  bool   raw    = true;
  String IrType = parseString(cmd, 2);

  if (IrType.isEmpty()) { return false; }

  if (IrType.equalsIgnoreCase(F("RAW"))) {
    raw = true;
  } else if (IrType.equalsIgnoreCase(F("RAW2"))) {
    raw = false;
  }

  String   IrRaw      = parseString(cmd, 3);         // Get the "Base32" encoded/compressed Ir signal
  uint16_t IrHz       = parseString(cmd, 4).toInt(); // Get the base freguency of the signal (allways 38)
  unsigned int IrPLen = parseString(cmd, 5).toInt(); // Get the Pulse Length in ms
  unsigned int IrBLen = parseString(cmd, 6).toInt(); // Get the Blank Pulse Length in ms

  uint16_t  idx = 0;                                 // If this goes above the buf.size then the esp will throw a 28 EXCCAUSE
  uint16_t *buf;

  buf = new (std::nothrow) uint16_t[P35_Ntimings];   // The Raw Timings that we can buffer.

  if (buf == nullptr) {                              // error assigning memory.
    return false;
  }


  if (raw) {
    unsigned int c0 = 0; // count consecutives 0s
    unsigned int c1 = 0; // count consecutives 1s

    // printWebString += F("Interpreted RAW Code: ");  //print the number of 1s and 0s just for debugging/info purposes
    // Loop throught every char in RAW string
    for (unsigned int i = 0; i < IrRaw.length(); i++) {
      // Get the decimal value from base32 table
      // See: https://en.wikipedia.org/wiki/Base32#base32hex
      char c = from_32hex(IrRaw[i]);

      // Loop through 5 LSB (bits 16, 8, 4, 2, 1)
      for (unsigned int shft = 1; shft < 6; shft++) {
        // if bit is 1 (5th position - 00010000 = 16)
        if ((c & 16) != 0) {
          // add 1 to counter c1
          c1++;

          // if we already have any 0s in counting (the previous
          // bit was 0)
          if (c0 > 0) {
            // add the total ms into the buffer (number of 0s multiplied
            // by defined blank length ms)
            buf[idx++] = c0 * IrBLen;

            // print the number of 0s just for debuging/info purpouses
            // for (uint t = 0; t < c0; t++)
            // printWebString += '0';
          }

          // So, as we receive a "1", and processed the counted 0s
          // sending them as a ms timing into the buffer, we clear
          // the 0s counter
          c0 = 0;
        } else {
          // So, bit is 0

          // On first call, ignore 0s (suppress left-most 0s)
          if (c0 + c1 != 0) {
            // add 1 to counter c0
            c0++;

            // if we already have any 1s in counting (the previous
            // bit was 1)
            if (c1 > 0) {
              // add the total ms into the buffer (number of 1s
              // multiplied by defined pulse length ms)
              buf[idx++] = c1 * IrPLen;

              // print the number of 1s just for debugging/info purposes
              //                          for (uint t = 0; t < c1; t++)
              //                            printWebString += '1';
            }

            // So, as we receive a "0", and processed the counted 1s
            // sending them as a ms timing into the buffer, we clear
            // the 1s counter
            c1 = 0;
          }
        }

        // shift to left the "c" variable to process the next bit that is
        // in 5th position (00010000 = 16)
        c <<= 1;
      }
    }

    // Finally, we need to process the last counted bit that we were
    // processing

    // If we have pendings 0s
    if (c0 > 0) {
      buf[idx++] = c0 * IrBLen;

      // for (uint t = 0; t < c0; t++)
      // printWebString += '0';
    }

    // If we have pendings 1s
    if (c1 > 0) {
      buf[idx++] = c1 * IrPLen;

      // for (uint t = 0; t < c1; t++)
      // printWebString += '1';
    }

    // printWebString += F("<BR>");
  }

  if (!raw) { // RAW2
    for (unsigned int i = 0, total = IrRaw.length(), gotRep = 0, rep = 0; i < total;) {
      char c = IrRaw[i++];

      if (c == '*') {
        if (((i + 2) >= total) || ((idx + (rep = from_32hex(IrRaw[i++])) * 2) > (sizeof(buf[0]) * P35_Ntimings))) {
          delete[] buf;
          buf = nullptr;
          return addErrorTrue();
        }
        gotRep = 2;
      } else {
        if (((c == '^') && ((i + 1u) >= total)) || ((idx + 2u) >= (sizeof(buf[0]) * P35_Ntimings))) {
          delete[] buf;
          buf = nullptr;
          return addErrorTrue();
        }

        uint16_t irLen = (idx & 1) ? IrBLen : IrPLen;

        if (c == '^') {
          buf[idx++] = (from_32hex(IrRaw[i]) * 32 + from_32hex(IrRaw[i + 1])) * irLen;
          i         += 2;
        } else {
          buf[idx++] = from_32hex(c) * irLen;
        }

        if (--gotRep == 0) {
          while (--rep) {
            buf[idx]     = buf[idx - 2];
            buf[idx + 1] = buf[idx - 1];
            idx         += 2;
          }
        }
      }
    }
  } // End RAW2

  Plugin_035_irSender->sendRaw(buf, idx, IrHz);

  // printWebString += IrType + String(F(": Base32Hex RAW Code: ")) + IrRaw + String(F("<BR>kHz: ")) + IrHz + String(F("<BR>Pulse Len: ")) +
  // IrPLen + String(F("<BR>Blank Len: ")) + IrBLen + String(F("<BR>"));
  printToLog(F(": Base32Hex RAW Code Send "), IrRaw, 0, 0);

  // printWebString += String(F(": Base32Hex RAW Code Send "));
  delete[] buf;
  buf = nullptr;
  return true;
}

void P035_data_struct::printToLog(const String& protocol, const String& data, int bits, int repeats) {
  if (!loglevelActiveFor(LOG_LEVEL_INFO) && !printToWeb) {
    return;
  }
  String tmp = F("IRTX: IR Code Sent: ");

  tmp += protocol;
  tmp += F(" Data: ");
  tmp += data;

  if (bits > 0) {
    tmp += F(" Bits: ");
    tmp += bits;
  }

  if (repeats > 0) {
    tmp += F(" Repeats: ");
    tmp += repeats;
  }

  if (printToWeb) {
    printWebString = tmp;
  }
  addLogMove(LOG_LEVEL_INFO, tmp);
}

# ifdef P035_DEBUG_LOG
String P035_data_struct::listProtocols() {
  String temp;

  if (temp.reserve(1024)) {
    for (uint32_t i = 0; i <= kLastDecodeType; i++) {
      if (IRsend::defaultBits((decode_type_t)i) > 0) {
        String typ = typeToString((decode_type_t)i);

        if (typ.length() > 1) {
          temp += typ;
          temp += ' ';
        }
      }
    }
  }
  return temp;
}

# endif // ifdef P035_DEBUG_LOG

# if defined(P016_P035_Extended_AC) && defined(P035_DEBUG_LOG)
String P035_data_struct::listACProtocols() {
  String temp;

  if (temp.reserve(1024)) {
    for (uint32_t i = 0; i <= kLastDecodeType; i++) {
      if (hasACState((decode_type_t)i)) {
        const String typ = typeToString((decode_type_t)i);

        if (typ.length() > 1) {
          temp +=  typ;
          temp += ' ';
        }
      }
    }
  }
  return temp;
}

# endif // if defined(P016_P035_Extended_AC) && defined(P035_DEBUG_LOG)

bool P035_data_struct::addErrorTrue() {
  addLog(LOG_LEVEL_ERROR, F("RAW2: Invalid encoding!"));
  return true;
}

// A lot of the following code has been taken directly (with permission) from the IRMQTTServer.ino example code
// of the IRremoteESP8266 library. (https://github.com/markszabo/IRremoteESP8266)

// Transmit the given IR message.
//
// Args:
//   irsend:   A pointer to a IRsend object to transmit via.
//   irtype:  enum of the protocol to be sent.
//   code:     Numeric payload of the IR message. Most protocols use this.
//   code_str: The unparsed code to be sent. Used by complex protocol encodings.
//   bits:     Nr. of bits in the protocol. 0 means use the protocol's default.
//   repeat:   Nr. of times the message is to be repeated. (Not all protcols.)
// Returns:
//   bool: Successfully sent or not.
bool P035_data_struct::sendIRCode(int const irtype,
                                  uint64_t const code, char const *code_str, uint16_t bits,
                                  uint16_t repeat) {
  decode_type_t irType = (decode_type_t)irtype;
  bool success         = true; // Assume success.

  repeat = std::max(IRsend::minRepeats(irType), repeat);

  if (bits == 0) {
    bits = IRsend::defaultBits(irType);
  }

  // send the IR message.

  if (hasACState(irType)) { // protocols with > 64 bits
    success = parseStringAndSendAirCon(irType, code_str);
  } else {                  // protocols with <= 64 bits
    success = Plugin_035_irSender->send(irType, code, bits, repeat);
  }

  return success;
}

// Parse an Air Conditioner A/C Hex String/code and send it.
// Args:
//   irtype: Nr. of the protocol we need to send.
//   str: A hexadecimal string containing the state to be sent.
bool P035_data_struct::parseStringAndSendAirCon(const int irtype, const String str) {
  decode_type_t irType          = (decode_type_t)irtype;
  uint8_t  strOffset            = 0;
  uint8_t  state[kStateSizeMax] = { 0 }; // All array elements are set to 0.
  uint16_t stateSize            = 0;

  if (str.startsWith("0x") || str.startsWith("0X")) {
    strOffset = 2;
  }

  // Calculate how many hexadecimal characters there are.
  uint16_t inputLength = str.length() - strOffset;

  if (inputLength == 0) {
    // debug("Zero length AirCon code encountered. Ignored.");
    return false;   // No input. Abort.
  }

  switch (irType) { // Get the correct state size for the protocol.
    case DAIKIN:
      // Daikin has 2 different possible size states.
      // (The correct size, and a legacy shorter size.)
      // Guess which one we are being presented with based on the number of
      // hexadecimal digits provided. i.e. Zero-pad if you need to to get
      // the correct length/uint8_t size.
      // This should provide backward compatiblity with legacy messages.
      stateSize = inputLength / 2; // Every two hex chars is a uint8_t.
      // Use at least the minimum size.
      stateSize = std::max(stateSize, kDaikinStateLengthShort);

      // If we think it isn't a "short" message.
      if (stateSize > kDaikinStateLengthShort) {
        // Then it has to be at least the version of the "normal" size.
        stateSize = std::max(stateSize, kDaikinStateLength);
      }

      // Lastly, it should never exceed the "normal" size.
      stateSize = std::min(stateSize, kDaikinStateLength);
      break;
    case FUJITSU_AC:
      // Fujitsu has four distinct & different size states, so make a best guess
      // which one we are being presented with based on the number of
      // hexadecimal digits provided. i.e. Zero-pad if you need to to get
      // the correct length/uint8_t size.
      stateSize = inputLength / 2; // Every two hex chars is a uint8_t.
      // Use at least the minimum size.
      stateSize = std::max(stateSize,
                           (uint16_t)(kFujitsuAcStateLengthShort - 1));

      // If we think it isn't a "short" message.
      if (stateSize > kFujitsuAcStateLengthShort) {
        // Then it has to be at least the smaller version of the "normal" size.
        stateSize = std::max(stateSize, (uint16_t)(kFujitsuAcStateLength - 1));
      }

      // Lastly, it should never exceed the maximum "normal" size.
      stateSize = std::min(stateSize, kFujitsuAcStateLength);
      break;
    case HITACHI_AC3:
      // HitachiAc3 has two distinct & different size states, so make a best
      // guess which one we are being presented with based on the number of
      // hexadecimal digits provided. i.e. Zero-pad if you need to to get
      // the correct length/uint8_t size.
      stateSize = inputLength / 2; // Every two hex chars is a uint8_t.
      // Use at least the minimum size.
      stateSize = std::max(stateSize,
                           (uint16_t)(kHitachiAc3MinStateLength));

      // If we think it isn't a "short" message.
      if (stateSize > kHitachiAc3MinStateLength) {
        // Then it probably the "normal" size.
        stateSize = std::max(stateSize,
                             (uint16_t)(kHitachiAc3StateLength));
      }

      // Lastly, it should never exceed the maximum "normal" size.
      stateSize = std::min(stateSize, kHitachiAc3StateLength);
      break;
    case MWM:
      // MWM has variable size states, so make a best guess
      // which one we are being presented with based on the number of
      // hexadecimal digits provided. i.e. Zero-pad if you need to to get
      // the correct length/uint8_t size.
      stateSize = inputLength / 2; // Every two hex chars is a uint8_t.
      // Use at least the minimum size.
      stateSize = std::max(stateSize, (uint16_t)3);

      // Cap the maximum size.
      stateSize = std::min(stateSize, kStateSizeMax);
      break;
    case SAMSUNG_AC:
      // Samsung has two distinct & different size states, so make a best guess
      // which one we are being presented with based on the number of
      // hexadecimal digits provided. i.e. Zero-pad if you need to to get
      // the correct length/uint8_t size.
      stateSize = inputLength / 2; // Every two hex chars is a uint8_t.
      // Use at least the minimum size.
      stateSize = std::max(stateSize, (uint16_t)(kSamsungAcStateLength));

      // If we think it isn't a "normal" message.
      if (stateSize > kSamsungAcStateLength) {
        // Then it probably the extended size.
        stateSize = std::max(stateSize,
                             (uint16_t)(kSamsungAcExtendedStateLength));
      }

      // Lastly, it should never exceed the maximum "extended" size.
      stateSize = std::min(stateSize, kSamsungAcExtendedStateLength);
      break;
    default: // Everything else.
      stateSize = (IRsend::defaultBits(irType) + 7) / 8;

      if (!stateSize || !hasACState(irType)) {
        // Not a protocol we expected. Abort.
        // debug("Unexpected AirCon protocol detected. Ignoring.");
        return false;
      }
  }

  if (inputLength > stateSize * 2) {
    // debug("AirCon code to large for the given protocol.");
    return false;
  }

  // Ptr to the least significant uint8_t of the resulting state for this protocol.
  uint8_t *statePtr = &state[stateSize - 1];

  // Convert the string into a state array of the correct length.
  for (uint16_t i = 0; i < inputLength; i++) {
    // Grab the next least sigificant hexadecimal digit from the string.
    uint8_t c = tolower(str[inputLength + strOffset - i - 1]);

    if (isxdigit(c)) {
      if (isdigit(c)) {
        c -= '0';
      } else {
        c = c - 'a' + 10;
      }
    } else {
      // debug("Aborting! Non-hexadecimal char found in AirCon state:");
      // debug(str.c_str());
      return false;
    }

    if (i % 2 == 1) { // Odd: Upper half of the uint8_t.
      *statePtr += (c << 4);
      statePtr--;     // Advance up to the next least significant uint8_t of state.
    } else {          // Even: Lower half of the uint8_t.
      *statePtr = c;
    }
  }

  if (!Plugin_035_irSender->send(irType, state, stateSize)) {
    // debug("Unexpected AirCon type in send request. Not sent.");
    return false;
  }
  return true; // We were successful as far as we can tell.
}

#endif // ifdef USES_P035

#include "_Plugin_Helper.h"
#ifdef USES_P035
//#######################################################################################################
//#################################### Plugin 035: Output IR ############################################
//#######################################################################################################
//
// Usage: Connect an IR led to ESP8266 GPIO14 (D5) preferably. (various schematics can be found online)
// On the device tab add a new device and select "Communication - IR Transmit"
// Enable the device and select the GPIO led pin
// Power on the ESP and connect to it
// Commands can be send to this plug in and it will translate them to IR signals.
// Possible commands are IRSEND and IRSENDAC
//---IRSEND: That commands format is: IRSEND,<protocol>,<data>,<bits>,<repeat>
// OR JSON formated:                  IRSEND,'{"protocol":"<protocol>","data":"<data>","bits":<bits>,"repeats":<repeat>}'
// bits and repeat default to 0 if not used and they are optional
// For protocols RAW and RAW2 there is no bits and repeat part, they are supposed to be replayed as they are calculated by a Google docs sheet or by plugin P016
//---IRSENDAC: That commands format is: IRSENDAC,'{"protocol":"COOLIX","power":"on","mode":"dry","fanspeed":"auto","temp":22,"swingv":"max","swingh":"off"}'
// The possible values
// Protocols: Argo Coolix Daikin Fujitsu Haier Hitachi Kelvinator Midea Mitsubishi MitsubishiHeavy Panasonic Samsung Sharp Tcl Teco Toshiba Trotec Vestel Whirlpool
//---opmodes:      ---fanspeed:   --swingv:       --swingh:
// - "off"          - "auto"       - "off"         - "off"
// - "auto"         - "min"        - "auto"        - "auto"
// - "cool"         - "low"        - "highest"     - "leftmax"
// - "heat"         - "medium"     - "high"        - "left"
// - "dry"          - "high"       - "middle"      - "middle"
// - "fan_only"     - "max"        - "low"         - "right"
//                                 - "lowest"      - "rightmax"
//                                                 - "wide"
// "on" - "off" parameters are:
// - "power" - "celsius" - "quiet" - "turbo" - "econo" - "light" - "filter" - "clean" - "light" - "beep"
// If Celsius is set to "off" then farenheit will be used
// - "sleep" Nr. of mins of sleep mode, or use sleep mode. (<= 0 means off.)
// - "clock" Nr. of mins past midnight to set the clock to. (< 0 means off.)
// - "model" . Nr or string representation of the model. Better to find it throught P016 - IR RX (0 means default.)
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRutils.h>
#include <IRsend.h>

#ifdef P016_P035_Extended_AC
#include <IRac.h>
IRac *Plugin_035_commonAc = nullptr;
stdAc::state_t st, prev;
#endif

IRsend *Plugin_035_irSender = nullptr;

#define PLUGIN_035
#define PLUGIN_ID_035 35
#define PLUGIN_NAME_035 "Communication - IR Transmit"
#define STATE_SIZE_MAX 53U
#define PRONTO_MIN_LENGTH 6U

#define from_32hex(c) ((((c) | ('A' ^ 'a')) - '0') % 39)

#define P35_Ntimings 250 //Defines the ammount of timings that can be stored. Used in RAW and RAW2 encodings

boolean Plugin_035(byte function, struct EventStruct *event, String &command)
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
        command = F(PLUGIN_NAME_035);
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
        addHtml(F("IRSENDAC,{JSON formated AC command}"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        int irPin = CONFIG_PIN1;
        if (Plugin_035_irSender == 0 && irPin != -1)
        {
          addLog(LOG_LEVEL_INFO, F("INIT: IR TX"));
          addLog(LOG_LEVEL_INFO, String(F("IR lib Version: ")) + _IRREMOTEESP8266_VERSION_);
          addLog(LOG_LEVEL_INFO, String(F("Supported Protocols by IRSEND: ")) + listProtocols());
          Plugin_035_irSender = new IRsend(irPin);
          Plugin_035_irSender->begin(); // Start the sender
        }
        if (Plugin_035_irSender != 0 && irPin == -1)
        {
          addLog(LOG_LEVEL_INFO, F("INIT: IR TX Removed"));
          delete Plugin_035_irSender;
          Plugin_035_irSender = 0;
        }

#ifdef P016_P035_Extended_AC
        if (Plugin_035_commonAc == nullptr && irPin != -1)
        {
          addLog(LOG_LEVEL_INFO, F("INIT AC: IR TX"));
          addLog(LOG_LEVEL_INFO, String(F("Supported Protocols by IRSENDAC: ")) + listACProtocols());
          Plugin_035_commonAc = new (std::nothrow) IRac(irPin);
        }
        if (Plugin_035_commonAc != nullptr && irPin == -1)
        {
          addLog(LOG_LEVEL_INFO, F("INIT AC: IR TX Removed"));
          delete Plugin_035_commonAc;
          Plugin_035_commonAc = 0;
        }
#endif

        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String cmdCode = parseString(command,1);

        if (cmdCode.equalsIgnoreCase(F("IRSEND")) && Plugin_035_irSender != 0)
        {
          success = true;
          enableIR_RX(false);

#ifdef P016_P035_USE_RAW_RAW2
          handleRawRaw2Encoding(command);
#endif //P016_P035_USE_RAW_RAW2

          handleIRremote(command);
        }

#ifdef P016_P035_Extended_AC
        else if (cmdCode.equalsIgnoreCase(F("IRSENDAC")) && Plugin_035_commonAc != 0) {
          success = true;
          enableIR_RX(false);
          handle_AC_IRremote(command);
        }
#endif // P016_P035_Extended_AC

        enableIR_RX(true);
        break;
      } //PLUGIN_WRITE END
  } // SWITCH END
  return success;
} // Plugin_035 END

boolean handleIRremote(const String &cmd) {

  String IrType = "";
  String TmpStr1 = "";

  uint64_t IrCode = 0;
  uint16_t IrBits = 0;
  uint16_t IrRepeat = 0;
  String ircodestr = "";

  StaticJsonDocument<200> docTemp;
  DeserializationError error = deserializeJson(docTemp, cmd.substring(cmd.indexOf(',') + 1, cmd.length()));
  if (!error) // If the command is in JSON format
  {
    IrType =  docTemp[F("protocol")].as<String>();
    ircodestr = docTemp[F("data")].as<String>(); //JSON does not support hex values, thus we use command representation
    IrCode = strtoull(ircodestr.c_str(), NULL, 16);
    IrBits  = docTemp[F("bits")] | 0;
    IrRepeat = docTemp[F("repeats")] | 0;
  }
  else { // If the command is NOT in JSON format (legacy)
    if (GetArgv(cmd.c_str(), TmpStr1, 2))
    {
      IrType = TmpStr1;

      if (GetArgv(cmd.c_str(), ircodestr, 3))
      {
        IrCode = strtoull(ircodestr.c_str(), NULL, 16);
      }
      IrBits = 0; //Leave it to 0 for default protocol bits
      if (GetArgv(cmd.c_str(), TmpStr1, 4))
        IrBits = str2int(TmpStr1.c_str()); // Number of bits to be sent. USE 0 for default protocol bits
      if (GetArgv(cmd.c_str(), TmpStr1, 5))
        IrRepeat = str2int(TmpStr1.c_str());  // Nr. of times the message is to be repeated
    }
  }

  bool IRsent = sendIRCode(strToDecodeType(IrType.c_str()), IrCode, ircodestr.c_str(), IrBits, IrRepeat); //Send the IR command
  if (IRsent) printToLog(IrType, ircodestr, IrBits, IrRepeat);
  return IRsent;
}

#ifdef P016_P035_Extended_AC
boolean handle_AC_IRremote(const String &cmd) {
  String irData = "";
  StaticJsonDocument<JSON_OBJECT_SIZE(18) + 190> doc;

  int argIndex = cmd.indexOf(',') + 1;
  if (argIndex)
    irData = cmd.substring(argIndex, cmd.length());
  //addLog(LOG_LEVEL_INFO, String(F("IRTX: JSON received: ")) + irData);
  irData.toLowerCase(); // Circumvent the need to have case sensitive JSON keys

  DeserializationError error = deserializeJson(doc, irData);         // Deserialize the JSON document
  if (error)         // Test if parsing succeeds.
  {
    addLog(LOG_LEVEL_INFO, String(F("IRTX: Deserialize Json failed: ")) + error.c_str());
    return false; //do not continue with sending the signal.
  }
  String sprotocol = doc[F("protocol")];
  st.protocol = strToDecodeType(sprotocol.c_str());
  if (!IRac::isProtocolSupported(st.protocol)) //Check if we support the protocol
  {
    addLog(LOG_LEVEL_INFO, String(F("IRTX: Protocol not supported:")) + sprotocol);
    return false; //do not continue with sending of the signal.
  }

  String tempstr = "";
  tempstr = doc[F("model")].as<String>();
  st.model = IRac::strToModel(tempstr.c_str(), -1); //The specific model of A/C if applicable. //strToModel();. Defaults to -1 (unknown) if missing from JSON
  tempstr = doc[F("power")].as<String>();
  st.power = IRac::strToBool(tempstr.c_str(), false); //POWER ON or OFF. Defaults to false if missing from JSON
  st.degrees = doc[F("temp")] | 22.0f;                //What temperature should the unit be set to?. Defaults to 22c if missing from JSON
  tempstr = doc[F("use_celsius")].as<String>();
  st.celsius = IRac::strToBool(tempstr.c_str(), true); //Use degreees Celsius, otherwise Fahrenheit. Defaults to true if missing from JSON
  tempstr = doc[F("mode")].as<String>();
  st.mode = IRac::strToOpmode(tempstr.c_str(), stdAc::opmode_t::kAuto); //What operating mode should the unit perform? e.g. Cool. Defaults to auto if missing from JSON
  tempstr = doc[F("fanspeed")].as<String>();
  st.fanspeed = IRac::strToFanspeed(tempstr.c_str(), stdAc::fanspeed_t::kAuto); //Fan Speed setting. Defaults to auto if missing from JSON
  tempstr = doc[F("swingv")].as<String>();
  st.swingv = IRac::strToSwingV(tempstr.c_str(), stdAc::swingv_t::kAuto); //Vertical swing setting. Defaults to auto if missing from JSON
  tempstr = doc[F("swingh")].as<String>();
  st.swingh = IRac::strToSwingH(tempstr.c_str(), stdAc::swingh_t::kAuto); //Horizontal Swing setting. Defaults to auto if missing from JSON
  tempstr = doc[F("quiet")].as<String>();
  st.quiet = IRac::strToBool(tempstr.c_str(), false); //Quiet setting ON or OFF. Defaults to false if missing from JSON
  tempstr = doc[F("turbo")].as<String>();
  st.turbo = IRac::strToBool(tempstr.c_str(), false); //Turbo setting ON or OFF. Defaults to false if missing from JSON
  tempstr = doc[F("econo")].as<String>();
  st.econo = IRac::strToBool(tempstr.c_str(), false); //Economy setting ON or OFF. Defaults to false if missing from JSON
  tempstr = doc[F("light")].as<String>();
  st.light = IRac::strToBool(tempstr.c_str(), true); //Light setting ON or OFF. Defaults to true if missing from JSON
  tempstr = doc[F("filter")].as<String>();
  st.filter = IRac::strToBool(tempstr.c_str(), false); //Filter setting ON or OFF. Defaults to false if missing from JSON
  tempstr = doc[F("clean")].as<String>();
  st.clean = IRac::strToBool(tempstr.c_str(), false); //Clean setting ON or OFF. Defaults to false if missing from JSON
  tempstr = doc[F("beep")].as<String>();
  st.beep = IRac::strToBool(tempstr.c_str(), false); //Beep setting ON or OFF. Defaults to false if missing from JSON
  st.sleep = doc[F("sleep")] | -1; //Nr. of mins of sleep mode, or use sleep mode. (<= 0 means off.). Defaults to -1 if missing from JSON
  st.clock = doc[F("clock")] | -1; //Nr. of mins past midnight to set the clock to. (< 0 means off.). Defaults to -1 if missing from JSON

  //Send the IR command
  bool IRsent = Plugin_035_commonAc->sendAc(st, &prev);
  if (IRsent) printToLog(typeToString(st.protocol), irData, 0, 0);
  return IRsent;
}
#endif


boolean handleRawRaw2Encoding(const String &cmd) {
  boolean raw=true;
  String IrType = "";
  if (!GetArgv(cmd.c_str(), IrType, 2)) return false;

  if (IrType.equalsIgnoreCase(F("RAW"))) raw = true;
  else if (IrType.equalsIgnoreCase(F("RAW2")))  raw = false;


  String IrRaw, TmpStr1;
  uint16_t IrHz = 0;
  unsigned int IrPLen = 0;
  unsigned int IrBLen = 0;

  if (GetArgv(cmd.c_str(), TmpStr1, 3))
    IrRaw = TmpStr1; //Get the "Base32" encoded/compressed Ir signal
  if (GetArgv(cmd.c_str(), TmpStr1, 4))
    IrHz = str2int(TmpStr1.c_str()); //Get the base freguency of the signal (allways 38)
  if (GetArgv(cmd.c_str(), TmpStr1, 5))
    IrPLen = str2int(TmpStr1.c_str()); //Get the Pulse Length in ms
  if (GetArgv(cmd.c_str(), TmpStr1, 6))
    IrBLen = str2int(TmpStr1.c_str()); //Get the Blank Pulse Length in ms

  uint16_t idx = 0; //If this goes above the buf.size then the esp will throw a 28 EXCCAUSE
  uint16_t *buf;
  buf = new (std::nothrow) uint16_t[P35_Ntimings]; //The Raw Timings that we can buffer.
  if (buf == nullptr)
  { // error assigning memory.
    return false;
  }


  if (raw) {
    unsigned int c0 = 0; //count consecutives 0s
    unsigned int c1 = 0; //count consecutives 1s

    //printWebString += F("Interpreted RAW Code: ");  //print the number of 1s and 0s just for debugging/info purposes
    //Loop throught every char in RAW string
    for (unsigned int i = 0; i < IrRaw.length(); i++)
    {
      //Get the decimal value from base32 table
      //See: https://en.wikipedia.org/wiki/Base32#base32hex
      char c = from_32hex(IrRaw[i]);

      //Loop through 5 LSB (bits 16, 8, 4, 2, 1)
      for (unsigned int shft = 1; shft < 6; shft++)
      {
        //if bit is 1 (5th position - 00010000 = 16)
        if ((c & 16) != 0)
        {
          //add 1 to counter c1
          c1++;
          //if we already have any 0s in counting (the previous
          //bit was 0)
          if (c0 > 0)
          {
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
        }
        else
        {
          //So, bit is 0

          //On first call, ignore 0s (suppress left-most 0s)
          if (c0 + c1 != 0)
          {
            //add 1 to counter c0
            c0++;
            //if we already have any 1s in counting (the previous
            //bit was 1)
            if (c1 > 0)
            {
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
    if (c0 > 0)
    {
      buf[idx++] = c0 * IrBLen;
      //for (uint t = 0; t < c0; t++)
      //printWebString += '0';
    }
    //If we have pendings 1s
    if (c1 > 0)
    {
      buf[idx++] = c1 * IrPLen;
      //for (uint t = 0; t < c1; t++)
      //printWebString += '1';
    }

    //printWebString += F("<BR>");
  }

  if (!raw) { // RAW2

    for (unsigned int i = 0, total = IrRaw.length(), gotRep = 0, rep = 0; i < total;)
    {
      char c = IrRaw[i++];
      if (c == '*')
      {
        if (i + 2 >= total || idx + (rep = from_32hex(IrRaw[i++])) * 2 > sizeof(buf[0]) * P35_Ntimings)
        {
          delete[] buf;
          buf = nullptr;
          return addErrorTrue();
        }
        gotRep = 2;
      }
      else
      {
        if ((c == '^' && i + 1 >= total) || idx >= sizeof(buf[0]) * P35_Ntimings)
        {
          delete[] buf;
          buf = nullptr;
          return addErrorTrue();
        }

        uint16_t irLen = (idx & 1) ? IrBLen : IrPLen;
        if (c == '^')
        {
          buf[idx++] = (from_32hex(IrRaw[i]) * 32 + from_32hex(IrRaw[i + 1])) * irLen;
          i += 2;
        }
        else
          buf[idx++] = from_32hex(c) * irLen;

        if (--gotRep == 0)
        {
          while (--rep)
          {
            buf[idx] = buf[idx - 2];
            buf[idx + 1] = buf[idx - 1];
            idx += 2;
          }
        }
      }
    }
  } //End RAW2

  Plugin_035_irSender->sendRaw(buf, idx, IrHz);
  //printWebString += IrType + String(F(": Base32Hex RAW Code: ")) + IrRaw + String(F("<BR>kHz: ")) + IrHz + String(F("<BR>Pulse Len: ")) + IrPLen + String(F("<BR>Blank Len: ")) + IrBLen + String(F("<BR>"));
  printToLog(String(F(": Base32Hex RAW Code Send ")), IrRaw, 0, 0);
  // printWebString += String(F(": Base32Hex RAW Code Send "));
  delete[] buf;
  buf = nullptr;
  return true;
}


void printToLog(String protocol, String data, int bits, int repeats) {
  String tmp = String(F("IRTX: IR Code Sent: ")) + protocol + String(F(" Data: ")) + data;
  if (bits > 0) tmp += String(F(" Bits: ")) + bits;
  if (repeats > 0) tmp += String(F(" Repeats: ")) + repeats;
  addLog(LOG_LEVEL_INFO, tmp);
  if (printToWeb)
  {
    printWebString = tmp;
  }
}

String listProtocols() {
  String temp;
  for (uint32_t i = 0; i <= kLastDecodeType; i++) {
    if (IRsend::defaultBits((decode_type_t)i) > 0 )
      temp += typeToString((decode_type_t)i) + ' ';
  }
  return temp;
}

#ifdef P016_P035_Extended_AC
String listACProtocols() {
  String temp;
  for (uint32_t i = 0; i <= kLastDecodeType; i++) {
    if (hasACState((decode_type_t)i))
      temp += typeToString((decode_type_t)i) + ' ';
  }
  return temp;
}
#endif

boolean addErrorTrue()
{
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
bool sendIRCode(int const irtype,
                uint64_t const code, char const *code_str, uint16_t bits,
                uint16_t repeat)
{
  decode_type_t irType = (decode_type_t)irtype;
  bool success = true; // Assume success.
  repeat = std::max(IRsend::minRepeats(irType), repeat);
  if (bits == 0)
    bits = IRsend::defaultBits(irType);
  // send the IR message.

  if (hasACState(irType)) // protocols with > 64 bits
    success = parseStringAndSendAirCon(irType, code_str);
  else // protocols with <= 64 bits
    success = Plugin_035_irSender->send(irType, code, bits, repeat);

  return success;
}

// Parse an Air Conditioner A/C Hex String/code and send it.
// Args:
//   irtype: Nr. of the protocol we need to send.
//   str: A hexadecimal string containing the state to be sent.
bool parseStringAndSendAirCon(const int irtype, const String str)
{
  decode_type_t irType = (decode_type_t)irtype;
  uint8_t strOffset = 0;
  uint8_t state[kStateSizeMax] = {0}; // All array elements are set to 0.
  uint16_t stateSize = 0;

  if (str.startsWith("0x") || str.startsWith("0X"))
    strOffset = 2;
  // Calculate how many hexadecimal characters there are.
  uint16_t inputLength = str.length() - strOffset;
  if (inputLength == 0)
  {
    // debug("Zero length AirCon code encountered. Ignored.");
    return false; // No input. Abort.
  }

   switch (irType) {  // Get the correct state size for the protocol.
    case DAIKIN:
      // Daikin has 2 different possible size states.
      // (The correct size, and a legacy shorter size.)
      // Guess which one we are being presented with based on the number of
      // hexadecimal digits provided. i.e. Zero-pad if you need to to get
      // the correct length/byte size.
      // This should provide backward compatiblity with legacy messages.
      stateSize = inputLength / 2;  // Every two hex chars is a byte.
      // Use at least the minimum size.
      stateSize = std::max(stateSize, kDaikinStateLengthShort);
      // If we think it isn't a "short" message.
      if (stateSize > kDaikinStateLengthShort)
        // Then it has to be at least the version of the "normal" size.
        stateSize = std::max(stateSize, kDaikinStateLength);
      // Lastly, it should never exceed the "normal" size.
      stateSize = std::min(stateSize, kDaikinStateLength);
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
    case HITACHI_AC3:
      // HitachiAc3 has two distinct & different size states, so make a best
      // guess which one we are being presented with based on the number of
      // hexadecimal digits provided. i.e. Zero-pad if you need to to get
      // the correct length/byte size.
      stateSize = inputLength / 2;  // Every two hex chars is a byte.
      // Use at least the minimum size.
      stateSize = std::max(stateSize,
                           (uint16_t) (kHitachiAc3MinStateLength));
      // If we think it isn't a "short" message.
      if (stateSize > kHitachiAc3MinStateLength)
        // Then it probably the "normal" size.
        stateSize = std::max(stateSize,
                             (uint16_t) (kHitachiAc3StateLength));
      // Lastly, it should never exceed the maximum "normal" size.
      stateSize = std::min(stateSize, kHitachiAc3StateLength);
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
    default:  // Everything else.
      stateSize = IRsend::defaultBits(irType) / 8;
      if (!stateSize || !hasACState(irType))
      {
        // Not a protocol we expected. Abort.
        // debug("Unexpected AirCon protocol detected. Ignoring.");
        return false;
      }
  }
  if (inputLength > stateSize * 2)
  {
    // debug("AirCon code to large for the given protocol.");
    return false;
  }

  // Ptr to the least significant byte of the resulting state for this protocol.
  uint8_t *statePtr = &state[stateSize - 1];

  // Convert the string into a state array of the correct length.
  for (uint16_t i = 0; i < inputLength; i++)
  {
    // Grab the next least sigificant hexadecimal digit from the string.
    uint8_t c = tolower(str[inputLength + strOffset - i - 1]);
    if (isxdigit(c))
    {
      if (isdigit(c))
        c -= '0';
      else
        c = c - 'a' + 10;
    }
    else
    {
      // debug("Aborting! Non-hexadecimal char found in AirCon state:");
      // debug(str.c_str());
      return false;
    }
    if (i % 2 == 1)
    { // Odd: Upper half of the byte.
      *statePtr += (c << 4);
      statePtr--; // Advance up to the next least significant byte of state.
    }
    else
    { // Even: Lower half of the byte.
      *statePtr = c;
    }
  }
  if (!Plugin_035_irSender->send(irType, state, stateSize))
  {
    //debug("Unexpected AirCon type in send request. Not sent.");
    return false;
  }
  return true; // We were successful as far as we can tell.
}

#endif // USES_P035

#include "../PluginStructs/P008_data_struct.h"

#ifdef USES_P008

/**
 * Convert/cast a hexadecimal input to a decimal representation, so 0x1234 (= 4660) comes out as 1234.
 *
 * //FIXME Move to a more global place to also be used elsewhere?
 */
uint64_t P008_data_struct::castHexAsDec(uint64_t hexValue) {
  uint64_t result = 0;
  uint8_t  digit;

  uint64_t factor = 1;

  for (int i = 0; i < 8; i++) {
    digit = (hexValue & 0x0000000F);

    if (digit > 10) {
      digit = 0; // Cast by dropping any non-decimal input
    }

    if (digit > 0) {
      result += (digit * factor);
    }
    hexValue >>= 4;

    if (hexValue == 0) {
      break; // Stop when no more to process
    }
    factor *= 10;
  }
  return result;
}

/**************************************************************************
* Constructor
**************************************************************************/
P008_data_struct::P008_data_struct(struct EventStruct *event) {
  _pin1 = CONFIG_PIN1;
  _pin2 = CONFIG_PIN2;
}

/*****************************************************
* Destructor
*****************************************************/
P008_data_struct::~P008_data_struct() {
  if (initialised) {
    detachInterrupt(digitalPinToInterrupt(_pin1));
    detachInterrupt(digitalPinToInterrupt(_pin2));
  }
}

/**************************************************************************
* plugin_init Initialize interrupt handling
**************************************************************************/
bool P008_data_struct::plugin_init(struct EventStruct *event) {
  if (validGpio(_pin1) && validGpio(_pin2)) {
    pinMode(_pin1, INPUT_PULLUP);
    pinMode(_pin2, INPUT_PULLUP);

    // Keep 'old' setting for backward compatibility
    uint8_t _p1 = _pin1;
    uint8_t _p2 = _pin2;

    if (P008_COMPATIBILITY != 0) {
      _p1 = _pin2; // Original code had the pins swapped, swap 'm back to be Wiegand compatible
      _p2 = _pin1;
    }
    attachInterruptArg(digitalPinToInterrupt(_p1),
                       reinterpret_cast<void (*)(void *)>(Plugin_008_interrupt1),
                       this,
                       FALLING);
    attachInterruptArg(digitalPinToInterrupt(_p2),
                       reinterpret_cast<void (*)(void *)>(Plugin_008_interrupt2),
                       this,
                       FALLING);
    initialised = true;
  }
  return initialised;
}

/*****************************************************
* plugin_once_a_second
*****************************************************/
bool P008_data_struct::plugin_once_a_second(struct EventStruct *event) {
  bool success = false;

  if (initialised) {
    if (bitCount > 0) {
      success = true; // Let's assume read succeeded
      uint64_t keyMask = 0ull;

      if ((bitCount % 4 == 0) && ((keyBuffer & 0xF) == 11)) {
        // a number of keys were pressed and finished by #
        keyBuffer   = keyBuffer >> 4; // Strip #
        bufferValid = true;
        bufferBits  = bitCount;
      } else if (bitCount == P008_DATA_BITS) {
        // read a tag
        keyBuffer = keyBuffer >> 1;                 // Strip leading and trailing parity bits from the keyBuffer

        keyMask = (0x1ull << (P008_DATA_BITS - 2)); // Shift in 1 just past the number of remaining bits
        keyMask--;                                  // Decrement by 1 to get 0xFFFFFFFFFFFF...
        keyBuffer  &= keyMask;
        bufferValid = true;
        bufferBits  = bitCount;
      } else {
        // not enough bits, maybe next time
        timeoutCount++;
        bufferValid = false;
        bufferBits  = 0u;

        if (timeoutCount > P008_TIMEOUT_LIMIT) {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, concat(F("RFID : reset bits: "), static_cast<int>(bitCount)));
          }

          // reset after ~5 sec
          keyBuffer    = 0ull;
          bitCount     = 0u;
          timeoutCount = 0u;
        }
        success = false;
      }

      if (success) {
        uint64_t old_key = UserVar.getSensorTypeLong(event->TaskIndex);
        bool     new_key = false;

        if (P008_HEX_AS_DEC == 1) {
          keyBuffer = castHexAsDec(keyBuffer);
        }

        if (old_key != keyBuffer) {
          UserVar.setSensorTypeLong(event->TaskIndex, keyBuffer);
          new_key = true;
        }

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          // write log
          String log = F("RFID : ");

          if (new_key) {
            log += F("New Tag: ");
          } else {
            log += F("Old Tag: ");
          }
          log += (unsigned long)keyBuffer;
          log += F(", 0x");
          log += ull2String(keyBuffer, 16);
          log += F(", mask: 0x");
          log += ull2String(keyMask, 16);
          log += F(" Bits: ");
          log += bitCount;
          addLogMove(LOG_LEVEL_INFO, log);
        }

        // reset everything
        keyBuffer    = 0ull;
        bitCount     = 0u;
        timeoutCount = 0u;

        if (new_key) { sendData(event); }
        uint32_t resetTimer = P008_REMOVE_TIMEOUT;

        if (resetTimer < 250) { resetTimer = 250; }
        Scheduler.setPluginTaskTimer(resetTimer, event->TaskIndex, event->Par1);

        // Used during debugging
        // String   info;
        // uint64_t invalue  = 0x1234;
        // uint64_t outvalue = castHexAsDec(invalue);
        // info.reserve(40);
        // info += F("Test castHexAsDec(");
        // info += (double)invalue;
        // info += F(") => ");
        // info += (double)outvalue;
        // addLog(LOG_LEVEL_INFO, info);
      }
    }
  }
  return success;
}

/*****************************************************
* plugin_timer_in
*****************************************************/
bool P008_data_struct::plugin_timer_in(struct EventStruct *event) {
  if (initialised && (P008_AUTO_REMOVE == 0)) { // P008_AUTO_REMOVE check uses inversed logic!
    // Reset card id on timeout
    UserVar.setSensorTypeLong(event->TaskIndex, P008_REMOVE_VALUE);
    bufferValid = true;
    bufferBits  = P008_DATA_BITS;
    addLog(LOG_LEVEL_INFO, F("RFID : Removed Tag"));

    if (P008_REMOVE_EVENT == 1) {
      sendData(event);
    }
    return true;
  }
  return false;
}

/***********************************************************************
 * shift_bit_in_buffer
 **********************************************************************/
void IRAM_ATTR P008_data_struct::Plugin_008_shift_bit_in_buffer(P008_data_struct *self,
                                                                uint8_t           bit) {
  self->keyBuffer = self->keyBuffer << 1; // Left shift the number (effectively multiplying by 2)

  if (bit) { self->keyBuffer |= 1ull; }   // Add the 1 (not necessary for the zeroes)
  self->bitCount++;                       // Increment the bit count
}

/*********************************************************************
* Interrupt 1 : Handle 1 bits
*********************************************************************/
void IRAM_ATTR P008_data_struct::Plugin_008_interrupt1(P008_data_struct *self) {
  // We've received a 1 bit. (bit 0 = high, bit 1 = low)
  Plugin_008_shift_bit_in_buffer(self, 1); // Shift in a 1
}

/*********************************************************************
* Interrupt 2 : Handle 0 bits
*********************************************************************/
void IRAM_ATTR P008_data_struct::Plugin_008_interrupt2(P008_data_struct *self) {
  // We've received a 0 bit. (bit 0 = low, bit 1 = high)
  Plugin_008_shift_bit_in_buffer(self, 0); // Shift in a 0
}

/********************************************************************
* plugin_get_config
********************************************************************/
bool P008_data_struct::plugin_get_config(struct EventStruct *event,
                                         String            & string) {
  bool success = false;

  if (initialised) {
    String sub = parseString(string, 1);

    if (sub.equals(F("tagstr"))) { // Format tag as hex/dec
      uint64_t tag  = UserVar.getSensorTypeLong(event->TaskIndex);
      uint8_t  bits = bufferBits == 0 ? (P008_DATA_BITS - (P008_DATA_BITS % 4)) / 4 : bufferBits;
      string  = formatToHex_no_prefix(tag, bits);
      success = true;
    } else
    if (sub.equals(F("tagsize")) && bufferValid) { // Last tagsize in characters
      string  = (bufferBits - (bufferBits % 4)) / 4;
      success = true;
    } else
    if (sub.equals(F("tagbits")) && bufferValid) { // Last tagsize in bits
      string  = bufferBits;
      success = true;
    } else
    if (sub.equals(F("tagvalid"))) { // Buffer valid
      string  = bufferValid;
      success = true;
    }
  }

  return success;
}

#endif // ifdef USES_P008

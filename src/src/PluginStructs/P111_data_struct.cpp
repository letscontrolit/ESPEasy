#include "../PluginStructs/P111_data_struct.h"

#ifdef USES_P111

# include "../PluginStructs/P111_data_struct.h"

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter

# include <MFRC522.h>

P111_data_struct::P111_data_struct(int8_t csPin,
                                   int8_t rstPin)
  : mfrc522(nullptr), _csPin(csPin), _rstPin(rstPin)
{}

P111_data_struct::~P111_data_struct() {
  delete mfrc522;
  mfrc522 = nullptr;
}

void P111_data_struct::init() {
  delete mfrc522;

  mfrc522 = new (std::nothrow) MFRC522(_csPin, _rstPin); // Instantiate a MFRC522

  if (mfrc522 != nullptr) {
    mfrc522->PCD_Init();                                 // Initialize MFRC522 reader
    initPhase = P111_initPhases::Ready;
  }
}

/**
 * read status and tag
 */
uint8_t P111_data_struct::readCardStatus(uint32_t *key,
                                         bool     *removedTag) {
  if (initPhase != P111_initPhases::Ready) { // No read during reset
    return P111_ERROR_RESET_BUSY;
  }

  uint8_t error = P111_NO_ERROR;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  error = readPassiveTargetID(uid, &uidLength);

  switch (error) {
    case P111_ERROR_READ: // Read error
    {
      errorCount++;
      removedState = false;

      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log = F("MFRC522: Read error: ");
        log += errorCount;
        addLogMove(LOG_LEVEL_ERROR, log);
      }
      break;
    }
    case P111_ERROR_NO_TAG: // No tag found

      if (!removedState) {
        removedState = true;
        *removedTag  = true;
        error        = 0; // pass through that removal just once
      }
      errorCount = 0;
      break;
    default:                // Read a tag correctly
      errorCount   = 0;
      removedState = false; // No longer removed
      break;
  }

  if (errorCount > 2) { // if three consecutive read errors, reset MFRC522
    if (!reset(_csPin, _rstPin)) {
      return P111_ERROR_RESET_BUSY;
    }
  }
  uint32_t tmpKey = uid[0];

  for (uint8_t i = 1; i < 4; i++) {
    tmpKey <<= 8;
    tmpKey  += uid[i];
  }
  *key = tmpKey;

  return error;
}

/**
 * Returns last read card (type) name
 */
String P111_data_struct::getCardName() {
  return mfrc522->PICC_GetTypeName(mfrc522->PICC_GetType(mfrc522->uid.sak));
}

/*********************************************************************************************\
* MFRC522 init
* Procedure when resetPin != -1:
* - pull reset pin low
* - set timer for 100 msec, phase = ResetTimer1
* - exit false
* - plugin_fifty_per_second counts down the timer, when done calls reset() again
* - pull reset pin high
* - set timer for 10 msec, phase = ResetTimer2
* - exit false
* - plugin_fifty_per_second counts down the timer, when done calls reset() again
* - reset initializes the cardreader and checks status
* - phase = Ready
* - exit true, assuming initialization succeeds
\*********************************************************************************************/
bool P111_data_struct::reset(int8_t csPin,
                             int8_t resetPin) {
  if ((resetPin != -1) &&
      (initPhase == P111_initPhases::Ready)) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("MFRC522: Reset on pin: ");
      log += resetPin;
      addLogMove(LOG_LEVEL_INFO, log);
    }
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
    timeToWait = 100;
    initPhase  = P111_initPhases::ResetDelay1; // Start 1st timer
    return false;
  }

  if ((resetPin != -1) &&
      (initPhase == P111_initPhases::ResetDelay1)) {
    digitalWrite(resetPin, HIGH);
    pinMode(resetPin, INPUT_PULLUP);
    timeToWait = 10;                           // Effectively 20 msec
    initPhase  = P111_initPhases::ResetDelay2; // Start 2nd timer
    return false;
  }

  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, LOW);

  mfrc522->PCD_Init(csPin, resetPin); // Init MFRC522 module

  // If you set Antenna Gain to Max it will increase reading distance
  mfrc522->PCD_SetAntennaGain(mfrc522->RxGain_max);

  bool result = mfrc522->PCD_PerformSelfTest(); // perform the test

  if (result) {
    // String log = F("RC522: Found");
    // Get the MFRC522 software version
    uint8_t v = mfrc522->PCD_ReadRegister(mfrc522->VersionReg);

    // When 0x00 or 0xFF is returned, communication probably failed
    if ((v == 0x00) || (v == 0xFF)) {
      addLog(LOG_LEVEL_ERROR, F("MFRC522: Communication failure, is the MFRC522 properly connected?"));
      result = false;
    } else {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("MFRC522: Software Version: ");

        if (v == 0x91) {
          log += F(" = v1.0");
        }
        else if (v == 0x92) {
          log += F(" = v2.0");
        }
        else {
          log += F(" (unknown),probably a chinese clone?");
        }

        addLogMove(LOG_LEVEL_INFO, log);
      }
    }
  }

  if ((resetPin != -1) &&
      (initPhase == P111_initPhases::ResetDelay1)) { // Last phase, done
    initPhase = P111_initPhases::Ready;              // Reading can commence again
  }

  return result;
}

/*********************************************************************************************\
* RC522 read tag ID
\*********************************************************************************************/
uint8_t P111_data_struct::readPassiveTargetID(uint8_t *uid,
                                              uint8_t *uidLength) { // needed ? see above (not PN532)
  // Getting ready for Reading PICCs
  if (!mfrc522->PICC_IsNewCardPresent()) {                          // If a new PICC placed to RFID reader continue
    return P111_ERROR_NO_TAG;
  }
  addLog(LOG_LEVEL_INFO, F("MFRC522: New Card Detected"));

  if (!mfrc522->PICC_ReadCardSerial()) { // Since a PICC placed get Serial and continue
    return P111_ERROR_READ;
  }

  // There are Mifare PICCs which have 4 uint8_t or 7 uint8_t UID care if you use 7 uint8_t PICC
  // I think we should assume every PICC as they have 4 uint8_t UID
  // Until we support 7 uint8_t PICCs
  addLog(LOG_LEVEL_INFO, F("MFRC522: Scanned PICC's UID"));

  for (uint8_t i = 0; i < 4; i++) { //
    uid[i] = mfrc522->uid.uidByte[i];
  }
  *uidLength = 4;
  mfrc522->PICC_HaltA(); // Stop reading
  return P111_NO_ERROR;
}

/*********************************************************************************************
 * Handle regular read and reset processing
 ********************************************************************************************/
bool P111_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  bool success = false;

  if (((initPhase == P111_initPhases::ResetDelay1) || // Whichever handler comes first
       (initPhase == P111_initPhases::ResetDelay2)) &&
      (timeToWait <= 0)) {
    timeToWait = 0;

    reset(_csPin, _rstPin); // Start next phase
    return success;
  }

  counter++;          // This variable replaces a static variable in the original implementation

  if (counter == 3) { // Only every 3rd 0.1 second we do a read
    counter = 0;

    uint32_t key        = P111_NO_KEY;
    bool     removedTag = false;
    const uint8_t error = readCardStatus(&key, &removedTag);

    if (error == P111_NO_ERROR) {
      const uint32_t old_key = UserVar.getSensorTypeLong(event->TaskIndex);
      bool new_key           = false;

      # ifdef P111_USE_REMOVAL

      if (removedTag && (P111_TAG_AUTOREMOVAL == 2)) { // removal detected and enabled
        key = P111_REMOVALVALUE;
      }
      # endif // P111_USE_REMOVAL

      if ((old_key != key) && (key != P111_NO_KEY)) {
        UserVar.setSensorTypeLong(event->TaskIndex, key);
        new_key = true;
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO) && (key != P111_NO_KEY)) {
        String log = F("MFRC522: ");

        if (new_key) {
          log += F("New Tag: ");
        } else {
          log += F("Old Tag: ");
        }
        log += key;

        if (!removedTag) {
          log += F(" card: ");
          log += getCardName();
        }
        addLogMove(LOG_LEVEL_INFO, log);
      }

      if (new_key && !removedTag) { // Removal event sent from PLUGIN_TIMER_IN, if any
        sendData(event);
      }
      Scheduler.setPluginTaskTimer(P111_REMOVALTIMEOUT, event->TaskIndex, event->Par1);
      success = true;
    }
  }
  return success;
}

/*********************************************************************************************
 * Handle timers instead of using delay()
 ********************************************************************************************/
bool P111_data_struct::plugin_fifty_per_second() {
  if ((initPhase == P111_initPhases::ResetDelay1) ||
      (initPhase == P111_initPhases::ResetDelay2)) {
    timeToWait -= 20; // milliseconds

    // String log = F("MFRC522: remaining wait: ");
    // log += timeToWait;
    // addLogMove(LOG_LEVEL_INFO, log);

    if (initPhase == P111_initPhases::ResetDelay1) { // Only handle ResetDelay1 here, as ResetDelay2 phase might be too much for 50/s
      if (timeToWait <= 0) {
        timeToWait = 0;

        reset(_csPin, _rstPin); // Start next phase
      }
    }
  }
  return true;
}

#endif // ifdef USES_P111

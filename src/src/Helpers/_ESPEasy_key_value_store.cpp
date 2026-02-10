#include "../Helpers/_ESPEasy_key_value_store.h"

#if FEATURE_ESPEASY_KEY_VALUE_STORE

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/ESPEasy_time_calc.h"
# include "../Helpers/KeyValueWriter.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/StringConverter_Numerical.h"


bool ESPEasy_key_value_store::isEmpty() const
{
  return
    _4byte_data.empty() &&
    _8byte_data.empty() &&
    _string_data.empty();
}

void ESPEasy_key_value_store::clear()
{
  _4byte_data.clear();
  _8byte_data.clear();
  _string_data.clear();
  _state = State::Empty;
}

bool ESPEasy_key_value_store::load(
  SettingsType::Enum settingsType,
  int                index,
  uint32_t           offset_in_block,
  uint16_t           id_to_match)
{
  if (_state == State::NotChanged) { return true; }
  int offset, max_size;

  if (!SettingsType::getSettingsParameters(settingsType, index, offset, max_size))
  {
    _lastError = F("KVS: Invalid index");
    # ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_ERROR, _lastError); // addLog(LOG_LEVEL_DEBUG, _lastError);
    # endif // ifndef BUILD_NO_DEBUG
    return false;
  }

  clear();

  const uint32_t bufferSize = 128;

  char buffer[bufferSize] = { 0 };
  const uint8_t*buf_start = (const uint8_t *)buffer;
  String   result;
  uint32_t readPos = offset_in_block;

  // Read header:
  // - uint8_t:    version of storage layer
  // - uint8_t[3]: rawSize (offset to start of checksum)
  result += LoadFromFile(settingsType,
                         index,
                         reinterpret_cast<uint8_t *>(&buffer),
                         6, // readSize
                         readPos);
  readPos += 6;


  // TODO TD-er: Compute checksum & check version

  const uint8_t version = buffer[0];

  // uint8_t[2]: ID to match, if not matched, then stop loading
  const uint16_t id = (buffer[2] << 8) | buffer[1];

  if (id != id_to_match) {
    # ifndef BUILD_NO_DEBUG
    _lastError = strformat(F("KVS: Stored ID %d != expected ID %d, stop loading"), id, id_to_match);
    addLog(LOG_LEVEL_DEBUG, _lastError);
    # endif // ifndef BUILD_NO_DEBUG
    return false;
  }

  const size_t payloadSize      = (buffer[5] << 16) | (buffer[4] << 8) | buffer[3];
  const size_t totalSize        = 6 + payloadSize + 16; // header + payload + checksum
  const size_t startChecksumPos = offset_in_block + 6 + payloadSize;

  if ((offset_in_block + totalSize) > static_cast<size_t>(max_size)) {
    _state = State::ErrorOnLoad;
    # ifndef BUILD_NO_DEBUG
    _lastError = strformat(F("KVS: Total size %d + offset %d exceeds max size %d"), totalSize, offset_in_block, max_size);
    addLog(LOG_LEVEL_ERROR, _lastError); // addLog(LOG_LEVEL_DEBUG, _lastError);
    # endif // ifndef BUILD_NO_DEBUG
    return false;
  }
# ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, strformat(F("KVS Load : Total size %d + offset %d, max size %d"), totalSize, offset_in_block, max_size));
# endif

  size_t bytesLeftPartialString{};
  String partialString;
  KVS_StorageType::Enum storageType{};
  size_t   sizePerType{};
  uint32_t key{};

  const unsigned long start = millis();

  while (readPos < startChecksumPos && timePassedSince(start) < 1000) {
    // This loop always starts at the beginning of a key/value type,
    // except when we're parsing a string which may be longer than the buffer
    const uint32_t readSize = std::min(bufferSize, startChecksumPos - readPos);
    result += LoadFromFile(settingsType,
                           index,
                           reinterpret_cast<uint8_t *>(&buffer),
                           readSize,
                           readPos);
    addLog(LOG_LEVEL_INFO, strformat(
             F("KVS: LoadFromFile  readSize %d readPos %d"),
             readSize,
             readPos));
    uint32_t bufPos           = 0;
    bool     loadNextFromFile = false;

    for (; !loadNextFromFile && bufPos < readSize;) {
      // In this loop, only update bufPos and at the end also correct readPos
      // as it might be easy to forget adding the same to both bufPos and readPos
      // However in logs we might need to refer to the actual pos in the file/block
      // For this the following define is used
      # define LOG_READPOS_OFFSET (readPos + (bufPos - bufPos_start_loop))
      const uint32_t bufPos_start_loop = bufPos;

      if (!bytesLeftPartialString) {
        // Parse KVS_StorageType::Enum + key
        if ((readSize - bufPos) < 4) {
          // Read next block
          loadNextFromFile = true;

        } else {
          const ESPEasy_key_value_store_4byte_data_t combined_key(buf_start + bufPos);
          storageType = KVS_StorageType::get_StorageType_from_combined_key(combined_key.getUint32());
          sizePerType = KVS_StorageType::getStorageSizePerType(storageType);
          key         = KVS_StorageType::getKey_from_combined_key(combined_key.getUint32());

          if ((sizePerType < 4) && (storageType != KVS_StorageType::Enum::string_type)) {
            // Should not happen as those should not be stored in the file
            _state = State::ErrorOnLoad;
            # ifndef BUILD_NO_DEBUG
            _lastError = strformat(F("KVS: Invalid storage type %d at readPos %d"), static_cast<int>(storageType), LOG_READPOS_OFFSET);
            addLog(LOG_LEVEL_ERROR, _lastError);
            # endif // ifndef BUILD_NO_DEBUG
            return false;
          }
        }
      }

      if (!loadNextFromFile) {
        switch (storageType)
        {
          case KVS_StorageType::Enum::bool_false:
          case KVS_StorageType::Enum::bool_type:
          case KVS_StorageType::Enum::bool_true:
          {
            // Store bool type
            const bool res = storageType != KVS_StorageType::Enum::bool_false;
            setValue(key, res);
            bufPos += 4;
            break;
          }

          case KVS_StorageType::Enum::string_type:
          {
            if (bytesLeftPartialString == 0) {
              // Found new string type
              // Read expected size in next 2 bytes
              if ((readSize - bufPos) < 6) {
                // Read next block
                loadNextFromFile = true;
              } else {
                bufPos                += 4;
                bytesLeftPartialString = (buffer[bufPos] << 8) | buffer[bufPos + 1];
                bufPos                += 2;

                if (!partialString.reserve(bytesLeftPartialString))
                {
                  _state = State::ErrorOnLoad;
                # ifndef BUILD_NO_DEBUG
                  _lastError = strformat(F("KVS: Could not allocate string size %d at readPos %d"), bytesLeftPartialString,
                                         LOG_READPOS_OFFSET);
                  addLog(LOG_LEVEL_ERROR, _lastError);
                # endif // ifndef BUILD_NO_DEBUG
                  return false;
                }
              }
            }

            while (bytesLeftPartialString > 0 && bufPos < readSize) {
              const char c = buffer[bufPos];
              --bytesLeftPartialString;
              ++bufPos;

              if (c != '\0') {
                partialString += c;
              } else if (bytesLeftPartialString > 0) {
                // What to do here if bytesLeftPartialString != 0 ????
                _state = State::ErrorOnLoad;
              # ifndef BUILD_NO_DEBUG
                _lastError = strformat(F("KVS: Unexpected end-of-string, expected %d more at readPos %d"),
                                       bytesLeftPartialString,
                                       LOG_READPOS_OFFSET);
                addLog(LOG_LEVEL_ERROR, _lastError);
              # endif // ifndef BUILD_NO_DEBUG
                return false;
              }
            }

            if ((bytesLeftPartialString == 0) && (partialString.length() > 0)) {
              setValue(key, std::move(partialString));
              partialString.clear();

              //            ++bufPos; // Skip over nul-termination char
            }
            break;

          }

          default:
          {
            if ((readSize - bufPos) < sizePerType) {
              // Read next block
              loadNextFromFile = true;
            } else {
              bufPos += 4;
              const size_t bytesLeftPerType = sizePerType - 4;

              if (bytesLeftPerType == 8) {
                // 8-byte value type
                const ESPEasy_key_value_store_8byte_data_t data_8byte(buf_start + bufPos);
                setValue(storageType, key, data_8byte);
              } else if (bytesLeftPerType > 0) {
                if (bytesLeftPerType == 4) {
                  const ESPEasy_key_value_store_4byte_data_t data_4byte(buf_start + bufPos);
                  setValue(storageType, key, data_4byte);
                } else {
                  ESPEasy_key_value_store_4byte_data_t data_4byte{};

                  // FIXME TD-er: Need to check the byte order
                  memcpy(data_4byte.getBinary(), buf_start + bufPos, bytesLeftPerType);
                  setValue(storageType, key, data_4byte);
                }
              }
              bufPos += bytesLeftPerType;
            }

            break;
          }
        }
      }

      if ((bufPos_start_loop == bufPos) && !loadNextFromFile) {
        _state = State::ErrorOnLoad;
        # ifndef BUILD_NO_DEBUG
        _lastError = strformat(F("KVS: BUG! Did not increment bufPos while processing KVS_StorageType::Enum %d at readPos %d"),
                               static_cast<int>(storageType), LOG_READPOS_OFFSET);
        addLog(LOG_LEVEL_ERROR, _lastError);
        # endif // ifndef BUILD_NO_DEBUG
        return false;
      }
      readPos += (bufPos - bufPos_start_loop);
      # undef LOG_READPOS_OFFSET
    }
  }

  if (timePassedSince(start) >= 1000) {
    _state = State::ErrorOnLoad;
# ifndef BUILD_NO_DEBUG
    _lastError = strformat(
      F("KVS: Timeout! bufPos while processing KVS_StorageType::Enum %d at readPos %d, startChecksumPos: %d"),
      static_cast<int>(storageType), readPos, startChecksumPos);
    addLog(LOG_LEVEL_ERROR, _lastError);
    dump();
# endif // ifndef BUILD_NO_DEBUG
    return false;

  }

  // TODO TD-er: Read checksum
  _state = State::NotChanged;
  return true;
}

bool ESPEasy_key_value_store::store(
  SettingsType::Enum settingsType,
  int                index,
  uint32_t           offset_in_block,
  uint16_t           id_to_match)
{
  if (getState() == State::NotChanged) {
    # ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_INFO, F("KVS: Content not changed, no need to save"));
    # endif
    return true;
  }

  // FIXME TD-er: Must add some check to see if the existing data has changed before saving.
  int offset, max_size;

  if (!SettingsType::getSettingsParameters(settingsType, index, offset, max_size))
  {
    _state = State::ErrorOnSave;
    # ifndef BUILD_NO_DEBUG
    _lastError = F("KVS: Invalid index");
    addLog(LOG_LEVEL_ERROR, F("KVS: Invalid index")); // addLog(LOG_LEVEL_DEBUG, _lastError);
    # endif // ifndef BUILD_NO_DEBUG
    return false;
  }

  # ifdef ESP8266
  uint16_t bufferSize = 256;
  # endif // ifdef ESP8266
  # ifdef ESP32
  uint16_t bufferSize = 1024;
  # endif // ifdef ESP32

  if (bufferSize > max_size) {
    bufferSize = max_size;
  }

  std::vector<uint8_t> buffer;
  buffer.resize(bufferSize, 0);
  uint8_t*buf_start = &(buffer[0]);

  String result;

  size_t writePos = offset_in_block;
  size_t bufPos   = 0;

  // Write header
  buffer[bufPos++] = 0; // TODO TD-er: Add version
  buffer[bufPos++] = id_to_match & 0xFF;
  buffer[bufPos++] = (id_to_match >> 8) & 0xFF;
  const size_t payloadSize      = getPayloadStorageSize();
  const size_t totalSize        = 6 + payloadSize + 16; // header + payload + checksum
  const size_t startChecksumPos = offset_in_block + 4 + payloadSize;

  if ((offset_in_block + totalSize) > static_cast<size_t>(max_size)) {
    _state = State::ErrorOnSave;
    # ifndef BUILD_NO_DEBUG
    _lastError = strformat(F("KVS: Total size %d + offset %d exceeds max size %d"), totalSize, offset_in_block, max_size);
    addLog(LOG_LEVEL_ERROR, _lastError); // addLog(LOG_LEVEL_DEBUG, _lastError);
    # endif // ifndef BUILD_NO_DEBUG
    return false;
  }

  buffer[bufPos++] = payloadSize & 0xFF;
  buffer[bufPos++] = (payloadSize >> 8) & 0xFF;
  buffer[bufPos++] = (payloadSize >> 16) & 0xFF;

  bool mustFlushToFile       = false;
  auto it_str                = _string_data.begin();
  auto it_4byte              = _4byte_data.begin();
  auto it_8byte              = _8byte_data.begin();
  int  pos_in_partial_string = -1;

  while (writePos < startChecksumPos) {
    if (it_str != _string_data.end()) {
      // Write strings

      /*
         const size_t expectedSize =
         4 +                     // key
         2 +                     // length
         it_str->second.length() // string
       + 1;                    // nul-terminator
       */
      const uint16_t strLength = it_str->second.length();

      if (pos_in_partial_string == -1) {
        // Need to write key + length
        if ((buffer.size() - bufPos) < 6) {
          mustFlushToFile = true;
        } else {
          ESPEasy_key_value_store_4byte_data_t key_data;
          key_data.setUint32(it_str->first);
          memcpy(buf_start + bufPos, key_data.getBinary(), 4);
          bufPos          += 4;
          buffer[bufPos++] = strLength >> 8;
          buffer[bufPos++] = strLength & 0xFF;

          // Now we're at the start of the string
          pos_in_partial_string = 0;
        }
      }

      if (!mustFlushToFile) {
        mustFlushToFile = bufPos == buffer.size();

        while (pos_in_partial_string >= 0 &&
               pos_in_partial_string < strLength &&
               !mustFlushToFile)
        {
          buffer[bufPos++] = it_str->second[pos_in_partial_string];
          ++pos_in_partial_string;
          mustFlushToFile = bufPos == buffer.size();
        }

        if ((pos_in_partial_string == strLength) &&
            !mustFlushToFile)
        {
          // Write 0-termination + move to next string
          //          buffer[bufPos++]      = 0;
          pos_in_partial_string = -1;
          ++it_str;
        }
      }

    } else if (it_8byte != _8byte_data.end()) {
      // Write 8-byte data
      if ((buffer.size() - bufPos) < 12) {
        mustFlushToFile = true;
      } else {
        ESPEasy_key_value_store_4byte_data_t key_data;
        key_data.setUint32(it_8byte->first);
        memcpy(buf_start + bufPos, key_data.getBinary(),         4);
        bufPos += 4;

        memcpy(buf_start + bufPos, it_8byte->second.getBinary(), 8);
        bufPos += 8;
        ++it_8byte;
      }

    } else if (it_4byte != _4byte_data.end()) {
      // Write 4-byte data
      uint32_t combined_key                            = it_4byte->first;
      KVS_StorageType::Enum storageType = KVS_StorageType::get_StorageType_from_combined_key(combined_key);
      const uint32_t key                               = KVS_StorageType::getKey_from_combined_key(combined_key);
      const size_t   sizePerType                       = KVS_StorageType::getStorageSizePerType(storageType);

      if (sizePerType >= 4) {
        if ((buffer.size() - bufPos) < sizePerType) {
          mustFlushToFile = true;
        } else {

          switch (storageType)
          {
            case KVS_StorageType::Enum::bool_false:
            case KVS_StorageType::Enum::bool_true:
            case KVS_StorageType::Enum::bool_type:
            {
              bool value{};
              getValue(key, value);
              combined_key = KVS_StorageType::combine_StorageType_and_key(
                value
                      ? KVS_StorageType::Enum::bool_true
                      : KVS_StorageType::Enum::bool_false
                , key);
              break;
            }
            default:
              break;

          }
          ESPEasy_key_value_store_4byte_data_t key_data;
          key_data.setUint32(combined_key);
          memcpy(buf_start + bufPos, key_data.getBinary(), 4);
          bufPos += 4;

          const size_t bytesLeftPerType = sizePerType - 4;

          if (bytesLeftPerType != 0) {
            if (bytesLeftPerType == 4) {
              memcpy(buf_start + bufPos, it_4byte->second.getBinary(), bytesLeftPerType);
            } else {
              // FIXME TD-er: Need to check the byte order
              memcpy(buf_start + bufPos, it_4byte->second.getBinary(), bytesLeftPerType);
            }
            bufPos += bytesLeftPerType;
            ++it_4byte;
          }

        }
      } else {
        _state = State::ErrorOnSave;
        # ifndef BUILD_NO_DEBUG
        _lastError = strformat(F("KVS: BUG! Should not have storage type %d stored with 0 bytes at writePos %d"),
                               static_cast<int>(storageType), writePos + bufPos);
        addLog(LOG_LEVEL_ERROR, _lastError);
        # endif // ifndef BUILD_NO_DEBUG
        return false;
      }
    } else {
      mustFlushToFile = true;
    }

    if (mustFlushToFile) {
      result         += SaveToFile(settingsType, index, buf_start, bufPos, writePos);
      writePos       += bufPos;
      mustFlushToFile = false;
      bufPos          = 0;

      for (size_t i = 0; i < buffer.size(); ++i) {
        buffer[i] = 0;
      }
    }
  }

  if (bufPos) {

    // Some kind of error??? We should be done and
    // writePos should be equal to startChecksumPos

    result         += SaveToFile(settingsType, index, buf_start, bufPos, writePos);
    writePos       += bufPos;
    mustFlushToFile = false;
    bufPos          = 0;
  }

  // Consider a successful save the same as a fresh load.
  // The data is now the same as what is stored
# ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, strformat(F("KVS: Written %d bytes"), writePos));
# endif
  _state = State::NotChanged;
  dump();
  return true;
}

size_t ESPEasy_key_value_store::getPayloadStorageSize() const
{
  size_t res{};

  for (auto it = _string_data.begin(); it != _string_data.end(); ++it)
  {
    res += 4;                   // key size
    res += 2;                   // string length
    res += it->second.length(); // + 1; // include 0-termination
  }

  for (auto it = _4byte_data.begin(); it != _4byte_data.end(); ++it)
  {
    res += KVS_StorageType::getStorageSizePerType(
      KVS_StorageType::get_StorageType_from_combined_key(it->first));
  }

  for (auto it = _8byte_data.begin(); it != _8byte_data.end(); ++it)
  {
    res += KVS_StorageType::getStorageSizePerType(
      KVS_StorageType::get_StorageType_from_combined_key(it->first));
  }
  return res;
}

bool ESPEasy_key_value_store::hasKey(KVS_StorageType::Enum storageType, uint32_t key) const
{
  if (!hasStorageType(storageType)) { return false; }

  const uint32_t combined_key = KVS_StorageType::combine_StorageType_and_key(storageType, key);

  if (storageType == KVS_StorageType::Enum::string_type)
  {
    return _string_data.find(combined_key) != _string_data.end();
  }
  const size_t size = KVS_StorageType::getStorageSizePerType(storageType);

  if (size == 12)
  {
    return _8byte_data.find(combined_key) != _8byte_data.end();
  }

  if (size >= 4) {
    return _4byte_data.find(combined_key) != _4byte_data.end();
  }
  return false;
}

KVS_StorageType::Enum ESPEasy_key_value_store::getStorageType(uint32_t key) const
{
  uint32_t cached_bitmap = _storage_type_present_cache;

  // Search all storage types, skipping the 'not_set'
  for (size_t i = 0;
       cached_bitmap &&
       i < static_cast<size_t>(KVS_StorageType::Enum::MAX_Type);
       ++i)
  {
    const KVS_StorageType::Enum res = static_cast<KVS_StorageType::Enum>(i);

    if (hasKey(res, key)) { return res; }
    cached_bitmap >>= 1;
  }
  return KVS_StorageType::Enum::not_set;
}
#define KVS_STRINGPAIR_SEPARATOR ((char)1)
bool ESPEasy_key_value_store::getValue(uint32_t key, StringPair& stringPair) const
{
  String str;
  if (!getValue(key, str)) { 
    return false;
  }
  const int separatorPos = str.indexOf(KVS_STRINGPAIR_SEPARATOR);
  if (separatorPos < 0) return false;
  stringPair.first = str.substring(0, separatorPos);
  stringPair.second = str.substring(separatorPos + 1);
  return true;
}

void ESPEasy_key_value_store::setValue(uint32_t key, const StringPair& stringPair)
{
  String str;
  str.reserve(stringPair.first.length() + 1 + stringPair.second.length());
  str += stringPair.first;
  str += KVS_STRINGPAIR_SEPARATOR;
  str += stringPair.second;
  setValue(key, str);
}
#undef KVS_STRINGPAIR_SEPARATOR

bool ESPEasy_key_value_store::getValue(uint32_t key, String& value) const
{
  const uint32_t combined_key = KVS_StorageType::combine_StorageType_and_key(
    KVS_StorageType::Enum::string_type,
    key);

  auto it = _string_data.find(combined_key);

  if (it == _string_data.end()) { return false; }
  value = it->second;
  return true;
}

void ESPEasy_key_value_store::setValue(uint32_t key, const String& value) { setValue(key, String(value)); }

void ESPEasy_key_value_store::setValue(uint32_t                  key,
                                       const __FlashStringHelper*value) { setValue(key, String(value)); }

void ESPEasy_key_value_store::setValue(uint32_t key, String&& value)
{
  const uint32_t combined_key = KVS_StorageType::combine_StorageType_and_key(
    KVS_StorageType::Enum::string_type,
    key);
  auto it = _string_data.find(combined_key);

  if (it == _string_data.end()) {
    // Does not yet exist.
    _string_data.emplace(combined_key, std::move(value));
    _state = State::Changed;
  } else {
    if (!it->second.equals(value)) {
      it->second = std::move(value);
      _state     = State::Changed;
    }
  }
  setHasStorageType(KVS_StorageType::Enum::string_type);
}

ESPEasy_key_value_store::map_4byte_data::const_iterator ESPEasy_key_value_store::get4byteIterator(
  KVS_StorageType::Enum storageType,
  uint32_t                             key) const
{
  if (!hasStorageType(storageType)) { return _4byte_data.end(); }
  const uint32_t combined_key = KVS_StorageType::combine_StorageType_and_key(storageType, key);
  return _4byte_data.find(combined_key);
}

ESPEasy_key_value_store::map_8byte_data::const_iterator ESPEasy_key_value_store::get8byteIterator(
  KVS_StorageType::Enum storageType,
  uint32_t                             key) const
{
  if (!hasStorageType(storageType)) { return _8byte_data.end(); }
  const uint32_t combined_key = KVS_StorageType::combine_StorageType_and_key(storageType, key);
  return _8byte_data.find(combined_key);
}

# define GET_4BYTE_TYPE(T, GF)                                                    \
        auto it = get4byteIterator(KVS_StorageType::Enum::T, key); \
        if (it == _4byte_data.end()) return false;                                \
        value = it->second.GF();                                                  \
        return true;                                                              \

# define GET_8BYTE_TYPE(T, GF)                                                    \
        auto it = get8byteIterator(KVS_StorageType::Enum::T, key); \
        if (it == _8byte_data.end()) return false;                                \
        value = it->second.GF();                                                  \
        return true;                                                              \


# define SET_4BYTE_TYPE(T, SF)                                            \
        const uint32_t combined_key = KVS_StorageType::combine_StorageType_and_key(        \
          KVS_StorageType::Enum::T, key);                  \
        if (_4byte_data[combined_key].SF(value)) _state = State::Changed; \
        setHasStorageType(KVS_StorageType::Enum::T);


# define SET_8BYTE_TYPE(T, SF)                                            \
        const uint32_t combined_key = KVS_StorageType::combine_StorageType_and_key(        \
          KVS_StorageType::Enum::T, key);                  \
        if (_8byte_data[combined_key].SF(value)) _state = State::Changed; \
        setHasStorageType(KVS_StorageType::Enum::T);

bool ESPEasy_key_value_store::getValue(uint32_t key, int8_t& value) const
{
  GET_4BYTE_TYPE(int8_type, getInt32)
}

void ESPEasy_key_value_store::setValue(uint32_t key, const int8_t& value) { SET_4BYTE_TYPE(int8_type, setInt32) }

bool ESPEasy_key_value_store::getValue(uint32_t key, uint8_t& value) const
{
  GET_4BYTE_TYPE(uint8_type, getUint32);
}

void ESPEasy_key_value_store::setValue(uint32_t key, const uint8_t& value) { SET_4BYTE_TYPE(uint8_type, setUint32); }

bool ESPEasy_key_value_store::getValue(uint32_t key, int16_t& value) const
{
  GET_4BYTE_TYPE(int16_type, getInt32);
}

void ESPEasy_key_value_store::setValue(uint32_t key, const int16_t& value) { SET_4BYTE_TYPE(int16_type, setInt32); }

bool ESPEasy_key_value_store::getValue(uint32_t key, uint16_t& value) const
{
  GET_4BYTE_TYPE(uint16_type, getUint32);

}

void ESPEasy_key_value_store::setValue(uint32_t key, const uint16_t& value) { SET_4BYTE_TYPE(uint16_type, setUint32); }

bool ESPEasy_key_value_store::getValue(uint32_t key, int32_t& value) const
{
  GET_4BYTE_TYPE(int32_type, getInt32);
}

void ESPEasy_key_value_store::setValue(uint32_t key, const int32_t& value) { SET_4BYTE_TYPE(int32_type, setInt32); }

bool ESPEasy_key_value_store::getValue(uint32_t key, uint32_t& value) const
{
  GET_4BYTE_TYPE(uint32_type, getUint32);
}

void ESPEasy_key_value_store::setValue(uint32_t key, const uint32_t& value) { SET_4BYTE_TYPE(uint32_type, setUint32); }

bool ESPEasy_key_value_store::getValue(uint32_t key, int64_t& value) const
{
  GET_8BYTE_TYPE(int64_type, getInt64);
}

void ESPEasy_key_value_store::setValue(uint32_t key, const int64_t& value) { SET_8BYTE_TYPE(int64_type, setInt64); }

bool ESPEasy_key_value_store::getValue(uint32_t key, uint64_t& value) const
{
  GET_8BYTE_TYPE(uint64_type, getUint64);
}

void ESPEasy_key_value_store::setValue(uint32_t key, const uint64_t& value) { SET_8BYTE_TYPE(uint64_type, setUint64); }

bool ESPEasy_key_value_store::getValue(uint32_t key, float& value) const
{
  GET_4BYTE_TYPE(float_type, getFloat);
}

void ESPEasy_key_value_store::setValue(uint32_t key, const float& value) { SET_4BYTE_TYPE(float_type, setFloat); }

bool ESPEasy_key_value_store::getValue(uint32_t key, double& value) const
{
  GET_8BYTE_TYPE(double_type, getDouble);
}

void ESPEasy_key_value_store::setValue(uint32_t key, const double& value) { SET_8BYTE_TYPE(double_type, setDouble); }

bool ESPEasy_key_value_store::getValue(uint32_t key, bool& boolValue) const
{
  boolValue = false;
  uint32_t value{};
  auto     it = get4byteIterator(KVS_StorageType::Enum::bool_type, key);

  if (it == _4byte_data.end()) { return false; }
  value     = it->second.getUint32();
  boolValue = value != 0u;
  return true;
}

void ESPEasy_key_value_store::setValue(uint32_t key, const bool& boolValue) {
  uint32_t value = boolValue ? 1 : 0;
  SET_4BYTE_TYPE(bool_type, setUint32);
}

bool ESPEasy_key_value_store::getValueAsString(const KVS_StorageType::Enum& storageType, uint32_t key, String& value) const
{
  if (!hasKey(storageType, key)) { return false; }

  switch (storageType)
  {
    case KVS_StorageType::Enum::string_type:
      return getValue(key, value);

    case KVS_StorageType::Enum::bool_type:
    case KVS_StorageType::Enum::int8_type:
    case KVS_StorageType::Enum::uint8_type:
    case KVS_StorageType::Enum::int16_type:
    case KVS_StorageType::Enum::uint16_type:
    case KVS_StorageType::Enum::int32_type:
    case KVS_StorageType::Enum::uint32_type:
    case KVS_StorageType::Enum::int64_type:
    {
      int64_t int64_value{};

      if (getValueAsInt(key, int64_value)) {
        value = ll2String(int64_value);
        return true;
      }
      break;
    }

    case KVS_StorageType::Enum::float_type:
    {
      float v{};

      if (getValue(key, v)) {
        value = floatToString(v, 6, true);
        return true;
      }
      break;
    }

    case KVS_StorageType::Enum::uint64_type:
    {
      uint64_t v{};

      if (getValue(key, v)) {
        value = ull2String(v);
        return true;
      }
      break;
    }

    case KVS_StorageType::Enum::double_type:
    {
      double v{};

      if (getValue(key, v)) {
        value = doubleToString(v, 6, true);
        return true;
      }
      break;
    }

    case KVS_StorageType::Enum::not_set:
    case KVS_StorageType::Enum::bool_true:
    case KVS_StorageType::Enum::bool_false:
    case KVS_StorageType::Enum::binary:
    case KVS_StorageType::Enum::MAX_Type:
      break;
  }
  return false;
}

bool ESPEasy_key_value_store::getValueAsString(uint32_t key, String& value) const
{
  return getValueAsString(getStorageType(key), key, value);
}

# define GET_TYPE_AS_INT64(T, CT)                       \
          case KVS_StorageType::Enum::T: \
            {                                           \
              CT v{};                                   \
              if (getValue(key, v)) {                   \
                value = v;                              \
                return true;                            \
              }                                         \
              break;                                    \
            }

bool ESPEasy_key_value_store::getValueAsInt(
  uint32_t key,
  int64_t& value) const
{
  auto storageType = getStorageType(key);

  if (!hasKey(storageType, key)) { return false; }

  switch (storageType)
  {
    case KVS_StorageType::Enum::bool_type:
    case KVS_StorageType::Enum::bool_true:
    case KVS_StorageType::Enum::bool_false:
    {
      bool v{};

      if (getValue(key, v))
      {
        value = v ? 1 : 0;
        return true;
      }
      break;
    }
      GET_TYPE_AS_INT64(int8_type,   int8_t)
      GET_TYPE_AS_INT64(uint8_type,  uint8_t)
      GET_TYPE_AS_INT64(int16_type,  int16_t)
      GET_TYPE_AS_INT64(uint16_type, uint16_t)
      GET_TYPE_AS_INT64(int32_type,  int32_t)
      GET_TYPE_AS_INT64(uint32_type, uint32_t)
      GET_TYPE_AS_INT64(float_type,  float)
      GET_TYPE_AS_INT64(uint64_type, uint64_t)
      GET_TYPE_AS_INT64(double_type, double)

    case KVS_StorageType::Enum::int64_type:
      return getValue(key, value);
    default: break;
  }
  return false;
}

int64_t ESPEasy_key_value_store::getValueAsInt_or_default(uint32_t key, int64_t default_value) const
{
  int64_t value = default_value;

  if (getValueAsInt(key, value)) { return value; }
  return default_value;
}

int64_t ESPEasy_key_value_store::getValueAsInt(uint32_t key) const
{
  return getValueAsInt_or_default(key, 0);
}

# define GET_4BYTE_INT_TYPE_FROM_STRING(T, CT)          \
          case KVS_StorageType::Enum::T: \
            {                                           \
              CT v(value.toInt());                      \
              setValue(key, v);                         \
              break;                                    \
            }

void ESPEasy_key_value_store::setValue(const KVS_StorageType::Enum& storageType, uint32_t key, const String& value)
{
  switch (storageType)
  {
    case KVS_StorageType::Enum::string_type:
      setValue(key, value);
      break;
      GET_4BYTE_INT_TYPE_FROM_STRING(int8_type,   int8_t)
      GET_4BYTE_INT_TYPE_FROM_STRING(uint8_type,  uint8_t)
      GET_4BYTE_INT_TYPE_FROM_STRING(int16_type,  int16_t)
      GET_4BYTE_INT_TYPE_FROM_STRING(uint16_type, uint16_t)
      GET_4BYTE_INT_TYPE_FROM_STRING(int32_type,  int32_t)
      GET_4BYTE_INT_TYPE_FROM_STRING(uint32_type, uint32_t)
    case KVS_StorageType::Enum::double_type:
      setValue(key, value.toDouble());
      break;
    case KVS_StorageType::Enum::float_type:
      setValue(key, value.toFloat());
      break;
    case KVS_StorageType::Enum::int64_type:
    {
      // TODO TD-er: Implement
      break;
    }

    case KVS_StorageType::Enum::uint64_type:
    {
      // TODO TD-er: Implement
      break;
    }
    case KVS_StorageType::Enum::bool_type:
    case KVS_StorageType::Enum::bool_true:
    case KVS_StorageType::Enum::bool_false:
    {
      const bool val = !(value.equalsIgnoreCase(F("false")) || value.equals(F("0")));
      setValue(key, val);
      break;
    }
    default: return;
  }
  setHasStorageType(storageType);
}

bool ESPEasy_key_value_store::getValue(
  KVS_StorageType::Enum                         & storageType,
  uint32_t                              key,
  ESPEasy_key_value_store_4byte_data_t& value) const
{
  auto it = _4byte_data.find(KVS_StorageType::combine_StorageType_and_key(storageType, key));

  if (it == _4byte_data.end()) { return false; }
  memcpy(value.getBinary(), it->second.getBinary(), 4);
  return true;
}

void ESPEasy_key_value_store::setValue(
  KVS_StorageType::Enum                               & storageType,
  uint32_t                                    key,
  const ESPEasy_key_value_store_4byte_data_t& value)
{
  auto combined_key = KVS_StorageType::combine_StorageType_and_key(storageType, key);
  auto it           = _4byte_data.find(combined_key);

  if (it == _4byte_data.end()) {
    // new entry
    _4byte_data.emplace(combined_key, value);
    _state = State::Changed;
  } else {
    if (it->second.set(value.getBinary())) { _state = State::Changed; }
  }
  setHasStorageType(storageType);
}

bool ESPEasy_key_value_store::getValue(
  KVS_StorageType::Enum                         & storageType,
  uint32_t                              key,
  ESPEasy_key_value_store_8byte_data_t& value) const
{
  auto it = _8byte_data.find(KVS_StorageType::combine_StorageType_and_key(storageType, key));

  if (it == _8byte_data.end()) { return false; }
  memcpy(value.getBinary(), it->second.getBinary(), 8);
  return true;
}

void ESPEasy_key_value_store::setValue(
  KVS_StorageType::Enum                               & storageType,
  uint32_t                                    key,
  const ESPEasy_key_value_store_8byte_data_t& value)
{
  auto combined_key = KVS_StorageType::combine_StorageType_and_key(storageType, key);
  auto it           = _8byte_data.find(combined_key);

  if (it == _8byte_data.end()) {
    // new entry
    _8byte_data.emplace(combined_key, value);
    _state = State::Changed;
  } else {
    if (it->second.set(value.getBinary())) { _state = State::Changed; }
  }
  setHasStorageType(storageType);
}

bool ESPEasy_key_value_store::hasStorageType(KVS_StorageType::Enum storageType) const
{
  //    return true;
  if ((storageType == KVS_StorageType::Enum::bool_true) ||
      (storageType == KVS_StorageType::Enum::bool_false)) {
    storageType = KVS_StorageType::Enum::bool_type;
  }

  const uint32_t bitnr         = static_cast<uint32_t>(storageType);
  constexpr uint32_t max_bitnr = static_cast<uint32_t>(KVS_StorageType::Enum::MAX_Type);

  if (bitnr >= max_bitnr) { return false; }
  return bitRead(_storage_type_present_cache, bitnr);
}

void ESPEasy_key_value_store::setHasStorageType(KVS_StorageType::Enum storageType)
{
  if ((storageType == KVS_StorageType::Enum::bool_true) ||
      (storageType == KVS_StorageType::Enum::bool_false)) {
    storageType = KVS_StorageType::Enum::bool_type;
  }
  const uint32_t bitnr         = static_cast<uint32_t>(storageType);
  constexpr uint32_t max_bitnr = static_cast<uint32_t>(KVS_StorageType::Enum::MAX_Type);

  if (bitnr < max_bitnr) {
    if (!bitRead(_storage_type_present_cache, bitnr)) {
      _state = State::Changed;
      bitSet(_storage_type_present_cache, bitnr);
    }
  }

  // TODO TD-er: Whenever this is called, there has been a change, so invalidate checksum
}

void ESPEasy_key_value_store::dump() const
{
  addLog(LOG_LEVEL_INFO, strformat(F("KVS: Payload Storage size : %d"), getPayloadStorageSize()));

  for (auto it = _string_data.begin(); it != _string_data.end(); ++it)
  {

    String val;

    if (!getValueAsString(
          KVS_StorageType::get_StorageType_from_combined_key(it->first),
          KVS_StorageType::getKey_from_combined_key(it->first),
          val)) {
      val = '-';
    }

    addLog(LOG_LEVEL_INFO, strformat(
             F("KVS: type: %d, combined-key: %x, key: %d, value: '%s' '%s'"),
             KVS_StorageType::get_StorageType_from_combined_key(it->first),
             it->first, KVS_StorageType::getKey_from_combined_key(it->first),
             it->second.c_str(),
             val.c_str()));

  }

  for (auto it = _4byte_data.begin(); it != _4byte_data.end(); ++it)
  {
    String val;

    if (!getValueAsString(
          KVS_StorageType::get_StorageType_from_combined_key(it->first),
          KVS_StorageType::getKey_from_combined_key(it->first),
          val)) {
      val = '-';
    }

    addLog(LOG_LEVEL_INFO, strformat(
             F("KVS: type: %d, comb: %x, key: %d, val: '%x %x %x %x' strval: '%s'"),
             KVS_StorageType::get_StorageType_from_combined_key(it->first),
             it->first, KVS_StorageType::getKey_from_combined_key(it->first),
             it->second.getBinary()[0],
             it->second.getBinary()[1],
             it->second.getBinary()[2],
             it->second.getBinary()[3],
             val.c_str()
             ));

  }

  for (auto it = _8byte_data.begin(); it != _8byte_data.end(); ++it)
  {
    String val;

    if (!getValueAsString(
          KVS_StorageType::get_StorageType_from_combined_key(it->first),
          KVS_StorageType::getKey_from_combined_key(it->first),
          val)) {
      val = '-';
    }

    addLog(LOG_LEVEL_INFO, strformat(
             F("KVS: type: %d, comb: %x, key: %d, val: '%x %x %x %x  %x %x %x %x' strval: '%s'"),
             KVS_StorageType::get_StorageType_from_combined_key(it->first),
             it->first, KVS_StorageType::getKey_from_combined_key(it->first),
             it->second.getBinary()[0],
             it->second.getBinary()[1],
             it->second.getBinary()[2],
             it->second.getBinary()[3],
             it->second.getBinary()[4],
             it->second.getBinary()[5],
             it->second.getBinary()[6],
             it->second.getBinary()[7],
             val.c_str()
             ));

  }

}


#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE

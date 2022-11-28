// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2022, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Deserialization/deserialize.hpp>
#include <ArduinoJson/Json/EscapeSequence.hpp>
#include <ArduinoJson/Json/Latch.hpp>
#include <ArduinoJson/Json/Utf16.hpp>
#include <ArduinoJson/Json/Utf8.hpp>
#include <ArduinoJson/Memory/MemoryPool.hpp>
#include <ArduinoJson/Numbers/parseNumber.hpp>
#include <ArduinoJson/Polyfills/assert.hpp>
#include <ArduinoJson/Polyfills/type_traits.hpp>
#include <ArduinoJson/Variant/VariantData.hpp>

namespace ARDUINOJSON_NAMESPACE {

template <typename TReader, typename TStringStorage>
class JsonDeserializer {
 public:
  JsonDeserializer(MemoryPool* pool, TReader reader,
                   TStringStorage stringStorage)
      : _stringStorage(stringStorage),
        _foundSomething(false),
        _latch(reader),
        _pool(pool) {}

  template <typename TFilter>
  DeserializationError parse(VariantData& variant, TFilter filter,
                             NestingLimit nestingLimit) {
    DeserializationError::Code err;

    err = parseVariant(variant, filter, nestingLimit);

    if (!err && _latch.last() != 0 && !variant.isEnclosed()) {
      // We don't detect trailing characters earlier, so we need to check now
      return DeserializationError::InvalidInput;
    }

    return err;
  }

 private:
  char current() {
    return _latch.current();
  }

  void move() {
    _latch.clear();
  }

  bool eat(char charToSkip) {
    if (current() != charToSkip)
      return false;
    move();
    return true;
  }

  template <typename TFilter>
  DeserializationError::Code parseVariant(VariantData& variant, TFilter filter,
                                          NestingLimit nestingLimit) {
    DeserializationError::Code err;

    err = skipSpacesAndComments();
    if (err)
      return err;

    switch (current()) {
      case '[':
        if (filter.allowArray())
          return parseArray(variant.toArray(), filter, nestingLimit);
        else
          return skipArray(nestingLimit);

      case '{':
        if (filter.allowObject())
          return parseObject(variant.toObject(), filter, nestingLimit);
        else
          return skipObject(nestingLimit);

      case '\"':
      case '\'':
        if (filter.allowValue())
          return parseStringValue(variant);
        else
          return skipQuotedString();

      case 't':
        if (filter.allowValue())
          variant.setBoolean(true);
        return skipKeyword("true");

      case 'f':
        if (filter.allowValue())
          variant.setBoolean(false);
        return skipKeyword("false");

      case 'n':
        // the variant should already by null, except if the same object key was
        // used twice, as in {"a":1,"a":null}
        return skipKeyword("null");

      default:
        if (filter.allowValue())
          return parseNumericValue(variant);
        else
          return skipNumericValue();
    }
  }

  DeserializationError::Code skipVariant(NestingLimit nestingLimit) {
    DeserializationError::Code err;

    err = skipSpacesAndComments();
    if (err)
      return err;

    switch (current()) {
      case '[':
        return skipArray(nestingLimit);

      case '{':
        return skipObject(nestingLimit);

      case '\"':
      case '\'':
        return skipQuotedString();

      case 't':
        return skipKeyword("true");

      case 'f':
        return skipKeyword("false");

      case 'n':
        return skipKeyword("null");

      default:
        return skipNumericValue();
    }
  }

  template <typename TFilter>
  DeserializationError::Code parseArray(CollectionData& array, TFilter filter,
                                        NestingLimit nestingLimit) {
    DeserializationError::Code err;

    if (nestingLimit.reached())
      return DeserializationError::TooDeep;

    // Skip opening braket
    ARDUINOJSON_ASSERT(current() == '[');
    move();

    // Skip spaces
    err = skipSpacesAndComments();
    if (err)
      return err;

    // Empty array?
    if (eat(']'))
      return DeserializationError::Ok;

    TFilter memberFilter = filter[0UL];

    // Read each value
    for (;;) {
      if (memberFilter.allow()) {
        // Allocate slot in array
        VariantData* value = array.addElement(_pool);
        if (!value)
          return DeserializationError::NoMemory;

        // 1 - Parse value
        err = parseVariant(*value, memberFilter, nestingLimit.decrement());
        if (err)
          return err;
      } else {
        err = skipVariant(nestingLimit.decrement());
        if (err)
          return err;
      }

      // 2 - Skip spaces
      err = skipSpacesAndComments();
      if (err)
        return err;

      // 3 - More values?
      if (eat(']'))
        return DeserializationError::Ok;
      if (!eat(','))
        return DeserializationError::InvalidInput;
    }
  }

  DeserializationError::Code skipArray(NestingLimit nestingLimit) {
    DeserializationError::Code err;

    if (nestingLimit.reached())
      return DeserializationError::TooDeep;

    // Skip opening braket
    ARDUINOJSON_ASSERT(current() == '[');
    move();

    // Read each value
    for (;;) {
      // 1 - Skip value
      err = skipVariant(nestingLimit.decrement());
      if (err)
        return err;

      // 2 - Skip spaces
      err = skipSpacesAndComments();
      if (err)
        return err;

      // 3 - More values?
      if (eat(']'))
        return DeserializationError::Ok;
      if (!eat(','))
        return DeserializationError::InvalidInput;
    }
  }

  template <typename TFilter>
  DeserializationError::Code parseObject(CollectionData& object, TFilter filter,
                                         NestingLimit nestingLimit) {
    DeserializationError::Code err;

    if (nestingLimit.reached())
      return DeserializationError::TooDeep;

    // Skip opening brace
    ARDUINOJSON_ASSERT(current() == '{');
    move();

    // Skip spaces
    err = skipSpacesAndComments();
    if (err)
      return err;

    // Empty object?
    if (eat('}'))
      return DeserializationError::Ok;

    // Read each key value pair
    for (;;) {
      // Parse key
      err = parseKey();
      if (err)
        return err;

      // Skip spaces
      err = skipSpacesAndComments();
      if (err)
        return err;

      // Colon
      if (!eat(':'))
        return DeserializationError::InvalidInput;

      String key = _stringStorage.str();

      TFilter memberFilter = filter[key.c_str()];

      if (memberFilter.allow()) {
        VariantData* variant = object.getMember(adaptString(key.c_str()));
        if (!variant) {
          // Save key in memory pool.
          // This MUST be done before adding the slot.
          key = _stringStorage.save();

          // Allocate slot in object
          VariantSlot* slot = object.addSlot(_pool);
          if (!slot)
            return DeserializationError::NoMemory;

          slot->setKey(key);

          variant = slot->data();
        }

        // Parse value
        err = parseVariant(*variant, memberFilter, nestingLimit.decrement());
        if (err)
          return err;
      } else {
        err = skipVariant(nestingLimit.decrement());
        if (err)
          return err;
      }

      // Skip spaces
      err = skipSpacesAndComments();
      if (err)
        return err;

      // More keys/values?
      if (eat('}'))
        return DeserializationError::Ok;
      if (!eat(','))
        return DeserializationError::InvalidInput;

      // Skip spaces
      err = skipSpacesAndComments();
      if (err)
        return err;
    }
  }

  DeserializationError::Code skipObject(NestingLimit nestingLimit) {
    DeserializationError::Code err;

    if (nestingLimit.reached())
      return DeserializationError::TooDeep;

    // Skip opening brace
    ARDUINOJSON_ASSERT(current() == '{');
    move();

    // Skip spaces
    err = skipSpacesAndComments();
    if (err)
      return err;

    // Empty object?
    if (eat('}'))
      return DeserializationError::Ok;

    // Read each key value pair
    for (;;) {
      // Skip key
      err = skipKey();
      if (err)
        return err;

      // Skip spaces
      err = skipSpacesAndComments();
      if (err)
        return err;

      // Colon
      if (!eat(':'))
        return DeserializationError::InvalidInput;

      // Skip value
      err = skipVariant(nestingLimit.decrement());
      if (err)
        return err;

      // Skip spaces
      err = skipSpacesAndComments();
      if (err)
        return err;

      // More keys/values?
      if (eat('}'))
        return DeserializationError::Ok;
      if (!eat(','))
        return DeserializationError::InvalidInput;

      err = skipSpacesAndComments();
      if (err)
        return err;
    }
  }

  DeserializationError::Code parseKey() {
    _stringStorage.startString();
    if (isQuote(current())) {
      return parseQuotedString();
    } else {
      return parseNonQuotedString();
    }
  }

  DeserializationError::Code parseStringValue(VariantData& variant) {
    DeserializationError::Code err;

    _stringStorage.startString();

    err = parseQuotedString();
    if (err)
      return err;

    variant.setString(_stringStorage.save());

    return DeserializationError::Ok;
  }

  DeserializationError::Code parseQuotedString() {
#if ARDUINOJSON_DECODE_UNICODE
    Utf16::Codepoint codepoint;
    DeserializationError::Code err;
#endif
    const char stopChar = current();

    move();
    for (;;) {
      char c = current();
      move();
      if (c == stopChar)
        break;

      if (c == '\0')
        return DeserializationError::IncompleteInput;

      if (c == '\\') {
        c = current();

        if (c == '\0')
          return DeserializationError::IncompleteInput;

        if (c == 'u') {
#if ARDUINOJSON_DECODE_UNICODE
          move();
          uint16_t codeunit;
          err = parseHex4(codeunit);
          if (err)
            return err;
          if (codepoint.append(codeunit))
            Utf8::encodeCodepoint(codepoint.value(), _stringStorage);
#else
          _stringStorage.append('\\');
#endif
          continue;
        }

        // replace char
        c = EscapeSequence::unescapeChar(c);
        if (c == '\0')
          return DeserializationError::InvalidInput;
        move();
      }

      _stringStorage.append(c);
    }

    if (!_stringStorage.isValid())
      return DeserializationError::NoMemory;

    return DeserializationError::Ok;
  }

  DeserializationError::Code parseNonQuotedString() {
    char c = current();
    ARDUINOJSON_ASSERT(c);

    if (canBeInNonQuotedString(c)) {  // no quotes
      do {
        move();
        _stringStorage.append(c);
        c = current();
      } while (canBeInNonQuotedString(c));
    } else {
      return DeserializationError::InvalidInput;
    }

    if (!_stringStorage.isValid())
      return DeserializationError::NoMemory;

    return DeserializationError::Ok;
  }

  DeserializationError::Code skipKey() {
    if (isQuote(current())) {
      return skipQuotedString();
    } else {
      return skipNonQuotedString();
    }
  }

  DeserializationError::Code skipQuotedString() {
    const char stopChar = current();

    move();
    for (;;) {
      char c = current();
      move();
      if (c == stopChar)
        break;
      if (c == '\0')
        return DeserializationError::IncompleteInput;
      if (c == '\\') {
        if (current() != '\0')
          move();
      }
    }

    return DeserializationError::Ok;
  }

  DeserializationError::Code skipNonQuotedString() {
    char c = current();
    while (canBeInNonQuotedString(c)) {
      move();
      c = current();
    }
    return DeserializationError::Ok;
  }

  DeserializationError::Code parseNumericValue(VariantData& result) {
    uint8_t n = 0;

    char c = current();
    while (canBeInNumber(c) && n < 63) {
      move();
      _buffer[n++] = c;
      c = current();
    }
    _buffer[n] = 0;

    if (!parseNumber(_buffer, result))
      return DeserializationError::InvalidInput;

    return DeserializationError::Ok;
  }

  DeserializationError::Code skipNumericValue() {
    char c = current();
    while (canBeInNumber(c)) {
      move();
      c = current();
    }
    return DeserializationError::Ok;
  }

  DeserializationError::Code parseHex4(uint16_t& result) {
    result = 0;
    for (uint8_t i = 0; i < 4; ++i) {
      char digit = current();
      if (!digit)
        return DeserializationError::IncompleteInput;
      uint8_t value = decodeHex(digit);
      if (value > 0x0F)
        return DeserializationError::InvalidInput;
      result = uint16_t((result << 4) | value);
      move();
    }
    return DeserializationError::Ok;
  }

  static inline bool isBetween(char c, char min, char max) {
    return min <= c && c <= max;
  }

  static inline bool canBeInNumber(char c) {
    return isBetween(c, '0', '9') || c == '+' || c == '-' || c == '.' ||
#if ARDUINOJSON_ENABLE_NAN || ARDUINOJSON_ENABLE_INFINITY
           isBetween(c, 'A', 'Z') || isBetween(c, 'a', 'z');
#else
           c == 'e' || c == 'E';
#endif
  }

  static inline bool canBeInNonQuotedString(char c) {
    return isBetween(c, '0', '9') || isBetween(c, '_', 'z') ||
           isBetween(c, 'A', 'Z');
  }

  static inline bool isQuote(char c) {
    return c == '\'' || c == '\"';
  }

  static inline uint8_t decodeHex(char c) {
    if (c < 'A')
      return uint8_t(c - '0');
    c = char(c & ~0x20);  // uppercase
    return uint8_t(c - 'A' + 10);
  }

  DeserializationError::Code skipSpacesAndComments() {
    for (;;) {
      switch (current()) {
        // end of string
        case '\0':
          return _foundSomething ? DeserializationError::IncompleteInput
                                 : DeserializationError::EmptyInput;

        // spaces
        case ' ':
        case '\t':
        case '\r':
        case '\n':
          move();
          continue;

#if ARDUINOJSON_ENABLE_COMMENTS
        // comments
        case '/':
          move();  // skip '/'
          switch (current()) {
            // block comment
            case '*': {
              move();  // skip '*'
              bool wasStar = false;
              for (;;) {
                char c = current();
                if (c == '\0')
                  return DeserializationError::IncompleteInput;
                if (c == '/' && wasStar) {
                  move();
                  break;
                }
                wasStar = c == '*';
                move();
              }
              break;
            }

            // trailing comment
            case '/':
              // no need to skip "//"
              for (;;) {
                move();
                char c = current();
                if (c == '\0')
                  return DeserializationError::IncompleteInput;
                if (c == '\n')
                  break;
              }
              break;

            // not a comment, just a '/'
            default:
              return DeserializationError::InvalidInput;
          }
          break;
#endif

        default:
          _foundSomething = true;
          return DeserializationError::Ok;
      }
    }
  }

  DeserializationError::Code skipKeyword(const char* s) {
    while (*s) {
      char c = current();
      if (c == '\0')
        return DeserializationError::IncompleteInput;
      if (*s != c)
        return DeserializationError::InvalidInput;
      ++s;
      move();
    }
    return DeserializationError::Ok;
  }

  TStringStorage _stringStorage;
  bool _foundSomething;
  Latch<TReader> _latch;
  MemoryPool* _pool;
  char _buffer[64];  // using a member instead of a local variable because it
                     // ended in the recursive path after compiler inlined the
                     // code
};

//
// deserializeJson(JsonDocument&, const std::string&, ...)
//
// ... = NestingLimit
template <typename TString>
DeserializationError deserializeJson(
    JsonDocument& doc, const TString& input,
    NestingLimit nestingLimit = NestingLimit()) {
  return deserialize<JsonDeserializer>(doc, input, nestingLimit,
                                       AllowAllFilter());
}
// ... = Filter, NestingLimit
template <typename TString>
DeserializationError deserializeJson(
    JsonDocument& doc, const TString& input, Filter filter,
    NestingLimit nestingLimit = NestingLimit()) {
  return deserialize<JsonDeserializer>(doc, input, nestingLimit, filter);
}
// ... = NestingLimit, Filter
template <typename TString>
DeserializationError deserializeJson(JsonDocument& doc, const TString& input,
                                     NestingLimit nestingLimit, Filter filter) {
  return deserialize<JsonDeserializer>(doc, input, nestingLimit, filter);
}

//
// deserializeJson(JsonDocument&, std::istream&, ...)
//
// ... = NestingLimit
template <typename TStream>
DeserializationError deserializeJson(
    JsonDocument& doc, TStream& input,
    NestingLimit nestingLimit = NestingLimit()) {
  return deserialize<JsonDeserializer>(doc, input, nestingLimit,
                                       AllowAllFilter());
}
// ... = Filter, NestingLimit
template <typename TStream>
DeserializationError deserializeJson(
    JsonDocument& doc, TStream& input, Filter filter,
    NestingLimit nestingLimit = NestingLimit()) {
  return deserialize<JsonDeserializer>(doc, input, nestingLimit, filter);
}
// ... = NestingLimit, Filter
template <typename TStream>
DeserializationError deserializeJson(JsonDocument& doc, TStream& input,
                                     NestingLimit nestingLimit, Filter filter) {
  return deserialize<JsonDeserializer>(doc, input, nestingLimit, filter);
}

//
// deserializeJson(JsonDocument&, char*, ...)
//
// ... = NestingLimit
template <typename TChar>
DeserializationError deserializeJson(
    JsonDocument& doc, TChar* input,
    NestingLimit nestingLimit = NestingLimit()) {
  return deserialize<JsonDeserializer>(doc, input, nestingLimit,
                                       AllowAllFilter());
}
// ... = Filter, NestingLimit
template <typename TChar>
DeserializationError deserializeJson(
    JsonDocument& doc, TChar* input, Filter filter,
    NestingLimit nestingLimit = NestingLimit()) {
  return deserialize<JsonDeserializer>(doc, input, nestingLimit, filter);
}
// ... = NestingLimit, Filter
template <typename TChar>
DeserializationError deserializeJson(JsonDocument& doc, TChar* input,
                                     NestingLimit nestingLimit, Filter filter) {
  return deserialize<JsonDeserializer>(doc, input, nestingLimit, filter);
}

//
// deserializeJson(JsonDocument&, char*, size_t, ...)
//
// ... = NestingLimit
template <typename TChar>
DeserializationError deserializeJson(
    JsonDocument& doc, TChar* input, size_t inputSize,
    NestingLimit nestingLimit = NestingLimit()) {
  return deserialize<JsonDeserializer>(doc, input, inputSize, nestingLimit,
                                       AllowAllFilter());
}
// ... = Filter, NestingLimit
template <typename TChar>
DeserializationError deserializeJson(
    JsonDocument& doc, TChar* input, size_t inputSize, Filter filter,
    NestingLimit nestingLimit = NestingLimit()) {
  return deserialize<JsonDeserializer>(doc, input, inputSize, nestingLimit,
                                       filter);
}
// ... = NestingLimit, Filter
template <typename TChar>
DeserializationError deserializeJson(JsonDocument& doc, TChar* input,
                                     size_t inputSize,
                                     NestingLimit nestingLimit, Filter filter) {
  return deserialize<JsonDeserializer>(doc, input, inputSize, nestingLimit,
                                       filter);
}

}  // namespace ARDUINOJSON_NAMESPACE

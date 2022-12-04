// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2022, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Memory/MemoryPool.hpp>
#include <ArduinoJson/Misc/SerializedValue.hpp>
#include <ArduinoJson/Numbers/convertNumber.hpp>
#include <ArduinoJson/Strings/String.hpp>
#include <ArduinoJson/Strings/StringAdapters.hpp>
#include <ArduinoJson/Variant/VariantContent.hpp>

// VariantData can't have a constructor (to be a POD), so we have no way to fix
// this warning
#if defined(__GNUC__)
#  if __GNUC__ >= 7
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#    pragma GCC diagnostic ignored "-Wuninitialized"
#  endif
#endif

namespace ARDUINOJSON_NAMESPACE {

class VariantData {
  VariantContent _content;  // must be first to allow cast from array to variant
  uint8_t _flags;

 public:
  // Must be a POD!
  // - no constructor
  // - no destructor
  // - no virtual
  // - no inheritance
  void init() {
    _flags = VALUE_IS_NULL;
  }

  template <typename TVisitor>
  typename TVisitor::result_type accept(TVisitor &visitor) const {
    switch (type()) {
      case VALUE_IS_FLOAT:
        return visitor.visitFloat(_content.asFloat);

      case VALUE_IS_ARRAY:
        return visitor.visitArray(_content.asCollection);

      case VALUE_IS_OBJECT:
        return visitor.visitObject(_content.asCollection);

      case VALUE_IS_LINKED_STRING:
      case VALUE_IS_OWNED_STRING:
        return visitor.visitString(_content.asString.data,
                                   _content.asString.size);

      case VALUE_IS_OWNED_RAW:
      case VALUE_IS_LINKED_RAW:
        return visitor.visitRawJson(_content.asString.data,
                                    _content.asString.size);

      case VALUE_IS_SIGNED_INTEGER:
        return visitor.visitSignedInteger(_content.asSignedInteger);

      case VALUE_IS_UNSIGNED_INTEGER:
        return visitor.visitUnsignedInteger(_content.asUnsignedInteger);

      case VALUE_IS_BOOLEAN:
        return visitor.visitBoolean(_content.asBoolean != 0);

      default:
        return visitor.visitNull();
    }
  }

  template <typename T>
  T asIntegral() const;

  template <typename T>
  T asFloat() const;

  String asString() const;

  bool asBoolean() const;

  CollectionData *asArray() {
    return isArray() ? &_content.asCollection : 0;
  }

  const CollectionData *asArray() const {
    return const_cast<VariantData *>(this)->asArray();
  }

  CollectionData *asObject() {
    return isObject() ? &_content.asCollection : 0;
  }

  const CollectionData *asObject() const {
    return const_cast<VariantData *>(this)->asObject();
  }

  bool copyFrom(const VariantData &src, MemoryPool *pool);

  bool isArray() const {
    return (_flags & VALUE_IS_ARRAY) != 0;
  }

  bool isBoolean() const {
    return type() == VALUE_IS_BOOLEAN;
  }

  bool isCollection() const {
    return (_flags & COLLECTION_MASK) != 0;
  }

  template <typename T>
  bool isInteger() const {
    switch (type()) {
      case VALUE_IS_UNSIGNED_INTEGER:
        return canConvertNumber<T>(_content.asUnsignedInteger);

      case VALUE_IS_SIGNED_INTEGER:
        return canConvertNumber<T>(_content.asSignedInteger);

      default:
        return false;
    }
  }

  bool isFloat() const {
    return (_flags & NUMBER_BIT) != 0;
  }

  bool isString() const {
    return type() == VALUE_IS_LINKED_STRING || type() == VALUE_IS_OWNED_STRING;
  }

  bool isObject() const {
    return (_flags & VALUE_IS_OBJECT) != 0;
  }

  bool isNull() const {
    return type() == VALUE_IS_NULL;
  }

  bool isEnclosed() const {
    return !isFloat();
  }

  void remove(size_t index) {
    if (isArray())
      _content.asCollection.removeElement(index);
  }

  template <typename TAdaptedString>
  void remove(TAdaptedString key) {
    if (isObject())
      _content.asCollection.removeMember(key);
  }

  void setBoolean(bool value) {
    setType(VALUE_IS_BOOLEAN);
    _content.asBoolean = value;
  }

  void setFloat(Float value) {
    setType(VALUE_IS_FLOAT);
    _content.asFloat = value;
  }

  void setLinkedRaw(SerializedValue<const char *> value) {
    if (value.data()) {
      setType(VALUE_IS_LINKED_RAW);
      _content.asString.data = value.data();
      _content.asString.size = value.size();
    } else {
      setType(VALUE_IS_NULL);
    }
  }

  template <typename T>
  bool storeOwnedRaw(SerializedValue<T> value, MemoryPool *pool) {
    const char *dup = pool->saveString(adaptString(value.data(), value.size()));
    if (dup) {
      setType(VALUE_IS_OWNED_RAW);
      _content.asString.data = dup;
      _content.asString.size = value.size();
      return true;
    } else {
      setType(VALUE_IS_NULL);
      return false;
    }
  }

  template <typename T>
  typename enable_if<is_unsigned<T>::value>::type setInteger(T value) {
    setType(VALUE_IS_UNSIGNED_INTEGER);
    _content.asUnsignedInteger = static_cast<UInt>(value);
  }

  template <typename T>
  typename enable_if<is_signed<T>::value>::type setInteger(T value) {
    setType(VALUE_IS_SIGNED_INTEGER);
    _content.asSignedInteger = value;
  }

  void setNull() {
    setType(VALUE_IS_NULL);
  }

  void setString(String s) {
    ARDUINOJSON_ASSERT(s);
    if (s.isLinked())
      setType(VALUE_IS_LINKED_STRING);
    else
      setType(VALUE_IS_OWNED_STRING);
    _content.asString.data = s.c_str();
    _content.asString.size = s.size();
  }

  CollectionData &toArray() {
    setType(VALUE_IS_ARRAY);
    _content.asCollection.clear();
    return _content.asCollection;
  }

  CollectionData &toObject() {
    setType(VALUE_IS_OBJECT);
    _content.asCollection.clear();
    return _content.asCollection;
  }

  size_t memoryUsage() const {
    switch (type()) {
      case VALUE_IS_OWNED_STRING:
      case VALUE_IS_OWNED_RAW:
        // We always add a zero at the end: the deduplication function uses it
        // to detect the beginning of the next string.
        return _content.asString.size + 1;
      case VALUE_IS_OBJECT:
      case VALUE_IS_ARRAY:
        return _content.asCollection.memoryUsage();
      default:
        return 0;
    }
  }

  size_t nesting() const {
    return isCollection() ? _content.asCollection.nesting() : 0;
  }

  size_t size() const {
    return isCollection() ? _content.asCollection.size() : 0;
  }

  VariantData *addElement(MemoryPool *pool) {
    if (isNull())
      toArray();
    if (!isArray())
      return 0;
    return _content.asCollection.addElement(pool);
  }

  VariantData *getElement(size_t index) const {
    return isArray() ? _content.asCollection.getElement(index) : 0;
  }

  VariantData *getOrAddElement(size_t index, MemoryPool *pool) {
    if (isNull())
      toArray();
    if (!isArray())
      return 0;
    return _content.asCollection.getOrAddElement(index, pool);
  }

  template <typename TAdaptedString>
  VariantData *getMember(TAdaptedString key) const {
    return isObject() ? _content.asCollection.getMember(key) : 0;
  }

  template <typename TAdaptedString, typename TStoragePolicy>
  VariantData *getOrAddMember(TAdaptedString key, MemoryPool *pool,
                              TStoragePolicy storage_policy) {
    if (isNull())
      toObject();
    if (!isObject())
      return 0;
    return _content.asCollection.getOrAddMember(key, pool, storage_policy);
  }

  void movePointers(ptrdiff_t stringDistance, ptrdiff_t variantDistance) {
    if (_flags & OWNED_VALUE_BIT)
      _content.asString.data += stringDistance;
    if (_flags & COLLECTION_MASK)
      _content.asCollection.movePointers(stringDistance, variantDistance);
  }

  uint8_t type() const {
    return _flags & VALUE_MASK;
  }

  template <typename TAdaptedString, typename TStoragePolicy>
  inline bool storeString(TAdaptedString value, MemoryPool *pool,
                          TStoragePolicy storage) {
    if (value.isNull()) {
      setNull();
      return true;
    }

    return storage.store(value, pool, VariantStringSetter(this));
  }

 private:
  void setType(uint8_t t) {
    _flags &= OWNED_KEY_BIT;
    _flags |= t;
  }

  struct VariantStringSetter {
    VariantStringSetter(VariantData *instance) : _instance(instance) {}

    template <typename TStoredString>
    void operator()(TStoredString s) {
      if (s)
        _instance->setString(s);
      else
        _instance->setNull();
    }

    VariantData *_instance;
  };
};

}  // namespace ARDUINOJSON_NAMESPACE

#if defined(__GNUC__)
#  if __GNUC__ >= 8
#    pragma GCC diagnostic pop
#  endif
#endif

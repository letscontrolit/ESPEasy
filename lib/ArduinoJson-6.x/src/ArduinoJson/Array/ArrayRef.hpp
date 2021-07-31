// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2020
// MIT License

#pragma once

#include <ArduinoJson/Array/ArrayFunctions.hpp>
#include <ArduinoJson/Array/ArrayIterator.hpp>
#include <ArduinoJson/Variant/VariantData.hpp>

// Returns the size (in bytes) of an array with n elements.
// Can be very handy to determine the size of a StaticMemoryPool.
#define JSON_ARRAY_SIZE(NUMBER_OF_ELEMENTS) \
  ((NUMBER_OF_ELEMENTS) * sizeof(ARDUINOJSON_NAMESPACE::VariantSlot))

namespace ARDUINOJSON_NAMESPACE {

class ObjectRef;
template <typename>
class ElementProxy;

template <typename TData>
class ArrayRefBase {
 public:
  operator VariantConstRef() const {
    const void* data = _data;  // prevent warning cast-align
    return VariantConstRef(reinterpret_cast<const VariantData*>(data));
  }

  template <typename TVisitor>
  FORCE_INLINE typename TVisitor::result_type accept(TVisitor& visitor) const {
    return arrayAccept(_data, visitor);
  }

  FORCE_INLINE bool isNull() const {
    return _data == 0;
  }

  FORCE_INLINE operator bool() const {
    return _data != 0;
  }

  FORCE_INLINE size_t memoryUsage() const {
    return _data ? _data->memoryUsage() : 0;
  }

  FORCE_INLINE size_t nesting() const {
    return _data ? _data->nesting() : 0;
  }

  FORCE_INLINE size_t size() const {
    return _data ? _data->size() : 0;
  }

 protected:
  ArrayRefBase(TData* data) : _data(data) {}
  TData* _data;
};

class ArrayConstRef : public ArrayRefBase<const CollectionData>,
                      public Visitable {
  friend class ArrayRef;
  typedef ArrayRefBase<const CollectionData> base_type;

 public:
  typedef ArrayConstRefIterator iterator;

  FORCE_INLINE iterator begin() const {
    if (!_data)
      return iterator();
    return iterator(_data->head());
  }

  FORCE_INLINE iterator end() const {
    return iterator();
  }

  FORCE_INLINE ArrayConstRef() : base_type(0) {}
  FORCE_INLINE ArrayConstRef(const CollectionData* data) : base_type(data) {}

  FORCE_INLINE bool operator==(ArrayConstRef rhs) const {
    return arrayEquals(_data, rhs._data);
  }

  FORCE_INLINE VariantConstRef operator[](size_t index) const {
    return getElement(index);
  }

  FORCE_INLINE VariantConstRef getElement(size_t index) const {
    return VariantConstRef(_data ? _data->getElement(index) : 0);
  }
};

class ArrayRef : public ArrayRefBase<CollectionData>,
                 public ArrayShortcuts<ArrayRef>,
                 public Visitable {
  typedef ArrayRefBase<CollectionData> base_type;

 public:
  typedef ArrayIterator iterator;

  FORCE_INLINE ArrayRef() : base_type(0), _pool(0) {}
  FORCE_INLINE ArrayRef(MemoryPool* pool, CollectionData* data)
      : base_type(data), _pool(pool) {}

  operator VariantRef() {
    void* data = _data;  // prevent warning cast-align
    return VariantRef(_pool, reinterpret_cast<VariantData*>(data));
  }

  operator ArrayConstRef() const {
    return ArrayConstRef(_data);
  }

  VariantRef addElement() const {
    return VariantRef(_pool, arrayAdd(_data, _pool));
  }

  FORCE_INLINE iterator begin() const {
    if (!_data)
      return iterator();
    return iterator(_pool, _data->head());
  }

  FORCE_INLINE iterator end() const {
    return iterator();
  }

  // Copy a ArrayRef
  FORCE_INLINE bool set(ArrayConstRef src) const {
    if (!_data || !src._data)
      return false;
    return _data->copyFrom(*src._data, _pool);
  }

  FORCE_INLINE bool operator==(ArrayRef rhs) const {
    return arrayEquals(_data, rhs._data);
  }

  // Internal use
  FORCE_INLINE VariantRef getOrAddElement(size_t index) const {
    return VariantRef(_pool, _data ? _data->getOrAddElement(index, _pool) : 0);
  }

  // Gets the value at the specified index.
  FORCE_INLINE VariantRef getElement(size_t index) const {
    return VariantRef(_pool, _data ? _data->getElement(index) : 0);
  }

  // Removes element at specified position.
  FORCE_INLINE void remove(iterator it) const {
    if (!_data)
      return;
    _data->removeSlot(it.internal());
  }

  // Removes element at specified index.
  FORCE_INLINE void remove(size_t index) const {
    if (!_data)
      return;
    _data->removeElement(index);
  }

 private:
  MemoryPool* _pool;
};
}  // namespace ARDUINOJSON_NAMESPACE

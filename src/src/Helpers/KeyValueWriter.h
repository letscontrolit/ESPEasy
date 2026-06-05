#pragma once

#include "../../ESPEasy_common.h"

#include "../DataStructs/KeyValueStruct.h"
#include "../Helpers/PrintToString.h"
#include "../WebServer/HTML_Print.h"

#include <memory>


class KeyValueWriter;
DEF_UP(KeyValueWriter);

// ********************************************************************************
// KeyValueWriter
// ********************************************************************************
class KeyValueWriter
{
public:

  KeyValueWriter(bool emptyHeader = false, PrintToString *toStr = nullptr) : _toString(toStr),  _hasHeader(emptyHeader) {}

protected:

  KeyValueWriter(KeyValueWriter*parent, PrintToString *toStr = nullptr) :  _toString(toStr), _parent(parent)
  {}

  KeyValueWriter(bool emptyHeader, KeyValueWriter*parent, PrintToString *toStr = nullptr) :  _toString(toStr), _parent(parent),
    _hasHeader(emptyHeader) {}

  KeyValueWriter(const String& header, KeyValueWriter*parent, PrintToString *toStr = nullptr) :  _toString(toStr),  _header(header),
    _parent(parent)
  {}

public:

  virtual ~KeyValueWriter() {}

  virtual void setHeader(const String& header) {
    _header    = header;
    _hasHeader = true;
  }

  virtual void setFooter(const String& footer) { _footer = footer; }

  virtual void setIsArray()                    { _isArray = true; }

  virtual void clear();

  // Mark a write, typically called from a child calling its parent it is about to write
  virtual void write() = 0;

  virtual void write(const KeyValueStruct& kv) = 0;

  void         writeLabels(const LabelType::Enum labels[],
                           bool                  extendedValues = false);

  virtual void writeNote(const String& note);
  virtual void writeNote(const __FlashStringHelper *note);

  //  virtual void setParent(KeyValueWriter*parent) { _parent = parent; }

  virtual int getLevel() const;

  // When set to 'plainText', the writer will not try to insert writer specific
  // markings, like <pre> or <br> for example for HTML output
  // When set to true, any child writer will also have this set to true
  virtual void setPlainText() { _plainText = true; }

  virtual bool plainText() const;

  virtual bool allowHTML() const; 

  // 'summaryValueOnly' means the key will not be output and this is also a hint to generate a summary of data.
  // Typically this is intended for human readable texts.
  // When set to true, any child writer will also have this set to true
  virtual void              setSummaryValueOnly() { _summaryValueOnly = true; }

  virtual bool              summaryValueOnly() const;

  // Force to even write the header when list/array is empty.
  virtual void              setWriteWhenEmpty() { _writeWhenEmpty = true; }

  // This should be override by any writer outputting data which is not intended to be human readable.
  virtual bool              dataOnlyOutput() const                      { return false; }

  // TODO TD-er: Change this to std::unique_ptr<PrintToString>
  virtual void              setOutputToString(PrintToString*printToStr) { _toString = printToStr; }

  // Create writer of the same derived type, with this set as parent
  virtual UP_KeyValueWriter createChild()                          = 0;
  virtual UP_KeyValueWriter createChild(const String& header)      = 0;
  virtual UP_KeyValueWriter createChildArray(const String& header) = 0;

  // Create new writer of the same derived type, without parent
  virtual UP_KeyValueWriter createNew()                     = 0;
  virtual UP_KeyValueWriter createNew(const String& header) = 0;

  const String&             get() const {
    if (_toString == nullptr) { return EMPTY_STRING; }
    return _toString->get();
  }

  String&& getMove()
  {
    if (_toString == nullptr) {
      static String tmp;
      return std::move(tmp);
    }
    return std::move(_toString->getMove());
  }

  bool         reserve(unsigned int size)       { return _toString && _toString->reserve(size); }

  virtual void indent()                         {}

  virtual void allowFormatOverrides(bool allow) { _allowFormatOverrides = allow; }

protected:

  Print& getPrint() {
    if (_toString == nullptr) { return _toWeb; }
    return *_toString;
  }

  // TODO TD-er: Change this to std::unique_ptr<PrintToString>
  PrintToString *_toString = nullptr;

private:

  PrintToWebServer _toWeb;

protected:

  String _header;

  // To be written from destructor
  String _footer;

  KeyValueWriter *_parent = nullptr;

  bool _hasHeader = true;

  bool _isEmpty = true;

  bool _writeWhenEmpty = false;

  bool _isArray{};

  // private:

  bool _plainText = false;

  bool _summaryValueOnly = false;

  bool _allowFormatOverrides = true;


}; // class KeyValueWriter

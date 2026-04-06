#include "../Helpers/KeyValueWriter_JSON.h"

#include "../Globals/Settings.h"
#include "../Helpers/StringConverter.h"

#include "../WebServer/HTML_wrappers.h"


KeyValueWriter_JSON::KeyValueWriter_JSON(bool emptyHeader, PrintToString *toStr)
  : KeyValueWriter(emptyHeader, toStr)
{}

KeyValueWriter_JSON::KeyValueWriter_JSON(KeyValueWriter_JSON*parent, PrintToString *toStr)
  : KeyValueWriter(parent, toStr)
{}

KeyValueWriter_JSON::KeyValueWriter_JSON(bool emptyHeader, KeyValueWriter_JSON*parent, PrintToString *toStr)
  : KeyValueWriter(emptyHeader, parent, toStr)
{}

KeyValueWriter_JSON::KeyValueWriter_JSON(const String& header, PrintToString *toStr)
  : KeyValueWriter(header, nullptr, toStr)
{}

KeyValueWriter_JSON::KeyValueWriter_JSON(const __FlashStringHelper *header, PrintToString *toStr)
  : KeyValueWriter(String(header), nullptr, toStr)
{}


KeyValueWriter_JSON::KeyValueWriter_JSON(const String& header, KeyValueWriter_JSON*parent, PrintToString *toStr)
  : KeyValueWriter(header, parent, toStr)
{}

KeyValueWriter_JSON::KeyValueWriter_JSON(const __FlashStringHelper *header, KeyValueWriter_JSON*parent, PrintToString *toStr)
  : KeyValueWriter(String(header), parent, toStr)
{}

KeyValueWriter_JSON::~KeyValueWriter_JSON()
{
  if (_writeWhenEmpty && _isEmpty) {
    // Make sure the header is written and then write the closing braces.
    write();
  }


  if (!_isEmpty) {
#ifdef USE_KWH_JSON_PRETTY_PRINT
    getPrint().write('\n');
#endif

    if (_hasHeader) {
#ifdef USE_KWH_JSON_PRETTY_PRINT
      indent();
#endif

      getPrint().write(_isArray ? ']' : '}');
#ifndef USE_KWH_JSON_PRETTY_PRINT
      if (_isArray) getPrint().write('\n');
#endif

    }
  }

  if (!_footer.isEmpty())
  {
    getPrint().print(_footer);
  }
}

void KeyValueWriter_JSON::write()
{
  if (_isEmpty) {
    if (_parent != nullptr) { _parent->write(); }

    if (_hasHeader) {
#ifdef USE_KWH_JSON_PRETTY_PRINT
      indent();
#endif

      if (_header.isEmpty()) {
        getPrint().write('{');
#ifdef USE_KWH_JSON_PRETTY_PRINT
        getPrint().write('\n');
#endif
      } else {
        getPrint().print(strformat(
#ifdef USE_KWH_JSON_PRETTY_PRINT
                           F("\"%s\":%c\n"),
#else
                           F("\"%s\":%c"),
#endif
                           _header.c_str(),
                           _isArray ? '[' : '{'));
      }
    }
    _isEmpty = false;
  } else {
    getPrint().write(',');
#ifdef USE_KWH_JSON_PRETTY_PRINT
    getPrint().write('\n');
#endif
  }
}

void KeyValueWriter_JSON::write(const KeyValueStruct& kv)
{
  if (kv._format == KeyValueStruct::Format::Note) { return; }
  write();
#ifdef USE_KWH_JSON_PRETTY_PRINT
  indent();
  getPrint().write('\t');
#endif // ifdef USE_KWH_JSON_PRETTY_PRINT

  if (!kv._key.isEmpty()) {
    auto& pr = getPrint();
    pr.write('"');
    kv._key.print(pr);
    pr.write('"');
    pr.write(':');
  }

  const size_t nrValues = kv._values.size();

  const bool forceString = kv._format == KeyValueStruct::Format::PreFormatted;

  if (!kv._isArray) {
    // Either 1 value or empty value
    if (nrValues == 0) {
      auto& pr = getPrint();
      pr.write('"');
      pr.write('"');
    }
    else {
      writeValue(kv._values[0], forceString);
    }
  } else {
    // Multiple values, so we must wrap it in []
    auto& pr = getPrint();
    pr.write('[');
#ifdef USE_KWH_JSON_PRETTY_PRINT
    pr.write('\n');
#endif
    for (size_t i = 0; i < nrValues; ++i) {
      if (i != 0) {
        auto& pr = getPrint();
        pr.write(',');
#ifdef USE_KWH_JSON_PRETTY_PRINT
        pr.write('\n');
#endif
      }
#ifdef USE_KWH_JSON_PRETTY_PRINT
      indent();
      auto& pr = getPrint();
      pr.write('\t');
      pr.write('\t');
#endif // ifdef USE_KWH_JSON_PRETTY_PRINT

      writeValue(kv._values[i], forceString);
    }
    getPrint().write(']');
#ifndef USE_KWH_JSON_PRETTY_PRINT
    getPrint().write('\n');
#endif
  }
}

void KeyValueWriter_JSON::writeValue(const ValueStruct& val, bool forceString)
{
  if (!val.isSet()) { return; }
  auto& pr = getPrint();

  ValueStruct::ValueType valueType = val.getValueType();
  ValueStruct::ValueType valueType_afterPrint;
  String str                       = val.toString(valueType_afterPrint);

  switch (valueType)
  {
    case ValueStruct::ValueType::Float:
    case ValueStruct::ValueType::Double:

      if (!_allowFormatOverrides && valueType_afterPrint != valueType) {
        pr.print(F("null"));
        return;
      }

    case ValueStruct::ValueType::Int:
    case ValueStruct::ValueType::UInt:
      pr.print(str);
      return;
    case ValueStruct::ValueType::Bool:

      if (_allowFormatOverrides && !Settings.JSONBoolWithoutQuotes()) { pr.write('"'); }
      pr.print(str.equals("0") ? F("false") : F("true"));

      if (_allowFormatOverrides && !Settings.JSONBoolWithoutQuotes()) { pr.write('"'); }
      return;

    case ValueStruct::ValueType::Unset:
    case ValueStruct::ValueType::String:
    case ValueStruct::ValueType::FlashString:
      if (forceString) {
        String tmp(to_json_value(str));
        if (!isWrappedWithQuotes(tmp)) {
          tmp = wrap_String(tmp, '"');
        }
        pr.print(tmp);
        return;
      }
      break;
  }
  pr.print(to_json_value(str));
}

UP_KeyValueWriter KeyValueWriter_JSON::createChild()
{
  UP_KeyValueWriter_JSON child(new (std::nothrow) KeyValueWriter_JSON(this, _toString));

  child->_allowFormatOverrides = _allowFormatOverrides;

  return std::move(child);

  // return std::make_unique<KeyValueWriter_JSON>(this, _toString);
}

UP_KeyValueWriter KeyValueWriter_JSON::createChild(const String& header)
{
  UP_KeyValueWriter_JSON child(new (std::nothrow) KeyValueWriter_JSON(header, this, _toString));

  child->_allowFormatOverrides = _allowFormatOverrides;

  return std::move(child);

  // return std::make_unique<KeyValueWriter_JSON>(header, this, _toString);
}

UP_KeyValueWriter KeyValueWriter_JSON::createChildArray(const String& header)
{
  auto child = createChild(header);

  if (child) {
    child->setIsArray();
  }
  //return std::move(child);
  return child;
}

UP_KeyValueWriter KeyValueWriter_JSON::createNew()
{
  UP_KeyValueWriter_JSON child(new (std::nothrow) KeyValueWriter_JSON(false, _toString));

  child->_allowFormatOverrides = _allowFormatOverrides;

  return std::move(child);

  // return std::make_unique<KeyValueWriter_JSON>(false, _toString);
}

UP_KeyValueWriter KeyValueWriter_JSON::createNew(const String& header)
{
  UP_KeyValueWriter_JSON child(new (std::nothrow) KeyValueWriter_JSON(header, _toString));

  child->_allowFormatOverrides = _allowFormatOverrides;

  return std::move(child);

  // return std::make_unique<KeyValueWriter_JSON>(header, _toString);
}

#ifdef USE_KWH_JSON_PRETTY_PRINT

void KeyValueWriter_JSON::indent()
{
  if (_parent != nullptr) {
    getPrint().write('\t');
    _parent->indent();
  }
}

#endif // ifdef USE_KWH_JSON_PRETTY_PRINT

#include "../Helpers/KeyValueWriter.h"




// ********************************************************************************
// KeyValueWriter
// ********************************************************************************

void KeyValueWriter::clear() {
  _isEmpty = true;

  if (_toString) {
    _toString->clear();
  }
}

void KeyValueWriter::writeLabels(const LabelType::Enum labels[], bool extendedValues)
{
  size_t i            = 0;
  LabelType::Enum cur = static_cast<const LabelType::Enum>(pgm_read_byte(labels + i));

  while (cur != LabelType::MAX_LABEL) {
    auto kv = getKeyValue(cur, extendedValues);
    if (!kv._key.isEmpty())
      write(kv);
    cur = static_cast<const LabelType::Enum>(pgm_read_byte(labels + i + 1));
    ++i;
  }
}

void KeyValueWriter::writeNote(const String& note)
{
  if (!dataOnlyOutput()) {
    write({
      EMPTY_STRING,
      note,
      KeyValueStruct::Format::Note });
  }
}

void KeyValueWriter::writeNote(const __FlashStringHelper *note)
{
  if (!dataOnlyOutput()) {
    write({
      EMPTY_STRING,
      note,
      KeyValueStruct::Format::Note });
  }
}

int KeyValueWriter::getLevel() const
{
  if (_parent == nullptr) { return 0; }
  return _parent->getLevel() + 1;
}

bool KeyValueWriter::plainText() const {
  if (_toString != nullptr) { return true; }

  if (_parent && _parent->plainText()) { return true; }
  return _plainText;
}

bool KeyValueWriter::allowHTML() const {
  return false;
}

bool KeyValueWriter::summaryValueOnly() const {
  if (_parent && _parent->summaryValueOnly()) { return true; }

  return _summaryValueOnly;
}

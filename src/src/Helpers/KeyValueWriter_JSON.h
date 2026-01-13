#pragma once

#include "../Helpers/KeyValueWriter.h"

#ifndef BUILD_NO_DEBUG

// # define USE_KWH_JSON_PRETTY_PRINT
#endif // ifndef BUILD_NO_DEBUG

class KeyValueWriter_JSON : public KeyValueWriter
{
public:

  /*
     TODO TD-er: Replace
     - stream_to_json_object_value used in ChartJS code

   */

  using KeyValueWriter::writeLabels;

  KeyValueWriter_JSON(bool           emptyHeader = false,
                      PrintToString *toStr       = nullptr);

  KeyValueWriter_JSON(KeyValueWriter_JSON*parent,
                      PrintToString      *toStr = nullptr);

  KeyValueWriter_JSON(bool                emptyHeader,
                      KeyValueWriter_JSON*parent,
                      PrintToString      *toStr = nullptr);

  KeyValueWriter_JSON(const String & header,
                      PrintToString *toStr = nullptr);
  KeyValueWriter_JSON(const __FlashStringHelper *header,
                      PrintToString             *toStr = nullptr);

  KeyValueWriter_JSON(const String      & header,
                      KeyValueWriter_JSON*parent,
                      PrintToString      *toStr = nullptr);
  KeyValueWriter_JSON(const __FlashStringHelper *header,
                      KeyValueWriter_JSON       *parent,
                      PrintToString             *toStr = nullptr);

  virtual ~KeyValueWriter_JSON();

  virtual void              write();

  virtual void              write(const KeyValueStruct& kv);

  // Create writer of the same derived type, with this set as parent
  virtual UP_KeyValueWriter createChild();
  virtual UP_KeyValueWriter createChild(const String& header);
  virtual UP_KeyValueWriter createChildArray(const String& header);

  // Create new writer of the same derived type, without parent
  virtual UP_KeyValueWriter createNew();
  virtual UP_KeyValueWriter createNew(const String& header);

  virtual bool              dataOnlyOutput() const override {
    // JSON is not intended to be human readable
    return true;
  }

  virtual bool plainText() const override {
    // No 'plain text' in JSON
    return false;
  }

  virtual bool summaryValueOnly() const override {
    // No 'summary mode' in JSON
    return false;
  }

private:

  void writeValue(const ValueStruct& value);

#ifdef USE_KWH_JSON_PRETTY_PRINT

protected:

  virtual void indent() override;
#endif // ifdef USE_KWH_JSON_PRETTY_PRINT


}; // class KeyValueWriter_JSON

DEF_UP(KeyValueWriter_JSON);

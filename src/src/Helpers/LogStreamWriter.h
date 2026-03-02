#pragma once

#include "../../ESPEasy_common.h"

class LogStreamWriter
{

public:

  LogStreamWriter(LogDestination log_destination) : _log_destination(log_destination) {}

  virtual ~LogStreamWriter() {}

  virtual bool     process(Print* stream, size_t availableForWrite);
  
  // Only use this from derived classes, as we need a Stream to further process
  virtual bool     process();

  virtual uint32_t getNrMessages() const;

  virtual void     clear();

protected:

  // Write continuously until either nrBytesToWrite was reached or no new messages were available to process.
  // @retval Number of bytes written. Zero when no new message was available to process.
  virtual size_t write(Print& stream,
                       size_t  nrBytesToWrite);

  // Write single item and clear() on return.
  // This way each call starts with a new item and long messages may get truncated based on nrBytesToWrite
  // @retval Number of bytes written. Zero when no new message was available to process.
  virtual size_t write_single_item(Print& stream,
                                   size_t  nrBytesToWrite);

  virtual size_t write_item(Print& stream,
                            size_t  nrBytesToWrite);


  virtual size_t write_skipping(Print& stream);

  virtual void   prepare_prefix();


  String _prefix;
  String _message;
  uint32_t _timestamp{};
  uint32_t _readpos{};
  uint8_t _loglevel{};

  const LogDestination _log_destination;


}; // class LogStreamWriter

#ifndef HELPERS_SERIALWRITEBUFFER_H
#define HELPERS_SERIALWRITEBUFFER_H

#include "../../ESPEasy_common.h"

#include "../Helpers/LogStreamWriter.h"

class SerialWriteBuffer_t : public LogStreamWriter {
public:

  SerialWriteBuffer_t(LogDestination log_destination) : LogStreamWriter(log_destination) {}

  

private:

  size_t write_skipping(Stream& stream) override;

  void prepare_prefix() override;



  String colorize(const String& str) const;

};

#endif // ifndef HELPERS_SERIALWRITEBUFFER_H

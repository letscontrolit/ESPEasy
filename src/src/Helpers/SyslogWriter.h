#pragma once

#include "../../ESPEasy_common.h"

#if FEATURE_SYSLOG

#include "../Helpers/LogStreamWriter.h"


/*********************************************************************************************\
   Syslog client
\*********************************************************************************************/

class SyslogWriter : public LogStreamWriter {
public:

  SyslogWriter(LogDestination log_destination) : LogStreamWriter(log_destination) {}

  virtual bool process() override;

private:

  void prepare_prefix() override;

};

#endif
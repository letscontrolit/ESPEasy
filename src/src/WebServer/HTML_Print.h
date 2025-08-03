#ifndef WEBSERVER_HTML_PRINT_H
#define WEBSERVER_HTML_PRINT_H


#include "../../ESPEasy_common.h"


// Class to redirect any function expecting a Print stream to output to the TXBuffer of our webserver.
class HTML_Print : public Print
{
public:

  virtual ~HTML_Print() {}

  size_t write(uint8_t c) override;

};


#endif // WEBSERVER_HTML_PRINT_H

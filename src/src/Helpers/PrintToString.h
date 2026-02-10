#ifndef HELPERS_PRINTTOSTRING_H
#define HELPERS_PRINTTOSTRING_H

#include "../../ESPEasy_common.h"

class PrintToString : public Print
{
public:

  virtual ~PrintToString() {}

  void          clear();

  size_t        length() const;

  bool          reserve(unsigned int size);


  size_t        write(uint8_t c) override;

  const String& get() const;

  String&& getMove();

private:

  String _str;
}; // class PrintToString


#endif // HELPERS_PRINTTOSTRING_H

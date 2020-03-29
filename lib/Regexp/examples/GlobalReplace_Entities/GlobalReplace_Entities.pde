#include <Regexp.h>

// called for every match
void replace_callback (const char * match,         // what we found
                       const unsigned int length,  // how long it was
                       const char * & replacement,       // put replacement here
                       unsigned int & replacement_length,  // put replacement length here
                       const MatchState & ms)      // for looking up captures
{
static byte c;  // for holding replacement byte, must be static

   char hexdigits [3];  // to hold hex string

    // get first capture
    ms.GetCapture (hexdigits, 0);
    // convert from hex to printable
    c = strtol (hexdigits, NULL, 16);

    // set as replacement
    replacement = (char *) &c;
    replacement_length = 1;
}  // end of replace_callback


void setup ()
{
  Serial.begin (115200);

  // what we are searching
  char buf [100] = "%7B%22John+Doe%22%7D";

  // for matching regular expressions
  MatchState ms (buf);

  // easy part, replace + by space
  ms.GlobalReplace ("%+", " ");

  // replace %xx (eg. %22) by what the hex code represents
  ms.GlobalReplace ("%%(%x%x)", replace_callback);

  Serial.println (buf);

}  // end of setup

void loop () {}

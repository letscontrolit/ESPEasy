#include <Regexp.h>

// called for every match
void replace_callback (const char * match,         // what we found
                       const unsigned int length,  // how long it was
                       const char * & replacement,       // put replacement here
                       unsigned int & replacement_length,  // put replacement length here
                       const MatchState & ms)      // for looking up captures
{

  // show matching text
  Serial.print("Match = ");
  Serial.write((byte *) match, length);
  Serial.println ();

  replacement = "Nick";
  replacement_length = 4;
}  // end of replace_callback

void setup ()
{
  Serial.begin (115200);
  Serial.println ();
  unsigned long count;

  // what we are searching (the target)
  char buf [100] = "The quick brown fox jumps over the lazy wolf";

  // match state object
  MatchState ms (buf);

  // original buffer
  Serial.println (buf);

  // search for three letters
  count = ms.GlobalReplace ("%a+", replace_callback);

  // show results
  Serial.print ("Converted string: ");
  Serial.println (buf);
  Serial.print ("Found ");
  Serial.print (count);            // 9 in this case
  Serial.println (" matches.");

  // copy in new target
  strcpy (buf, "But does it get goat's blood out?");
  ms.Target (buf);    // recompute length

  // replace vowels with *
  count = ms.GlobalReplace ("[aeiou]", "*");

  // show results
  Serial.print ("Converted string: ");
  Serial.println (buf);
  Serial.print ("Found ");
  Serial.print (count);            // 13 in this case
  Serial.println (" matches.");

}  // end of setup

void loop () {}

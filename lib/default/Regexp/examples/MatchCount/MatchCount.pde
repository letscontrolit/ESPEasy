#include <Regexp.h>

void setup ()
{
  Serial.begin (115200);

  // match state object
  MatchState ms;

  // what we are searching (the target)
  char buf [100] = "The quick brown fox jumps over the lazy wolf";
  ms.Target (buf);  // set its address

  unsigned int count = ms.MatchCount ("[aeiou]");
  
  Serial.println (buf);
  Serial.print ("Found ");
  Serial.print (count);            // 11 in this case
  Serial.println (" matches.");
  
}  // end of setup  

void loop () {}

#include <Regexp.h>

void setup ()
{
  Serial.begin (115200);

  // match state object
  MatchState ms;

  // what we are searching (the target)
  char buf [100] = "The quick brown fox jumps over the lazy wolf";
  ms.Target (buf);  // set its address
  Serial.println (buf);

  char result = ms.Match ("f.x");
  
  if (result > 0)
    {
    Serial.print ("Found match at: ");
    Serial.println (ms.MatchStart);        // 16 in this case     
    Serial.print ("Match length: ");
    Serial.println (ms.MatchLength);       // 3 in this case
    }
  else
    Serial.println ("No match.");
    
}  // end of setup  

void loop () {}

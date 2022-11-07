#include <Regexp.h>

// called for each match
void match_callback  (const char * match,          // matching string (not null-terminated)
                      const unsigned int length,   // length of matching string
                      const MatchState & ms)      // MatchState in use (to get captures)
{
char cap [10];   // must be large enough to hold captures
  
  Serial.print ("Matched: ");
  Serial.write ((byte *) match, length);
  Serial.println ();
  
  for (byte i = 0; i < ms.level; i++)
    {
    Serial.print ("Capture "); 
    Serial.print (i, DEC);
    Serial.print (" = ");
    ms.GetCapture (cap, i);
    Serial.println (cap); 
    }  // end of for each capture

}  // end of match_callback 


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

  // search for three letters followed by a space (two captures)
  count = ms.GlobalMatch ("(%a+)( )", match_callback);

  // show results
  Serial.print ("Found ");
  Serial.print (count);            // 8 in this case
  Serial.println (" matches.");
 

}  // end of setup  

void loop () {}
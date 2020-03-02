#include "Plugins_other.h"


 
 void (*parseTemplate_CallBack_ptr)(String& tmpString, bool useURLencode) = nullptr;
 void (*substitute_eventvalue_CallBack_ptr)(String& line, const String& event) = nullptr;
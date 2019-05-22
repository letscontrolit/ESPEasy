//**********************************************************************************************************************
//
//	Gets the TaskIndex from the Task Name - returns 255 if not found
//
byte getTaskIndex(String TaskName)


// This routine returns the Task Index of the task called TaskName - note that TaskName is not case sensitive
// If TaskName is not found, then it returns 255

{

  for (byte y = 0; y < TASKS_MAX; y++)
  {
     LoadTaskSettings(y);
     String DName = ExtraTaskSettings.TaskDeviceName;
     if (( ExtraTaskSettings.TaskDeviceName[0] != 0 ) && ( DName.equalsIgnoreCase(TaskName) ))
     {
        return y;
     }  
  }
  
  return 255;
}

// Extracts the Task Name and ValueName from the Ident

boolean getTaskandValueName(String IdentIn,String &TaskName,String &ValueName)
{
	String Ident;

	Ident = IdentIn;

	// Get rid of any []

	Ident.replace("[", " ");
	Ident.replace("]", " ");
	Ident.trim();

	// Find the location of # - it must exist and must not be at the beginning or the end of the ident

	int loc = Ident.indexOf("#");
	if ((loc == -1) || (loc == 0) || (loc == Ident.length()))
	{
		return false;
	}

	// Seperate out the taskname and the valuename

	TaskName = Ident.substring(0, loc);
	ValueName = Ident.substring(loc + 1);

	return true;
}
//  ************************************************************************************************************************
//
//	Gets the ValuenameIndex from the TaskIndex and the ValueName - returns 255 if not found
//
byte getValueNameIndex(int TaskIndex, String ValueName)
{

//  This routine checks to see if ValueName is valid for a given taskindex.
//  If the Valuename is found then we return its number - else 255

   LoadTaskSettings(TaskIndex);

   for (byte y = 0; y < VARS_PER_TASK; y++)
   {
     if (ValueName.equalsIgnoreCase(ExtraTaskSettings.TaskDeviceValueNames[y]))
     {
        return y;
     }
   }
   return 255;
    
}

//  ******************************************************************************************************************
//
//	Standard routine for checking parameters
//
boolean CheckParam(String Name, int Val, int Min, int Max)

// 
{     
  if (Val < Min or Val > Max)
  {
    String log = F("ERR  : '");
    log += Name;
    log += "' has value ";
    log += Val;
    log += " but should be between ";
    log += Min;
    log += " and ";
    log += Max;
    addLog(LOG_LEVEL_ERROR, log);
      
    return false;
  }
  return true;
}
//	**************************************************************************
//
// Convert String to Integer
//
int string2Integer(String myString) { 
  int i, value, len; 
  len = myString.length(); 
  char tmp[(len+1)];       // one extra for the zero termination
  byte start=0;
  if ( myString.substring(0,1) == "-"){
    tmp[0]='-';
    start=1;   //allow a minus in front of string
  }
  for(i=start; i<len; i++) 
  { 
    tmp[i] = myString.charAt(i);
    if ( ! isdigit(tmp[i]) ) return -999;   
  } 

  tmp[i]=0; 
  value = atoi(tmp); 
  return value;  
} 

//	***************************************************************************
//
// Convert String to float
//
float string2float(String myString) { 
  int i, len; 
  float value;
  len = myString.length(); 
  char tmp[(len+1)];       // one extra for the zero termination
  byte start=0;

//  Look for decimal point - they can be anywhere but no more than one of them!

  int dotIndex=myString.indexOf(".");
  //Serial.println(dotIndex);
  
  if (dotIndex != -1)
  {
    int dotIndex2=(myString.substring(dotIndex+1)).indexOf(".");
    //Serial.println(dotIndex2);
    if (dotIndex2 != -1 )return -999.00;    // Give error if there is more than one dot
  }
  
  if ( myString.substring(0,1) == "-"){
    start=1;   //allow a minus in front of string
    tmp[i] = '-';
  }
  
  for(i=start; i<len; i++) 
  { 
    tmp[i] = myString.charAt(i);
    if ( ! isdigit(tmp[i]) ) 
    {
      if (tmp[i] != '.' )return -999;   
    }
  } 

  tmp[i]=0; 
  value = atof(tmp);
  return value;  
} 

//	*****************************************************************************
//
//	Log Updates
//
void logUpdates(byte ModNum,byte TaskNum, byte ValueNum, float NewValue)
{
        LoadTaskSettings(TaskNum);

        String log=F("");
        log+=ModNum;
        log += "  : [";
        log+=ExtraTaskSettings.TaskDeviceName;
        log+="#";
        log += ExtraTaskSettings.TaskDeviceValueNames[ValueNum];
        log += "] set to ";
        log += NewValue;
        addLog(LOG_LEVEL_INFO,log);
}
//***************************************************************************
//	Send a pushbullet note
boolean pushbulletSend(String PushBulletAPIKEY, String Title, String Body) {

	String log;

	const char* host = "api.pushbullet.com";
	const int Port = 443;

	// Use WiFiClientSecure class to create TLS connection - this is too heavy on resources!!

	WiFiClientSecure client;

	if (!client.connect(host, Port)) {
		log = F("ERR  : Connection to ");
		log += host;
		log += " failed";
		addLog(LOG_LEVEL_ERROR, log);
		return false;
	}

	log = F("CON  : Connected to ");
	log += host;
	addLog(LOG_LEVEL_INFO, log);

	//	Construct the POST Data String

	String df1 = F("{\"type\": \"note\", \"title\":");
	String df2 = F("\"body\":");

	String PushBullet_Data = df1 + "\"" + Title + "\"" + "," + df2 + "\"" + Body + "\"" + "}";

	// Determine the Data size

	String PushBullet_Data_Size = String(PushBullet_Data.length());

	// Now send stuff to pushbullet

	String url = "/v2/pushes";

	client.print(String("POST ") + url + " HTTP/1.1\r\n" +
		"Host: " + host + "\r\n" +
		"Authorization: Bearer " + PushBulletAPIKEY + "\r\n" +
		"Content-Type: application/json\r\n" +
		"Content-Length: " + PushBullet_Data_Size + "\r\n\r\n" + PushBullet_Data);

	//print the response

	// If all is OK, he first line should contain the string "200 OK" 
	// If this is the case then don't print anything and return true

	while (client.connected())
	{
		if (client.available())
		{
			String line = client.readStringUntil('\r');
			if (line.lastIndexOf("200 OK") >= 0) {
				log = F("IFTT : Success - PushBullet Note Sent");
				addLog(LOG_LEVEL_INFO, log);
				client.stop();
				return true;
			}
			log = F("ERR  : ");
			log += line;
			addLog(LOG_LEVEL_ERROR, log);
		}
		else {
			// No data yet, wait a while
			delay(50);
		};
	}

	// All done

	client.stop();
	addLog(LOG_LEVEL_ERROR, "ERR : Error sending PushBullet Note");

	return false;
}

//****************************************************************
// Get the most recent value of parameter IdentIn
// Return -999 in case of error
float getLatestValue(String IdentIn) {

	String TaskName;
	String ValueName;
	String Ident;

	Ident = IdentIn;
	// Get rid of any []

	Ident.replace("[", " ");
	Ident.replace("]", " ");
	Ident.trim();

	// Find the location of # - it must exist and must not be at the beginning or the end of the ident

	int loc = Ident.indexOf("#");
	if ((loc == -1) || (loc == 0) || (loc == Ident.length()))
	{
		String log = F("Err  : Illegal Identifier Syntax - ");
		log += IdentIn;
		addLog(LOG_LEVEL_ERROR, log);
		return -999;
	}

	// Seperate out the taskname and the valuename

	TaskName = Ident.substring(0, loc);
	ValueName = Ident.substring(loc + 1);

	// Get the indices from the names

	int TaskIndex = getTaskIndex(TaskName);
	if (TaskIndex == 255) {
		String log = F("Err  : Unknown Task - ");
		log += TaskName;
		addLog(LOG_LEVEL_ERROR, log);
		return -999;
	}
	int ValueIndex = getValueNameIndex(TaskIndex, ValueName);
	if (ValueIndex == 255) {
		String log = F("Err  : Unknown ValueName - ");
		log += ValueName;
		addLog(LOG_LEVEL_ERROR, log);
		return -999;
	}
	// And get the last reading from Uservar

	return UserVar[TaskIndex*VARS_PER_TASK + ValueIndex];

}

//****************************************************************************************************
// Send a trigger to IFTTT with user specified key, Event and Data fields
boolean IFTTT_Trigger(String IFTTT_APIKey, String IFTTT_Event, String IFTTT_Value1, String IFTTT_Value2, String IFTTT_Value3)
{
	const char *host = "maker.ifttt.com";

	String log;

	// Use WiFiClient class to create TCP connections

	WiFiClient client;
	const int Port = 80;

	if (!client.connect(host, Port)) {
		log = F("ERR  : Connection to ");
		log += host;
		log += " failed";
		addLog(LOG_LEVEL_ERROR, log);
		return false;
	}

	log = F("CON  : Connected to ");
	log += host;
	addLog(LOG_LEVEL_INFO, log);

	//	Construct the POST Data String

	String df1 = F("{\"value1\":");
	String df2 = F("\"value2\":");
	String df3 = F("\"value3\":");

	String IFTTT_Data = df1 + "\"" + IFTTT_Value1 + "\"" + "," + df2 + "\"" + IFTTT_Value2 + "\"" + "," + df3 + "\"" + IFTTT_Value3 + "\"" + "}";

	// Determine the Data size

	String IFTTT_Data_Size = String(IFTTT_Data.length());

	// We now create a URL for the request as per the IFTTT API instructions

	String url = F("/trigger/");
	url += IFTTT_Event;
	url += "/with/key/";
	url += IFTTT_APIKey;

	// This will send the request to the server

	client.print(String("POST ") + url + " HTTP/1.1\r\n"
		+ "Host: " + host + "\r\n"
		+ "Connection: close\r\n"
		+ "Content-Type: application/json\r\n"
		+ "Content-Length: " + IFTTT_Data_Size + "\r\n"
		+ "\r\n"
		+ IFTTT_Data + "\r\n");


	// Read all the lines of the reply from server and print them to Serial,
	// the connection will close when the server has sent all the data.

	// If all is OK, he first line should contain the string "200 OK" 
	// If this is the case then don't print anything and return true

	while (client.connected())
	{
		if (client.available())
		{
			String line = client.readStringUntil('\r');
			if (line.lastIndexOf("200 OK") >= 0) {
				log = F("IFTT : Success - Trigger '");
				log += IFTTT_Event;
				log += "' Sent to IFTTT";
				addLog(LOG_LEVEL_INFO, log);
				client.stop();
				return true;
			}
			log = F("ERR  : ");
			log += line;
			addLog(LOG_LEVEL_ERROR, log);
		}
		else {
			// No data yet, wait a bit
			delay(50);
		};
	}

	// All done

	client.stop();
	addLog(LOG_LEVEL_ERROR, "IFTT : Error sending trigger to IFTTT");

	return false;

}


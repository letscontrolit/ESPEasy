#ifndef WEBSERVER_WEBSERVER_JSON_H
#define WEBSERVER_WEBSERVER_JSON_H


#include "../WebServer/common.h"


// ********************************************************************************
// Web Interface get CSV value from task
// ********************************************************************************
void handle_csvval();

// ********************************************************************************
// Web Interface JSON page (no password!)
// ********************************************************************************
void handle_json();

// ********************************************************************************
// JSON formatted timing statistics
// ********************************************************************************

#ifdef WEBSERVER_NEW_UI
void handle_timingstats_json();

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_NEW_UI
void handle_nodes_list_json();

void handle_buildinfo();

#endif // WEBSERVER_NEW_UI


/*********************************************************************************************\
   Streaming versions directly to TXBuffer
\*********************************************************************************************/
void stream_to_json_value(const String& value);

void stream_to_json_object_value(const String& object, const String& value);

String jsonBool(bool value);

// Add JSON formatted data directly to the TXbuffer, including a trailing comma.
void stream_next_json_object_value(const String& object, const String& value);

// Add JSON formatted data directly to the TXbuffer, including a closing '}'
void stream_last_json_object_value(const String& object, const String& value);

void stream_next_json_object_value(LabelType::Enum label);

void stream_last_json_object_value(LabelType::Enum label);




#endif
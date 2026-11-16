#pragma once

#include "../WebServer/common.h"

struct TaskValuesWriterHelper;

// ********************************************************************************
// Web Interface get CSV value from task
// ********************************************************************************
#ifdef WEBSERVER_CSVVAL
void handle_csvval();
#endif

#ifdef WEBSERVER_JSON

// ********************************************************************************
// Web Interface JSON page (no password!)
// ********************************************************************************
void handle_json();

void handle_json_stream_task_value_data(TaskValuesWriterHelper*data);

#endif // ifdef WEBSERVER_JSON

// ********************************************************************************
// JSON formatted timing statistics
// ********************************************************************************

#ifdef WEBSERVER_NEW_UI
void handle_timingstats_json();

# if FEATURE_ESPEASY_P2P
void handle_nodes_list_json();
# endif
void handle_buildinfo();

#endif // WEBSERVER_NEW_UI


/*********************************************************************************************\
   Streaming versions directly to TXBuffer
\*********************************************************************************************/
String jsonBool(bool value);
#ifdef WEBSERVER_NEW_UI

// Add JSON formatted data directly to the TXbuffer, including a trailing comma.
void stream_next_json_object_value(const __FlashStringHelper *object,
                                   const String             & value);
void stream_next_json_object_value(const __FlashStringHelper *object,
                                   String                  && value);
void stream_next_json_object_value(const String& object,
                                   const String& value);
void stream_next_json_object_value(const __FlashStringHelper *object,
                                   int                        value);
void stream_next_json_object_value(const String& object,
                                   int           value);

// Add JSON formatted data directly to the TXbuffer, including a closing '}'
void stream_last_json_object_value(const __FlashStringHelper *object,
                                   const String             & value);
void stream_last_json_object_value(const __FlashStringHelper *object,
                                   String                  && value);
void stream_last_json_object_value(const String& object,
                                   const String& value);
void stream_last_json_object_value(const __FlashStringHelper *object,
                                   int                        value);

// void stream_json_object_values(const LabelType::Enum labels[]);

void stream_next_json_object_value(LabelType::Enum label);

void stream_last_json_object_value(LabelType::Enum label);
#endif // ifdef WEBSERVER_NEW_UI

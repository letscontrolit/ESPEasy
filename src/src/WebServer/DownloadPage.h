#pragma once

#include "../WebServer/common.h"

#ifdef WEBSERVER_DOWNLOAD

// ********************************************************************************
// Web Interface download page
// ********************************************************************************
void handle_download();
# if FEATURE_TARSTREAM_SUPPORT
void handle_full_backup();
void handle_full_backup_no_usr_pwd();
# endif // if FEATURE_TARSTREAM_SUPPORT
#endif // ifdef WEBSERVER_DOWNLOAD

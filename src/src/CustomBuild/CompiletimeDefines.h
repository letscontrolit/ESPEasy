#ifndef CUSTOMBUILD_COMPILETIMEDEFINES_H
#define CUSTOMBUILD_COMPILETIMEDEFINES_H

#include <Arduino.h>


// Build NR is used as a "revision" nr for settings
// As of 2022-08-18, it is the nr of days since 2022-08-18 + 20200
uint16_t get_build_nr();
const __FlashStringHelper * get_binary_filename();
const __FlashStringHelper * get_build_time();
const __FlashStringHelper * get_build_date();
uint32_t                    get_build_unixtime();
const __FlashStringHelper * get_build_date_RFC1123();
const __FlashStringHelper * get_build_origin();
const __FlashStringHelper * get_build_platform();
const __FlashStringHelper * get_git_head();
const __FlashStringHelper * get_board_name();
const __FlashStringHelper * get_CDN_url_prefix();


#endif // CUSTOMBUILD_COMPILETIMEDEFINES_H

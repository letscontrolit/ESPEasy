#include "../Helpers/_Feature_init.h"

#if FEATURE_BUILD_DESCRIPTION

# include "../DataTypes/ESPEasy_plugin_functions.h"

# include "../Helpers/Misc.h"

// ********************************************************************************
// Initialize all Features that where defined earlier
// ********************************************************************************

const uint16_t featuresBitmap[] = {
  0u
# if FEATURE_MQTT
  | 1 << 15
# endif // if FEATURE_MQTT
# if FEATURE_ESPEASY_P2P
  | 1 << 14
# endif // if FEATURE_ESPEASY_P2P
# if FEATURE_SD
  | 1 << 13
# endif // if FEATURE_SD
# if FEATURE_ARDUINO_OTA
  | 1 << 12
# endif // if FEATURE_ARDUINO_OTA
# if FEATURE_RULES_EASY_COLOR_CODE
  | 1 << 11
# endif // if FEATURE_RULES_EASY_COLOR_CODE
# if FEATURE_DOWNLOAD
  | 1 << 10
# endif // if FEATURE_DOWNLOAD
# if FEATURE_I2C_DEVICE_SCAN
  | 1 << 9
# endif // if FEATURE_I2C_DEVICE_SCAN
# if FEATURE_SSDP
  | 1 << 8
# endif // if FEATURE_SSDP
# if FEATURE_EXT_RTC
  | 1 << 7
# endif // if FEATURE_EXT_RTC
# if FEATURE_PLUGIN_STATS
  | 1 << 6
# endif // if FEATURE_PLUGIN_STATS
# if FEATURE_CHART_JS
  | 1 << 5
# endif // if FEATURE_CHART_JS
# if FEATURE_SETTINGS_ARCHIVE
  | 1 << 4
# endif // if FEATURE_SETTINGS_ARCHIVE
# if FEATURE_I2CMULTIPLEXER
  | 1 << 3
# endif // if FEATURE_I2CMULTIPLEXER
# if FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES
  | 1 << 2
# endif // if FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES
# if FEATURE_SEND_TO_HTTP
  | 1 << 1
# endif // if FEATURE_SEND_TO_HTTP
# if FEATURE_PUT_TO_HTTP
  | 1 << 0
# endif // if FEATURE_PUT_TO_HTTP
  , 0u // n=17 : m=0
# if FEATURE_POST_TO_HTTP
  | 1 << 15
# endif // if FEATURE_POST_TO_HTTP
# if FEATURE_I2C_DEVICE_CHECK
  | 1 << 14
# endif // if FEATURE_I2C_DEVICE_CHECK
# if FEATURE_RTTTL
  | 1 << 13
# endif // if FEATURE_RTTTL
# if FEATURE_AUTO_DARK_MODE
  | 1 << 12
# endif // if FEATURE_AUTO_DARK_MODE
# if FEATURE_SERVO
  | 1 << 11
# endif // if FEATURE_SERVO
# if FEATURE_CUSTOM_PROVISIONING
  | 1 << 10
# endif // if FEATURE_CUSTOM_PROVISIONING
# if FEATURE_ETHERNET
  | 1 << 9
# endif // if FEATURE_ETHERNET
# if FEATURE_TIMING_STATS
  | 1 << 8
# endif // if FEATURE_TIMING_STATS
# if FEATURE_TOOLTIPS
  | 1 << 7
# endif // if FEATURE_TOOLTIPS
# if FEATURE_ADC_VCC
  | 1 << 6
# endif // if FEATURE_ADC_VCC
# if FEATURE_MDNS
  | 1 << 5
# endif // if FEATURE_MDNS
# if FEATURE_MODBUS
  | 1 << 4
# endif // if FEATURE_MODBUS
# if FEATURE_PACKED_RAW_DATA
  | 1 << 3
# endif // if FEATURE_PACKED_RAW_DATA
# if feature_adagfx_fonts
  | 1 << 2
# endif // if feature_adagfx_fonts
# if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  | 1 << 1
# endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# if DEFAULT_APPEND_UNIT_TO_HOSTNAME
  | 1 << 0
# endif // if DEFAULT_APPEND_UNIT_TO_HOSTNAME
  , 0 // n=32 m=0
# if DEFAULT_SEND_TO_HTTP_ACK
  | 1 << 15
# endif // if DEFAULT_SEND_TO_HTTP_ACK
# if FEATURE_IMPROV
  | 1 << 14
# endif // if FEATURE_IMPROV
# if CONFIGURATION_CODE_not_available
  | 1 << 13
# endif // if CONFIGURATION_CODE_not_available
# if CPLUGIN_015_SSL
  | 1 << 12
# endif // if CPLUGIN_015_SSL
# ifdef LIMIT_BUILD_SIZE
  | 1 << 11
# endif // ifdef LIMIT_BUILD_SIZE
# ifdef BUILD_NO_DEBUG
  | 1 << 10
# endif // ifdef BUILD_NO_DEBUG
# if FEATURE_ZEROFILLED_UNITNUMBER
  | 1 << 9
# endif // if FEATURE_ZEROFILLED_UNITNUMBER
# if FEATURE_f040
  | 1 << 8  // F040
# endif
# if FEATURE_f041
  | 1 << 7  // F041
# endif
# if FEATURE_f042
  | 1 << 6  // F042
# endif
# if FEATURE_f043
  | 1 << 5  // F043
# endif
# if FEATURE_f044
  | 1 << 4  // F044
# endif
# if FEATURE_f045
  | 1 << 3  // F045
# endif
# if FEATURE_f046
  | 1 << 2  // F046
# endif
# if FEATURE_f047
  | 1 << 1  // F047
# endif
# if FEATURE_f048
  | 1 << 0  // F048
# endif
  , 0u      // n=49 : m=0
# if FEATURE_f049
  | 1 << 15 // F049
# endif
# if FEATURE_f050
  | 1 << 14 // F050
# endif
# if FEATURE_f051
  | 1 << 13 // F051
# endif
# if FEATURE_f052
  | 1 << 12 // F052
# endif
# if FEATURE_f053
  | 1 << 11 // F053
# endif
# if FEATURE_f054
  | 1 << 10 // F054
# endif
# if FEATURE_f055
  | 1 << 9  // F055
# endif
# if FEATURE_f056
  | 1 << 8  // F056
# endif
# if FEATURE_f057
  | 1 << 7  // F057
# endif
# if FEATURE_f058
  | 1 << 6  // F058
# endif
# if FEATURE_f059
  | 1 << 5  // F059
# endif
# if FEATURE_f060
  | 1 << 4  // F060
# endif
# if FEATURE_f061
  | 1 << 3  // F061
# endif
# if FEATURE_f062
  | 1 << 2  // F062
# endif
# if FEATURE_f063
  | 1 << 1  // F063
# endif
# if FEATURE_f064
  | 1 << 0  // F064
# endif
  , 0u      // n=65 : m=0
# if FEATURE_f065
  | 1 << 15 // F065
# endif
# if FEATURE_f066
  | 1 << 14 // F066
# endif
# if FEATURE_f067
  | 1 << 13 // F067
# endif
# if FEATURE_f068
  | 1 << 12 // F068
# endif
# if FEATURE_f069
  | 1 << 11 // F069
# endif
# if FEATURE_f070
  | 1 << 10 // F070
# endif
# if FEATURE_f071
  | 1 << 9  // F071
# endif
# if FEATURE_f072
  | 1 << 8  // F072
# endif
# if FEATURE_f073
  | 1 << 7  // F073
# endif
# if FEATURE_f074
  | 1 << 6  // F074
# endif
# if FEATURE_f075
  | 1 << 5  // F075
# endif
# if FEATURE_f076
  | 1 << 4  // F076
# endif
# if FEATURE_f077
  | 1 << 3  // F077
# endif
# if FEATURE_f078
  | 1 << 2  // F078
# endif
# if FEATURE_f079
  | 1 << 1  // F079
# endif
# if FEATURE_f080
  | 1 << 0  // F080
# endif
  , 0u      // n=81 : m=0
# if FEATURE_f081
  | 1 << 15 // F081
# endif
# if FEATURE_f082
  | 1 << 14 // F082
# endif
# if FEATURE_f083
  | 1 << 13 // F083
# endif
# if FEATURE_f084
  | 1 << 12 // F084
# endif
# if FEATURE_f085
  | 1 << 11 // F085
# endif
# if FEATURE_f086
  | 1 << 10 // F086
# endif
# if FEATURE_f087
  | 1 << 9  // F087
# endif
# if FEATURE_f088
  | 1 << 8  // F088
# endif
# if FEATURE_f089
  | 1 << 7  // F089
# endif
# if FEATURE_f090
  | 1 << 6  // F090
# endif
# if FEATURE_f091
  | 1 << 5  // F091
# endif
# if FEATURE_f092
  | 1 << 4  // F092
# endif
# if FEATURE_f093
  | 1 << 3  // F093
# endif
# if FEATURE_f094
  | 1 << 2  // F094
# endif
# if FEATURE_f095
  | 1 << 1  // F095
# endif
# if FEATURE_f096
  | 1 << 0  // F096
# endif
  , 0u      // n=97 : m=0
# if FEATURE_f097
  | 1 << 15 // F097
# endif
# if FEATURE_f098
  | 1 << 14 // F098
# endif
# if FEATURE_f099
  | 1 << 13 // F099
# endif
# if FEATURE_f100
  | 1 << 12 // F100
# endif
# if FEATURE_f101
  | 1 << 11 // F101
# endif
# if FEATURE_f102
  | 1 << 10 // F102
# endif
# if FEATURE_f103
  | 1 << 9  // F103
# endif
# if FEATURE_f104
  | 1 << 8  // F104
# endif
# if FEATURE_f105
  | 1 << 7  // F105
# endif
# if FEATURE_f106
  | 1 << 6  // F106
# endif
# if FEATURE_f107
  | 1 << 5  // F107
# endif
# if FEATURE_f108
  | 1 << 4  // F108
# endif
# if FEATURE_f109
  | 1 << 3  // F109
# endif
# if FEATURE_f110
  | 1 << 2  // F110
# endif
# if FEATURE_f111
  | 1 << 1  // F111
# endif
# if FEATURE_f112
  | 1 << 0  // F112
# endif
  , 0u      // n=113 : m=0
# if FEATURE_f113
  | 1 << 15 // F113
# endif
# if FEATURE_f114
  | 1 << 14 // F114
# endif
# if FEATURE_f115
  | 1 << 13 // F115
# endif
# if FEATURE_f116
  | 1 << 12 // F116
# endif
# if FEATURE_f117
  | 1 << 11 // F117
# endif
# if FEATURE_f118
  | 1 << 10 // F118
# endif
# if FEATURE_f119
  | 1 << 9  // F119
# endif
# if FEATURE_f120
  | 1 << 8  // F120
# endif
# if FEATURE_f121
  | 1 << 7  // F121
# endif
# if FEATURE_f122
  | 1 << 6  // F122
# endif
# if FEATURE_f123
  | 1 << 5  // F123
# endif
# if FEATURE_f124
  | 1 << 4  // F124
# endif
# if FEATURE_f125
  | 1 << 3  // F125
# endif
# if FEATURE_f126
  | 1 << 2  // F126
# endif
# if FEATURE_f127
  | 1 << 1  // F127
# endif
# if FEATURE_f128
  | 1 << 0  // F128
# endif
  , 0u      // n=129 : m=0
# if FEATURE_f129
  | 1 << 15 // F129
# endif
# if FEATURE_f130
  | 1 << 14 // F130
# endif
# if FEATURE_f131
  | 1 << 13 // F131
# endif
# if FEATURE_f132
  | 1 << 12 // F132
# endif
# if FEATURE_f133
  | 1 << 11 // F133
# endif
# if FEATURE_f134
  | 1 << 10 // F134
# endif
# if FEATURE_f135
  | 1 << 9  // F135
# endif
# if FEATURE_f136
  | 1 << 8  // F136
# endif
# if FEATURE_f137
  | 1 << 7  // F137
# endif
# if FEATURE_f138
  | 1 << 6  // F138
# endif
# if FEATURE_f139
  | 1 << 5  // F139
# endif
# if FEATURE_f140
  | 1 << 4  // F140
# endif
# if FEATURE_f141
  | 1 << 3  // F141
# endif
# if FEATURE_f142
  | 1 << 2  // F142
# endif
# if FEATURE_f143
  | 1 << 1  // F143
# endif
# if FEATURE_f144
  | 1 << 0  // F144
# endif
  , 0u      // n=145 : m=0
# if FEATURE_f145
  | 1 << 15 // F145
# endif
# if FEATURE_f146
  | 1 << 14 // F146
# endif
# if FEATURE_f147
  | 1 << 13 // F147
# endif
# if FEATURE_f148
  | 1 << 12 // F148
# endif
# if FEATURE_f149
  | 1 << 11 // F149
# endif
# if FEATURE_f150
  | 1 << 10 // F150
# endif
# if FEATURE_f151
  | 1 << 9  // F151
# endif
# if FEATURE_f152
  | 1 << 8  // F152
# endif
# if FEATURE_f153
  | 1 << 7  // F153
# endif
# if FEATURE_f154
  | 1 << 6  // F154
# endif
# if FEATURE_f155
  | 1 << 5  // F155
# endif
# if FEATURE_f156
  | 1 << 4  // F156
# endif
# if FEATURE_f157
  | 1 << 3  // F157
# endif
# if FEATURE_f158
  | 1 << 2  // F158
# endif
# if FEATURE_f159
  | 1 << 1  // F159
# endif
# if FEATURE_f160
  | 1 << 0  // F160
# endif
  , 0u      // n=161 : m=0
# if FEATURE_f161
  | 1 << 15 // F161
# endif
# if FEATURE_f162
  | 1 << 14 // F162
# endif
# if FEATURE_f163
  | 1 << 13 // F163
# endif
# if FEATURE_f164
  | 1 << 12 // F164
# endif
# if FEATURE_f165
  | 1 << 11 // F165
# endif
# if FEATURE_f166
  | 1 << 10 // F166
# endif
# if FEATURE_f167
  | 1 << 9  // F167
# endif
# if FEATURE_f168
  | 1 << 8  // F168
# endif
# if FEATURE_f169
  | 1 << 7  // F169
# endif
# if FEATURE_f170
  | 1 << 6  // F170
# endif
# if FEATURE_f171
  | 1 << 5  // F171
# endif
# if FEATURE_f172
  | 1 << 4  // F172
# endif
# if FEATURE_f173
  | 1 << 3  // F173
# endif
# if FEATURE_f174
  | 1 << 2  // F174
# endif
# if FEATURE_f175
  | 1 << 1  // F175
# endif
# if FEATURE_f176
  | 1 << 0  // F176
# endif
  , 0u      // n=177 : m=0
# if FEATURE_f177
  | 1 << 15 // F177
# endif
# if FEATURE_f178
  | 1 << 14 // F178
# endif
# if FEATURE_f179
  | 1 << 13 // F179
# endif
# if FEATURE_f180
  | 1 << 12 // F180
# endif
# if FEATURE_f181
  | 1 << 11 // F181
# endif
# if FEATURE_f182
  | 1 << 10 // F182
# endif
# if FEATURE_f183
  | 1 << 9  // F183
# endif
# if FEATURE_f184
  | 1 << 8  // F184
# endif
# if FEATURE_f185
  | 1 << 7  // F185
# endif
# if FEATURE_f186
  | 1 << 6  // F186
# endif
# if FEATURE_f187
  | 1 << 5  // F187
# endif
# if FEATURE_f188
  | 1 << 4  // F188
# endif
# if FEATURE_f189
  | 1 << 3  // F189
# endif
# if FEATURE_f190
  | 1 << 2  // F190
# endif
# if FEATURE_f191
  | 1 << 1  // F191
# endif
# if FEATURE_f192
  | 1 << 0  // F192
# endif
  , 0u      // n=193 : m=0
# if FEATURE_f193
  | 1 << 15 // F193
# endif
# if FEATURE_f194
  | 1 << 14 // F194
# endif
# if FEATURE_f195
  | 1 << 13 // F195
# endif
# if FEATURE_f196
  | 1 << 12 // F196
# endif
# if FEATURE_f197
  | 1 << 11 // F197
# endif
# if FEATURE_f198
  | 1 << 10 // F198
# endif
# if FEATURE_f199
  | 1 << 9  // F199
# endif
# if FEATURE_f200
  | 1 << 8  // F200
# endif
# if FEATURE_f201
  | 1 << 7  // F201
# endif
# if FEATURE_f202
  | 1 << 6  // F202
# endif
# if FEATURE_f203
  | 1 << 5  // F203
# endif
# if FEATURE_f204
  | 1 << 4  // F204
# endif
# if FEATURE_f205
  | 1 << 3  // F205
# endif
# if FEATURE_f206
  | 1 << 2  // F206
# endif
# if FEATURE_f207
  | 1 << 1  // F207
# endif
# if FEATURE_f208
  | 1 << 0  // F208
# endif
  , 0u      // n=209 : m=0
# if FEATURE_f209
  | 1 << 15 // F209
# endif
# if FEATURE_f210
  | 1 << 14 // F210
# endif
# if FEATURE_f211
  | 1 << 13 // F211
# endif
# if FEATURE_f212
  | 1 << 12 // F212
# endif
# if FEATURE_f213
  | 1 << 11 // F213
# endif
# if FEATURE_f214
  | 1 << 10 // F214
# endif
# if FEATURE_f215
  | 1 << 9  // F215
# endif
# if FEATURE_f216
  | 1 << 8  // F216
# endif
# if FEATURE_f217
  | 1 << 7  // F217
# endif
# if FEATURE_f218
  | 1 << 6  // F218
# endif
# if FEATURE_f219
  | 1 << 5  // F219
# endif
# if FEATURE_f220
  | 1 << 4  // F220
# endif
# if FEATURE_f221
  | 1 << 3  // F221
# endif
# if FEATURE_f222
  | 1 << 2  // F222
# endif
# if FEATURE_f223
  | 1 << 1  // F223
# endif
# if FEATURE_f224
  | 1 << 0  // F224
# endif
  , 0u      // n=225 : m=0
# if FEATURE_f225
  | 1 << 15 // F225
# endif
# if FEATURE_f226
  | 1 << 14 // F226
# endif
# if FEATURE_f227
  | 1 << 13 // F227
# endif
# if FEATURE_f228
  | 1 << 12 // F228
# endif
# if FEATURE_f229
  | 1 << 11 // F229
# endif
# if FEATURE_f230
  | 1 << 10 // F230
# endif
# if FEATURE_f231
  | 1 << 9  // F231
# endif
# if FEATURE_f232
  | 1 << 8  // F232
# endif
# if FEATURE_f233
  | 1 << 7  // F233
# endif
# if FEATURE_f234
  | 1 << 6  // F234
# endif
# if FEATURE_f235
  | 1 << 5  // F235
# endif
# if FEATURE_f236
  | 1 << 4  // F236
# endif
# if FEATURE_f237
  | 1 << 3  // F237
# endif
# if FEATURE_f238
  | 1 << 2  // F238
# endif
# if FEATURE_f239
  | 1 << 1  // F239
# endif
# if FEATURE_f240
  | 1 << 0  // F240
# endif
  , 0u      // n=241 : m=0
# if FEATURE_f241
  | 1 << 15 // F241
# endif
# if FEATURE_f242
  | 1 << 14 // F242
# endif
# if FEATURE_f243
  | 1 << 13 // F243
# endif
# if FEATURE_f244
  | 1 << 12 // F244
# endif
# if FEATURE_f245
  | 1 << 11 // F245
# endif
# if FEATURE_f246
  | 1 << 10 // F246
# endif
# if FEATURE_f247
  | 1 << 9  // F247
# endif
# if FEATURE_f248
  | 1 << 8  // F248
# endif
# if FEATURE_f249
  | 1 << 7  // F249
# endif
# if FEATURE_f250
  | 1 << 6  // F250
# endif
# if FEATURE_f251
  | 1 << 5  // F251
# endif
# if FEATURE_f252
  | 1 << 4  // F252
# endif
# if FEATURE_f253
  | 1 << 3  // F253
# endif
# if FEATURE_f254
  | 1 << 2  // F254
# endif
# if FEATURE_f255
  | 1 << 1  // F255
# endif
};

#endif // if FEATURE_BUILD_DESCRIPTION

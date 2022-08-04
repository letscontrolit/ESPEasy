#include "../Helpers/_NPlugin_init.h"

#if FEATURE_NOTIFIER

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/TimingStats.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include "../Globals/NPlugins.h"
#include "../Globals/Protocol.h"
#include "../Globals/Settings.h"

#include "../Helpers/Misc.h"

// ********************************************************************************
// Initialize all Controller NPlugins that where defined earlier
// and initialize the function call pointer into the CNPlugin array
// ********************************************************************************

// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*
#define ADDNPLUGIN(NNN, ID) if (addNPlugin(ID, x)) { NPlugin_ptr[x++] = &NPlugin_##NNN; }
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*

void NPluginInit()
{
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif
//  ProtocolIndex_to_NPlugin_id[NPLUGIN_MAX] = INVALID_N_PLUGIN_ID;
  uint8_t x;

  // Clear pointer table for all plugins
  for (x = 0; x < NPLUGIN_MAX; x++)
  {
    NPlugin_ptr[x] = nullptr;
    NPlugin_id[x]  = INVALID_N_PLUGIN_ID;
//    ProtocolIndex_to_NPlugin_id[x] = INVALID_N_PLUGIN_ID;
    // Do not initialize NPlugin_id_to_ProtocolIndex[x] to an invalid value. (it is map)
  }

  x = 0;

#ifdef USES_N001
  ADDNPLUGIN(001, 1)
#endif

#ifdef USES_N002
  ADDNPLUGIN(002, 2)
#endif

#ifdef USES_N003
  ADDNPLUGIN(003, 3)
#endif

#ifdef USES_N004
  ADDNPLUGIN(004, 4)
#endif

#ifdef USES_N005
  ADDNPLUGIN(005, 5)
#endif

#ifdef USES_N006
  ADDNPLUGIN(006, 6)
#endif

#ifdef USES_N007
  ADDNPLUGIN(007, 7)
#endif

#ifdef USES_N008
  ADDNPLUGIN(008, 8)
#endif

#ifdef USES_N009
  ADDNPLUGIN(009, 9)
#endif

#ifdef USES_N010
  ADDNPLUGIN(010, 10)
#endif

#ifdef USES_N011
  ADDNPLUGIN(011, 11)
#endif

#ifdef USES_N012
  ADDNPLUGIN(012, 12)
#endif

#ifdef USES_N013
  ADDNPLUGIN(013, 13)
#endif

#ifdef USES_N014
  ADDNPLUGIN(014, 14)
#endif

#ifdef USES_N015
  ADDNPLUGIN(015, 15)
#endif

#ifdef USES_N016
  ADDNPLUGIN(016, 16)
#endif

#ifdef USES_N017
  ADDNPLUGIN(017, 17)
#endif

#ifdef USES_N018
  ADDNPLUGIN(018, 18)
#endif

#ifdef USES_N019
  ADDNPLUGIN(019, 19)
#endif

#ifdef USES_N020
  ADDNPLUGIN(020, 20)
#endif

#ifdef USES_N021
  ADDNPLUGIN(021, 21)
#endif

#ifdef USES_N022
  ADDNPLUGIN(022, 22)
#endif

#ifdef USES_N023
  ADDNPLUGIN(023, 23)
#endif

#ifdef USES_N024
  ADDNPLUGIN(024, 24)
#endif

#ifdef USES_N025
  ADDNPLUGIN(025, 25)
#endif

#ifdef USES_N026
  ADDNPLUGIN(026, 26)
#endif

#ifdef USES_N027
  ADDNPLUGIN(027, 27)
#endif

#ifdef USES_N028
  ADDNPLUGIN(028, 28)
#endif

#ifdef USES_N029
  ADDNPLUGIN(029, 29)
#endif

#ifdef USES_N030
  ADDNPLUGIN(030, 30)
#endif

#ifdef USES_N031
  ADDNPLUGIN(031, 31)
#endif

#ifdef USES_N032
  ADDNPLUGIN(032, 32)
#endif

#ifdef USES_N033
  ADDNPLUGIN(033, 33)
#endif

#ifdef USES_N034
  ADDNPLUGIN(034, 34)
#endif

#ifdef USES_N035
  ADDNPLUGIN(035, 35)
#endif

#ifdef USES_N036
  ADDNPLUGIN(036, 36)
#endif

#ifdef USES_N037
  ADDNPLUGIN(037, 37)
#endif

#ifdef USES_N038
  ADDNPLUGIN(038, 38)
#endif

#ifdef USES_N039
  ADDNPLUGIN(039, 39)
#endif

#ifdef USES_N040
  ADDNPLUGIN(040, 40)
#endif

#ifdef USES_N041
  ADDNPLUGIN(041, 41)
#endif

#ifdef USES_N042
  ADDNPLUGIN(042, 42)
#endif

#ifdef USES_N043
  ADDNPLUGIN(043, 43)
#endif

#ifdef USES_N044
  ADDNPLUGIN(044, 44)
#endif

#ifdef USES_N045
  ADDNPLUGIN(045, 45)
#endif

#ifdef USES_N046
  ADDNPLUGIN(046, 46)
#endif

#ifdef USES_N047
  ADDNPLUGIN(047, 47)
#endif

#ifdef USES_N048
  ADDNPLUGIN(048, 48)
#endif

#ifdef USES_N049
  ADDNPLUGIN(049, 49)
#endif

#ifdef USES_N050
  ADDNPLUGIN(050, 50)
#endif

#ifdef USES_N051
  ADDNPLUGIN(051, 51)
#endif

#ifdef USES_N052
  ADDNPLUGIN(052, 52)
#endif

#ifdef USES_N053
  ADDNPLUGIN(053, 53)
#endif

#ifdef USES_N054
  ADDNPLUGIN(054, 54)
#endif

#ifdef USES_N055
  ADDNPLUGIN(055, 55)
#endif

#ifdef USES_N056
  ADDNPLUGIN(056, 56)
#endif

#ifdef USES_N057
  ADDNPLUGIN(057, 57)
#endif

#ifdef USES_N058
  ADDNPLUGIN(058, 58)
#endif

#ifdef USES_N059
  ADDNPLUGIN(059, 59)
#endif

#ifdef USES_N060
  ADDNPLUGIN(060, 60)
#endif

#ifdef USES_N061
  ADDNPLUGIN(061, 61)
#endif

#ifdef USES_N062
  ADDNPLUGIN(062, 62)
#endif

#ifdef USES_N063
  ADDNPLUGIN(063, 63)
#endif

#ifdef USES_N064
  ADDNPLUGIN(064, 64)
#endif

#ifdef USES_N065
  ADDNPLUGIN(065, 65)
#endif

#ifdef USES_N066
  ADDNPLUGIN(066, 66)
#endif

#ifdef USES_N067
  ADDNPLUGIN(067, 67)
#endif

#ifdef USES_N068
  ADDNPLUGIN(068, 68)
#endif

#ifdef USES_N069
  ADDNPLUGIN(069, 69)
#endif

#ifdef USES_N070
  ADDNPLUGIN(070, 70)
#endif

#ifdef USES_N071
  ADDNPLUGIN(071, 71)
#endif

#ifdef USES_N072
  ADDNPLUGIN(072, 72)
#endif

#ifdef USES_N073
  ADDNPLUGIN(073, 73)
#endif

#ifdef USES_N074
  ADDNPLUGIN(074, 74)
#endif

#ifdef USES_N075
  ADDNPLUGIN(075, 75)
#endif

#ifdef USES_N076
  ADDNPLUGIN(076, 76)
#endif

#ifdef USES_N077
  ADDNPLUGIN(077, 77)
#endif

#ifdef USES_N078
  ADDNPLUGIN(078, 78)
#endif

#ifdef USES_N079
  ADDNPLUGIN(079, 79)
#endif

#ifdef USES_N080
  ADDNPLUGIN(080, 80)
#endif

#ifdef USES_N081
  ADDNPLUGIN(081, 81)
#endif

#ifdef USES_N082
  ADDNPLUGIN(082, 82)
#endif

#ifdef USES_N083
  ADDNPLUGIN(083, 83)
#endif

#ifdef USES_N084
  ADDNPLUGIN(084, 84)
#endif

#ifdef USES_N085
  ADDNPLUGIN(085, 85)
#endif

#ifdef USES_N086
  ADDNPLUGIN(086, 86)
#endif

#ifdef USES_N087
  ADDNPLUGIN(087, 87)
#endif

#ifdef USES_N088
  ADDNPLUGIN(088, 88)
#endif

#ifdef USES_N089
  ADDNPLUGIN(089, 89)
#endif

#ifdef USES_N090
  ADDNPLUGIN(090, 90)
#endif

#ifdef USES_N091
  ADDNPLUGIN(091, 91)
#endif

#ifdef USES_N092
  ADDNPLUGIN(092, 92)
#endif

#ifdef USES_N093
  ADDNPLUGIN(093, 93)
#endif

#ifdef USES_N094
  ADDNPLUGIN(094, 94)
#endif

#ifdef USES_N095
  ADDNPLUGIN(095, 95)
#endif

#ifdef USES_N096
  ADDNPLUGIN(096, 96)
#endif

#ifdef USES_N097
  ADDNPLUGIN(097, 97)
#endif

#ifdef USES_N098
  ADDNPLUGIN(098, 98)
#endif

#ifdef USES_N099
  ADDNPLUGIN(099, 99)
#endif

#ifdef USES_N100
  ADDNPLUGIN(100, 100)
#endif

#ifdef USES_N101
  ADDNPLUGIN(101, 101)
#endif

#ifdef USES_N102
  ADDNPLUGIN(102, 102)
#endif

#ifdef USES_N103
  ADDNPLUGIN(103, 103)
#endif

#ifdef USES_N104
  ADDNPLUGIN(104, 104)
#endif

#ifdef USES_N105
  ADDNPLUGIN(105, 105)
#endif

#ifdef USES_N106
  ADDNPLUGIN(106, 106)
#endif

#ifdef USES_N107
  ADDNPLUGIN(107, 107)
#endif

#ifdef USES_N108
  ADDNPLUGIN(108, 108)
#endif

#ifdef USES_N109
  ADDNPLUGIN(109, 109)
#endif

#ifdef USES_N110
  ADDNPLUGIN(110, 110)
#endif

#ifdef USES_N111
  ADDNPLUGIN(111, 111)
#endif

#ifdef USES_N112
  ADDNPLUGIN(112, 112)
#endif

#ifdef USES_N113
  ADDNPLUGIN(113, 113)
#endif

#ifdef USES_N114
  ADDNPLUGIN(114, 114)
#endif

#ifdef USES_N115
  ADDNPLUGIN(115, 115)
#endif

#ifdef USES_N116
  ADDNPLUGIN(116, 116)
#endif

#ifdef USES_N117
  ADDNPLUGIN(117, 117)
#endif

#ifdef USES_N118
  ADDNPLUGIN(118, 118)
#endif

#ifdef USES_N119
  ADDNPLUGIN(119, 119)
#endif

#ifdef USES_N120
  ADDNPLUGIN(120, 120)
#endif

#ifdef USES_N121
  ADDNPLUGIN(121, 121)
#endif

#ifdef USES_N122
  ADDNPLUGIN(122, 122)
#endif

#ifdef USES_N123
  ADDNPLUGIN(123, 123)
#endif

#ifdef USES_N124
  ADDNPLUGIN(124, 124)
#endif

#ifdef USES_N125
  ADDNPLUGIN(125, 125)
#endif

#ifdef USES_N126
  ADDNPLUGIN(126, 126)
#endif

#ifdef USES_N127
  ADDNPLUGIN(127, 127)
#endif

#ifdef USES_N128
  ADDNPLUGIN(128, 128)
#endif

#ifdef USES_N129
  ADDNPLUGIN(129, 129)
#endif

#ifdef USES_N130
  ADDNPLUGIN(130, 130)
#endif

#ifdef USES_N131
  ADDNPLUGIN(131, 131)
#endif

#ifdef USES_N132
  ADDNPLUGIN(132, 132)
#endif

#ifdef USES_N133
  ADDNPLUGIN(133, 133)
#endif

#ifdef USES_N134
  ADDNPLUGIN(134, 134)
#endif

#ifdef USES_N135
  ADDNPLUGIN(135, 135)
#endif

#ifdef USES_N136
  ADDNPLUGIN(136, 136)
#endif

#ifdef USES_N137
  ADDNPLUGIN(137, 137)
#endif

#ifdef USES_N138
  ADDNPLUGIN(138, 138)
#endif

#ifdef USES_N139
  ADDNPLUGIN(139, 139)
#endif

#ifdef USES_N140
  ADDNPLUGIN(140, 140)
#endif

#ifdef USES_N141
  ADDNPLUGIN(141, 141)
#endif

#ifdef USES_N142
  ADDNPLUGIN(142, 142)
#endif

#ifdef USES_N143
  ADDNPLUGIN(143, 143)
#endif

#ifdef USES_N144
  ADDNPLUGIN(144, 144)
#endif

#ifdef USES_N145
  ADDNPLUGIN(145, 145)
#endif

#ifdef USES_N146
  ADDNPLUGIN(146, 146)
#endif

#ifdef USES_N147
  ADDNPLUGIN(147, 147)
#endif

#ifdef USES_N148
  ADDNPLUGIN(148, 148)
#endif

#ifdef USES_N149
  ADDNPLUGIN(149, 149)
#endif

#ifdef USES_N150
  ADDNPLUGIN(150, 150)
#endif

#ifdef USES_N151
  ADDNPLUGIN(151, 151)
#endif

#ifdef USES_N152
  ADDNPLUGIN(152, 152)
#endif

#ifdef USES_N153
  ADDNPLUGIN(153, 153)
#endif

#ifdef USES_N154
  ADDNPLUGIN(154, 154)
#endif

#ifdef USES_N155
  ADDNPLUGIN(155, 155)
#endif

#ifdef USES_N156
  ADDNPLUGIN(156, 156)
#endif

#ifdef USES_N157
  ADDNPLUGIN(157, 157)
#endif

#ifdef USES_N158
  ADDNPLUGIN(158, 158)
#endif

#ifdef USES_N159
  ADDNPLUGIN(159, 159)
#endif

#ifdef USES_N160
  ADDNPLUGIN(160, 160)
#endif

#ifdef USES_N161
  ADDNPLUGIN(161, 161)
#endif

#ifdef USES_N162
  ADDNPLUGIN(162, 162)
#endif

#ifdef USES_N163
  ADDNPLUGIN(163, 163)
#endif

#ifdef USES_N164
  ADDNPLUGIN(164, 164)
#endif

#ifdef USES_N165
  ADDNPLUGIN(165, 165)
#endif

#ifdef USES_N166
  ADDNPLUGIN(166, 166)
#endif

#ifdef USES_N167
  ADDNPLUGIN(167, 167)
#endif

#ifdef USES_N168
  ADDNPLUGIN(168, 168)
#endif

#ifdef USES_N169
  ADDNPLUGIN(169, 169)
#endif

#ifdef USES_N170
  ADDNPLUGIN(170, 170)
#endif

#ifdef USES_N171
  ADDNPLUGIN(171, 171)
#endif

#ifdef USES_N172
  ADDNPLUGIN(172, 172)
#endif

#ifdef USES_N173
  ADDNPLUGIN(173, 173)
#endif

#ifdef USES_N174
  ADDNPLUGIN(174, 174)
#endif

#ifdef USES_N175
  ADDNPLUGIN(175, 175)
#endif

#ifdef USES_N176
  ADDNPLUGIN(176, 176)
#endif

#ifdef USES_N177
  ADDNPLUGIN(177, 177)
#endif

#ifdef USES_N178
  ADDNPLUGIN(178, 178)
#endif

#ifdef USES_N179
  ADDNPLUGIN(179, 179)
#endif

#ifdef USES_N180
  ADDNPLUGIN(180, 180)
#endif

#ifdef USES_N181
  ADDNPLUGIN(181, 181)
#endif

#ifdef USES_N182
  ADDNPLUGIN(182, 182)
#endif

#ifdef USES_N183
  ADDNPLUGIN(183, 183)
#endif

#ifdef USES_N184
  ADDNPLUGIN(184, 184)
#endif

#ifdef USES_N185
  ADDNPLUGIN(185, 185)
#endif

#ifdef USES_N186
  ADDNPLUGIN(186, 186)
#endif

#ifdef USES_N187
  ADDNPLUGIN(187, 187)
#endif

#ifdef USES_N188
  ADDNPLUGIN(188, 188)
#endif

#ifdef USES_N189
  ADDNPLUGIN(189, 189)
#endif

#ifdef USES_N190
  ADDNPLUGIN(190, 190)
#endif

#ifdef USES_N191
  ADDNPLUGIN(191, 191)
#endif

#ifdef USES_N192
  ADDNPLUGIN(192, 192)
#endif

#ifdef USES_N193
  ADDNPLUGIN(193, 193)
#endif

#ifdef USES_N194
  ADDNPLUGIN(194, 194)
#endif

#ifdef USES_N195
  ADDNPLUGIN(195, 195)
#endif

#ifdef USES_N196
  ADDNPLUGIN(196, 196)
#endif

#ifdef USES_N197
  ADDNPLUGIN(197, 197)
#endif

#ifdef USES_N198
  ADDNPLUGIN(198, 198)
#endif

#ifdef USES_N199
  ADDNPLUGIN(199, 199)
#endif

#ifdef USES_N200
  ADDNPLUGIN(200, 200)
#endif

#ifdef USES_N201
  ADDNPLUGIN(201, 201)
#endif

#ifdef USES_N202
  ADDNPLUGIN(202, 202)
#endif

#ifdef USES_N203
  ADDNPLUGIN(203, 203)
#endif

#ifdef USES_N204
  ADDNPLUGIN(204, 204)
#endif

#ifdef USES_N205
  ADDNPLUGIN(205, 205)
#endif

#ifdef USES_N206
  ADDNPLUGIN(206, 206)
#endif

#ifdef USES_N207
  ADDNPLUGIN(207, 207)
#endif

#ifdef USES_N208
  ADDNPLUGIN(208, 208)
#endif

#ifdef USES_N209
  ADDNPLUGIN(209, 209)
#endif

#ifdef USES_N210
  ADDNPLUGIN(210, 210)
#endif

#ifdef USES_N211
  ADDNPLUGIN(211, 211)
#endif

#ifdef USES_N212
  ADDNPLUGIN(212, 212)
#endif

#ifdef USES_N213
  ADDNPLUGIN(213, 213)
#endif

#ifdef USES_N214
  ADDNPLUGIN(214, 214)
#endif

#ifdef USES_N215
  ADDNPLUGIN(215, 215)
#endif

#ifdef USES_N216
  ADDNPLUGIN(216, 216)
#endif

#ifdef USES_N217
  ADDNPLUGIN(217, 217)
#endif

#ifdef USES_N218
  ADDNPLUGIN(218, 218)
#endif

#ifdef USES_N219
  ADDNPLUGIN(219, 219)
#endif

#ifdef USES_N220
  ADDNPLUGIN(220, 220)
#endif

#ifdef USES_N221
  ADDNPLUGIN(221, 221)
#endif

#ifdef USES_N222
  ADDNPLUGIN(222, 222)
#endif

#ifdef USES_N223
  ADDNPLUGIN(223, 223)
#endif

#ifdef USES_N224
  ADDNPLUGIN(224, 224)
#endif

#ifdef USES_N225
  ADDNPLUGIN(225, 225)
#endif

#ifdef USES_N226
  ADDNPLUGIN(226, 226)
#endif

#ifdef USES_N227
  ADDNPLUGIN(227, 227)
#endif

#ifdef USES_N228
  ADDNPLUGIN(228, 228)
#endif

#ifdef USES_N229
  ADDNPLUGIN(229, 229)
#endif

#ifdef USES_N230
  ADDNPLUGIN(230, 230)
#endif

#ifdef USES_N231
  ADDNPLUGIN(231, 231)
#endif

#ifdef USES_N232
  ADDNPLUGIN(232, 232)
#endif

#ifdef USES_N233
  ADDNPLUGIN(233, 233)
#endif

#ifdef USES_N234
  ADDNPLUGIN(234, 234)
#endif

#ifdef USES_N235
  ADDNPLUGIN(235, 235)
#endif

#ifdef USES_N236
  ADDNPLUGIN(236, 236)
#endif

#ifdef USES_N237
  ADDNPLUGIN(237, 237)
#endif

#ifdef USES_N238
  ADDNPLUGIN(238, 238)
#endif

#ifdef USES_N239
  ADDNPLUGIN(239, 239)
#endif

#ifdef USES_N240
  ADDNPLUGIN(240, 240)
#endif

#ifdef USES_N241
  ADDNPLUGIN(241, 241)
#endif

#ifdef USES_N242
  ADDNPLUGIN(242, 242)
#endif

#ifdef USES_N243
  ADDNPLUGIN(243, 243)
#endif

#ifdef USES_N244
  ADDNPLUGIN(244, 244)
#endif

#ifdef USES_N245
  ADDNPLUGIN(245, 245)
#endif

#ifdef USES_N246
  ADDNPLUGIN(246, 246)
#endif

#ifdef USES_N247
  ADDNPLUGIN(247, 247)
#endif

#ifdef USES_N248
  ADDNPLUGIN(248, 248)
#endif

#ifdef USES_N249
  ADDNPLUGIN(249, 249)
#endif

#ifdef USES_N250
  ADDNPLUGIN(250, 250)
#endif

#ifdef USES_N251
  ADDNPLUGIN(251, 251)
#endif

#ifdef USES_N252
  ADDNPLUGIN(252, 252)
#endif

#ifdef USES_N253
  ADDNPLUGIN(253, 253)
#endif

#ifdef USES_N254
  ADDNPLUGIN(254, 254)
#endif

#ifdef USES_N255
  ADDNPLUGIN(255, 255)
#endif

// When extending this, search for EXTEND_CONTROLLER_IDS 
// in the code to find all places that need to be updated too.

  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("ADDNPLUGIN(...)"));
  #endif

  NPluginCall(NPlugin::Function::NPLUGIN_PROTOCOL_ADD, 0);
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("NPLUGIN_PROTOCOL_ADD"));
  #endif

}
#endif
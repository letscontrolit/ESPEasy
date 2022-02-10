#include "../Helpers/_CPlugin_init.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/TimingStats.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include "../Globals/CPlugins.h"
#include "../Globals/Protocol.h"
#include "../Globals/Settings.h"

#include "../Helpers/Misc.h"

// ********************************************************************************
// Initialize all Controller CPlugins that where defined earlier
// and initialize the function call pointer into the CCPlugin array
// ********************************************************************************

// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*
#define ADDCPLUGIN(NNN, ID) if (addCPlugin(ID, x)) { CPlugin_ptr[x++] = &CPlugin_##NNN; }
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*

void CPluginInit()
{
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif
  ProtocolIndex_to_CPlugin_id[CPLUGIN_MAX] = INVALID_C_PLUGIN_ID;
  uint8_t x;

  // Clear pointer table for all plugins
  for (x = 0; x < CPLUGIN_MAX; x++)
  {
    CPlugin_ptr[x]                 = nullptr;
    ProtocolIndex_to_CPlugin_id[x] = INVALID_C_PLUGIN_ID;
    // Do not initialize CPlugin_id_to_ProtocolIndex[x] to an invalid value. (it is map)
  }

  x = 0;

#ifdef USES_C001
  ADDCPLUGIN(001, 1)
#endif

#ifdef USES_C002
  ADDCPLUGIN(002, 2)
#endif

#ifdef USES_C003
  ADDCPLUGIN(003, 3)
#endif

#ifdef USES_C004
  ADDCPLUGIN(004, 4)
#endif

#ifdef USES_C005
  ADDCPLUGIN(005, 5)
#endif

#ifdef USES_C006
  ADDCPLUGIN(006, 6)
#endif

#ifdef USES_C007
  ADDCPLUGIN(007, 7)
#endif

#ifdef USES_C008
  ADDCPLUGIN(008, 8)
#endif

#ifdef USES_C009
  ADDCPLUGIN(009, 9)
#endif

#ifdef USES_C010
  ADDCPLUGIN(010, 10)
#endif

#ifdef USES_C011
  ADDCPLUGIN(011, 11)
#endif

#ifdef USES_C012
  ADDCPLUGIN(012, 12)
#endif

#ifdef USES_C013
  ADDCPLUGIN(013, 13)
#endif

#ifdef USES_C014
  ADDCPLUGIN(014, 14)
#endif

#ifdef USES_C015
  ADDCPLUGIN(015, 15)
#endif

#ifdef USES_C016
  ADDCPLUGIN(016, 16)
#endif

#ifdef USES_C017
  ADDCPLUGIN(017, 17)
#endif

#ifdef USES_C018
  ADDCPLUGIN(018, 18)
#endif

#ifdef USES_C019
  ADDCPLUGIN(019, 19)
#endif

#ifdef USES_C020
  ADDCPLUGIN(020, 20)
#endif

#ifdef USES_C021
  ADDCPLUGIN(021, 21)
#endif

#ifdef USES_C022
  ADDCPLUGIN(022, 22)
#endif

#ifdef USES_C023
  ADDCPLUGIN(023, 23)
#endif

#ifdef USES_C024
  ADDCPLUGIN(024, 24)
#endif

#ifdef USES_C025
  ADDCPLUGIN(025, 25)
#endif

#ifdef USES_C026
  ADDCPLUGIN(026, 26)
#endif

#ifdef USES_C027
  ADDCPLUGIN(027, 27)
#endif

#ifdef USES_C028
  ADDCPLUGIN(028, 28)
#endif

#ifdef USES_C029
  ADDCPLUGIN(029, 29)
#endif

#ifdef USES_C030
  ADDCPLUGIN(030, 30)
#endif

#ifdef USES_C031
  ADDCPLUGIN(031, 31)
#endif

#ifdef USES_C032
  ADDCPLUGIN(032, 32)
#endif

#ifdef USES_C033
  ADDCPLUGIN(033, 33)
#endif

#ifdef USES_C034
  ADDCPLUGIN(034, 34)
#endif

#ifdef USES_C035
  ADDCPLUGIN(035, 35)
#endif

#ifdef USES_C036
  ADDCPLUGIN(036, 36)
#endif

#ifdef USES_C037
  ADDCPLUGIN(037, 37)
#endif

#ifdef USES_C038
  ADDCPLUGIN(038, 38)
#endif

#ifdef USES_C039
  ADDCPLUGIN(039, 39)
#endif

#ifdef USES_C040
  ADDCPLUGIN(040, 40)
#endif

#ifdef USES_C041
  ADDCPLUGIN(041, 41)
#endif

#ifdef USES_C042
  ADDCPLUGIN(042, 42)
#endif

#ifdef USES_C043
  ADDCPLUGIN(043, 43)
#endif

#ifdef USES_C044
  ADDCPLUGIN(044, 44)
#endif

#ifdef USES_C045
  ADDCPLUGIN(045, 45)
#endif

#ifdef USES_C046
  ADDCPLUGIN(046, 46)
#endif

#ifdef USES_C047
  ADDCPLUGIN(047, 47)
#endif

#ifdef USES_C048
  ADDCPLUGIN(048, 48)
#endif

#ifdef USES_C049
  ADDCPLUGIN(049, 49)
#endif

#ifdef USES_C050
  ADDCPLUGIN(050, 50)
#endif

#ifdef USES_C051
  ADDCPLUGIN(051, 51)
#endif

#ifdef USES_C052
  ADDCPLUGIN(052, 52)
#endif

#ifdef USES_C053
  ADDCPLUGIN(053, 53)
#endif

#ifdef USES_C054
  ADDCPLUGIN(054, 54)
#endif

#ifdef USES_C055
  ADDCPLUGIN(055, 55)
#endif

#ifdef USES_C056
  ADDCPLUGIN(056, 56)
#endif

#ifdef USES_C057
  ADDCPLUGIN(057, 57)
#endif

#ifdef USES_C058
  ADDCPLUGIN(058, 58)
#endif

#ifdef USES_C059
  ADDCPLUGIN(059, 59)
#endif

#ifdef USES_C060
  ADDCPLUGIN(060, 60)
#endif

#ifdef USES_C061
  ADDCPLUGIN(061, 61)
#endif

#ifdef USES_C062
  ADDCPLUGIN(062, 62)
#endif

#ifdef USES_C063
  ADDCPLUGIN(063, 63)
#endif

#ifdef USES_C064
  ADDCPLUGIN(064, 64)
#endif

#ifdef USES_C065
  ADDCPLUGIN(065, 65)
#endif

#ifdef USES_C066
  ADDCPLUGIN(066, 66)
#endif

#ifdef USES_C067
  ADDCPLUGIN(067, 67)
#endif

#ifdef USES_C068
  ADDCPLUGIN(068, 68)
#endif

#ifdef USES_C069
  ADDCPLUGIN(069, 69)
#endif

#ifdef USES_C070
  ADDCPLUGIN(070, 70)
#endif

#ifdef USES_C071
  ADDCPLUGIN(071, 71)
#endif

#ifdef USES_C072
  ADDCPLUGIN(072, 72)
#endif

#ifdef USES_C073
  ADDCPLUGIN(073, 73)
#endif

#ifdef USES_C074
  ADDCPLUGIN(074, 74)
#endif

#ifdef USES_C075
  ADDCPLUGIN(075, 75)
#endif

#ifdef USES_C076
  ADDCPLUGIN(076, 76)
#endif

#ifdef USES_C077
  ADDCPLUGIN(077, 77)
#endif

#ifdef USES_C078
  ADDCPLUGIN(078, 78)
#endif

#ifdef USES_C079
  ADDCPLUGIN(079, 79)
#endif

#ifdef USES_C080
  ADDCPLUGIN(080, 80)
#endif

#ifdef USES_C081
  ADDCPLUGIN(081, 81)
#endif

#ifdef USES_C082
  ADDCPLUGIN(082, 82)
#endif

#ifdef USES_C083
  ADDCPLUGIN(083, 83)
#endif

#ifdef USES_C084
  ADDCPLUGIN(084, 84)
#endif

#ifdef USES_C085
  ADDCPLUGIN(085, 85)
#endif

#ifdef USES_C086
  ADDCPLUGIN(086, 86)
#endif

#ifdef USES_C087
  ADDCPLUGIN(087, 87)
#endif

#ifdef USES_C088
  ADDCPLUGIN(088, 88)
#endif

#ifdef USES_C089
  ADDCPLUGIN(089, 89)
#endif

#ifdef USES_C090
  ADDCPLUGIN(090, 90)
#endif

#ifdef USES_C091
  ADDCPLUGIN(091, 91)
#endif

#ifdef USES_C092
  ADDCPLUGIN(092, 92)
#endif

#ifdef USES_C093
  ADDCPLUGIN(093, 93)
#endif

#ifdef USES_C094
  ADDCPLUGIN(094, 94)
#endif

#ifdef USES_C095
  ADDCPLUGIN(095, 95)
#endif

#ifdef USES_C096
  ADDCPLUGIN(096, 96)
#endif

#ifdef USES_C097
  ADDCPLUGIN(097, 97)
#endif

#ifdef USES_C098
  ADDCPLUGIN(098, 98)
#endif

#ifdef USES_C099
  ADDCPLUGIN(099, 99)
#endif

#ifdef USES_C100
  ADDCPLUGIN(100, 100)
#endif

#ifdef USES_C101
  ADDCPLUGIN(101, 101)
#endif

#ifdef USES_C102
  ADDCPLUGIN(102, 102)
#endif

#ifdef USES_C103
  ADDCPLUGIN(103, 103)
#endif

#ifdef USES_C104
  ADDCPLUGIN(104, 104)
#endif

#ifdef USES_C105
  ADDCPLUGIN(105, 105)
#endif

#ifdef USES_C106
  ADDCPLUGIN(106, 106)
#endif

#ifdef USES_C107
  ADDCPLUGIN(107, 107)
#endif

#ifdef USES_C108
  ADDCPLUGIN(108, 108)
#endif

#ifdef USES_C109
  ADDCPLUGIN(109, 109)
#endif

#ifdef USES_C110
  ADDCPLUGIN(110, 110)
#endif

#ifdef USES_C111
  ADDCPLUGIN(111, 111)
#endif

#ifdef USES_C112
  ADDCPLUGIN(112, 112)
#endif

#ifdef USES_C113
  ADDCPLUGIN(113, 113)
#endif

#ifdef USES_C114
  ADDCPLUGIN(114, 114)
#endif

#ifdef USES_C115
  ADDCPLUGIN(115, 115)
#endif

#ifdef USES_C116
  ADDCPLUGIN(116, 116)
#endif

#ifdef USES_C117
  ADDCPLUGIN(117, 117)
#endif

#ifdef USES_C118
  ADDCPLUGIN(118, 118)
#endif

#ifdef USES_C119
  ADDCPLUGIN(119, 119)
#endif

#ifdef USES_C120
  ADDCPLUGIN(120, 120)
#endif

#ifdef USES_C121
  ADDCPLUGIN(121, 121)
#endif

#ifdef USES_C122
  ADDCPLUGIN(122, 122)
#endif

#ifdef USES_C123
  ADDCPLUGIN(123, 123)
#endif

#ifdef USES_C124
  ADDCPLUGIN(124, 124)
#endif

#ifdef USES_C125
  ADDCPLUGIN(125, 125)
#endif

#ifdef USES_C126
  ADDCPLUGIN(126, 126)
#endif

#ifdef USES_C127
  ADDCPLUGIN(127, 127)
#endif

#ifdef USES_C128
  ADDCPLUGIN(128, 128)
#endif

#ifdef USES_C129
  ADDCPLUGIN(129, 129)
#endif

#ifdef USES_C130
  ADDCPLUGIN(130, 130)
#endif

#ifdef USES_C131
  ADDCPLUGIN(131, 131)
#endif

#ifdef USES_C132
  ADDCPLUGIN(132, 132)
#endif

#ifdef USES_C133
  ADDCPLUGIN(133, 133)
#endif

#ifdef USES_C134
  ADDCPLUGIN(134, 134)
#endif

#ifdef USES_C135
  ADDCPLUGIN(135, 135)
#endif

#ifdef USES_C136
  ADDCPLUGIN(136, 136)
#endif

#ifdef USES_C137
  ADDCPLUGIN(137, 137)
#endif

#ifdef USES_C138
  ADDCPLUGIN(138, 138)
#endif

#ifdef USES_C139
  ADDCPLUGIN(139, 139)
#endif

#ifdef USES_C140
  ADDCPLUGIN(140, 140)
#endif

#ifdef USES_C141
  ADDCPLUGIN(141, 141)
#endif

#ifdef USES_C142
  ADDCPLUGIN(142, 142)
#endif

#ifdef USES_C143
  ADDCPLUGIN(143, 143)
#endif

#ifdef USES_C144
  ADDCPLUGIN(144, 144)
#endif

#ifdef USES_C145
  ADDCPLUGIN(145, 145)
#endif

#ifdef USES_C146
  ADDCPLUGIN(146, 146)
#endif

#ifdef USES_C147
  ADDCPLUGIN(147, 147)
#endif

#ifdef USES_C148
  ADDCPLUGIN(148, 148)
#endif

#ifdef USES_C149
  ADDCPLUGIN(149, 149)
#endif

#ifdef USES_C150
  ADDCPLUGIN(150, 150)
#endif

#ifdef USES_C151
  ADDCPLUGIN(151, 151)
#endif

#ifdef USES_C152
  ADDCPLUGIN(152, 152)
#endif

#ifdef USES_C153
  ADDCPLUGIN(153, 153)
#endif

#ifdef USES_C154
  ADDCPLUGIN(154, 154)
#endif

#ifdef USES_C155
  ADDCPLUGIN(155, 155)
#endif

#ifdef USES_C156
  ADDCPLUGIN(156, 156)
#endif

#ifdef USES_C157
  ADDCPLUGIN(157, 157)
#endif

#ifdef USES_C158
  ADDCPLUGIN(158, 158)
#endif

#ifdef USES_C159
  ADDCPLUGIN(159, 159)
#endif

#ifdef USES_C160
  ADDCPLUGIN(160, 160)
#endif

#ifdef USES_C161
  ADDCPLUGIN(161, 161)
#endif

#ifdef USES_C162
  ADDCPLUGIN(162, 162)
#endif

#ifdef USES_C163
  ADDCPLUGIN(163, 163)
#endif

#ifdef USES_C164
  ADDCPLUGIN(164, 164)
#endif

#ifdef USES_C165
  ADDCPLUGIN(165, 165)
#endif

#ifdef USES_C166
  ADDCPLUGIN(166, 166)
#endif

#ifdef USES_C167
  ADDCPLUGIN(167, 167)
#endif

#ifdef USES_C168
  ADDCPLUGIN(168, 168)
#endif

#ifdef USES_C169
  ADDCPLUGIN(169, 169)
#endif

#ifdef USES_C170
  ADDCPLUGIN(170, 170)
#endif

#ifdef USES_C171
  ADDCPLUGIN(171, 171)
#endif

#ifdef USES_C172
  ADDCPLUGIN(172, 172)
#endif

#ifdef USES_C173
  ADDCPLUGIN(173, 173)
#endif

#ifdef USES_C174
  ADDCPLUGIN(174, 174)
#endif

#ifdef USES_C175
  ADDCPLUGIN(175, 175)
#endif

#ifdef USES_C176
  ADDCPLUGIN(176, 176)
#endif

#ifdef USES_C177
  ADDCPLUGIN(177, 177)
#endif

#ifdef USES_C178
  ADDCPLUGIN(178, 178)
#endif

#ifdef USES_C179
  ADDCPLUGIN(179, 179)
#endif

#ifdef USES_C180
  ADDCPLUGIN(180, 180)
#endif

#ifdef USES_C181
  ADDCPLUGIN(181, 181)
#endif

#ifdef USES_C182
  ADDCPLUGIN(182, 182)
#endif

#ifdef USES_C183
  ADDCPLUGIN(183, 183)
#endif

#ifdef USES_C184
  ADDCPLUGIN(184, 184)
#endif

#ifdef USES_C185
  ADDCPLUGIN(185, 185)
#endif

#ifdef USES_C186
  ADDCPLUGIN(186, 186)
#endif

#ifdef USES_C187
  ADDCPLUGIN(187, 187)
#endif

#ifdef USES_C188
  ADDCPLUGIN(188, 188)
#endif

#ifdef USES_C189
  ADDCPLUGIN(189, 189)
#endif

#ifdef USES_C190
  ADDCPLUGIN(190, 190)
#endif

#ifdef USES_C191
  ADDCPLUGIN(191, 191)
#endif

#ifdef USES_C192
  ADDCPLUGIN(192, 192)
#endif

#ifdef USES_C193
  ADDCPLUGIN(193, 193)
#endif

#ifdef USES_C194
  ADDCPLUGIN(194, 194)
#endif

#ifdef USES_C195
  ADDCPLUGIN(195, 195)
#endif

#ifdef USES_C196
  ADDCPLUGIN(196, 196)
#endif

#ifdef USES_C197
  ADDCPLUGIN(197, 197)
#endif

#ifdef USES_C198
  ADDCPLUGIN(198, 198)
#endif

#ifdef USES_C199
  ADDCPLUGIN(199, 199)
#endif

#ifdef USES_C200
  ADDCPLUGIN(200, 200)
#endif

#ifdef USES_C201
  ADDCPLUGIN(201, 201)
#endif

#ifdef USES_C202
  ADDCPLUGIN(202, 202)
#endif

#ifdef USES_C203
  ADDCPLUGIN(203, 203)
#endif

#ifdef USES_C204
  ADDCPLUGIN(204, 204)
#endif

#ifdef USES_C205
  ADDCPLUGIN(205, 205)
#endif

#ifdef USES_C206
  ADDCPLUGIN(206, 206)
#endif

#ifdef USES_C207
  ADDCPLUGIN(207, 207)
#endif

#ifdef USES_C208
  ADDCPLUGIN(208, 208)
#endif

#ifdef USES_C209
  ADDCPLUGIN(209, 209)
#endif

#ifdef USES_C210
  ADDCPLUGIN(210, 210)
#endif

#ifdef USES_C211
  ADDCPLUGIN(211, 211)
#endif

#ifdef USES_C212
  ADDCPLUGIN(212, 212)
#endif

#ifdef USES_C213
  ADDCPLUGIN(213, 213)
#endif

#ifdef USES_C214
  ADDCPLUGIN(214, 214)
#endif

#ifdef USES_C215
  ADDCPLUGIN(215, 215)
#endif

#ifdef USES_C216
  ADDCPLUGIN(216, 216)
#endif

#ifdef USES_C217
  ADDCPLUGIN(217, 217)
#endif

#ifdef USES_C218
  ADDCPLUGIN(218, 218)
#endif

#ifdef USES_C219
  ADDCPLUGIN(219, 219)
#endif

#ifdef USES_C220
  ADDCPLUGIN(220, 220)
#endif

#ifdef USES_C221
  ADDCPLUGIN(221, 221)
#endif

#ifdef USES_C222
  ADDCPLUGIN(222, 222)
#endif

#ifdef USES_C223
  ADDCPLUGIN(223, 223)
#endif

#ifdef USES_C224
  ADDCPLUGIN(224, 224)
#endif

#ifdef USES_C225
  ADDCPLUGIN(225, 225)
#endif

#ifdef USES_C226
  ADDCPLUGIN(226, 226)
#endif

#ifdef USES_C227
  ADDCPLUGIN(227, 227)
#endif

#ifdef USES_C228
  ADDCPLUGIN(228, 228)
#endif

#ifdef USES_C229
  ADDCPLUGIN(229, 229)
#endif

#ifdef USES_C230
  ADDCPLUGIN(230, 230)
#endif

#ifdef USES_C231
  ADDCPLUGIN(231, 231)
#endif

#ifdef USES_C232
  ADDCPLUGIN(232, 232)
#endif

#ifdef USES_C233
  ADDCPLUGIN(233, 233)
#endif

#ifdef USES_C234
  ADDCPLUGIN(234, 234)
#endif

#ifdef USES_C235
  ADDCPLUGIN(235, 235)
#endif

#ifdef USES_C236
  ADDCPLUGIN(236, 236)
#endif

#ifdef USES_C237
  ADDCPLUGIN(237, 237)
#endif

#ifdef USES_C238
  ADDCPLUGIN(238, 238)
#endif

#ifdef USES_C239
  ADDCPLUGIN(239, 239)
#endif

#ifdef USES_C240
  ADDCPLUGIN(240, 240)
#endif

#ifdef USES_C241
  ADDCPLUGIN(241, 241)
#endif

#ifdef USES_C242
  ADDCPLUGIN(242, 242)
#endif

#ifdef USES_C243
  ADDCPLUGIN(243, 243)
#endif

#ifdef USES_C244
  ADDCPLUGIN(244, 244)
#endif

#ifdef USES_C245
  ADDCPLUGIN(245, 245)
#endif

#ifdef USES_C246
  ADDCPLUGIN(246, 246)
#endif

#ifdef USES_C247
  ADDCPLUGIN(247, 247)
#endif

#ifdef USES_C248
  ADDCPLUGIN(248, 248)
#endif

#ifdef USES_C249
  ADDCPLUGIN(249, 249)
#endif

#ifdef USES_C250
  ADDCPLUGIN(250, 250)
#endif

#ifdef USES_C251
  ADDCPLUGIN(251, 251)
#endif

#ifdef USES_C252
  ADDCPLUGIN(252, 252)
#endif

#ifdef USES_C253
  ADDCPLUGIN(253, 253)
#endif

#ifdef USES_C254
  ADDCPLUGIN(254, 254)
#endif

#ifdef USES_C255
  ADDCPLUGIN(255, 255)
#endif

// When extending this, search for EXTEND_CONTROLLER_IDS 
// in the code to find all places that need to be updated too.

  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("ADDCPLUGIN(...)"));
  #endif

  CPluginCall(CPlugin::Function::CPLUGIN_PROTOCOL_ADD, 0);
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("CPLUGIN_PROTOCOL_ADD"));
  #endif


  // Set all not supported cplugins to disabled.
  for (controllerIndex_t controller = 0; controller < CONTROLLER_MAX; ++controller) {
    if (!supportedCPluginID(Settings.Protocol[controller])) {
      Settings.ControllerEnabled[controller] = false;
    }
  }
  CPluginCall(CPlugin::Function::CPLUGIN_INIT_ALL, 0);
}

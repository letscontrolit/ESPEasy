#include "../Helpers/_Plugin_init.h"

#include "../../ESPEasy_common.h"

#include "../Globals/Plugins.h"
#include "../Globals/Settings.h"

#include "../Helpers/Misc.h"


// ********************************************************************************
// Initialize all plugins that where defined earlier
// and initialize the function call pointer into the plugin array
// ********************************************************************************


// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*
#define ADDPLUGIN(NNN, ID) if (addPlugin(ID, x)) Plugin_ptr[x++] = &Plugin_##NNN;
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*

void PluginInit()
{
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  DeviceIndex_to_Plugin_id[PLUGIN_MAX] = INVALID_PLUGIN_ID;
  // Clear pointer table for all plugins
  for (deviceIndex_t x = 0; x < PLUGIN_MAX; x++)
  {
    Plugin_ptr[x] = nullptr;
    DeviceIndex_to_Plugin_id[x] = INVALID_PLUGIN_ID;
    // Do not initialize Plugin_id_to_DeviceIndex[x] to an invalid value. (it is map)
  }
  uint32_t x = 0; // Used in ADDPLUGIN macro

#ifdef USES_P001
  ADDPLUGIN(001, 1)
#endif

#ifdef USES_P002
  ADDPLUGIN(002, 2)
#endif

#ifdef USES_P003
  ADDPLUGIN(003, 3)
#endif

#ifdef USES_P004
  ADDPLUGIN(004, 4)
#endif

#ifdef USES_P005
  ADDPLUGIN(005, 5)
#endif

#ifdef USES_P006
  ADDPLUGIN(006, 6)
#endif

#ifdef USES_P007
  ADDPLUGIN(007, 7)
#endif

#ifdef USES_P008
  ADDPLUGIN(008, 8)
#endif

#ifdef USES_P009
  ADDPLUGIN(009, 9)
#endif

#ifdef USES_P010
  ADDPLUGIN(010, 10)
#endif

#ifdef USES_P011
  ADDPLUGIN(011, 11)
#endif

#ifdef USES_P012
  ADDPLUGIN(012, 12)
#endif

#ifdef USES_P013
  ADDPLUGIN(013, 13)
#endif

#ifdef USES_P014
  ADDPLUGIN(014, 14)
#endif

#ifdef USES_P015
  ADDPLUGIN(015, 15)
#endif

#ifdef USES_P016
  ADDPLUGIN(016, 16)
#endif

#ifdef USES_P017
  ADDPLUGIN(017, 17)
#endif

#ifdef USES_P018
  ADDPLUGIN(018, 18)
#endif

#ifdef USES_P019
  ADDPLUGIN(019, 19)
#endif

#ifdef USES_P020
  ADDPLUGIN(020, 20)
#endif

#ifdef USES_P021
  ADDPLUGIN(021, 21)
#endif

#ifdef USES_P022
  ADDPLUGIN(022, 22)
#endif

#ifdef USES_P023
  ADDPLUGIN(023, 23)
#endif

#ifdef USES_P024
  ADDPLUGIN(024, 24)
#endif

#ifdef USES_P025
  ADDPLUGIN(025, 25)
#endif

#ifdef USES_P026
  ADDPLUGIN(026, 26)
#endif

#ifdef USES_P027
  ADDPLUGIN(027, 27)
#endif

#ifdef USES_P028
  ADDPLUGIN(028, 28)
#endif

#ifdef USES_P029
  ADDPLUGIN(029, 29)
#endif

#ifdef USES_P030
  ADDPLUGIN(030, 30)
#endif

#ifdef USES_P031
  ADDPLUGIN(031, 31)
#endif

#ifdef USES_P032
  ADDPLUGIN(032, 32)
#endif

#ifdef USES_P033
  ADDPLUGIN(033, 33)
#endif

#ifdef USES_P034
  ADDPLUGIN(034, 34)
#endif

#ifdef USES_P035
  ADDPLUGIN(035, 35)
#endif

#ifdef USES_P036
  ADDPLUGIN(036, 36)
#endif

#ifdef USES_P037
  ADDPLUGIN(037, 37)
#endif

#ifdef USES_P038
  ADDPLUGIN(038, 38)
#endif

#ifdef USES_P039
  ADDPLUGIN(039, 39)
#endif

#ifdef USES_P040
  ADDPLUGIN(040, 40)
#endif

#ifdef USES_P041
  ADDPLUGIN(041, 41)
#endif

#ifdef USES_P042
  ADDPLUGIN(042, 42)
#endif

#ifdef USES_P043
  ADDPLUGIN(043, 43)
#endif

#ifdef USES_P044
  ADDPLUGIN(044, 44)
#endif

#ifdef USES_P045
  ADDPLUGIN(045, 45)
#endif

#ifdef USES_P046
  ADDPLUGIN(046, 46)
#endif

#ifdef USES_P047
  ADDPLUGIN(047, 47)
#endif

#ifdef USES_P048
  ADDPLUGIN(048, 48)
#endif

#ifdef USES_P049
  ADDPLUGIN(049, 49)
#endif

#ifdef USES_P050
  ADDPLUGIN(050, 50)
#endif

#ifdef USES_P051
  ADDPLUGIN(051, 51)
#endif

#ifdef USES_P052
  ADDPLUGIN(052, 52)
#endif

#ifdef USES_P053
  ADDPLUGIN(053, 53)
#endif

#ifdef USES_P054
  ADDPLUGIN(054, 54)
#endif

#ifdef USES_P055
  ADDPLUGIN(055, 55)
#endif

#ifdef USES_P056
  ADDPLUGIN(056, 56)
#endif

#ifdef USES_P057
  ADDPLUGIN(057, 57)
#endif

#ifdef USES_P058
  ADDPLUGIN(058, 58)
#endif

#ifdef USES_P059
  ADDPLUGIN(059, 59)
#endif

#ifdef USES_P060
  ADDPLUGIN(060, 60)
#endif

#ifdef USES_P061
  ADDPLUGIN(061, 61)
#endif

#ifdef USES_P062
  ADDPLUGIN(062, 62)
#endif

#ifdef USES_P063
  ADDPLUGIN(063, 63)
#endif

#ifdef USES_P064
  ADDPLUGIN(064, 64)
#endif

#ifdef USES_P065
  ADDPLUGIN(065, 65)
#endif

#ifdef USES_P066
  ADDPLUGIN(066, 66)
#endif

#ifdef USES_P067
  ADDPLUGIN(067, 67)
#endif

#ifdef USES_P068
  ADDPLUGIN(068, 68)
#endif

#ifdef USES_P069
  ADDPLUGIN(069, 69)
#endif

#ifdef USES_P070
  ADDPLUGIN(070, 70)
#endif

#ifdef USES_P071
  ADDPLUGIN(071, 71)
#endif

#ifdef USES_P072
  ADDPLUGIN(072, 72)
#endif

#ifdef USES_P073
  ADDPLUGIN(073, 73)
#endif

#ifdef USES_P074
  ADDPLUGIN(074, 74)
#endif

#ifdef USES_P075
  ADDPLUGIN(075, 75)
#endif

#ifdef USES_P076
  ADDPLUGIN(076, 76)
#endif

#ifdef USES_P077
  ADDPLUGIN(077, 77)
#endif

#ifdef USES_P078
  ADDPLUGIN(078, 78)
#endif

#ifdef USES_P079
  ADDPLUGIN(079, 79)
#endif

#ifdef USES_P080
  ADDPLUGIN(080, 80)
#endif

#ifdef USES_P081
  ADDPLUGIN(081, 81)
#endif

#ifdef USES_P082
  ADDPLUGIN(082, 82)
#endif

#ifdef USES_P083
  ADDPLUGIN(083, 83)
#endif

#ifdef USES_P084
  ADDPLUGIN(084, 84)
#endif

#ifdef USES_P085
  ADDPLUGIN(085, 85)
#endif

#ifdef USES_P086
  ADDPLUGIN(086, 86)
#endif

#ifdef USES_P087
  ADDPLUGIN(087, 87)
#endif

#ifdef USES_P088
  ADDPLUGIN(088, 88)
#endif

#ifdef USES_P089
  ADDPLUGIN(089, 89)
#endif

#ifdef USES_P090
  ADDPLUGIN(090, 90)
#endif

#ifdef USES_P091
  ADDPLUGIN(091, 91)
#endif

#ifdef USES_P092
  ADDPLUGIN(092, 92)
#endif

#ifdef USES_P093
  ADDPLUGIN(093, 93)
#endif

#ifdef USES_P094
  ADDPLUGIN(094, 94)
#endif

#ifdef USES_P095
  ADDPLUGIN(095, 95)
#endif

#ifdef USES_P096
  ADDPLUGIN(096, 96)
#endif

#ifdef USES_P097
  ADDPLUGIN(097, 97)
#endif

#ifdef USES_P098
  ADDPLUGIN(098, 98)
#endif

#ifdef USES_P099
  ADDPLUGIN(099, 99)
#endif

#ifdef USES_P100
  ADDPLUGIN(100, 100)
#endif

#ifdef USES_P101
  ADDPLUGIN(101, 101)
#endif

#ifdef USES_P102
  ADDPLUGIN(102, 102)
#endif

#ifdef USES_P103
  ADDPLUGIN(103, 103)
#endif

#ifdef USES_P104
  ADDPLUGIN(104, 104)
#endif

#ifdef USES_P105
  ADDPLUGIN(105, 105)
#endif

#ifdef USES_P106
  ADDPLUGIN(106, 106)
#endif

#ifdef USES_P107
  ADDPLUGIN(107, 107)
#endif

#ifdef USES_P108
  ADDPLUGIN(108, 108)
#endif

#ifdef USES_P109
  ADDPLUGIN(109, 109)
#endif

#ifdef USES_P110
  ADDPLUGIN(110, 110)
#endif

#ifdef USES_P111
  ADDPLUGIN(111, 111)
#endif

#ifdef USES_P112
  ADDPLUGIN(112, 112)
#endif

#ifdef USES_P113
  ADDPLUGIN(113, 113)
#endif

#ifdef USES_P114
  ADDPLUGIN(114, 114)
#endif

#ifdef USES_P115
  ADDPLUGIN(115, 115)
#endif

#ifdef USES_P116
  ADDPLUGIN(116, 116)
#endif

#ifdef USES_P117
  ADDPLUGIN(117, 117)
#endif

#ifdef USES_P118
  ADDPLUGIN(118, 118)
#endif

#ifdef USES_P119
  ADDPLUGIN(119, 119)
#endif

#ifdef USES_P120
  ADDPLUGIN(120, 120)
#endif

#ifdef USES_P121
  ADDPLUGIN(121, 121)
#endif

#ifdef USES_P122
  ADDPLUGIN(122, 122)
#endif

#ifdef USES_P123
  ADDPLUGIN(123, 123)
#endif

#ifdef USES_P124
  ADDPLUGIN(124, 124)
#endif

#ifdef USES_P125
  ADDPLUGIN(125, 125)
#endif

#ifdef USES_P126
  ADDPLUGIN(126, 126)
#endif

#ifdef USES_P127
  ADDPLUGIN(127, 127)
#endif

#ifdef USES_P128
  ADDPLUGIN(128, 128)
#endif

#ifdef USES_P129
  ADDPLUGIN(129, 129)
#endif

#ifdef USES_P130
  ADDPLUGIN(130, 130)
#endif

#ifdef USES_P131
  ADDPLUGIN(131, 131)
#endif

#ifdef USES_P132
  ADDPLUGIN(132, 132)
#endif

#ifdef USES_P133
  ADDPLUGIN(133, 133)
#endif

#ifdef USES_P134
  ADDPLUGIN(134, 134)
#endif

#ifdef USES_P135
  ADDPLUGIN(135, 135)
#endif

#ifdef USES_P136
  ADDPLUGIN(136, 136)
#endif

#ifdef USES_P137
  ADDPLUGIN(137, 137)
#endif

#ifdef USES_P138
  ADDPLUGIN(138, 138)
#endif

#ifdef USES_P139
  ADDPLUGIN(139, 139)
#endif

#ifdef USES_P140
  ADDPLUGIN(140, 140)
#endif

#ifdef USES_P141
  ADDPLUGIN(141, 141)
#endif

#ifdef USES_P142
  ADDPLUGIN(142, 142)
#endif

#ifdef USES_P143
  ADDPLUGIN(143, 143)
#endif

#ifdef USES_P144
  ADDPLUGIN(144, 144)
#endif

#ifdef USES_P145
  ADDPLUGIN(145, 145)
#endif

#ifdef USES_P146
  ADDPLUGIN(146, 146)
#endif

#ifdef USES_P147
  ADDPLUGIN(147, 147)
#endif

#ifdef USES_P148
  ADDPLUGIN(148, 148)
#endif

#ifdef USES_P149
  ADDPLUGIN(149, 149)
#endif

#ifdef USES_P150
  ADDPLUGIN(150, 150)
#endif

#ifdef USES_P151
  ADDPLUGIN(151, 151)
#endif

#ifdef USES_P152
  ADDPLUGIN(152, 152)
#endif

#ifdef USES_P153
  ADDPLUGIN(153, 153)
#endif

#ifdef USES_P154
  ADDPLUGIN(154, 154)
#endif

#ifdef USES_P155
  ADDPLUGIN(155, 155)
#endif

#ifdef USES_P156
  ADDPLUGIN(156, 156)
#endif

#ifdef USES_P157
  ADDPLUGIN(157, 157)
#endif

#ifdef USES_P158
  ADDPLUGIN(158, 158)
#endif

#ifdef USES_P159
  ADDPLUGIN(159, 159)
#endif

#ifdef USES_P160
  ADDPLUGIN(160, 160)
#endif

#ifdef USES_P161
  ADDPLUGIN(161, 161)
#endif

#ifdef USES_P162
  ADDPLUGIN(162, 162)
#endif

#ifdef USES_P163
  ADDPLUGIN(163, 163)
#endif

#ifdef USES_P164
  ADDPLUGIN(164, 164)
#endif

#ifdef USES_P165
  ADDPLUGIN(165, 165)
#endif

#ifdef USES_P166
  ADDPLUGIN(166, 166)
#endif

#ifdef USES_P167
  ADDPLUGIN(167, 167)
#endif

#ifdef USES_P168
  ADDPLUGIN(168, 168)
#endif

#ifdef USES_P169
  ADDPLUGIN(169, 169)
#endif

#ifdef USES_P170
  ADDPLUGIN(170, 170)
#endif

#ifdef USES_P171
  ADDPLUGIN(171, 171)
#endif

#ifdef USES_P172
  ADDPLUGIN(172, 172)
#endif

#ifdef USES_P173
  ADDPLUGIN(173, 173)
#endif

#ifdef USES_P174
  ADDPLUGIN(174, 174)
#endif

#ifdef USES_P175
  ADDPLUGIN(175, 175)
#endif

#ifdef USES_P176
  ADDPLUGIN(176, 176)
#endif

#ifdef USES_P177
  ADDPLUGIN(177, 177)
#endif

#ifdef USES_P178
  ADDPLUGIN(178, 178)
#endif

#ifdef USES_P179
  ADDPLUGIN(179, 179)
#endif

#ifdef USES_P180
  ADDPLUGIN(180, 180)
#endif

#ifdef USES_P181
  ADDPLUGIN(181, 181)
#endif

#ifdef USES_P182
  ADDPLUGIN(182, 182)
#endif

#ifdef USES_P183
  ADDPLUGIN(183, 183)
#endif

#ifdef USES_P184
  ADDPLUGIN(184, 184)
#endif

#ifdef USES_P185
  ADDPLUGIN(185, 185)
#endif

#ifdef USES_P186
  ADDPLUGIN(186, 186)
#endif

#ifdef USES_P187
  ADDPLUGIN(187, 187)
#endif

#ifdef USES_P188
  ADDPLUGIN(188, 188)
#endif

#ifdef USES_P189
  ADDPLUGIN(189, 189)
#endif

#ifdef USES_P190
  ADDPLUGIN(190, 190)
#endif

#ifdef USES_P191
  ADDPLUGIN(191, 191)
#endif

#ifdef USES_P192
  ADDPLUGIN(192, 192)
#endif

#ifdef USES_P193
  ADDPLUGIN(193, 193)
#endif

#ifdef USES_P194
  ADDPLUGIN(194, 194)
#endif

#ifdef USES_P195
  ADDPLUGIN(195, 195)
#endif

#ifdef USES_P196
  ADDPLUGIN(196, 196)
#endif

#ifdef USES_P197
  ADDPLUGIN(197, 197)
#endif

#ifdef USES_P198
  ADDPLUGIN(198, 198)
#endif

#ifdef USES_P199
  ADDPLUGIN(199, 199)
#endif

#ifdef USES_P200
  ADDPLUGIN(200, 200)
#endif

#ifdef USES_P201
  ADDPLUGIN(201, 201)
#endif

#ifdef USES_P202
  ADDPLUGIN(202, 202)
#endif

#ifdef USES_P203
  ADDPLUGIN(203, 203)
#endif

#ifdef USES_P204
  ADDPLUGIN(204, 204)
#endif

#ifdef USES_P205
  ADDPLUGIN(205, 205)
#endif

#ifdef USES_P206
  ADDPLUGIN(206, 206)
#endif

#ifdef USES_P207
  ADDPLUGIN(207, 207)
#endif

#ifdef USES_P208
  ADDPLUGIN(208, 208)
#endif

#ifdef USES_P209
  ADDPLUGIN(209, 209)
#endif

#ifdef USES_P210
  ADDPLUGIN(210, 210)
#endif

#ifdef USES_P211
  ADDPLUGIN(211, 211)
#endif

#ifdef USES_P212
  ADDPLUGIN(212, 212)
#endif

#ifdef USES_P213
  ADDPLUGIN(213, 213)
#endif

#ifdef USES_P214
  ADDPLUGIN(214, 214)
#endif

#ifdef USES_P215
  ADDPLUGIN(215, 215)
#endif

#ifdef USES_P216
  ADDPLUGIN(216, 216)
#endif

#ifdef USES_P217
  ADDPLUGIN(217, 217)
#endif

#ifdef USES_P218
  ADDPLUGIN(218, 218)
#endif

#ifdef USES_P219
  ADDPLUGIN(219, 219)
#endif

#ifdef USES_P220
  ADDPLUGIN(220, 220)
#endif

#ifdef USES_P221
  ADDPLUGIN(221, 221)
#endif

#ifdef USES_P222
  ADDPLUGIN(222, 222)
#endif

#ifdef USES_P223
  ADDPLUGIN(223, 223)
#endif

#ifdef USES_P224
  ADDPLUGIN(224, 224)
#endif

#ifdef USES_P225
  ADDPLUGIN(225, 225)
#endif

#ifdef USES_P226
  ADDPLUGIN(226, 226)
#endif

#ifdef USES_P227
  ADDPLUGIN(227, 227)
#endif

#ifdef USES_P228
  ADDPLUGIN(228, 228)
#endif

#ifdef USES_P229
  ADDPLUGIN(229, 229)
#endif

#ifdef USES_P230
  ADDPLUGIN(230, 230)
#endif

#ifdef USES_P231
  ADDPLUGIN(231, 231)
#endif

#ifdef USES_P232
  ADDPLUGIN(232, 232)
#endif

#ifdef USES_P233
  ADDPLUGIN(233, 233)
#endif

#ifdef USES_P234
  ADDPLUGIN(234, 234)
#endif

#ifdef USES_P235
  ADDPLUGIN(235, 235)
#endif

#ifdef USES_P236
  ADDPLUGIN(236, 236)
#endif

#ifdef USES_P237
  ADDPLUGIN(237, 237)
#endif

#ifdef USES_P238
  ADDPLUGIN(238, 238)
#endif

#ifdef USES_P239
  ADDPLUGIN(239, 239)
#endif

#ifdef USES_P240
  ADDPLUGIN(240, 240)
#endif

#ifdef USES_P241
  ADDPLUGIN(241, 241)
#endif

#ifdef USES_P242
  ADDPLUGIN(242, 242)
#endif

#ifdef USES_P243
  ADDPLUGIN(243, 243)
#endif

#ifdef USES_P244
  ADDPLUGIN(244, 244)
#endif

#ifdef USES_P245
  ADDPLUGIN(245, 245)
#endif

#ifdef USES_P246
  ADDPLUGIN(246, 246)
#endif

#ifdef USES_P247
  ADDPLUGIN(247, 247)
#endif

#ifdef USES_P248
  ADDPLUGIN(248, 248)
#endif

#ifdef USES_P249
  ADDPLUGIN(249, 249)
#endif

#ifdef USES_P250
  ADDPLUGIN(250, 250)
#endif

#ifdef USES_P251
  ADDPLUGIN(251, 251)
#endif

#ifdef USES_P252
  ADDPLUGIN(252, 252)
#endif

#ifdef USES_P253
  ADDPLUGIN(253, 253)
#endif

#ifdef USES_P254
  ADDPLUGIN(254, 254)
#endif

#ifdef USES_P255
  ADDPLUGIN(255, 255)
#endif

#ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("ADDPLUGIN(...)"));
#endif

  String dummy;
  PluginCall(PLUGIN_DEVICE_ADD, nullptr, dummy);
    // Set all not supported plugins to disabled.
  for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; ++taskIndex) {
    if (!supportedPluginID(Settings.TaskDeviceNumber[taskIndex])) {
      Settings.TaskDeviceEnabled[taskIndex] = false;
    }
  }

#ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("PLUGIN_DEVICE_ADD"));
#endif

  PluginCall(PLUGIN_INIT_ALL, nullptr, dummy);
#ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("PLUGIN_INIT_ALL"));
#endif

  sortDeviceIndexArray(); // Used in device selector dropdown.
}


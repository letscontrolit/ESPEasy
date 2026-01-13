#pragma once

#include "../../../ESPEasy_common.h"

#include "../../../src/DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/NetworkDriverStruct.h"
#include "../../../src/DataTypes/ESPEasy_plugin_functions.h"
#include "../DataTypes/NWPluginID.h"
#include "../DataTypes/NetworkDriverIndex.h"


namespace ESPEasy {
namespace net {

networkDriverIndex_t do_getNetworkDriverIndex_from_NWPluginID(nwpluginID_t pluginID);
nwpluginID_t do_getNWPluginID_from_NetworkDriverIndex(networkDriverIndex_t networkDriverIndex);
bool do_check_validNetworkDriverIndex(networkDriverIndex_t networkDriverIndex);

nwpluginID_t getHighestIncludedNWPluginID();

NetworkDriverStruct& getNetworkDriverStruct(networkDriverIndex_t networkDriverIndex);

// Should only be called from NWPluginCall, or maybe for very special occasions
bool do_NWPluginCall(networkDriverIndex_t networkDriverIndex, NWPlugin::Function Function, EventStruct *event, String& string);


void NWPluginSetup();
void NWPluginInit();

// Start or stop network as how it is set in the Settings
void NWPlugin_Exit_Init(networkIndex_t networkIndex);


// Macro to forward declare the NWPlugin_NNN functions.
// N.B. Some controllers also have a do_process_cNNN_delay_queue function.
//      Forward declaration of these is done in ControllerQueue/ControllerDelayHandlerStruct.h
//
// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*
#define ADDNWPLUGIN_H(NNN) bool NWPlugin_##NNN(NWPlugin::Function function, EventStruct *event, String& string);
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*

#ifdef USES_NW001
  ADDNWPLUGIN_H(001)
#endif

#ifdef USES_NW002
  ADDNWPLUGIN_H(002)
#endif

#ifdef USES_NW003
  ADDNWPLUGIN_H(003)
#endif

#ifdef USES_NW004
  ADDNWPLUGIN_H(004)
#endif

#ifdef USES_NW005
  ADDNWPLUGIN_H(005)
#endif

#ifdef USES_NW006
  ADDNWPLUGIN_H(006)
#endif

#ifdef USES_NW007
  ADDNWPLUGIN_H(007)
#endif

#ifdef USES_NW008
  ADDNWPLUGIN_H(008)
#endif

#ifdef USES_NW009
  ADDNWPLUGIN_H(009)
#endif

#ifdef USES_NW010
  ADDNWPLUGIN_H(010)
#endif

#ifdef USES_NW011
  ADDNWPLUGIN_H(011)
#endif

#ifdef USES_NW012
  ADDNWPLUGIN_H(012)
#endif

#ifdef USES_NW013
  ADDNWPLUGIN_H(013)
#endif

#ifdef USES_NW014
  ADDNWPLUGIN_H(014)
#endif

#ifdef USES_NW015
  ADDNWPLUGIN_H(015)
#endif

#ifdef USES_NW016
  ADDNWPLUGIN_H(016)
#endif

#ifdef USES_NW017
  ADDNWPLUGIN_H(017)
#endif

#ifdef USES_NW018
  ADDNWPLUGIN_H(018)
#endif

#ifdef USES_NW019
  ADDNWPLUGIN_H(019)
#endif

#ifdef USES_NW020
  ADDNWPLUGIN_H(020)
#endif

#ifdef USES_NW021
  ADDNWPLUGIN_H(021)
#endif

#ifdef USES_NW022
  ADDNWPLUGIN_H(022)
#endif

#ifdef USES_NW023
  ADDNWPLUGIN_H(023)
#endif

#ifdef USES_NW024
  ADDNWPLUGIN_H(024)
#endif

#ifdef USES_NW025
  ADDNWPLUGIN_H(025)
#endif

#ifdef USES_NW026
  ADDNWPLUGIN_H(026)
#endif

#ifdef USES_NW027
  ADDNWPLUGIN_H(027)
#endif

#ifdef USES_NW028
  ADDNWPLUGIN_H(028)
#endif

#ifdef USES_NW029
  ADDNWPLUGIN_H(029)
#endif

#ifdef USES_NW030
  ADDNWPLUGIN_H(030)
#endif

#ifdef USES_NW031
  ADDNWPLUGIN_H(031)
#endif

#ifdef USES_NW032
  ADDNWPLUGIN_H(032)
#endif

#ifdef USES_NW033
  ADDNWPLUGIN_H(033)
#endif

#ifdef USES_NW034
  ADDNWPLUGIN_H(034)
#endif

#ifdef USES_NW035
  ADDNWPLUGIN_H(035)
#endif

#ifdef USES_NW036
  ADDNWPLUGIN_H(036)
#endif

#ifdef USES_NW037
  ADDNWPLUGIN_H(037)
#endif

#ifdef USES_NW038
  ADDNWPLUGIN_H(038)
#endif

#ifdef USES_NW039
  ADDNWPLUGIN_H(039)
#endif

#ifdef USES_NW040
  ADDNWPLUGIN_H(040)
#endif

#ifdef USES_NW041
  ADDNWPLUGIN_H(041)
#endif

#ifdef USES_NW042
  ADDNWPLUGIN_H(042)
#endif

#ifdef USES_NW043
  ADDNWPLUGIN_H(043)
#endif

#ifdef USES_NW044
  ADDNWPLUGIN_H(044)
#endif

#ifdef USES_NW045
  ADDNWPLUGIN_H(045)
#endif

#ifdef USES_NW046
  ADDNWPLUGIN_H(046)
#endif

#ifdef USES_NW047
  ADDNWPLUGIN_H(047)
#endif

#ifdef USES_NW048
  ADDNWPLUGIN_H(048)
#endif

#ifdef USES_NW049
  ADDNWPLUGIN_H(049)
#endif

#ifdef USES_NW050
  ADDNWPLUGIN_H(050)
#endif

#ifdef USES_NW051
  ADDNWPLUGIN_H(051)
#endif

#ifdef USES_NW052
  ADDNWPLUGIN_H(052)
#endif

#ifdef USES_NW053
  ADDNWPLUGIN_H(053)
#endif

#ifdef USES_NW054
  ADDNWPLUGIN_H(054)
#endif

#ifdef USES_NW055
  ADDNWPLUGIN_H(055)
#endif

#ifdef USES_NW056
  ADDNWPLUGIN_H(056)
#endif

#ifdef USES_NW057
  ADDNWPLUGIN_H(057)
#endif

#ifdef USES_NW058
  ADDNWPLUGIN_H(058)
#endif

#ifdef USES_NW059
  ADDNWPLUGIN_H(059)
#endif

#ifdef USES_NW060
  ADDNWPLUGIN_H(060)
#endif

#ifdef USES_NW061
  ADDNWPLUGIN_H(061)
#endif

#ifdef USES_NW062
  ADDNWPLUGIN_H(062)
#endif

#ifdef USES_NW063
  ADDNWPLUGIN_H(063)
#endif

#ifdef USES_NW064
  ADDNWPLUGIN_H(064)
#endif

#ifdef USES_NW065
  ADDNWPLUGIN_H(065)
#endif

#ifdef USES_NW066
  ADDNWPLUGIN_H(066)
#endif

#ifdef USES_NW067
  ADDNWPLUGIN_H(067)
#endif

#ifdef USES_NW068
  ADDNWPLUGIN_H(068)
#endif

#ifdef USES_NW069
  ADDNWPLUGIN_H(069)
#endif

#ifdef USES_NW070
  ADDNWPLUGIN_H(070)
#endif

#ifdef USES_NW071
  ADDNWPLUGIN_H(071)
#endif

#ifdef USES_NW072
  ADDNWPLUGIN_H(072)
#endif

#ifdef USES_NW073
  ADDNWPLUGIN_H(073)
#endif

#ifdef USES_NW074
  ADDNWPLUGIN_H(074)
#endif

#ifdef USES_NW075
  ADDNWPLUGIN_H(075)
#endif

#ifdef USES_NW076
  ADDNWPLUGIN_H(076)
#endif

#ifdef USES_NW077
  ADDNWPLUGIN_H(077)
#endif

#ifdef USES_NW078
  ADDNWPLUGIN_H(078)
#endif

#ifdef USES_NW079
  ADDNWPLUGIN_H(079)
#endif

#ifdef USES_NW080
  ADDNWPLUGIN_H(080)
#endif

#ifdef USES_NW081
  ADDNWPLUGIN_H(081)
#endif

#ifdef USES_NW082
  ADDNWPLUGIN_H(082)
#endif

#ifdef USES_NW083
  ADDNWPLUGIN_H(083)
#endif

#ifdef USES_NW084
  ADDNWPLUGIN_H(084)
#endif

#ifdef USES_NW085
  ADDNWPLUGIN_H(085)
#endif

#ifdef USES_NW086
  ADDNWPLUGIN_H(086)
#endif

#ifdef USES_NW087
  ADDNWPLUGIN_H(087)
#endif

#ifdef USES_NW088
  ADDNWPLUGIN_H(088)
#endif

#ifdef USES_NW089
  ADDNWPLUGIN_H(089)
#endif

#ifdef USES_NW090
  ADDNWPLUGIN_H(090)
#endif

#ifdef USES_NW091
  ADDNWPLUGIN_H(091)
#endif

#ifdef USES_NW092
  ADDNWPLUGIN_H(092)
#endif

#ifdef USES_NW093
  ADDNWPLUGIN_H(093)
#endif

#ifdef USES_NW094
  ADDNWPLUGIN_H(094)
#endif

#ifdef USES_NW095
  ADDNWPLUGIN_H(095)
#endif

#ifdef USES_NW096
  ADDNWPLUGIN_H(096)
#endif

#ifdef USES_NW097
  ADDNWPLUGIN_H(097)
#endif

#ifdef USES_NW098
  ADDNWPLUGIN_H(098)
#endif

#ifdef USES_NW099
  ADDNWPLUGIN_H(099)
#endif

#ifdef USES_NW100
  ADDNWPLUGIN_H(100)
#endif

#ifdef USES_NW101
  ADDNWPLUGIN_H(101)
#endif

#ifdef USES_NW102
  ADDNWPLUGIN_H(102)
#endif

#ifdef USES_NW103
  ADDNWPLUGIN_H(103)
#endif

#ifdef USES_NW104
  ADDNWPLUGIN_H(104)
#endif

#ifdef USES_NW105
  ADDNWPLUGIN_H(105)
#endif

#ifdef USES_NW106
  ADDNWPLUGIN_H(106)
#endif

#ifdef USES_NW107
  ADDNWPLUGIN_H(107)
#endif

#ifdef USES_NW108
  ADDNWPLUGIN_H(108)
#endif

#ifdef USES_NW109
  ADDNWPLUGIN_H(109)
#endif

#ifdef USES_NW110
  ADDNWPLUGIN_H(110)
#endif

#ifdef USES_NW111
  ADDNWPLUGIN_H(111)
#endif

#ifdef USES_NW112
  ADDNWPLUGIN_H(112)
#endif

#ifdef USES_NW113
  ADDNWPLUGIN_H(113)
#endif

#ifdef USES_NW114
  ADDNWPLUGIN_H(114)
#endif

#ifdef USES_NW115
  ADDNWPLUGIN_H(115)
#endif

#ifdef USES_NW116
  ADDNWPLUGIN_H(116)
#endif

#ifdef USES_NW117
  ADDNWPLUGIN_H(117)
#endif

#ifdef USES_NW118
  ADDNWPLUGIN_H(118)
#endif

#ifdef USES_NW119
  ADDNWPLUGIN_H(119)
#endif

#ifdef USES_NW120
  ADDNWPLUGIN_H(120)
#endif

#ifdef USES_NW121
  ADDNWPLUGIN_H(121)
#endif

#ifdef USES_NW122
  ADDNWPLUGIN_H(122)
#endif

#ifdef USES_NW123
  ADDNWPLUGIN_H(123)
#endif

#ifdef USES_NW124
  ADDNWPLUGIN_H(124)
#endif

#ifdef USES_NW125
  ADDNWPLUGIN_H(125)
#endif

#ifdef USES_NW126
  ADDNWPLUGIN_H(126)
#endif

#ifdef USES_NW127
  ADDNWPLUGIN_H(127)
#endif

#ifdef USES_NW128
  ADDNWPLUGIN_H(128)
#endif

#ifdef USES_NW129
  ADDNWPLUGIN_H(129)
#endif

#ifdef USES_NW130
  ADDNWPLUGIN_H(130)
#endif

#ifdef USES_NW131
  ADDNWPLUGIN_H(131)
#endif

#ifdef USES_NW132
  ADDNWPLUGIN_H(132)
#endif

#ifdef USES_NW133
  ADDNWPLUGIN_H(133)
#endif

#ifdef USES_NW134
  ADDNWPLUGIN_H(134)
#endif

#ifdef USES_NW135
  ADDNWPLUGIN_H(135)
#endif

#ifdef USES_NW136
  ADDNWPLUGIN_H(136)
#endif

#ifdef USES_NW137
  ADDNWPLUGIN_H(137)
#endif

#ifdef USES_NW138
  ADDNWPLUGIN_H(138)
#endif

#ifdef USES_NW139
  ADDNWPLUGIN_H(139)
#endif

#ifdef USES_NW140
  ADDNWPLUGIN_H(140)
#endif

#ifdef USES_NW141
  ADDNWPLUGIN_H(141)
#endif

#ifdef USES_NW142
  ADDNWPLUGIN_H(142)
#endif

#ifdef USES_NW143
  ADDNWPLUGIN_H(143)
#endif

#ifdef USES_NW144
  ADDNWPLUGIN_H(144)
#endif

#ifdef USES_NW145
  ADDNWPLUGIN_H(145)
#endif

#ifdef USES_NW146
  ADDNWPLUGIN_H(146)
#endif

#ifdef USES_NW147
  ADDNWPLUGIN_H(147)
#endif

#ifdef USES_NW148
  ADDNWPLUGIN_H(148)
#endif

#ifdef USES_NW149
  ADDNWPLUGIN_H(149)
#endif

#ifdef USES_NW150
  ADDNWPLUGIN_H(150)
#endif

#ifdef USES_NW151
  ADDNWPLUGIN_H(151)
#endif

#ifdef USES_NW152
  ADDNWPLUGIN_H(152)
#endif

#ifdef USES_NW153
  ADDNWPLUGIN_H(153)
#endif

#ifdef USES_NW154
  ADDNWPLUGIN_H(154)
#endif

#ifdef USES_NW155
  ADDNWPLUGIN_H(155)
#endif

#ifdef USES_NW156
  ADDNWPLUGIN_H(156)
#endif

#ifdef USES_NW157
  ADDNWPLUGIN_H(157)
#endif

#ifdef USES_NW158
  ADDNWPLUGIN_H(158)
#endif

#ifdef USES_NW159
  ADDNWPLUGIN_H(159)
#endif

#ifdef USES_NW160
  ADDNWPLUGIN_H(160)
#endif

#ifdef USES_NW161
  ADDNWPLUGIN_H(161)
#endif

#ifdef USES_NW162
  ADDNWPLUGIN_H(162)
#endif

#ifdef USES_NW163
  ADDNWPLUGIN_H(163)
#endif

#ifdef USES_NW164
  ADDNWPLUGIN_H(164)
#endif

#ifdef USES_NW165
  ADDNWPLUGIN_H(165)
#endif

#ifdef USES_NW166
  ADDNWPLUGIN_H(166)
#endif

#ifdef USES_NW167
  ADDNWPLUGIN_H(167)
#endif

#ifdef USES_NW168
  ADDNWPLUGIN_H(168)
#endif

#ifdef USES_NW169
  ADDNWPLUGIN_H(169)
#endif

#ifdef USES_NW170
  ADDNWPLUGIN_H(170)
#endif

#ifdef USES_NW171
  ADDNWPLUGIN_H(171)
#endif

#ifdef USES_NW172
  ADDNWPLUGIN_H(172)
#endif

#ifdef USES_NW173
  ADDNWPLUGIN_H(173)
#endif

#ifdef USES_NW174
  ADDNWPLUGIN_H(174)
#endif

#ifdef USES_NW175
  ADDNWPLUGIN_H(175)
#endif

#ifdef USES_NW176
  ADDNWPLUGIN_H(176)
#endif

#ifdef USES_NW177
  ADDNWPLUGIN_H(177)
#endif

#ifdef USES_NW178
  ADDNWPLUGIN_H(178)
#endif

#ifdef USES_NW179
  ADDNWPLUGIN_H(179)
#endif

#ifdef USES_NW180
  ADDNWPLUGIN_H(180)
#endif

#ifdef USES_NW181
  ADDNWPLUGIN_H(181)
#endif

#ifdef USES_NW182
  ADDNWPLUGIN_H(182)
#endif

#ifdef USES_NW183
  ADDNWPLUGIN_H(183)
#endif

#ifdef USES_NW184
  ADDNWPLUGIN_H(184)
#endif

#ifdef USES_NW185
  ADDNWPLUGIN_H(185)
#endif

#ifdef USES_NW186
  ADDNWPLUGIN_H(186)
#endif

#ifdef USES_NW187
  ADDNWPLUGIN_H(187)
#endif

#ifdef USES_NW188
  ADDNWPLUGIN_H(188)
#endif

#ifdef USES_NW189
  ADDNWPLUGIN_H(189)
#endif

#ifdef USES_NW190
  ADDNWPLUGIN_H(190)
#endif

#ifdef USES_NW191
  ADDNWPLUGIN_H(191)
#endif

#ifdef USES_NW192
  ADDNWPLUGIN_H(192)
#endif

#ifdef USES_NW193
  ADDNWPLUGIN_H(193)
#endif

#ifdef USES_NW194
  ADDNWPLUGIN_H(194)
#endif

#ifdef USES_NW195
  ADDNWPLUGIN_H(195)
#endif

#ifdef USES_NW196
  ADDNWPLUGIN_H(196)
#endif

#ifdef USES_NW197
  ADDNWPLUGIN_H(197)
#endif

#ifdef USES_NW198
  ADDNWPLUGIN_H(198)
#endif

#ifdef USES_NW199
  ADDNWPLUGIN_H(199)
#endif

#ifdef USES_NW200
  ADDNWPLUGIN_H(200)
#endif

#ifdef USES_NW201
  ADDNWPLUGIN_H(201)
#endif

#ifdef USES_NW202
  ADDNWPLUGIN_H(202)
#endif

#ifdef USES_NW203
  ADDNWPLUGIN_H(203)
#endif

#ifdef USES_NW204
  ADDNWPLUGIN_H(204)
#endif

#ifdef USES_NW205
  ADDNWPLUGIN_H(205)
#endif

#ifdef USES_NW206
  ADDNWPLUGIN_H(206)
#endif

#ifdef USES_NW207
  ADDNWPLUGIN_H(207)
#endif

#ifdef USES_NW208
  ADDNWPLUGIN_H(208)
#endif

#ifdef USES_NW209
  ADDNWPLUGIN_H(209)
#endif

#ifdef USES_NW210
  ADDNWPLUGIN_H(210)
#endif

#ifdef USES_NW211
  ADDNWPLUGIN_H(211)
#endif

#ifdef USES_NW212
  ADDNWPLUGIN_H(212)
#endif

#ifdef USES_NW213
  ADDNWPLUGIN_H(213)
#endif

#ifdef USES_NW214
  ADDNWPLUGIN_H(214)
#endif

#ifdef USES_NW215
  ADDNWPLUGIN_H(215)
#endif

#ifdef USES_NW216
  ADDNWPLUGIN_H(216)
#endif

#ifdef USES_NW217
  ADDNWPLUGIN_H(217)
#endif

#ifdef USES_NW218
  ADDNWPLUGIN_H(218)
#endif

#ifdef USES_NW219
  ADDNWPLUGIN_H(219)
#endif

#ifdef USES_NW220
  ADDNWPLUGIN_H(220)
#endif

#ifdef USES_NW221
  ADDNWPLUGIN_H(221)
#endif

#ifdef USES_NW222
  ADDNWPLUGIN_H(222)
#endif

#ifdef USES_NW223
  ADDNWPLUGIN_H(223)
#endif

#ifdef USES_NW224
  ADDNWPLUGIN_H(224)
#endif

#ifdef USES_NW225
  ADDNWPLUGIN_H(225)
#endif

#ifdef USES_NW226
  ADDNWPLUGIN_H(226)
#endif

#ifdef USES_NW227
  ADDNWPLUGIN_H(227)
#endif

#ifdef USES_NW228
  ADDNWPLUGIN_H(228)
#endif

#ifdef USES_NW229
  ADDNWPLUGIN_H(229)
#endif

#ifdef USES_NW230
  ADDNWPLUGIN_H(230)
#endif

#ifdef USES_NW231
  ADDNWPLUGIN_H(231)
#endif

#ifdef USES_NW232
  ADDNWPLUGIN_H(232)
#endif

#ifdef USES_NW233
  ADDNWPLUGIN_H(233)
#endif

#ifdef USES_NW234
  ADDNWPLUGIN_H(234)
#endif

#ifdef USES_NW235
  ADDNWPLUGIN_H(235)
#endif

#ifdef USES_NW236
  ADDNWPLUGIN_H(236)
#endif

#ifdef USES_NW237
  ADDNWPLUGIN_H(237)
#endif

#ifdef USES_NW238
  ADDNWPLUGIN_H(238)
#endif

#ifdef USES_NW239
  ADDNWPLUGIN_H(239)
#endif

#ifdef USES_NW240
  ADDNWPLUGIN_H(240)
#endif

#ifdef USES_NW241
  ADDNWPLUGIN_H(241)
#endif

#ifdef USES_NW242
  ADDNWPLUGIN_H(242)
#endif

#ifdef USES_NW243
  ADDNWPLUGIN_H(243)
#endif

#ifdef USES_NW244
  ADDNWPLUGIN_H(244)
#endif

#ifdef USES_NW245
  ADDNWPLUGIN_H(245)
#endif

#ifdef USES_NW246
  ADDNWPLUGIN_H(246)
#endif

#ifdef USES_NW247
  ADDNWPLUGIN_H(247)
#endif

#ifdef USES_NW248
  ADDNWPLUGIN_H(248)
#endif

#ifdef USES_NW249
  ADDNWPLUGIN_H(249)
#endif

#ifdef USES_NW250
  ADDNWPLUGIN_H(250)
#endif

#ifdef USES_NW251
  ADDNWPLUGIN_H(251)
#endif

#ifdef USES_NW252
  ADDNWPLUGIN_H(252)
#endif

#ifdef USES_NW253
  ADDNWPLUGIN_H(253)
#endif

#ifdef USES_NW254
  ADDNWPLUGIN_H(254)
#endif

#ifdef USES_NW255
  ADDNWPLUGIN_H(255)
#endif

#undef ADDNWPLUGIN_H

} // namespace net
} // namespace ESPEasy

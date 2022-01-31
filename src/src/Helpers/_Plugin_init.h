#ifndef HELPERS__PLUGIN_INIT_H
#define HELPERS__PLUGIN_INIT_H

#include "../../ESPEasy_common.h"



#include <Arduino.h>


struct EventStruct;

void PluginInit();

// Macro to forward declare the Plugin_NNN functions.
//
// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*
#define ADDPLUGIN_H(NNN) boolean Plugin_##NNN(uint8_t function, struct EventStruct *event, String& string);
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*


#ifdef USES_P001
  ADDPLUGIN_H(001)
#endif

#ifdef USES_P002
  ADDPLUGIN_H(002)
#endif

#ifdef USES_P003
  ADDPLUGIN_H(003)
#endif

#ifdef USES_P004
  ADDPLUGIN_H(004)
#endif

#ifdef USES_P005
  ADDPLUGIN_H(005)
#endif

#ifdef USES_P006
  ADDPLUGIN_H(006)
#endif

#ifdef USES_P007
  ADDPLUGIN_H(007)
#endif

#ifdef USES_P008
  ADDPLUGIN_H(008)
#endif

#ifdef USES_P009
  ADDPLUGIN_H(009)
#endif

#ifdef USES_P010
  ADDPLUGIN_H(010)
#endif

#ifdef USES_P011
  ADDPLUGIN_H(011)
#endif

#ifdef USES_P012
  ADDPLUGIN_H(012)
#endif

#ifdef USES_P013
  ADDPLUGIN_H(013)
#endif

#ifdef USES_P014
  ADDPLUGIN_H(014)
#endif

#ifdef USES_P015
  ADDPLUGIN_H(015)
#endif

#ifdef USES_P016
  ADDPLUGIN_H(016)
#endif

#ifdef USES_P017
  ADDPLUGIN_H(017)
#endif

#ifdef USES_P018
  ADDPLUGIN_H(018)
#endif

#ifdef USES_P019
  ADDPLUGIN_H(019)
#endif

#ifdef USES_P020
  ADDPLUGIN_H(020)
#endif

#ifdef USES_P021
  ADDPLUGIN_H(021)
#endif

#ifdef USES_P022
  ADDPLUGIN_H(022)
#endif

#ifdef USES_P023
  ADDPLUGIN_H(023)
#endif

#ifdef USES_P024
  ADDPLUGIN_H(024)
#endif

#ifdef USES_P025
  ADDPLUGIN_H(025)
#endif

#ifdef USES_P026
  ADDPLUGIN_H(026)
#endif

#ifdef USES_P027
  ADDPLUGIN_H(027)
#endif

#ifdef USES_P028
  ADDPLUGIN_H(028)
#endif

#ifdef USES_P029
  ADDPLUGIN_H(029)
#endif

#ifdef USES_P030
  ADDPLUGIN_H(030)
#endif

#ifdef USES_P031
  ADDPLUGIN_H(031)
#endif

#ifdef USES_P032
  ADDPLUGIN_H(032)
#endif

#ifdef USES_P033
  ADDPLUGIN_H(033)
#endif

#ifdef USES_P034
  ADDPLUGIN_H(034)
#endif

#ifdef USES_P035
  ADDPLUGIN_H(035)
#endif

#ifdef USES_P036
  ADDPLUGIN_H(036)
#endif

#ifdef USES_P037
  ADDPLUGIN_H(037)
#endif

#ifdef USES_P038
  ADDPLUGIN_H(038)
#endif

#ifdef USES_P039
  ADDPLUGIN_H(039)
#endif

#ifdef USES_P040
  ADDPLUGIN_H(040)
#endif

#ifdef USES_P041
  ADDPLUGIN_H(041)
#endif

#ifdef USES_P042
  ADDPLUGIN_H(042)
#endif

#ifdef USES_P043
  ADDPLUGIN_H(043)
#endif

#ifdef USES_P044
  ADDPLUGIN_H(044)
#endif

#ifdef USES_P045
  ADDPLUGIN_H(045)
#endif

#ifdef USES_P046
  ADDPLUGIN_H(046)
#endif

#ifdef USES_P047
  ADDPLUGIN_H(047)
#endif

#ifdef USES_P048
  ADDPLUGIN_H(048)
#endif

#ifdef USES_P049
  ADDPLUGIN_H(049)
#endif

#ifdef USES_P050
  ADDPLUGIN_H(050)
#endif

#ifdef USES_P051
  ADDPLUGIN_H(051)
#endif

#ifdef USES_P052
  ADDPLUGIN_H(052)
#endif

#ifdef USES_P053
  ADDPLUGIN_H(053)
#endif

#ifdef USES_P054
  ADDPLUGIN_H(054)
#endif

#ifdef USES_P055
  ADDPLUGIN_H(055)
#endif

#ifdef USES_P056
  ADDPLUGIN_H(056)
#endif

#ifdef USES_P057
  ADDPLUGIN_H(057)
#endif

#ifdef USES_P058
  ADDPLUGIN_H(058)
#endif

#ifdef USES_P059
  ADDPLUGIN_H(059)
#endif

#ifdef USES_P060
  ADDPLUGIN_H(060)
#endif

#ifdef USES_P061
  ADDPLUGIN_H(061)
#endif

#ifdef USES_P062
  ADDPLUGIN_H(062)
#endif

#ifdef USES_P063
  ADDPLUGIN_H(063)
#endif

#ifdef USES_P064
  ADDPLUGIN_H(064)
#endif

#ifdef USES_P065
  ADDPLUGIN_H(065)
#endif

#ifdef USES_P066
  ADDPLUGIN_H(066)
#endif

#ifdef USES_P067
  ADDPLUGIN_H(067)
#endif

#ifdef USES_P068
  ADDPLUGIN_H(068)
#endif

#ifdef USES_P069
  ADDPLUGIN_H(069)
#endif

#ifdef USES_P070
  ADDPLUGIN_H(070)
#endif

#ifdef USES_P071
  ADDPLUGIN_H(071)
#endif

#ifdef USES_P072
  ADDPLUGIN_H(072)
#endif

#ifdef USES_P073
  ADDPLUGIN_H(073)
#endif

#ifdef USES_P074
  ADDPLUGIN_H(074)
#endif

#ifdef USES_P075
  ADDPLUGIN_H(075)
#endif

#ifdef USES_P076
  ADDPLUGIN_H(076)
#endif

#ifdef USES_P077
  ADDPLUGIN_H(077)
#endif

#ifdef USES_P078
  ADDPLUGIN_H(078)
#endif

#ifdef USES_P079
  ADDPLUGIN_H(079)
#endif

#ifdef USES_P080
  ADDPLUGIN_H(080)
#endif

#ifdef USES_P081
  ADDPLUGIN_H(081)
#endif

#ifdef USES_P082
  ADDPLUGIN_H(082)
#endif

#ifdef USES_P083
  ADDPLUGIN_H(083)
#endif

#ifdef USES_P084
  ADDPLUGIN_H(084)
#endif

#ifdef USES_P085
  ADDPLUGIN_H(085)
#endif

#ifdef USES_P086
  ADDPLUGIN_H(086)
#endif

#ifdef USES_P087
  ADDPLUGIN_H(087)
#endif

#ifdef USES_P088
  ADDPLUGIN_H(088)
#endif

#ifdef USES_P089
  #ifdef ESP8266
  // FIXME TD-er: Support Ping plugin for ESP32
  ADDPLUGIN_H(089)
  #endif
#endif

#ifdef USES_P090
  ADDPLUGIN_H(090)
#endif

#ifdef USES_P091
  ADDPLUGIN_H(091)
#endif

#ifdef USES_P092
  ADDPLUGIN_H(092)
#endif

#ifdef USES_P093
  ADDPLUGIN_H(093)
#endif

#ifdef USES_P094
  ADDPLUGIN_H(094)
#endif

#ifdef USES_P095
  ADDPLUGIN_H(095)
#endif

#ifdef USES_P096
  ADDPLUGIN_H(096)
#endif

#ifdef USES_P097
  #ifdef ESP32
  ADDPLUGIN_H(097) // Touch (ESP32)
  #endif
#endif

#ifdef USES_P098
  ADDPLUGIN_H(098)
#endif

#ifdef USES_P099
  ADDPLUGIN_H(099)
#endif

#ifdef USES_P100
  ADDPLUGIN_H(100)
#endif

#ifdef USES_P101
  ADDPLUGIN_H(101)
#endif

#ifdef USES_P102
  ADDPLUGIN_H(102)
#endif

#ifdef USES_P103
  ADDPLUGIN_H(103)
#endif

#ifdef USES_P104
  ADDPLUGIN_H(104)
#endif

#ifdef USES_P105
  ADDPLUGIN_H(105)
#endif

#ifdef USES_P106
  ADDPLUGIN_H(106)
#endif

#ifdef USES_P107
  ADDPLUGIN_H(107)
#endif

#ifdef USES_P108
  ADDPLUGIN_H(108)
#endif

#ifdef USES_P109
  ADDPLUGIN_H(109)
#endif

#ifdef USES_P110
  ADDPLUGIN_H(110)
#endif

#ifdef USES_P111
  ADDPLUGIN_H(111)
#endif

#ifdef USES_P112
  ADDPLUGIN_H(112)
#endif

#ifdef USES_P113
  ADDPLUGIN_H(113)
#endif

#ifdef USES_P114
  ADDPLUGIN_H(114)
#endif

#ifdef USES_P115
  ADDPLUGIN_H(115)
#endif

#ifdef USES_P116
  ADDPLUGIN_H(116)
#endif

#ifdef USES_P117
  ADDPLUGIN_H(117)
#endif

#ifdef USES_P118
  ADDPLUGIN_H(118)
#endif

#ifdef USES_P119
  ADDPLUGIN_H(119)
#endif

#ifdef USES_P120
  ADDPLUGIN_H(120)
#endif

#ifdef USES_P121
  ADDPLUGIN_H(121)
#endif

#ifdef USES_P122
  ADDPLUGIN_H(122)
#endif

#ifdef USES_P123
  ADDPLUGIN_H(123)
#endif

#ifdef USES_P124
  ADDPLUGIN_H(124)
#endif

#ifdef USES_P125
  ADDPLUGIN_H(125)
#endif

#ifdef USES_P126
  ADDPLUGIN_H(126)
#endif

#ifdef USES_P127
  ADDPLUGIN_H(127)
#endif

#ifdef USES_P128
  ADDPLUGIN_H(128)
#endif

#ifdef USES_P129
  ADDPLUGIN_H(129)
#endif

#ifdef USES_P130
  ADDPLUGIN_H(130)
#endif

#ifdef USES_P131
  ADDPLUGIN_H(131)
#endif

#ifdef USES_P132
  ADDPLUGIN_H(132)
#endif

#ifdef USES_P133
  ADDPLUGIN_H(133)
#endif

#ifdef USES_P134
  ADDPLUGIN_H(134)
#endif

#ifdef USES_P135
  ADDPLUGIN_H(135)
#endif

#ifdef USES_P136
  ADDPLUGIN_H(136)
#endif

#ifdef USES_P137
  ADDPLUGIN_H(137)
#endif

#ifdef USES_P138
  ADDPLUGIN_H(138)
#endif

#ifdef USES_P139
  ADDPLUGIN_H(139)
#endif

#ifdef USES_P140
  ADDPLUGIN_H(140)
#endif

#ifdef USES_P141
  ADDPLUGIN_H(141)
#endif

#ifdef USES_P142
  ADDPLUGIN_H(142)
#endif

#ifdef USES_P143
  ADDPLUGIN_H(143)
#endif

#ifdef USES_P144
  ADDPLUGIN_H(144)
#endif

#ifdef USES_P145
  ADDPLUGIN_H(145)
#endif

#ifdef USES_P146
  ADDPLUGIN_H(146)
#endif

#ifdef USES_P147
  ADDPLUGIN_H(147)
#endif

#ifdef USES_P148
  ADDPLUGIN_H(148)
#endif

#ifdef USES_P149
  ADDPLUGIN_H(149)
#endif

#ifdef USES_P150
  ADDPLUGIN_H(150)
#endif

#ifdef USES_P151
  ADDPLUGIN_H(151)
#endif

#ifdef USES_P152
  ADDPLUGIN_H(152)
#endif

#ifdef USES_P153
  ADDPLUGIN_H(153)
#endif

#ifdef USES_P154
  ADDPLUGIN_H(154)
#endif

#ifdef USES_P155
  ADDPLUGIN_H(155)
#endif

#ifdef USES_P156
  ADDPLUGIN_H(156)
#endif

#ifdef USES_P157
  ADDPLUGIN_H(157)
#endif

#ifdef USES_P158
  ADDPLUGIN_H(158)
#endif

#ifdef USES_P159
  ADDPLUGIN_H(159)
#endif

#ifdef USES_P160
  ADDPLUGIN_H(160)
#endif

#ifdef USES_P161
  ADDPLUGIN_H(161)
#endif

#ifdef USES_P162
  ADDPLUGIN_H(162)
#endif

#ifdef USES_P163
  ADDPLUGIN_H(163)
#endif

#ifdef USES_P164
  ADDPLUGIN_H(164)
#endif

#ifdef USES_P165
  ADDPLUGIN_H(165)
#endif

#ifdef USES_P166
  ADDPLUGIN_H(166)
#endif

#ifdef USES_P167
  ADDPLUGIN_H(167)
#endif

#ifdef USES_P168
  ADDPLUGIN_H(168)
#endif

#ifdef USES_P169
  ADDPLUGIN_H(169)
#endif

#ifdef USES_P170
  ADDPLUGIN_H(170)
#endif

#ifdef USES_P171
  ADDPLUGIN_H(171)
#endif

#ifdef USES_P172
  ADDPLUGIN_H(172)
#endif

#ifdef USES_P173
  ADDPLUGIN_H(173)
#endif

#ifdef USES_P174
  ADDPLUGIN_H(174)
#endif

#ifdef USES_P175
  ADDPLUGIN_H(175)
#endif

#ifdef USES_P176
  ADDPLUGIN_H(176)
#endif

#ifdef USES_P177
  ADDPLUGIN_H(177)
#endif

#ifdef USES_P178
  ADDPLUGIN_H(178)
#endif

#ifdef USES_P179
  ADDPLUGIN_H(179)
#endif

#ifdef USES_P180
  ADDPLUGIN_H(180)
#endif

#ifdef USES_P181
  ADDPLUGIN_H(181)
#endif

#ifdef USES_P182
  ADDPLUGIN_H(182)
#endif

#ifdef USES_P183
  ADDPLUGIN_H(183)
#endif

#ifdef USES_P184
  ADDPLUGIN_H(184)
#endif

#ifdef USES_P185
  ADDPLUGIN_H(185)
#endif

#ifdef USES_P186
  ADDPLUGIN_H(186)
#endif

#ifdef USES_P187
  ADDPLUGIN_H(187)
#endif

#ifdef USES_P188
  ADDPLUGIN_H(188)
#endif

#ifdef USES_P189
  ADDPLUGIN_H(189)
#endif

#ifdef USES_P190
  ADDPLUGIN_H(190)
#endif

#ifdef USES_P191
  ADDPLUGIN_H(191)
#endif

#ifdef USES_P192
  ADDPLUGIN_H(192)
#endif

#ifdef USES_P193
  ADDPLUGIN_H(193)
#endif

#ifdef USES_P194
  ADDPLUGIN_H(194)
#endif

#ifdef USES_P195
  ADDPLUGIN_H(195)
#endif

#ifdef USES_P196
  ADDPLUGIN_H(196)
#endif

#ifdef USES_P197
  ADDPLUGIN_H(197)
#endif

#ifdef USES_P198
  ADDPLUGIN_H(198)
#endif

#ifdef USES_P199
  ADDPLUGIN_H(199)
#endif

#ifdef USES_P200
  ADDPLUGIN_H(200)
#endif

#ifdef USES_P201
  ADDPLUGIN_H(201)
#endif

#ifdef USES_P202
  ADDPLUGIN_H(202)
#endif

#ifdef USES_P203
  ADDPLUGIN_H(203)
#endif

#ifdef USES_P204
  ADDPLUGIN_H(204)
#endif

#ifdef USES_P205
  ADDPLUGIN_H(205)
#endif

#ifdef USES_P206
  ADDPLUGIN_H(206)
#endif

#ifdef USES_P207
  ADDPLUGIN_H(207)
#endif

#ifdef USES_P208
  ADDPLUGIN_H(208)
#endif

#ifdef USES_P209
  ADDPLUGIN_H(209)
#endif

#ifdef USES_P210
  ADDPLUGIN_H(210)
#endif

#ifdef USES_P211
  ADDPLUGIN_H(211)
#endif

#ifdef USES_P212
  ADDPLUGIN_H(212)
#endif

#ifdef USES_P213
  ADDPLUGIN_H(213)
#endif

#ifdef USES_P214
  ADDPLUGIN_H(214)
#endif

#ifdef USES_P215
  ADDPLUGIN_H(215)
#endif

#ifdef USES_P216
  ADDPLUGIN_H(216)
#endif

#ifdef USES_P217
  ADDPLUGIN_H(217)
#endif

#ifdef USES_P218
  ADDPLUGIN_H(218)
#endif

#ifdef USES_P219
  ADDPLUGIN_H(219)
#endif

#ifdef USES_P220
  ADDPLUGIN_H(220)
#endif

#ifdef USES_P221
  ADDPLUGIN_H(221)
#endif

#ifdef USES_P222
  ADDPLUGIN_H(222)
#endif

#ifdef USES_P223
  ADDPLUGIN_H(223)
#endif

#ifdef USES_P224
  ADDPLUGIN_H(224)
#endif

#ifdef USES_P225
  ADDPLUGIN_H(225)
#endif

#ifdef USES_P226
  ADDPLUGIN_H(226)
#endif

#ifdef USES_P227
  ADDPLUGIN_H(227)
#endif

#ifdef USES_P228
  ADDPLUGIN_H(228)
#endif

#ifdef USES_P229
  ADDPLUGIN_H(229)
#endif

#ifdef USES_P230
  ADDPLUGIN_H(230)
#endif

#ifdef USES_P231
  ADDPLUGIN_H(231)
#endif

#ifdef USES_P232
  ADDPLUGIN_H(232)
#endif

#ifdef USES_P233
  ADDPLUGIN_H(233)
#endif

#ifdef USES_P234
  ADDPLUGIN_H(234)
#endif

#ifdef USES_P235
  ADDPLUGIN_H(235)
#endif

#ifdef USES_P236
  ADDPLUGIN_H(236)
#endif

#ifdef USES_P237
  ADDPLUGIN_H(237)
#endif

#ifdef USES_P238
  ADDPLUGIN_H(238)
#endif

#ifdef USES_P239
  ADDPLUGIN_H(239)
#endif

#ifdef USES_P240
  ADDPLUGIN_H(240)
#endif

#ifdef USES_P241
  ADDPLUGIN_H(241)
#endif

#ifdef USES_P242
  ADDPLUGIN_H(242)
#endif

#ifdef USES_P243
  ADDPLUGIN_H(243)
#endif

#ifdef USES_P244
  ADDPLUGIN_H(244)
#endif

#ifdef USES_P245
  ADDPLUGIN_H(245)
#endif

#ifdef USES_P246
  ADDPLUGIN_H(246)
#endif

#ifdef USES_P247
  ADDPLUGIN_H(247)
#endif

#ifdef USES_P248
  ADDPLUGIN_H(248)
#endif

#ifdef USES_P249
  ADDPLUGIN_H(249)
#endif

#ifdef USES_P250
  ADDPLUGIN_H(250)
#endif

#ifdef USES_P251
  ADDPLUGIN_H(251)
#endif

#ifdef USES_P252
  ADDPLUGIN_H(252)
#endif

#ifdef USES_P253
  ADDPLUGIN_H(253)
#endif

#ifdef USES_P254
  ADDPLUGIN_H(254)
#endif

#ifdef USES_P255
  ADDPLUGIN_H(255)
#endif

#undef ADDPLUGIN_H

#endif
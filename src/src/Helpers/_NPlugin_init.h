#ifndef HELPERS__NPLUGIN_INIT_H
#define HELPERS__NPLUGIN_INIT_H

#include "../../ESPEasy_common.h"

#if FEATURE_NOTIFIER

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include <Arduino.h>


struct EventStruct;

void NPluginInit();

// Macro to forward declare the NPlugin_NNN functions.
//
// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*
#define ADDNPLUGIN_H(NNN) bool NPlugin_##NNN(NPlugin::Function function, struct EventStruct *event, String& string);
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*


#ifdef USES_N001
  ADDNPLUGIN_H(001)
#endif

#ifdef USES_N002
  ADDNPLUGIN_H(002)
#endif

#ifdef USES_N003
  ADDNPLUGIN_H(003)
#endif

#ifdef USES_N004
  ADDNPLUGIN_H(004)
#endif

#ifdef USES_N005
  ADDNPLUGIN_H(005)
#endif

#ifdef USES_N006
  ADDNPLUGIN_H(006)
#endif

#ifdef USES_N007
  ADDNPLUGIN_H(007)
#endif

#ifdef USES_N008
  ADDNPLUGIN_H(008)
#endif

#ifdef USES_N009
  ADDNPLUGIN_H(009)
#endif

#ifdef USES_N010
  ADDNPLUGIN_H(010)
#endif

#ifdef USES_N011
  ADDNPLUGIN_H(011)
#endif

#ifdef USES_N012
  ADDNPLUGIN_H(012)
#endif

#ifdef USES_N013
  ADDNPLUGIN_H(013)
#endif

#ifdef USES_N014
  ADDNPLUGIN_H(014)
#endif

#ifdef USES_N015
  ADDNPLUGIN_H(015)
#endif

#ifdef USES_N016
  ADDNPLUGIN_H(016)
#endif

#ifdef USES_N017
  ADDNPLUGIN_H(017)
#endif

#ifdef USES_N018
  ADDNPLUGIN_H(018)
#endif

#ifdef USES_N019
  ADDNPLUGIN_H(019)
#endif

#ifdef USES_N020
  ADDNPLUGIN_H(020)
#endif

#ifdef USES_N021
  ADDNPLUGIN_H(021)
#endif

#ifdef USES_N022
  ADDNPLUGIN_H(022)
#endif

#ifdef USES_N023
  ADDNPLUGIN_H(023)
#endif

#ifdef USES_N024
  ADDNPLUGIN_H(024)
#endif

#ifdef USES_N025
  ADDNPLUGIN_H(025)
#endif

#ifdef USES_N026
  ADDNPLUGIN_H(026)
#endif

#ifdef USES_N027
  ADDNPLUGIN_H(027)
#endif

#ifdef USES_N028
  ADDNPLUGIN_H(028)
#endif

#ifdef USES_N029
  ADDNPLUGIN_H(029)
#endif

#ifdef USES_N030
  ADDNPLUGIN_H(030)
#endif

#ifdef USES_N031
  ADDNPLUGIN_H(031)
#endif

#ifdef USES_N032
  ADDNPLUGIN_H(032)
#endif

#ifdef USES_N033
  ADDNPLUGIN_H(033)
#endif

#ifdef USES_N034
  ADDNPLUGIN_H(034)
#endif

#ifdef USES_N035
  ADDNPLUGIN_H(035)
#endif

#ifdef USES_N036
  ADDNPLUGIN_H(036)
#endif

#ifdef USES_N037
  ADDNPLUGIN_H(037)
#endif

#ifdef USES_N038
  ADDNPLUGIN_H(038)
#endif

#ifdef USES_N039
  ADDNPLUGIN_H(039)
#endif

#ifdef USES_N040
  ADDNPLUGIN_H(040)
#endif

#ifdef USES_N041
  ADDNPLUGIN_H(041)
#endif

#ifdef USES_N042
  ADDNPLUGIN_H(042)
#endif

#ifdef USES_N043
  ADDNPLUGIN_H(043)
#endif

#ifdef USES_N044
  ADDNPLUGIN_H(044)
#endif

#ifdef USES_N045
  ADDNPLUGIN_H(045)
#endif

#ifdef USES_N046
  ADDNPLUGIN_H(046)
#endif

#ifdef USES_N047
  ADDNPLUGIN_H(047)
#endif

#ifdef USES_N048
  ADDNPLUGIN_H(048)
#endif

#ifdef USES_N049
  ADDNPLUGIN_H(049)
#endif

#ifdef USES_N050
  ADDNPLUGIN_H(050)
#endif

#ifdef USES_N051
  ADDNPLUGIN_H(051)
#endif

#ifdef USES_N052
  ADDNPLUGIN_H(052)
#endif

#ifdef USES_N053
  ADDNPLUGIN_H(053)
#endif

#ifdef USES_N054
  ADDNPLUGIN_H(054)
#endif

#ifdef USES_N055
  ADDNPLUGIN_H(055)
#endif

#ifdef USES_N056
  ADDNPLUGIN_H(056)
#endif

#ifdef USES_N057
  ADDNPLUGIN_H(057)
#endif

#ifdef USES_N058
  ADDNPLUGIN_H(058)
#endif

#ifdef USES_N059
  ADDNPLUGIN_H(059)
#endif

#ifdef USES_N060
  ADDNPLUGIN_H(060)
#endif

#ifdef USES_N061
  ADDNPLUGIN_H(061)
#endif

#ifdef USES_N062
  ADDNPLUGIN_H(062)
#endif

#ifdef USES_N063
  ADDNPLUGIN_H(063)
#endif

#ifdef USES_N064
  ADDNPLUGIN_H(064)
#endif

#ifdef USES_N065
  ADDNPLUGIN_H(065)
#endif

#ifdef USES_N066
  ADDNPLUGIN_H(066)
#endif

#ifdef USES_N067
  ADDNPLUGIN_H(067)
#endif

#ifdef USES_N068
  ADDNPLUGIN_H(068)
#endif

#ifdef USES_N069
  ADDNPLUGIN_H(069)
#endif

#ifdef USES_N070
  ADDNPLUGIN_H(070)
#endif

#ifdef USES_N071
  ADDNPLUGIN_H(071)
#endif

#ifdef USES_N072
  ADDNPLUGIN_H(072)
#endif

#ifdef USES_N073
  ADDNPLUGIN_H(073)
#endif

#ifdef USES_N074
  ADDNPLUGIN_H(074)
#endif

#ifdef USES_N075
  ADDNPLUGIN_H(075)
#endif

#ifdef USES_N076
  ADDNPLUGIN_H(076)
#endif

#ifdef USES_N077
  ADDNPLUGIN_H(077)
#endif

#ifdef USES_N078
  ADDNPLUGIN_H(078)
#endif

#ifdef USES_N079
  ADDNPLUGIN_H(079)
#endif

#ifdef USES_N080
  ADDNPLUGIN_H(080)
#endif

#ifdef USES_N081
  ADDNPLUGIN_H(081)
#endif

#ifdef USES_N082
  ADDNPLUGIN_H(082)
#endif

#ifdef USES_N083
  ADDNPLUGIN_H(083)
#endif

#ifdef USES_N084
  ADDNPLUGIN_H(084)
#endif

#ifdef USES_N085
  ADDNPLUGIN_H(085)
#endif

#ifdef USES_N086
  ADDNPLUGIN_H(086)
#endif

#ifdef USES_N087
  ADDNPLUGIN_H(087)
#endif

#ifdef USES_N088
  ADDNPLUGIN_H(088)
#endif

#ifdef USES_N089
  ADDNPLUGIN_H(089)
#endif

#ifdef USES_N090
  ADDNPLUGIN_H(090)
#endif

#ifdef USES_N091
  ADDNPLUGIN_H(091)
#endif

#ifdef USES_N092
  ADDNPLUGIN_H(092)
#endif

#ifdef USES_N093
  ADDNPLUGIN_H(093)
#endif

#ifdef USES_N094
  ADDNPLUGIN_H(094)
#endif

#ifdef USES_N095
  ADDNPLUGIN_H(095)
#endif

#ifdef USES_N096
  ADDNPLUGIN_H(096)
#endif

#ifdef USES_N097
  ADDNPLUGIN_H(097)
#endif

#ifdef USES_N098
  ADDNPLUGIN_H(098)
#endif

#ifdef USES_N099
  ADDNPLUGIN_H(099)
#endif

#ifdef USES_N100
  ADDNPLUGIN_H(100)
#endif

#ifdef USES_N101
  ADDNPLUGIN_H(101)
#endif

#ifdef USES_N102
  ADDNPLUGIN_H(102)
#endif

#ifdef USES_N103
  ADDNPLUGIN_H(103)
#endif

#ifdef USES_N104
  ADDNPLUGIN_H(104)
#endif

#ifdef USES_N105
  ADDNPLUGIN_H(105)
#endif

#ifdef USES_N106
  ADDNPLUGIN_H(106)
#endif

#ifdef USES_N107
  ADDNPLUGIN_H(107)
#endif

#ifdef USES_N108
  ADDNPLUGIN_H(108)
#endif

#ifdef USES_N109
  ADDNPLUGIN_H(109)
#endif

#ifdef USES_N110
  ADDNPLUGIN_H(110)
#endif

#ifdef USES_N111
  ADDNPLUGIN_H(111)
#endif

#ifdef USES_N112
  ADDNPLUGIN_H(112)
#endif

#ifdef USES_N113
  ADDNPLUGIN_H(113)
#endif

#ifdef USES_N114
  ADDNPLUGIN_H(114)
#endif

#ifdef USES_N115
  ADDNPLUGIN_H(115)
#endif

#ifdef USES_N116
  ADDNPLUGIN_H(116)
#endif

#ifdef USES_N117
  ADDNPLUGIN_H(117)
#endif

#ifdef USES_N118
  ADDNPLUGIN_H(118)
#endif

#ifdef USES_N119
  ADDNPLUGIN_H(119)
#endif

#ifdef USES_N120
  ADDNPLUGIN_H(120)
#endif

#ifdef USES_N121
  ADDNPLUGIN_H(121)
#endif

#ifdef USES_N122
  ADDNPLUGIN_H(122)
#endif

#ifdef USES_N123
  ADDNPLUGIN_H(123)
#endif

#ifdef USES_N124
  ADDNPLUGIN_H(124)
#endif

#ifdef USES_N125
  ADDNPLUGIN_H(125)
#endif

#ifdef USES_N126
  ADDNPLUGIN_H(126)
#endif

#ifdef USES_N127
  ADDNPLUGIN_H(127)
#endif

#ifdef USES_N128
  ADDNPLUGIN_H(128)
#endif

#ifdef USES_N129
  ADDNPLUGIN_H(129)
#endif

#ifdef USES_N130
  ADDNPLUGIN_H(130)
#endif

#ifdef USES_N131
  ADDNPLUGIN_H(131)
#endif

#ifdef USES_N132
  ADDNPLUGIN_H(132)
#endif

#ifdef USES_N133
  ADDNPLUGIN_H(133)
#endif

#ifdef USES_N134
  ADDNPLUGIN_H(134)
#endif

#ifdef USES_N135
  ADDNPLUGIN_H(135)
#endif

#ifdef USES_N136
  ADDNPLUGIN_H(136)
#endif

#ifdef USES_N137
  ADDNPLUGIN_H(137)
#endif

#ifdef USES_N138
  ADDNPLUGIN_H(138)
#endif

#ifdef USES_N139
  ADDNPLUGIN_H(139)
#endif

#ifdef USES_N140
  ADDNPLUGIN_H(140)
#endif

#ifdef USES_N141
  ADDNPLUGIN_H(141)
#endif

#ifdef USES_N142
  ADDNPLUGIN_H(142)
#endif

#ifdef USES_N143
  ADDNPLUGIN_H(143)
#endif

#ifdef USES_N144
  ADDNPLUGIN_H(144)
#endif

#ifdef USES_N145
  ADDNPLUGIN_H(145)
#endif

#ifdef USES_N146
  ADDNPLUGIN_H(146)
#endif

#ifdef USES_N147
  ADDNPLUGIN_H(147)
#endif

#ifdef USES_N148
  ADDNPLUGIN_H(148)
#endif

#ifdef USES_N149
  ADDNPLUGIN_H(149)
#endif

#ifdef USES_N150
  ADDNPLUGIN_H(150)
#endif

#ifdef USES_N151
  ADDNPLUGIN_H(151)
#endif

#ifdef USES_N152
  ADDNPLUGIN_H(152)
#endif

#ifdef USES_N153
  ADDNPLUGIN_H(153)
#endif

#ifdef USES_N154
  ADDNPLUGIN_H(154)
#endif

#ifdef USES_N155
  ADDNPLUGIN_H(155)
#endif

#ifdef USES_N156
  ADDNPLUGIN_H(156)
#endif

#ifdef USES_N157
  ADDNPLUGIN_H(157)
#endif

#ifdef USES_N158
  ADDNPLUGIN_H(158)
#endif

#ifdef USES_N159
  ADDNPLUGIN_H(159)
#endif

#ifdef USES_N160
  ADDNPLUGIN_H(160)
#endif

#ifdef USES_N161
  ADDNPLUGIN_H(161)
#endif

#ifdef USES_N162
  ADDNPLUGIN_H(162)
#endif

#ifdef USES_N163
  ADDNPLUGIN_H(163)
#endif

#ifdef USES_N164
  ADDNPLUGIN_H(164)
#endif

#ifdef USES_N165
  ADDNPLUGIN_H(165)
#endif

#ifdef USES_N166
  ADDNPLUGIN_H(166)
#endif

#ifdef USES_N167
  ADDNPLUGIN_H(167)
#endif

#ifdef USES_N168
  ADDNPLUGIN_H(168)
#endif

#ifdef USES_N169
  ADDNPLUGIN_H(169)
#endif

#ifdef USES_N170
  ADDNPLUGIN_H(170)
#endif

#ifdef USES_N171
  ADDNPLUGIN_H(171)
#endif

#ifdef USES_N172
  ADDNPLUGIN_H(172)
#endif

#ifdef USES_N173
  ADDNPLUGIN_H(173)
#endif

#ifdef USES_N174
  ADDNPLUGIN_H(174)
#endif

#ifdef USES_N175
  ADDNPLUGIN_H(175)
#endif

#ifdef USES_N176
  ADDNPLUGIN_H(176)
#endif

#ifdef USES_N177
  ADDNPLUGIN_H(177)
#endif

#ifdef USES_N178
  ADDNPLUGIN_H(178)
#endif

#ifdef USES_N179
  ADDNPLUGIN_H(179)
#endif

#ifdef USES_N180
  ADDNPLUGIN_H(180)
#endif

#ifdef USES_N181
  ADDNPLUGIN_H(181)
#endif

#ifdef USES_N182
  ADDNPLUGIN_H(182)
#endif

#ifdef USES_N183
  ADDNPLUGIN_H(183)
#endif

#ifdef USES_N184
  ADDNPLUGIN_H(184)
#endif

#ifdef USES_N185
  ADDNPLUGIN_H(185)
#endif

#ifdef USES_N186
  ADDNPLUGIN_H(186)
#endif

#ifdef USES_N187
  ADDNPLUGIN_H(187)
#endif

#ifdef USES_N188
  ADDNPLUGIN_H(188)
#endif

#ifdef USES_N189
  ADDNPLUGIN_H(189)
#endif

#ifdef USES_N190
  ADDNPLUGIN_H(190)
#endif

#ifdef USES_N191
  ADDNPLUGIN_H(191)
#endif

#ifdef USES_N192
  ADDNPLUGIN_H(192)
#endif

#ifdef USES_N193
  ADDNPLUGIN_H(193)
#endif

#ifdef USES_N194
  ADDNPLUGIN_H(194)
#endif

#ifdef USES_N195
  ADDNPLUGIN_H(195)
#endif

#ifdef USES_N196
  ADDNPLUGIN_H(196)
#endif

#ifdef USES_N197
  ADDNPLUGIN_H(197)
#endif

#ifdef USES_N198
  ADDNPLUGIN_H(198)
#endif

#ifdef USES_N199
  ADDNPLUGIN_H(199)
#endif

#ifdef USES_N200
  ADDNPLUGIN_H(200)
#endif

#ifdef USES_N201
  ADDNPLUGIN_H(201)
#endif

#ifdef USES_N202
  ADDNPLUGIN_H(202)
#endif

#ifdef USES_N203
  ADDNPLUGIN_H(203)
#endif

#ifdef USES_N204
  ADDNPLUGIN_H(204)
#endif

#ifdef USES_N205
  ADDNPLUGIN_H(205)
#endif

#ifdef USES_N206
  ADDNPLUGIN_H(206)
#endif

#ifdef USES_N207
  ADDNPLUGIN_H(207)
#endif

#ifdef USES_N208
  ADDNPLUGIN_H(208)
#endif

#ifdef USES_N209
  ADDNPLUGIN_H(209)
#endif

#ifdef USES_N210
  ADDNPLUGIN_H(210)
#endif

#ifdef USES_N211
  ADDNPLUGIN_H(211)
#endif

#ifdef USES_N212
  ADDNPLUGIN_H(212)
#endif

#ifdef USES_N213
  ADDNPLUGIN_H(213)
#endif

#ifdef USES_N214
  ADDNPLUGIN_H(214)
#endif

#ifdef USES_N215
  ADDNPLUGIN_H(215)
#endif

#ifdef USES_N216
  ADDNPLUGIN_H(216)
#endif

#ifdef USES_N217
  ADDNPLUGIN_H(217)
#endif

#ifdef USES_N218
  ADDNPLUGIN_H(218)
#endif

#ifdef USES_N219
  ADDNPLUGIN_H(219)
#endif

#ifdef USES_N220
  ADDNPLUGIN_H(220)
#endif

#ifdef USES_N221
  ADDNPLUGIN_H(221)
#endif

#ifdef USES_N222
  ADDNPLUGIN_H(222)
#endif

#ifdef USES_N223
  ADDNPLUGIN_H(223)
#endif

#ifdef USES_N224
  ADDNPLUGIN_H(224)
#endif

#ifdef USES_N225
  ADDNPLUGIN_H(225)
#endif

#ifdef USES_N226
  ADDNPLUGIN_H(226)
#endif

#ifdef USES_N227
  ADDNPLUGIN_H(227)
#endif

#ifdef USES_N228
  ADDNPLUGIN_H(228)
#endif

#ifdef USES_N229
  ADDNPLUGIN_H(229)
#endif

#ifdef USES_N230
  ADDNPLUGIN_H(230)
#endif

#ifdef USES_N231
  ADDNPLUGIN_H(231)
#endif

#ifdef USES_N232
  ADDNPLUGIN_H(232)
#endif

#ifdef USES_N233
  ADDNPLUGIN_H(233)
#endif

#ifdef USES_N234
  ADDNPLUGIN_H(234)
#endif

#ifdef USES_N235
  ADDNPLUGIN_H(235)
#endif

#ifdef USES_N236
  ADDNPLUGIN_H(236)
#endif

#ifdef USES_N237
  ADDNPLUGIN_H(237)
#endif

#ifdef USES_N238
  ADDNPLUGIN_H(238)
#endif

#ifdef USES_N239
  ADDNPLUGIN_H(239)
#endif

#ifdef USES_N240
  ADDNPLUGIN_H(240)
#endif

#ifdef USES_N241
  ADDNPLUGIN_H(241)
#endif

#ifdef USES_N242
  ADDNPLUGIN_H(242)
#endif

#ifdef USES_N243
  ADDNPLUGIN_H(243)
#endif

#ifdef USES_N244
  ADDNPLUGIN_H(244)
#endif

#ifdef USES_N245
  ADDNPLUGIN_H(245)
#endif

#ifdef USES_N246
  ADDNPLUGIN_H(246)
#endif

#ifdef USES_N247
  ADDNPLUGIN_H(247)
#endif

#ifdef USES_N248
  ADDNPLUGIN_H(248)
#endif

#ifdef USES_N249
  ADDNPLUGIN_H(249)
#endif

#ifdef USES_N250
  ADDNPLUGIN_H(250)
#endif

#ifdef USES_N251
  ADDNPLUGIN_H(251)
#endif

#ifdef USES_N252
  ADDNPLUGIN_H(252)
#endif

#ifdef USES_N253
  ADDNPLUGIN_H(253)
#endif

#ifdef USES_N254
  ADDNPLUGIN_H(254)
#endif

#ifdef USES_N255
  ADDNPLUGIN_H(255)
#endif

#undef ADDNPLUGIN_H

#endif

#endif
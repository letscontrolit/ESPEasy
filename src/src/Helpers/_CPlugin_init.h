#ifndef HELPERS__CPLUGIN_INIT_H
#define HELPERS__CPLUGIN_INIT_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include <Arduino.h>


struct EventStruct;

void CPluginInit();

// Macro to forward declare the CPlugin_NNN functions.
// N.B. Some controllers also have a do_process_cNNN_delay_queue function.
//      Forward declaration of these is done in ControllerQueue/ControllerDelayHandlerStruct.h
//
// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*
#define ADDCPLUGIN_H(NNN) bool CPlugin_##NNN(CPlugin::Function function, struct EventStruct *event, String& string);
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*


#ifdef USES_C001
  ADDCPLUGIN_H(001)
#endif

#ifdef USES_C002
  ADDCPLUGIN_H(002)
#endif

#ifdef USES_C003
  ADDCPLUGIN_H(003)
#endif

#ifdef USES_C004
  ADDCPLUGIN_H(004)
#endif

#ifdef USES_C005
  ADDCPLUGIN_H(005)
#endif

#ifdef USES_C006
  ADDCPLUGIN_H(006)
#endif

#ifdef USES_C007
  ADDCPLUGIN_H(007)
#endif

#ifdef USES_C008
  ADDCPLUGIN_H(008)
#endif

#ifdef USES_C009
  ADDCPLUGIN_H(009)
#endif

#ifdef USES_C010
  ADDCPLUGIN_H(010)
#endif

#ifdef USES_C011
  ADDCPLUGIN_H(011)
#endif

#ifdef USES_C012
  ADDCPLUGIN_H(012)
#endif

#ifdef USES_C013
  ADDCPLUGIN_H(013)
#endif

#ifdef USES_C014
  ADDCPLUGIN_H(014)
#endif

#ifdef USES_C015
  ADDCPLUGIN_H(015)
#endif

#ifdef USES_C016
  ADDCPLUGIN_H(016)
#endif

#ifdef USES_C017
  ADDCPLUGIN_H(017)
#endif

#ifdef USES_C018
  ADDCPLUGIN_H(018)
#endif

#ifdef USES_C019
  ADDCPLUGIN_H(019)
#endif

#ifdef USES_C020
  ADDCPLUGIN_H(020)
#endif

#ifdef USES_C021
  ADDCPLUGIN_H(021)
#endif

#ifdef USES_C022
  ADDCPLUGIN_H(022)
#endif

#ifdef USES_C023
  ADDCPLUGIN_H(023)
#endif

#ifdef USES_C024
  ADDCPLUGIN_H(024)
#endif

#ifdef USES_C025
  ADDCPLUGIN_H(025)
#endif

#ifdef USES_C026
  ADDCPLUGIN_H(026)
#endif

#ifdef USES_C027
  ADDCPLUGIN_H(027)
#endif

#ifdef USES_C028
  ADDCPLUGIN_H(028)
#endif

#ifdef USES_C029
  ADDCPLUGIN_H(029)
#endif

#ifdef USES_C030
  ADDCPLUGIN_H(030)
#endif

#ifdef USES_C031
  ADDCPLUGIN_H(031)
#endif

#ifdef USES_C032
  ADDCPLUGIN_H(032)
#endif

#ifdef USES_C033
  ADDCPLUGIN_H(033)
#endif

#ifdef USES_C034
  ADDCPLUGIN_H(034)
#endif

#ifdef USES_C035
  ADDCPLUGIN_H(035)
#endif

#ifdef USES_C036
  ADDCPLUGIN_H(036)
#endif

#ifdef USES_C037
  ADDCPLUGIN_H(037)
#endif

#ifdef USES_C038
  ADDCPLUGIN_H(038)
#endif

#ifdef USES_C039
  ADDCPLUGIN_H(039)
#endif

#ifdef USES_C040
  ADDCPLUGIN_H(040)
#endif

#ifdef USES_C041
  ADDCPLUGIN_H(041)
#endif

#ifdef USES_C042
  ADDCPLUGIN_H(042)
#endif

#ifdef USES_C043
  ADDCPLUGIN_H(043)
#endif

#ifdef USES_C044
  ADDCPLUGIN_H(044)
#endif

#ifdef USES_C045
  ADDCPLUGIN_H(045)
#endif

#ifdef USES_C046
  ADDCPLUGIN_H(046)
#endif

#ifdef USES_C047
  ADDCPLUGIN_H(047)
#endif

#ifdef USES_C048
  ADDCPLUGIN_H(048)
#endif

#ifdef USES_C049
  ADDCPLUGIN_H(049)
#endif

#ifdef USES_C050
  ADDCPLUGIN_H(050)
#endif

#ifdef USES_C051
  ADDCPLUGIN_H(051)
#endif

#ifdef USES_C052
  ADDCPLUGIN_H(052)
#endif

#ifdef USES_C053
  ADDCPLUGIN_H(053)
#endif

#ifdef USES_C054
  ADDCPLUGIN_H(054)
#endif

#ifdef USES_C055
  ADDCPLUGIN_H(055)
#endif

#ifdef USES_C056
  ADDCPLUGIN_H(056)
#endif

#ifdef USES_C057
  ADDCPLUGIN_H(057)
#endif

#ifdef USES_C058
  ADDCPLUGIN_H(058)
#endif

#ifdef USES_C059
  ADDCPLUGIN_H(059)
#endif

#ifdef USES_C060
  ADDCPLUGIN_H(060)
#endif

#ifdef USES_C061
  ADDCPLUGIN_H(061)
#endif

#ifdef USES_C062
  ADDCPLUGIN_H(062)
#endif

#ifdef USES_C063
  ADDCPLUGIN_H(063)
#endif

#ifdef USES_C064
  ADDCPLUGIN_H(064)
#endif

#ifdef USES_C065
  ADDCPLUGIN_H(065)
#endif

#ifdef USES_C066
  ADDCPLUGIN_H(066)
#endif

#ifdef USES_C067
  ADDCPLUGIN_H(067)
#endif

#ifdef USES_C068
  ADDCPLUGIN_H(068)
#endif

#ifdef USES_C069
  ADDCPLUGIN_H(069)
#endif

#ifdef USES_C070
  ADDCPLUGIN_H(070)
#endif

#ifdef USES_C071
  ADDCPLUGIN_H(071)
#endif

#ifdef USES_C072
  ADDCPLUGIN_H(072)
#endif

#ifdef USES_C073
  ADDCPLUGIN_H(073)
#endif

#ifdef USES_C074
  ADDCPLUGIN_H(074)
#endif

#ifdef USES_C075
  ADDCPLUGIN_H(075)
#endif

#ifdef USES_C076
  ADDCPLUGIN_H(076)
#endif

#ifdef USES_C077
  ADDCPLUGIN_H(077)
#endif

#ifdef USES_C078
  ADDCPLUGIN_H(078)
#endif

#ifdef USES_C079
  ADDCPLUGIN_H(079)
#endif

#ifdef USES_C080
  ADDCPLUGIN_H(080)
#endif

#ifdef USES_C081
  ADDCPLUGIN_H(081)
#endif

#ifdef USES_C082
  ADDCPLUGIN_H(082)
#endif

#ifdef USES_C083
  ADDCPLUGIN_H(083)
#endif

#ifdef USES_C084
  ADDCPLUGIN_H(084)
#endif

#ifdef USES_C085
  ADDCPLUGIN_H(085)
#endif

#ifdef USES_C086
  ADDCPLUGIN_H(086)
#endif

#ifdef USES_C087
  ADDCPLUGIN_H(087)
#endif

#ifdef USES_C088
  ADDCPLUGIN_H(088)
#endif

#ifdef USES_C089
  ADDCPLUGIN_H(089)
#endif

#ifdef USES_C090
  ADDCPLUGIN_H(090)
#endif

#ifdef USES_C091
  ADDCPLUGIN_H(091)
#endif

#ifdef USES_C092
  ADDCPLUGIN_H(092)
#endif

#ifdef USES_C093
  ADDCPLUGIN_H(093)
#endif

#ifdef USES_C094
  ADDCPLUGIN_H(094)
#endif

#ifdef USES_C095
  ADDCPLUGIN_H(095)
#endif

#ifdef USES_C096
  ADDCPLUGIN_H(096)
#endif

#ifdef USES_C097
  ADDCPLUGIN_H(097)
#endif

#ifdef USES_C098
  ADDCPLUGIN_H(098)
#endif

#ifdef USES_C099
  ADDCPLUGIN_H(099)
#endif

#ifdef USES_C100
  ADDCPLUGIN_H(100)
#endif

#ifdef USES_C101
  ADDCPLUGIN_H(101)
#endif

#ifdef USES_C102
  ADDCPLUGIN_H(102)
#endif

#ifdef USES_C103
  ADDCPLUGIN_H(103)
#endif

#ifdef USES_C104
  ADDCPLUGIN_H(104)
#endif

#ifdef USES_C105
  ADDCPLUGIN_H(105)
#endif

#ifdef USES_C106
  ADDCPLUGIN_H(106)
#endif

#ifdef USES_C107
  ADDCPLUGIN_H(107)
#endif

#ifdef USES_C108
  ADDCPLUGIN_H(108)
#endif

#ifdef USES_C109
  ADDCPLUGIN_H(109)
#endif

#ifdef USES_C110
  ADDCPLUGIN_H(110)
#endif

#ifdef USES_C111
  ADDCPLUGIN_H(111)
#endif

#ifdef USES_C112
  ADDCPLUGIN_H(112)
#endif

#ifdef USES_C113
  ADDCPLUGIN_H(113)
#endif

#ifdef USES_C114
  ADDCPLUGIN_H(114)
#endif

#ifdef USES_C115
  ADDCPLUGIN_H(115)
#endif

#ifdef USES_C116
  ADDCPLUGIN_H(116)
#endif

#ifdef USES_C117
  ADDCPLUGIN_H(117)
#endif

#ifdef USES_C118
  ADDCPLUGIN_H(118)
#endif

#ifdef USES_C119
  ADDCPLUGIN_H(119)
#endif

#ifdef USES_C120
  ADDCPLUGIN_H(120)
#endif

#ifdef USES_C121
  ADDCPLUGIN_H(121)
#endif

#ifdef USES_C122
  ADDCPLUGIN_H(122)
#endif

#ifdef USES_C123
  ADDCPLUGIN_H(123)
#endif

#ifdef USES_C124
  ADDCPLUGIN_H(124)
#endif

#ifdef USES_C125
  ADDCPLUGIN_H(125)
#endif

#ifdef USES_C126
  ADDCPLUGIN_H(126)
#endif

#ifdef USES_C127
  ADDCPLUGIN_H(127)
#endif

#ifdef USES_C128
  ADDCPLUGIN_H(128)
#endif

#ifdef USES_C129
  ADDCPLUGIN_H(129)
#endif

#ifdef USES_C130
  ADDCPLUGIN_H(130)
#endif

#ifdef USES_C131
  ADDCPLUGIN_H(131)
#endif

#ifdef USES_C132
  ADDCPLUGIN_H(132)
#endif

#ifdef USES_C133
  ADDCPLUGIN_H(133)
#endif

#ifdef USES_C134
  ADDCPLUGIN_H(134)
#endif

#ifdef USES_C135
  ADDCPLUGIN_H(135)
#endif

#ifdef USES_C136
  ADDCPLUGIN_H(136)
#endif

#ifdef USES_C137
  ADDCPLUGIN_H(137)
#endif

#ifdef USES_C138
  ADDCPLUGIN_H(138)
#endif

#ifdef USES_C139
  ADDCPLUGIN_H(139)
#endif

#ifdef USES_C140
  ADDCPLUGIN_H(140)
#endif

#ifdef USES_C141
  ADDCPLUGIN_H(141)
#endif

#ifdef USES_C142
  ADDCPLUGIN_H(142)
#endif

#ifdef USES_C143
  ADDCPLUGIN_H(143)
#endif

#ifdef USES_C144
  ADDCPLUGIN_H(144)
#endif

#ifdef USES_C145
  ADDCPLUGIN_H(145)
#endif

#ifdef USES_C146
  ADDCPLUGIN_H(146)
#endif

#ifdef USES_C147
  ADDCPLUGIN_H(147)
#endif

#ifdef USES_C148
  ADDCPLUGIN_H(148)
#endif

#ifdef USES_C149
  ADDCPLUGIN_H(149)
#endif

#ifdef USES_C150
  ADDCPLUGIN_H(150)
#endif

#ifdef USES_C151
  ADDCPLUGIN_H(151)
#endif

#ifdef USES_C152
  ADDCPLUGIN_H(152)
#endif

#ifdef USES_C153
  ADDCPLUGIN_H(153)
#endif

#ifdef USES_C154
  ADDCPLUGIN_H(154)
#endif

#ifdef USES_C155
  ADDCPLUGIN_H(155)
#endif

#ifdef USES_C156
  ADDCPLUGIN_H(156)
#endif

#ifdef USES_C157
  ADDCPLUGIN_H(157)
#endif

#ifdef USES_C158
  ADDCPLUGIN_H(158)
#endif

#ifdef USES_C159
  ADDCPLUGIN_H(159)
#endif

#ifdef USES_C160
  ADDCPLUGIN_H(160)
#endif

#ifdef USES_C161
  ADDCPLUGIN_H(161)
#endif

#ifdef USES_C162
  ADDCPLUGIN_H(162)
#endif

#ifdef USES_C163
  ADDCPLUGIN_H(163)
#endif

#ifdef USES_C164
  ADDCPLUGIN_H(164)
#endif

#ifdef USES_C165
  ADDCPLUGIN_H(165)
#endif

#ifdef USES_C166
  ADDCPLUGIN_H(166)
#endif

#ifdef USES_C167
  ADDCPLUGIN_H(167)
#endif

#ifdef USES_C168
  ADDCPLUGIN_H(168)
#endif

#ifdef USES_C169
  ADDCPLUGIN_H(169)
#endif

#ifdef USES_C170
  ADDCPLUGIN_H(170)
#endif

#ifdef USES_C171
  ADDCPLUGIN_H(171)
#endif

#ifdef USES_C172
  ADDCPLUGIN_H(172)
#endif

#ifdef USES_C173
  ADDCPLUGIN_H(173)
#endif

#ifdef USES_C174
  ADDCPLUGIN_H(174)
#endif

#ifdef USES_C175
  ADDCPLUGIN_H(175)
#endif

#ifdef USES_C176
  ADDCPLUGIN_H(176)
#endif

#ifdef USES_C177
  ADDCPLUGIN_H(177)
#endif

#ifdef USES_C178
  ADDCPLUGIN_H(178)
#endif

#ifdef USES_C179
  ADDCPLUGIN_H(179)
#endif

#ifdef USES_C180
  ADDCPLUGIN_H(180)
#endif

#ifdef USES_C181
  ADDCPLUGIN_H(181)
#endif

#ifdef USES_C182
  ADDCPLUGIN_H(182)
#endif

#ifdef USES_C183
  ADDCPLUGIN_H(183)
#endif

#ifdef USES_C184
  ADDCPLUGIN_H(184)
#endif

#ifdef USES_C185
  ADDCPLUGIN_H(185)
#endif

#ifdef USES_C186
  ADDCPLUGIN_H(186)
#endif

#ifdef USES_C187
  ADDCPLUGIN_H(187)
#endif

#ifdef USES_C188
  ADDCPLUGIN_H(188)
#endif

#ifdef USES_C189
  ADDCPLUGIN_H(189)
#endif

#ifdef USES_C190
  ADDCPLUGIN_H(190)
#endif

#ifdef USES_C191
  ADDCPLUGIN_H(191)
#endif

#ifdef USES_C192
  ADDCPLUGIN_H(192)
#endif

#ifdef USES_C193
  ADDCPLUGIN_H(193)
#endif

#ifdef USES_C194
  ADDCPLUGIN_H(194)
#endif

#ifdef USES_C195
  ADDCPLUGIN_H(195)
#endif

#ifdef USES_C196
  ADDCPLUGIN_H(196)
#endif

#ifdef USES_C197
  ADDCPLUGIN_H(197)
#endif

#ifdef USES_C198
  ADDCPLUGIN_H(198)
#endif

#ifdef USES_C199
  ADDCPLUGIN_H(199)
#endif

#ifdef USES_C200
  ADDCPLUGIN_H(200)
#endif

#ifdef USES_C201
  ADDCPLUGIN_H(201)
#endif

#ifdef USES_C202
  ADDCPLUGIN_H(202)
#endif

#ifdef USES_C203
  ADDCPLUGIN_H(203)
#endif

#ifdef USES_C204
  ADDCPLUGIN_H(204)
#endif

#ifdef USES_C205
  ADDCPLUGIN_H(205)
#endif

#ifdef USES_C206
  ADDCPLUGIN_H(206)
#endif

#ifdef USES_C207
  ADDCPLUGIN_H(207)
#endif

#ifdef USES_C208
  ADDCPLUGIN_H(208)
#endif

#ifdef USES_C209
  ADDCPLUGIN_H(209)
#endif

#ifdef USES_C210
  ADDCPLUGIN_H(210)
#endif

#ifdef USES_C211
  ADDCPLUGIN_H(211)
#endif

#ifdef USES_C212
  ADDCPLUGIN_H(212)
#endif

#ifdef USES_C213
  ADDCPLUGIN_H(213)
#endif

#ifdef USES_C214
  ADDCPLUGIN_H(214)
#endif

#ifdef USES_C215
  ADDCPLUGIN_H(215)
#endif

#ifdef USES_C216
  ADDCPLUGIN_H(216)
#endif

#ifdef USES_C217
  ADDCPLUGIN_H(217)
#endif

#ifdef USES_C218
  ADDCPLUGIN_H(218)
#endif

#ifdef USES_C219
  ADDCPLUGIN_H(219)
#endif

#ifdef USES_C220
  ADDCPLUGIN_H(220)
#endif

#ifdef USES_C221
  ADDCPLUGIN_H(221)
#endif

#ifdef USES_C222
  ADDCPLUGIN_H(222)
#endif

#ifdef USES_C223
  ADDCPLUGIN_H(223)
#endif

#ifdef USES_C224
  ADDCPLUGIN_H(224)
#endif

#ifdef USES_C225
  ADDCPLUGIN_H(225)
#endif

#ifdef USES_C226
  ADDCPLUGIN_H(226)
#endif

#ifdef USES_C227
  ADDCPLUGIN_H(227)
#endif

#ifdef USES_C228
  ADDCPLUGIN_H(228)
#endif

#ifdef USES_C229
  ADDCPLUGIN_H(229)
#endif

#ifdef USES_C230
  ADDCPLUGIN_H(230)
#endif

#ifdef USES_C231
  ADDCPLUGIN_H(231)
#endif

#ifdef USES_C232
  ADDCPLUGIN_H(232)
#endif

#ifdef USES_C233
  ADDCPLUGIN_H(233)
#endif

#ifdef USES_C234
  ADDCPLUGIN_H(234)
#endif

#ifdef USES_C235
  ADDCPLUGIN_H(235)
#endif

#ifdef USES_C236
  ADDCPLUGIN_H(236)
#endif

#ifdef USES_C237
  ADDCPLUGIN_H(237)
#endif

#ifdef USES_C238
  ADDCPLUGIN_H(238)
#endif

#ifdef USES_C239
  ADDCPLUGIN_H(239)
#endif

#ifdef USES_C240
  ADDCPLUGIN_H(240)
#endif

#ifdef USES_C241
  ADDCPLUGIN_H(241)
#endif

#ifdef USES_C242
  ADDCPLUGIN_H(242)
#endif

#ifdef USES_C243
  ADDCPLUGIN_H(243)
#endif

#ifdef USES_C244
  ADDCPLUGIN_H(244)
#endif

#ifdef USES_C245
  ADDCPLUGIN_H(245)
#endif

#ifdef USES_C246
  ADDCPLUGIN_H(246)
#endif

#ifdef USES_C247
  ADDCPLUGIN_H(247)
#endif

#ifdef USES_C248
  ADDCPLUGIN_H(248)
#endif

#ifdef USES_C249
  ADDCPLUGIN_H(249)
#endif

#ifdef USES_C250
  ADDCPLUGIN_H(250)
#endif

#ifdef USES_C251
  ADDCPLUGIN_H(251)
#endif

#ifdef USES_C252
  ADDCPLUGIN_H(252)
#endif

#ifdef USES_C253
  ADDCPLUGIN_H(253)
#endif

#ifdef USES_C254
  ADDCPLUGIN_H(254)
#endif

#ifdef USES_C255
  ADDCPLUGIN_H(255)
#endif

#undef ADDCPLUGIN_H


#endif
#include "../Helpers/_CPlugin_init.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/TimingStats.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include "../Globals/CPlugins.h"
#include "../Globals/Settings.h"

#include "../Helpers/Misc.h"

// ********************************************************************************
// Initialize all Controller CPlugins that where defined earlier
// and initialize the function call pointer into the CCPlugin array
// ********************************************************************************

constexpr cpluginID_t ProtocolIndex_to_CPlugin_id[] PROGMEM =
{
#ifdef USES_C001
  1,
#endif // ifdef USES_C001

#ifdef USES_C002
  2,
#endif // ifdef USES_C002

#ifdef USES_C003
  3,
#endif // ifdef USES_C003

#ifdef USES_C004
  4,
#endif // ifdef USES_C004

#ifdef USES_C005
  5,
#endif // ifdef USES_C005

#ifdef USES_C006
  6,
#endif // ifdef USES_C006

#ifdef USES_C007
  7,
#endif // ifdef USES_C007

#ifdef USES_C008
  8,
#endif // ifdef USES_C008

#ifdef USES_C009
  9,
#endif // ifdef USES_C009

#ifdef USES_C010
  10,
#endif // ifdef USES_C010

#ifdef USES_C011
  11,
#endif // ifdef USES_C011

#ifdef USES_C012
  12,
#endif // ifdef USES_C012

#ifdef USES_C013
  13,
#endif // ifdef USES_C013

#ifdef USES_C014
  14,
#endif // ifdef USES_C014

#ifdef USES_C015
  15,
#endif // ifdef USES_C015

#ifdef USES_C016
  16,
#endif // ifdef USES_C016

#ifdef USES_C017
  17,
#endif // ifdef USES_C017

#ifdef USES_C018
  18,
#endif // ifdef USES_C018

#ifdef USES_C019
  19,
#endif // ifdef USES_C019

#ifdef USES_C020
  20,
#endif // ifdef USES_C020

#ifdef USES_C021
  21,
#endif // ifdef USES_C021

#ifdef USES_C022
  22,
#endif // ifdef USES_C022

#ifdef USES_C023
  23,
#endif // ifdef USES_C023

#ifdef USES_C024
  24,
#endif // ifdef USES_C024

#ifdef USES_C025
  25,
#endif // ifdef USES_C025

#ifdef USES_C026
  26,
#endif // ifdef USES_C026

#ifdef USES_C027
  27,
#endif // ifdef USES_C027

#ifdef USES_C028
  28,
#endif // ifdef USES_C028

#ifdef USES_C029
  29,
#endif // ifdef USES_C029

#ifdef USES_C030
  30,
#endif // ifdef USES_C030

#ifdef USES_C031
  31,
#endif // ifdef USES_C031

#ifdef USES_C032
  32,
#endif // ifdef USES_C032

#ifdef USES_C033
  33,
#endif // ifdef USES_C033

#ifdef USES_C034
  34,
#endif // ifdef USES_C034

#ifdef USES_C035
  35,
#endif // ifdef USES_C035

#ifdef USES_C036
  36,
#endif // ifdef USES_C036

#ifdef USES_C037
  37,
#endif // ifdef USES_C037

#ifdef USES_C038
  38,
#endif // ifdef USES_C038

#ifdef USES_C039
  39,
#endif // ifdef USES_C039

#ifdef USES_C040
  40,
#endif // ifdef USES_C040

#ifdef USES_C041
  41,
#endif // ifdef USES_C041

#ifdef USES_C042
  42,
#endif // ifdef USES_C042

#ifdef USES_C043
  43,
#endif // ifdef USES_C043

#ifdef USES_C044
  44,
#endif // ifdef USES_C044

#ifdef USES_C045
  45,
#endif // ifdef USES_C045

#ifdef USES_C046
  46,
#endif // ifdef USES_C046

#ifdef USES_C047
  47,
#endif // ifdef USES_C047

#ifdef USES_C048
  48,
#endif // ifdef USES_C048

#ifdef USES_C049
  49,
#endif // ifdef USES_C049

#ifdef USES_C050
  50,
#endif // ifdef USES_C050

#ifdef USES_C051
  51,
#endif // ifdef USES_C051

#ifdef USES_C052
  52,
#endif // ifdef USES_C052

#ifdef USES_C053
  53,
#endif // ifdef USES_C053

#ifdef USES_C054
  54,
#endif // ifdef USES_C054

#ifdef USES_C055
  55,
#endif // ifdef USES_C055

#ifdef USES_C056
  56,
#endif // ifdef USES_C056

#ifdef USES_C057
  57,
#endif // ifdef USES_C057

#ifdef USES_C058
  58,
#endif // ifdef USES_C058

#ifdef USES_C059
  59,
#endif // ifdef USES_C059

#ifdef USES_C060
  60,
#endif // ifdef USES_C060

#ifdef USES_C061
  61,
#endif // ifdef USES_C061

#ifdef USES_C062
  62,
#endif // ifdef USES_C062

#ifdef USES_C063
  63,
#endif // ifdef USES_C063

#ifdef USES_C064
  64,
#endif // ifdef USES_C064

#ifdef USES_C065
  65,
#endif // ifdef USES_C065

#ifdef USES_C066
  66,
#endif // ifdef USES_C066

#ifdef USES_C067
  67,
#endif // ifdef USES_C067

#ifdef USES_C068
  68,
#endif // ifdef USES_C068

#ifdef USES_C069
  69,
#endif // ifdef USES_C069

#ifdef USES_C070
  70,
#endif // ifdef USES_C070

#ifdef USES_C071
  71,
#endif // ifdef USES_C071

#ifdef USES_C072
  72,
#endif // ifdef USES_C072

#ifdef USES_C073
  73,
#endif // ifdef USES_C073

#ifdef USES_C074
  74,
#endif // ifdef USES_C074

#ifdef USES_C075
  75,
#endif // ifdef USES_C075

#ifdef USES_C076
  76,
#endif // ifdef USES_C076

#ifdef USES_C077
  77,
#endif // ifdef USES_C077

#ifdef USES_C078
  78,
#endif // ifdef USES_C078

#ifdef USES_C079
  79,
#endif // ifdef USES_C079

#ifdef USES_C080
  80,
#endif // ifdef USES_C080

#ifdef USES_C081
  81,
#endif // ifdef USES_C081

#ifdef USES_C082
  82,
#endif // ifdef USES_C082

#ifdef USES_C083
  83,
#endif // ifdef USES_C083

#ifdef USES_C084
  84,
#endif // ifdef USES_C084

#ifdef USES_C085
  85,
#endif // ifdef USES_C085

#ifdef USES_C086
  86,
#endif // ifdef USES_C086

#ifdef USES_C087
  87,
#endif // ifdef USES_C087

#ifdef USES_C088
  88,
#endif // ifdef USES_C088

#ifdef USES_C089
  89,
#endif // ifdef USES_C089

#ifdef USES_C090
  90,
#endif // ifdef USES_C090

#ifdef USES_C091
  91,
#endif // ifdef USES_C091

#ifdef USES_C092
  92,
#endif // ifdef USES_C092

#ifdef USES_C093
  93,
#endif // ifdef USES_C093

#ifdef USES_C094
  94,
#endif // ifdef USES_C094

#ifdef USES_C095
  95,
#endif // ifdef USES_C095

#ifdef USES_C096
  96,
#endif // ifdef USES_C096

#ifdef USES_C097
  97,
#endif // ifdef USES_C097

#ifdef USES_C098
  98,
#endif // ifdef USES_C098

#ifdef USES_C099
  99,
#endif // ifdef USES_C099

#ifdef USES_C100
  100,
#endif // ifdef USES_C100

#ifdef USES_C101
  101,
#endif // ifdef USES_C101

#ifdef USES_C102
  102,
#endif // ifdef USES_C102

#ifdef USES_C103
  103,
#endif // ifdef USES_C103

#ifdef USES_C104
  104,
#endif // ifdef USES_C104

#ifdef USES_C105
  105,
#endif // ifdef USES_C105

#ifdef USES_C106
  106,
#endif // ifdef USES_C106

#ifdef USES_C107
  107,
#endif // ifdef USES_C107

#ifdef USES_C108
  108,
#endif // ifdef USES_C108

#ifdef USES_C109
  109,
#endif // ifdef USES_C109

#ifdef USES_C110
  110,
#endif // ifdef USES_C110

#ifdef USES_C111
  111,
#endif // ifdef USES_C111

#ifdef USES_C112
  112,
#endif // ifdef USES_C112

#ifdef USES_C113
  113,
#endif // ifdef USES_C113

#ifdef USES_C114
  114,
#endif // ifdef USES_C114

#ifdef USES_C115
  115,
#endif // ifdef USES_C115

#ifdef USES_C116
  116,
#endif // ifdef USES_C116

#ifdef USES_C117
  117,
#endif // ifdef USES_C117

#ifdef USES_C118
  118,
#endif // ifdef USES_C118

#ifdef USES_C119
  119,
#endif // ifdef USES_C119

#ifdef USES_C120
  120,
#endif // ifdef USES_C120

#ifdef USES_C121
  121,
#endif // ifdef USES_C121

#ifdef USES_C122
  122,
#endif // ifdef USES_C122

#ifdef USES_C123
  123,
#endif // ifdef USES_C123

#ifdef USES_C124
  124,
#endif // ifdef USES_C124

#ifdef USES_C125
  125,
#endif // ifdef USES_C125

#ifdef USES_C126
  126,
#endif // ifdef USES_C126

#ifdef USES_C127
  127,
#endif // ifdef USES_C127

#ifdef USES_C128
  128,
#endif // ifdef USES_C128

#ifdef USES_C129
  129,
#endif // ifdef USES_C129

#ifdef USES_C130
  130,
#endif // ifdef USES_C130

#ifdef USES_C131
  131,
#endif // ifdef USES_C131

#ifdef USES_C132
  132,
#endif // ifdef USES_C132

#ifdef USES_C133
  133,
#endif // ifdef USES_C133

#ifdef USES_C134
  134,
#endif // ifdef USES_C134

#ifdef USES_C135
  135,
#endif // ifdef USES_C135

#ifdef USES_C136
  136,
#endif // ifdef USES_C136

#ifdef USES_C137
  137,
#endif // ifdef USES_C137

#ifdef USES_C138
  138,
#endif // ifdef USES_C138

#ifdef USES_C139
  139,
#endif // ifdef USES_C139

#ifdef USES_C140
  140,
#endif // ifdef USES_C140

#ifdef USES_C141
  141,
#endif // ifdef USES_C141

#ifdef USES_C142
  142,
#endif // ifdef USES_C142

#ifdef USES_C143
  143,
#endif // ifdef USES_C143

#ifdef USES_C144
  144,
#endif // ifdef USES_C144

#ifdef USES_C145
  145,
#endif // ifdef USES_C145

#ifdef USES_C146
  146,
#endif // ifdef USES_C146

#ifdef USES_C147
  147,
#endif // ifdef USES_C147

#ifdef USES_C148
  148,
#endif // ifdef USES_C148

#ifdef USES_C149
  149,
#endif // ifdef USES_C149

#ifdef USES_C150
  150,
#endif // ifdef USES_C150

#ifdef USES_C151
  151,
#endif // ifdef USES_C151

#ifdef USES_C152
  152,
#endif // ifdef USES_C152

#ifdef USES_C153
  153,
#endif // ifdef USES_C153

#ifdef USES_C154
  154,
#endif // ifdef USES_C154

#ifdef USES_C155
  155,
#endif // ifdef USES_C155

#ifdef USES_C156
  156,
#endif // ifdef USES_C156

#ifdef USES_C157
  157,
#endif // ifdef USES_C157

#ifdef USES_C158
  158,
#endif // ifdef USES_C158

#ifdef USES_C159
  159,
#endif // ifdef USES_C159

#ifdef USES_C160
  160,
#endif // ifdef USES_C160

#ifdef USES_C161
  161,
#endif // ifdef USES_C161

#ifdef USES_C162
  162,
#endif // ifdef USES_C162

#ifdef USES_C163
  163,
#endif // ifdef USES_C163

#ifdef USES_C164
  164,
#endif // ifdef USES_C164

#ifdef USES_C165
  165,
#endif // ifdef USES_C165

#ifdef USES_C166
  166,
#endif // ifdef USES_C166

#ifdef USES_C167
  167,
#endif // ifdef USES_C167

#ifdef USES_C168
  168,
#endif // ifdef USES_C168

#ifdef USES_C169
  169,
#endif // ifdef USES_C169

#ifdef USES_C170
  170,
#endif // ifdef USES_C170

#ifdef USES_C171
  171,
#endif // ifdef USES_C171

#ifdef USES_C172
  172,
#endif // ifdef USES_C172

#ifdef USES_C173
  173,
#endif // ifdef USES_C173

#ifdef USES_C174
  174,
#endif // ifdef USES_C174

#ifdef USES_C175
  175,
#endif // ifdef USES_C175

#ifdef USES_C176
  176,
#endif // ifdef USES_C176

#ifdef USES_C177
  177,
#endif // ifdef USES_C177

#ifdef USES_C178
  178,
#endif // ifdef USES_C178

#ifdef USES_C179
  179,
#endif // ifdef USES_C179

#ifdef USES_C180
  180,
#endif // ifdef USES_C180

#ifdef USES_C181
  181,
#endif // ifdef USES_C181

#ifdef USES_C182
  182,
#endif // ifdef USES_C182

#ifdef USES_C183
  183,
#endif // ifdef USES_C183

#ifdef USES_C184
  184,
#endif // ifdef USES_C184

#ifdef USES_C185
  185,
#endif // ifdef USES_C185

#ifdef USES_C186
  186,
#endif // ifdef USES_C186

#ifdef USES_C187
  187,
#endif // ifdef USES_C187

#ifdef USES_C188
  188,
#endif // ifdef USES_C188

#ifdef USES_C189
  189,
#endif // ifdef USES_C189

#ifdef USES_C190
  190,
#endif // ifdef USES_C190

#ifdef USES_C191
  191,
#endif // ifdef USES_C191

#ifdef USES_C192
  192,
#endif // ifdef USES_C192

#ifdef USES_C193
  193,
#endif // ifdef USES_C193

#ifdef USES_C194
  194,
#endif // ifdef USES_C194

#ifdef USES_C195
  195,
#endif // ifdef USES_C195

#ifdef USES_C196
  196,
#endif // ifdef USES_C196

#ifdef USES_C197
  197,
#endif // ifdef USES_C197

#ifdef USES_C198
  198,
#endif // ifdef USES_C198

#ifdef USES_C199
  199,
#endif // ifdef USES_C199

#ifdef USES_C200
  200,
#endif // ifdef USES_C200

#ifdef USES_C201
  201,
#endif // ifdef USES_C201

#ifdef USES_C202
  202,
#endif // ifdef USES_C202

#ifdef USES_C203
  203,
#endif // ifdef USES_C203

#ifdef USES_C204
  204,
#endif // ifdef USES_C204

#ifdef USES_C205
  205,
#endif // ifdef USES_C205

#ifdef USES_C206
  206,
#endif // ifdef USES_C206

#ifdef USES_C207
  207,
#endif // ifdef USES_C207

#ifdef USES_C208
  208,
#endif // ifdef USES_C208

#ifdef USES_C209
  209,
#endif // ifdef USES_C209

#ifdef USES_C210
  210,
#endif // ifdef USES_C210

#ifdef USES_C211
  211,
#endif // ifdef USES_C211

#ifdef USES_C212
  212,
#endif // ifdef USES_C212

#ifdef USES_C213
  213,
#endif // ifdef USES_C213

#ifdef USES_C214
  214,
#endif // ifdef USES_C214

#ifdef USES_C215
  215,
#endif // ifdef USES_C215

#ifdef USES_C216
  216,
#endif // ifdef USES_C216

#ifdef USES_C217
  217,
#endif // ifdef USES_C217

#ifdef USES_C218
  218,
#endif // ifdef USES_C218

#ifdef USES_C219
  219,
#endif // ifdef USES_C219

#ifdef USES_C220
  220,
#endif // ifdef USES_C220

#ifdef USES_C221
  221,
#endif // ifdef USES_C221

#ifdef USES_C222
  222,
#endif // ifdef USES_C222

#ifdef USES_C223
  223,
#endif // ifdef USES_C223

#ifdef USES_C224
  224,
#endif // ifdef USES_C224

#ifdef USES_C225
  225,
#endif // ifdef USES_C225

#ifdef USES_C226
  226,
#endif // ifdef USES_C226

#ifdef USES_C227
  227,
#endif // ifdef USES_C227

#ifdef USES_C228
  228,
#endif // ifdef USES_C228

#ifdef USES_C229
  229,
#endif // ifdef USES_C229

#ifdef USES_C230
  230,
#endif // ifdef USES_C230

#ifdef USES_C231
  231,
#endif // ifdef USES_C231

#ifdef USES_C232
  232,
#endif // ifdef USES_C232

#ifdef USES_C233
  233,
#endif // ifdef USES_C233

#ifdef USES_C234
  234,
#endif // ifdef USES_C234

#ifdef USES_C235
  235,
#endif // ifdef USES_C235

#ifdef USES_C236
  236,
#endif // ifdef USES_C236

#ifdef USES_C237
  237,
#endif // ifdef USES_C237

#ifdef USES_C238
  238,
#endif // ifdef USES_C238

#ifdef USES_C239
  239,
#endif // ifdef USES_C239

#ifdef USES_C240
  240,
#endif // ifdef USES_C240

#ifdef USES_C241
  241,
#endif // ifdef USES_C241

#ifdef USES_C242
  242,
#endif // ifdef USES_C242

#ifdef USES_C243
  243,
#endif // ifdef USES_C243

#ifdef USES_C244
  244,
#endif // ifdef USES_C244

#ifdef USES_C245
  245,
#endif // ifdef USES_C245

#ifdef USES_C246
  246,
#endif // ifdef USES_C246

#ifdef USES_C247
  247,
#endif // ifdef USES_C247

#ifdef USES_C248
  248,
#endif // ifdef USES_C248

#ifdef USES_C249
  249,
#endif // ifdef USES_C249

#ifdef USES_C250
  250,
#endif // ifdef USES_C250

#ifdef USES_C251
  251,
#endif // ifdef USES_C251

#ifdef USES_C252
  252,
#endif // ifdef USES_C252

#ifdef USES_C253
  253,
#endif // ifdef USES_C253

#ifdef USES_C254
  254,
#endif // ifdef USES_C254

#ifdef USES_C255
  255,
#endif // ifdef USES_C255
};


typedef bool (*CPlugin_ptr_t)(CPlugin::Function,
                      struct EventStruct *,
                      String&);

const CPlugin_ptr_t PROGMEM CPlugin_ptr[] =
{
#ifdef USES_C001
  &CPlugin_001,
#endif // ifdef USES_C001

#ifdef USES_C002
  &CPlugin_002,
#endif // ifdef USES_C002

#ifdef USES_C003
  &CPlugin_003,
#endif // ifdef USES_C003

#ifdef USES_C004
  &CPlugin_004,
#endif // ifdef USES_C004

#ifdef USES_C005
  &CPlugin_005,
#endif // ifdef USES_C005

#ifdef USES_C006
  &CPlugin_006,
#endif // ifdef USES_C006

#ifdef USES_C007
  &CPlugin_007,
#endif // ifdef USES_C007

#ifdef USES_C008
  &CPlugin_008,
#endif // ifdef USES_C008

#ifdef USES_C009
  &CPlugin_009,
#endif // ifdef USES_C009

#ifdef USES_C010
  &CPlugin_010,
#endif // ifdef USES_C010

#ifdef USES_C011
  &CPlugin_011,
#endif // ifdef USES_C011

#ifdef USES_C012
  &CPlugin_012,
#endif // ifdef USES_C012

#ifdef USES_C013
  &CPlugin_013,
#endif // ifdef USES_C013

#ifdef USES_C014
  &CPlugin_014,
#endif // ifdef USES_C014

#ifdef USES_C015
  &CPlugin_015,
#endif // ifdef USES_C015

#ifdef USES_C016
  &CPlugin_016,
#endif // ifdef USES_C016

#ifdef USES_C017
  &CPlugin_017,
#endif // ifdef USES_C017

#ifdef USES_C018
  &CPlugin_018,
#endif // ifdef USES_C018

#ifdef USES_C019
  &CPlugin_019,
#endif // ifdef USES_C019

#ifdef USES_C020
  &CPlugin_020,
#endif // ifdef USES_C020

#ifdef USES_C021
  &CPlugin_021,
#endif // ifdef USES_C021

#ifdef USES_C022
  &CPlugin_022,
#endif // ifdef USES_C022

#ifdef USES_C023
  &CPlugin_023,
#endif // ifdef USES_C023

#ifdef USES_C024
  &CPlugin_024,
#endif // ifdef USES_C024

#ifdef USES_C025
  &CPlugin_025,
#endif // ifdef USES_C025

#ifdef USES_C026
  &CPlugin_026,
#endif // ifdef USES_C026

#ifdef USES_C027
  &CPlugin_027,
#endif // ifdef USES_C027

#ifdef USES_C028
  &CPlugin_028,
#endif // ifdef USES_C028

#ifdef USES_C029
  &CPlugin_029,
#endif // ifdef USES_C029

#ifdef USES_C030
  &CPlugin_030,
#endif // ifdef USES_C030

#ifdef USES_C031
  &CPlugin_031,
#endif // ifdef USES_C031

#ifdef USES_C032
  &CPlugin_032,
#endif // ifdef USES_C032

#ifdef USES_C033
  &CPlugin_033,
#endif // ifdef USES_C033

#ifdef USES_C034
  &CPlugin_034,
#endif // ifdef USES_C034

#ifdef USES_C035
  &CPlugin_035,
#endif // ifdef USES_C035

#ifdef USES_C036
  &CPlugin_036,
#endif // ifdef USES_C036

#ifdef USES_C037
  &CPlugin_037,
#endif // ifdef USES_C037

#ifdef USES_C038
  &CPlugin_038,
#endif // ifdef USES_C038

#ifdef USES_C039
  &CPlugin_039,
#endif // ifdef USES_C039

#ifdef USES_C040
  &CPlugin_040,
#endif // ifdef USES_C040

#ifdef USES_C041
  &CPlugin_041,
#endif // ifdef USES_C041

#ifdef USES_C042
  &CPlugin_042,
#endif // ifdef USES_C042

#ifdef USES_C043
  &CPlugin_043,
#endif // ifdef USES_C043

#ifdef USES_C044
  &CPlugin_044,
#endif // ifdef USES_C044

#ifdef USES_C045
  &CPlugin_045,
#endif // ifdef USES_C045

#ifdef USES_C046
  &CPlugin_046,
#endif // ifdef USES_C046

#ifdef USES_C047
  &CPlugin_047,
#endif // ifdef USES_C047

#ifdef USES_C048
  &CPlugin_048,
#endif // ifdef USES_C048

#ifdef USES_C049
  &CPlugin_049,
#endif // ifdef USES_C049

#ifdef USES_C050
  &CPlugin_050,
#endif // ifdef USES_C050

#ifdef USES_C051
  &CPlugin_051,
#endif // ifdef USES_C051

#ifdef USES_C052
  &CPlugin_052,
#endif // ifdef USES_C052

#ifdef USES_C053
  &CPlugin_053,
#endif // ifdef USES_C053

#ifdef USES_C054
  &CPlugin_054,
#endif // ifdef USES_C054

#ifdef USES_C055
  &CPlugin_055,
#endif // ifdef USES_C055

#ifdef USES_C056
  &CPlugin_056,
#endif // ifdef USES_C056

#ifdef USES_C057
  &CPlugin_057,
#endif // ifdef USES_C057

#ifdef USES_C058
  &CPlugin_058,
#endif // ifdef USES_C058

#ifdef USES_C059
  &CPlugin_059,
#endif // ifdef USES_C059

#ifdef USES_C060
  &CPlugin_060,
#endif // ifdef USES_C060

#ifdef USES_C061
  &CPlugin_061,
#endif // ifdef USES_C061

#ifdef USES_C062
  &CPlugin_062,
#endif // ifdef USES_C062

#ifdef USES_C063
  &CPlugin_063,
#endif // ifdef USES_C063

#ifdef USES_C064
  &CPlugin_064,
#endif // ifdef USES_C064

#ifdef USES_C065
  &CPlugin_065,
#endif // ifdef USES_C065

#ifdef USES_C066
  &CPlugin_066,
#endif // ifdef USES_C066

#ifdef USES_C067
  &CPlugin_067,
#endif // ifdef USES_C067

#ifdef USES_C068
  &CPlugin_068,
#endif // ifdef USES_C068

#ifdef USES_C069
  &CPlugin_069,
#endif // ifdef USES_C069

#ifdef USES_C070
  &CPlugin_070,
#endif // ifdef USES_C070

#ifdef USES_C071
  &CPlugin_071,
#endif // ifdef USES_C071

#ifdef USES_C072
  &CPlugin_072,
#endif // ifdef USES_C072

#ifdef USES_C073
  &CPlugin_073,
#endif // ifdef USES_C073

#ifdef USES_C074
  &CPlugin_074,
#endif // ifdef USES_C074

#ifdef USES_C075
  &CPlugin_075,
#endif // ifdef USES_C075

#ifdef USES_C076
  &CPlugin_076,
#endif // ifdef USES_C076

#ifdef USES_C077
  &CPlugin_077,
#endif // ifdef USES_C077

#ifdef USES_C078
  &CPlugin_078,
#endif // ifdef USES_C078

#ifdef USES_C079
  &CPlugin_079,
#endif // ifdef USES_C079

#ifdef USES_C080
  &CPlugin_080,
#endif // ifdef USES_C080

#ifdef USES_C081
  &CPlugin_081,
#endif // ifdef USES_C081

#ifdef USES_C082
  &CPlugin_082,
#endif // ifdef USES_C082

#ifdef USES_C083
  &CPlugin_083,
#endif // ifdef USES_C083

#ifdef USES_C084
  &CPlugin_084,
#endif // ifdef USES_C084

#ifdef USES_C085
  &CPlugin_085,
#endif // ifdef USES_C085

#ifdef USES_C086
  &CPlugin_086,
#endif // ifdef USES_C086

#ifdef USES_C087
  &CPlugin_087,
#endif // ifdef USES_C087

#ifdef USES_C088
  &CPlugin_088,
#endif // ifdef USES_C088

#ifdef USES_C089
  &CPlugin_089,
#endif // ifdef USES_C089

#ifdef USES_C090
  &CPlugin_090,
#endif // ifdef USES_C090

#ifdef USES_C091
  &CPlugin_091,
#endif // ifdef USES_C091

#ifdef USES_C092
  &CPlugin_092,
#endif // ifdef USES_C092

#ifdef USES_C093
  &CPlugin_093,
#endif // ifdef USES_C093

#ifdef USES_C094
  &CPlugin_094,
#endif // ifdef USES_C094

#ifdef USES_C095
  &CPlugin_095,
#endif // ifdef USES_C095

#ifdef USES_C096
  &CPlugin_096,
#endif // ifdef USES_C096

#ifdef USES_C097
  &CPlugin_097,
#endif // ifdef USES_C097

#ifdef USES_C098
  &CPlugin_098,
#endif // ifdef USES_C098

#ifdef USES_C099
  &CPlugin_099,
#endif // ifdef USES_C099

#ifdef USES_C100
  &CPlugin_100,
#endif // ifdef USES_C100

#ifdef USES_C101
  &CPlugin_101,
#endif // ifdef USES_C101

#ifdef USES_C102
  &CPlugin_102,
#endif // ifdef USES_C102

#ifdef USES_C103
  &CPlugin_103,
#endif // ifdef USES_C103

#ifdef USES_C104
  &CPlugin_104,
#endif // ifdef USES_C104

#ifdef USES_C105
  &CPlugin_105,
#endif // ifdef USES_C105

#ifdef USES_C106
  &CPlugin_106,
#endif // ifdef USES_C106

#ifdef USES_C107
  &CPlugin_107,
#endif // ifdef USES_C107

#ifdef USES_C108
  &CPlugin_108,
#endif // ifdef USES_C108

#ifdef USES_C109
  &CPlugin_109,
#endif // ifdef USES_C109

#ifdef USES_C110
  &CPlugin_110,
#endif // ifdef USES_C110

#ifdef USES_C111
  &CPlugin_111,
#endif // ifdef USES_C111

#ifdef USES_C112
  &CPlugin_112,
#endif // ifdef USES_C112

#ifdef USES_C113
  &CPlugin_113,
#endif // ifdef USES_C113

#ifdef USES_C114
  &CPlugin_114,
#endif // ifdef USES_C114

#ifdef USES_C115
  &CPlugin_115,
#endif // ifdef USES_C115

#ifdef USES_C116
  &CPlugin_116,
#endif // ifdef USES_C116

#ifdef USES_C117
  &CPlugin_117,
#endif // ifdef USES_C117

#ifdef USES_C118
  &CPlugin_118,
#endif // ifdef USES_C118

#ifdef USES_C119
  &CPlugin_119,
#endif // ifdef USES_C119

#ifdef USES_C120
  &CPlugin_120,
#endif // ifdef USES_C120

#ifdef USES_C121
  &CPlugin_121,
#endif // ifdef USES_C121

#ifdef USES_C122
  &CPlugin_122,
#endif // ifdef USES_C122

#ifdef USES_C123
  &CPlugin_123,
#endif // ifdef USES_C123

#ifdef USES_C124
  &CPlugin_124,
#endif // ifdef USES_C124

#ifdef USES_C125
  &CPlugin_125,
#endif // ifdef USES_C125

#ifdef USES_C126
  &CPlugin_126,
#endif // ifdef USES_C126

#ifdef USES_C127
  &CPlugin_127,
#endif // ifdef USES_C127

#ifdef USES_C128
  &CPlugin_128,
#endif // ifdef USES_C128

#ifdef USES_C129
  &CPlugin_129,
#endif // ifdef USES_C129

#ifdef USES_C130
  &CPlugin_130,
#endif // ifdef USES_C130

#ifdef USES_C131
  &CPlugin_131,
#endif // ifdef USES_C131

#ifdef USES_C132
  &CPlugin_132,
#endif // ifdef USES_C132

#ifdef USES_C133
  &CPlugin_133,
#endif // ifdef USES_C133

#ifdef USES_C134
  &CPlugin_134,
#endif // ifdef USES_C134

#ifdef USES_C135
  &CPlugin_135,
#endif // ifdef USES_C135

#ifdef USES_C136
  &CPlugin_136,
#endif // ifdef USES_C136

#ifdef USES_C137
  &CPlugin_137,
#endif // ifdef USES_C137

#ifdef USES_C138
  &CPlugin_138,
#endif // ifdef USES_C138

#ifdef USES_C139
  &CPlugin_139,
#endif // ifdef USES_C139

#ifdef USES_C140
  &CPlugin_140,
#endif // ifdef USES_C140

#ifdef USES_C141
  &CPlugin_141,
#endif // ifdef USES_C141

#ifdef USES_C142
  &CPlugin_142,
#endif // ifdef USES_C142

#ifdef USES_C143
  &CPlugin_143,
#endif // ifdef USES_C143

#ifdef USES_C144
  &CPlugin_144,
#endif // ifdef USES_C144

#ifdef USES_C145
  &CPlugin_145,
#endif // ifdef USES_C145

#ifdef USES_C146
  &CPlugin_146,
#endif // ifdef USES_C146

#ifdef USES_C147
  &CPlugin_147,
#endif // ifdef USES_C147

#ifdef USES_C148
  &CPlugin_148,
#endif // ifdef USES_C148

#ifdef USES_C149
  &CPlugin_149,
#endif // ifdef USES_C149

#ifdef USES_C150
  &CPlugin_150,
#endif // ifdef USES_C150

#ifdef USES_C151
  &CPlugin_151,
#endif // ifdef USES_C151

#ifdef USES_C152
  &CPlugin_152,
#endif // ifdef USES_C152

#ifdef USES_C153
  &CPlugin_153,
#endif // ifdef USES_C153

#ifdef USES_C154
  &CPlugin_154,
#endif // ifdef USES_C154

#ifdef USES_C155
  &CPlugin_155,
#endif // ifdef USES_C155

#ifdef USES_C156
  &CPlugin_156,
#endif // ifdef USES_C156

#ifdef USES_C157
  &CPlugin_157,
#endif // ifdef USES_C157

#ifdef USES_C158
  &CPlugin_158,
#endif // ifdef USES_C158

#ifdef USES_C159
  &CPlugin_159,
#endif // ifdef USES_C159

#ifdef USES_C160
  &CPlugin_160,
#endif // ifdef USES_C160

#ifdef USES_C161
  &CPlugin_161,
#endif // ifdef USES_C161

#ifdef USES_C162
  &CPlugin_162,
#endif // ifdef USES_C162

#ifdef USES_C163
  &CPlugin_163,
#endif // ifdef USES_C163

#ifdef USES_C164
  &CPlugin_164,
#endif // ifdef USES_C164

#ifdef USES_C165
  &CPlugin_165,
#endif // ifdef USES_C165

#ifdef USES_C166
  &CPlugin_166,
#endif // ifdef USES_C166

#ifdef USES_C167
  &CPlugin_167,
#endif // ifdef USES_C167

#ifdef USES_C168
  &CPlugin_168,
#endif // ifdef USES_C168

#ifdef USES_C169
  &CPlugin_169,
#endif // ifdef USES_C169

#ifdef USES_C170
  &CPlugin_170,
#endif // ifdef USES_C170

#ifdef USES_C171
  &CPlugin_171,
#endif // ifdef USES_C171

#ifdef USES_C172
  &CPlugin_172,
#endif // ifdef USES_C172

#ifdef USES_C173
  &CPlugin_173,
#endif // ifdef USES_C173

#ifdef USES_C174
  &CPlugin_174,
#endif // ifdef USES_C174

#ifdef USES_C175
  &CPlugin_175,
#endif // ifdef USES_C175

#ifdef USES_C176
  &CPlugin_176,
#endif // ifdef USES_C176

#ifdef USES_C177
  &CPlugin_177,
#endif // ifdef USES_C177

#ifdef USES_C178
  &CPlugin_178,
#endif // ifdef USES_C178

#ifdef USES_C179
  &CPlugin_179,
#endif // ifdef USES_C179

#ifdef USES_C180
  &CPlugin_180,
#endif // ifdef USES_C180

#ifdef USES_C181
  &CPlugin_181,
#endif // ifdef USES_C181

#ifdef USES_C182
  &CPlugin_182,
#endif // ifdef USES_C182

#ifdef USES_C183
  &CPlugin_183,
#endif // ifdef USES_C183

#ifdef USES_C184
  &CPlugin_184,
#endif // ifdef USES_C184

#ifdef USES_C185
  &CPlugin_185,
#endif // ifdef USES_C185

#ifdef USES_C186
  &CPlugin_186,
#endif // ifdef USES_C186

#ifdef USES_C187
  &CPlugin_187,
#endif // ifdef USES_C187

#ifdef USES_C188
  &CPlugin_188,
#endif // ifdef USES_C188

#ifdef USES_C189
  &CPlugin_189,
#endif // ifdef USES_C189

#ifdef USES_C190
  &CPlugin_190,
#endif // ifdef USES_C190

#ifdef USES_C191
  &CPlugin_191,
#endif // ifdef USES_C191

#ifdef USES_C192
  &CPlugin_192,
#endif // ifdef USES_C192

#ifdef USES_C193
  &CPlugin_193,
#endif // ifdef USES_C193

#ifdef USES_C194
  &CPlugin_194,
#endif // ifdef USES_C194

#ifdef USES_C195
  &CPlugin_195,
#endif // ifdef USES_C195

#ifdef USES_C196
  &CPlugin_196,
#endif // ifdef USES_C196

#ifdef USES_C197
  &CPlugin_197,
#endif // ifdef USES_C197

#ifdef USES_C198
  &CPlugin_198,
#endif // ifdef USES_C198

#ifdef USES_C199
  &CPlugin_199,
#endif // ifdef USES_C199

#ifdef USES_C200
  &CPlugin_200,
#endif // ifdef USES_C200

#ifdef USES_C201
  &CPlugin_201,
#endif // ifdef USES_C201

#ifdef USES_C202
  &CPlugin_202,
#endif // ifdef USES_C202

#ifdef USES_C203
  &CPlugin_203,
#endif // ifdef USES_C203

#ifdef USES_C204
  &CPlugin_204,
#endif // ifdef USES_C204

#ifdef USES_C205
  &CPlugin_205,
#endif // ifdef USES_C205

#ifdef USES_C206
  &CPlugin_206,
#endif // ifdef USES_C206

#ifdef USES_C207
  &CPlugin_207,
#endif // ifdef USES_C207

#ifdef USES_C208
  &CPlugin_208,
#endif // ifdef USES_C208

#ifdef USES_C209
  &CPlugin_209,
#endif // ifdef USES_C209

#ifdef USES_C210
  &CPlugin_210,
#endif // ifdef USES_C210

#ifdef USES_C211
  &CPlugin_211,
#endif // ifdef USES_C211

#ifdef USES_C212
  &CPlugin_212,
#endif // ifdef USES_C212

#ifdef USES_C213
  &CPlugin_213,
#endif // ifdef USES_C213

#ifdef USES_C214
  &CPlugin_214,
#endif // ifdef USES_C214

#ifdef USES_C215
  &CPlugin_215,
#endif // ifdef USES_C215

#ifdef USES_C216
  &CPlugin_216,
#endif // ifdef USES_C216

#ifdef USES_C217
  &CPlugin_217,
#endif // ifdef USES_C217

#ifdef USES_C218
  &CPlugin_218,
#endif // ifdef USES_C218

#ifdef USES_C219
  &CPlugin_219,
#endif // ifdef USES_C219

#ifdef USES_C220
  &CPlugin_220,
#endif // ifdef USES_C220

#ifdef USES_C221
  &CPlugin_221,
#endif // ifdef USES_C221

#ifdef USES_C222
  &CPlugin_222,
#endif // ifdef USES_C222

#ifdef USES_C223
  &CPlugin_223,
#endif // ifdef USES_C223

#ifdef USES_C224
  &CPlugin_224,
#endif // ifdef USES_C224

#ifdef USES_C225
  &CPlugin_225,
#endif // ifdef USES_C225

#ifdef USES_C226
  &CPlugin_226,
#endif // ifdef USES_C226

#ifdef USES_C227
  &CPlugin_227,
#endif // ifdef USES_C227

#ifdef USES_C228
  &CPlugin_228,
#endif // ifdef USES_C228

#ifdef USES_C229
  &CPlugin_229,
#endif // ifdef USES_C229

#ifdef USES_C230
  &CPlugin_230,
#endif // ifdef USES_C230

#ifdef USES_C231
  &CPlugin_231,
#endif // ifdef USES_C231

#ifdef USES_C232
  &CPlugin_232,
#endif // ifdef USES_C232

#ifdef USES_C233
  &CPlugin_233,
#endif // ifdef USES_C233

#ifdef USES_C234
  &CPlugin_234,
#endif // ifdef USES_C234

#ifdef USES_C235
  &CPlugin_235,
#endif // ifdef USES_C235

#ifdef USES_C236
  &CPlugin_236,
#endif // ifdef USES_C236

#ifdef USES_C237
  &CPlugin_237,
#endif // ifdef USES_C237

#ifdef USES_C238
  &CPlugin_238,
#endif // ifdef USES_C238

#ifdef USES_C239
  &CPlugin_239,
#endif // ifdef USES_C239

#ifdef USES_C240
  &CPlugin_240,
#endif // ifdef USES_C240

#ifdef USES_C241
  &CPlugin_241,
#endif // ifdef USES_C241

#ifdef USES_C242
  &CPlugin_242,
#endif // ifdef USES_C242

#ifdef USES_C243
  &CPlugin_243,
#endif // ifdef USES_C243

#ifdef USES_C244
  &CPlugin_244,
#endif // ifdef USES_C244

#ifdef USES_C245
  &CPlugin_245,
#endif // ifdef USES_C245

#ifdef USES_C246
  &CPlugin_246,
#endif // ifdef USES_C246

#ifdef USES_C247
  &CPlugin_247,
#endif // ifdef USES_C247

#ifdef USES_C248
  &CPlugin_248,
#endif // ifdef USES_C248

#ifdef USES_C249
  &CPlugin_249,
#endif // ifdef USES_C249

#ifdef USES_C250
  &CPlugin_250,
#endif // ifdef USES_C250

#ifdef USES_C251
  &CPlugin_251,
#endif // ifdef USES_C251

#ifdef USES_C252
  &CPlugin_252,
#endif // ifdef USES_C252

#ifdef USES_C253
  &CPlugin_253,
#endif // ifdef USES_C253

#ifdef USES_C254
  &CPlugin_254,
#endif // ifdef USES_C254

#ifdef USES_C255
  &CPlugin_255,
#endif // ifdef USES_C255
};


constexpr size_t ProtocolIndex_to_CPlugin_id_size = sizeof(ProtocolIndex_to_CPlugin_id);

// Highest CPlugin ID included in the build
constexpr size_t Highest_CPlugin_id = ProtocolIndex_to_CPlugin_id_size == 0 ? 0 : ProtocolIndex_to_CPlugin_id[ProtocolIndex_to_CPlugin_id_size - 1];

constexpr size_t CPlugin_id_to_ProtocolIndex_size = Highest_CPlugin_id + 1;

// Array filled during init.
// Valid index: 1 ... Highest_CPlugin_id
// Returns index to the ProtocolIndex_to_CPlugin_id array
protocolIndex_t CPlugin_id_to_ProtocolIndex[CPlugin_id_to_ProtocolIndex_size]{};


ProtocolStruct ProtocolArray[ProtocolIndex_to_CPlugin_id_size + 1]{};

ProtocolStruct& getProtocolStruct(protocolIndex_t protocolIndex)
{
  if (protocolIndex >= ProtocolIndex_to_CPlugin_id_size) {
    protocolIndex = ProtocolIndex_to_CPlugin_id_size;
  }
  return ProtocolArray[protocolIndex];
}

protocolIndex_t getProtocolIndex_from_CPluginID_(cpluginID_t cpluginID)
{
  if (cpluginID < CPlugin_id_to_ProtocolIndex_size)
  {
    return static_cast<protocolIndex_t>(CPlugin_id_to_ProtocolIndex[cpluginID]);
  }
  return INVALID_PROTOCOL_INDEX;
}

cpluginID_t getCPluginID_from_ProtocolIndex_(protocolIndex_t protocolIndex)
{
  if (protocolIndex < ProtocolIndex_to_CPlugin_id_size)
  {
    //    return static_cast<cpluginID_t>(ProtocolIndex_to_CPlugin_id[protocolIndex]);
    return static_cast<cpluginID_t>(pgm_read_byte(ProtocolIndex_to_CPlugin_id + protocolIndex));
  }
  return INVALID_C_PLUGIN_ID;
}

bool validProtocolIndex_init(protocolIndex_t protocolIndex)
{
  return protocolIndex < ProtocolIndex_to_CPlugin_id_size;
}

cpluginID_t getHighestIncludedCPluginID()
{ 
  return Highest_CPlugin_id;
}


bool CPluginCall(protocolIndex_t protocolIndex, CPlugin::Function Function, struct EventStruct *event, String& string)
{
  if (protocolIndex < ProtocolIndex_to_CPlugin_id_size)
  {
    START_TIMER;
    CPlugin_ptr_t cplugin_call = (CPlugin_ptr_t)pgm_read_ptr(CPlugin_ptr + protocolIndex);
    const bool res = cplugin_call(Function, event, string);
    STOP_TIMER_CONTROLLER(protocolIndex, Function);
    return res;
  }
  return false;
}

void CPluginSetup()
{
  static bool setupDone = false;

  if (setupDone) { return; }

  for (size_t id = 0; id < CPlugin_id_to_ProtocolIndex_size; ++id)
  {
    CPlugin_id_to_ProtocolIndex[id] = INVALID_PROTOCOL_INDEX;
  }

  for (protocolIndex_t protocolIndex = 0; protocolIndex < ProtocolIndex_to_CPlugin_id_size; ++protocolIndex)
  {
    const cpluginID_t cpluginID = getCPluginID_from_ProtocolIndex_(protocolIndex);

    if (INVALID_C_PLUGIN_ID != cpluginID) {
      CPlugin_id_to_ProtocolIndex[cpluginID] = protocolIndex;
      struct EventStruct TempEvent;
      TempEvent.idx = protocolIndex;
      String dummy;
      CPluginCall(protocolIndex, CPlugin::Function::CPLUGIN_PROTOCOL_ADD, &TempEvent, dummy);
    }
  }
  setupDone = true;
}

void CPluginInit()
{
  // Set all not supported cplugins to disabled.
  for (controllerIndex_t controller = 0; controller < CONTROLLER_MAX; ++controller) {
    if (!supportedCPluginID(Settings.Protocol[controller])) {
      Settings.ControllerEnabled[controller] = false;
    }
  }
  CPluginCall(CPlugin::Function::CPLUGIN_INIT_ALL, 0);
}

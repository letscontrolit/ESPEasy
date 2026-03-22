#include "../Helpers/_NWPlugin_init.h"

#include "../../../src/DataStructs/ESPEasy_EventStruct.h"
#include "../../../src/DataStructs/TimingStats.h"
#include "../../../src/DataTypes/ESPEasy_plugin_functions.h"
#include "../../../src/Globals/Settings.h"
#include "../../../src/Helpers/Misc.h"
#include "../../../src/Helpers/StringConverter.h"
#include "../Globals/NWPlugins.h"

namespace ESPEasy {
namespace net {

// ********************************************************************************
// Initialize all Network NWPlugins that where defined earlier
// and initialize the function call pointer into the NWPlugin array
// ********************************************************************************

constexpr /*nwpluginID_t*/ uint8_t NetworkDriverIndex_to_NWPlugin_id[] PROGMEM =
{
#ifdef USES_NW001
  1,
#endif // ifdef USES_NW001

#ifdef USES_NW002
  2,
#endif // ifdef USES_NW002

#ifdef USES_NW003
  3,
#endif // ifdef USES_NW003

#ifdef USES_NW004
  4,
#endif // ifdef USES_NW004

#ifdef USES_NW005
  5,
#endif // ifdef USES_NW005

#ifdef USES_NW006
  6,
#endif // ifdef USES_NW006

#ifdef USES_NW007
  7,
#endif // ifdef USES_NW007

#ifdef USES_NW008
  8,
#endif // ifdef USES_NW008

#ifdef USES_NW009
  9,
#endif // ifdef USES_NW009

#ifdef USES_NW010
  10,
#endif // ifdef USES_NW010

#ifdef USES_NW011
  11,
#endif // ifdef USES_NW011

#ifdef USES_NW012
  12,
#endif // ifdef USES_NW012

#ifdef USES_NW013
  13,
#endif // ifdef USES_NW013

#ifdef USES_NW014
  14,
#endif // ifdef USES_NW014

#ifdef USES_NW015
  15,
#endif // ifdef USES_NW015

#ifdef USES_NW016
  16,
#endif // ifdef USES_NW016

#ifdef USES_NW017
  17,
#endif // ifdef USES_NW017

#ifdef USES_NW018
  18,
#endif // ifdef USES_NW018

#ifdef USES_NW019
  19,
#endif // ifdef USES_NW019

#ifdef USES_NW020
  20,
#endif // ifdef USES_NW020

#ifdef USES_NW021
  21,
#endif // ifdef USES_NW021

#ifdef USES_NW022
  22,
#endif // ifdef USES_NW022

#ifdef USES_NW023
  23,
#endif // ifdef USES_NW023

#ifdef USES_NW024
  24,
#endif // ifdef USES_NW024

#ifdef USES_NW025
  25,
#endif // ifdef USES_NW025

#ifdef USES_NW026
  26,
#endif // ifdef USES_NW026

#ifdef USES_NW027
  27,
#endif // ifdef USES_NW027

#ifdef USES_NW028
  28,
#endif // ifdef USES_NW028

#ifdef USES_NW029
  29,
#endif // ifdef USES_NW029

#ifdef USES_NW030
  30,
#endif // ifdef USES_NW030

#ifdef USES_NW031
  31,
#endif // ifdef USES_NW031

#ifdef USES_NW032
  32,
#endif // ifdef USES_NW032

#ifdef USES_NW033
  33,
#endif // ifdef USES_NW033

#ifdef USES_NW034
  34,
#endif // ifdef USES_NW034

#ifdef USES_NW035
  35,
#endif // ifdef USES_NW035

#ifdef USES_NW036
  36,
#endif // ifdef USES_NW036

#ifdef USES_NW037
  37,
#endif // ifdef USES_NW037

#ifdef USES_NW038
  38,
#endif // ifdef USES_NW038

#ifdef USES_NW039
  39,
#endif // ifdef USES_NW039

#ifdef USES_NW040
  40,
#endif // ifdef USES_NW040

#ifdef USES_NW041
  41,
#endif // ifdef USES_NW041

#ifdef USES_NW042
  42,
#endif // ifdef USES_NW042

#ifdef USES_NW043
  43,
#endif // ifdef USES_NW043

#ifdef USES_NW044
  44,
#endif // ifdef USES_NW044

#ifdef USES_NW045
  45,
#endif // ifdef USES_NW045

#ifdef USES_NW046
  46,
#endif // ifdef USES_NW046

#ifdef USES_NW047
  47,
#endif // ifdef USES_NW047

#ifdef USES_NW048
  48,
#endif // ifdef USES_NW048

#ifdef USES_NW049
  49,
#endif // ifdef USES_NW049

#ifdef USES_NW050
  50,
#endif // ifdef USES_NW050

#ifdef USES_NW051
  51,
#endif // ifdef USES_NW051

#ifdef USES_NW052
  52,
#endif // ifdef USES_NW052

#ifdef USES_NW053
  53,
#endif // ifdef USES_NW053

#ifdef USES_NW054
  54,
#endif // ifdef USES_NW054

#ifdef USES_NW055
  55,
#endif // ifdef USES_NW055

#ifdef USES_NW056
  56,
#endif // ifdef USES_NW056

#ifdef USES_NW057
  57,
#endif // ifdef USES_NW057

#ifdef USES_NW058
  58,
#endif // ifdef USES_NW058

#ifdef USES_NW059
  59,
#endif // ifdef USES_NW059

#ifdef USES_NW060
  60,
#endif // ifdef USES_NW060

#ifdef USES_NW061
  61,
#endif // ifdef USES_NW061

#ifdef USES_NW062
  62,
#endif // ifdef USES_NW062

#ifdef USES_NW063
  63,
#endif // ifdef USES_NW063

#ifdef USES_NW064
  64,
#endif // ifdef USES_NW064

#ifdef USES_NW065
  65,
#endif // ifdef USES_NW065

#ifdef USES_NW066
  66,
#endif // ifdef USES_NW066

#ifdef USES_NW067
  67,
#endif // ifdef USES_NW067

#ifdef USES_NW068
  68,
#endif // ifdef USES_NW068

#ifdef USES_NW069
  69,
#endif // ifdef USES_NW069

#ifdef USES_NW070
  70,
#endif // ifdef USES_NW070

#ifdef USES_NW071
  71,
#endif // ifdef USES_NW071

#ifdef USES_NW072
  72,
#endif // ifdef USES_NW072

#ifdef USES_NW073
  73,
#endif // ifdef USES_NW073

#ifdef USES_NW074
  74,
#endif // ifdef USES_NW074

#ifdef USES_NW075
  75,
#endif // ifdef USES_NW075

#ifdef USES_NW076
  76,
#endif // ifdef USES_NW076

#ifdef USES_NW077
  77,
#endif // ifdef USES_NW077

#ifdef USES_NW078
  78,
#endif // ifdef USES_NW078

#ifdef USES_NW079
  79,
#endif // ifdef USES_NW079

#ifdef USES_NW080
  80,
#endif // ifdef USES_NW080

#ifdef USES_NW081
  81,
#endif // ifdef USES_NW081

#ifdef USES_NW082
  82,
#endif // ifdef USES_NW082

#ifdef USES_NW083
  83,
#endif // ifdef USES_NW083

#ifdef USES_NW084
  84,
#endif // ifdef USES_NW084

#ifdef USES_NW085
  85,
#endif // ifdef USES_NW085

#ifdef USES_NW086
  86,
#endif // ifdef USES_NW086

#ifdef USES_NW087
  87,
#endif // ifdef USES_NW087

#ifdef USES_NW088
  88,
#endif // ifdef USES_NW088

#ifdef USES_NW089
  89,
#endif // ifdef USES_NW089

#ifdef USES_NW090
  90,
#endif // ifdef USES_NW090

#ifdef USES_NW091
  91,
#endif // ifdef USES_NW091

#ifdef USES_NW092
  92,
#endif // ifdef USES_NW092

#ifdef USES_NW093
  93,
#endif // ifdef USES_NW093

#ifdef USES_NW094
  94,
#endif // ifdef USES_NW094

#ifdef USES_NW095
  95,
#endif // ifdef USES_NW095

#ifdef USES_NW096
  96,
#endif // ifdef USES_NW096

#ifdef USES_NW097
  97,
#endif // ifdef USES_NW097

#ifdef USES_NW098
  98,
#endif // ifdef USES_NW098

#ifdef USES_NW099
  99,
#endif // ifdef USES_NW099

#ifdef USES_NW100
  100,
#endif // ifdef USES_NW100

#ifdef USES_NW101
  101,
#endif // ifdef USES_NW101

#ifdef USES_NW102
  102,
#endif // ifdef USES_NW102

#ifdef USES_NW103
  103,
#endif // ifdef USES_NW103

#ifdef USES_NW104
  104,
#endif // ifdef USES_NW104

#ifdef USES_NW105
  105,
#endif // ifdef USES_NW105

#ifdef USES_NW106
  106,
#endif // ifdef USES_NW106

#ifdef USES_NW107
  107,
#endif // ifdef USES_NW107

#ifdef USES_NW108
  108,
#endif // ifdef USES_NW108

#ifdef USES_NW109
  109,
#endif // ifdef USES_NW109

#ifdef USES_NW110
  110,
#endif // ifdef USES_NW110

#ifdef USES_NW111
  111,
#endif // ifdef USES_NW111

#ifdef USES_NW112
  112,
#endif // ifdef USES_NW112

#ifdef USES_NW113
  113,
#endif // ifdef USES_NW113

#ifdef USES_NW114
  114,
#endif // ifdef USES_NW114

#ifdef USES_NW115
  115,
#endif // ifdef USES_NW115

#ifdef USES_NW116
  116,
#endif // ifdef USES_NW116

#ifdef USES_NW117
  117,
#endif // ifdef USES_NW117

#ifdef USES_NW118
  118,
#endif // ifdef USES_NW118

#ifdef USES_NW119
  119,
#endif // ifdef USES_NW119

#ifdef USES_NW120
  120,
#endif // ifdef USES_NW120

#ifdef USES_NW121
  121,
#endif // ifdef USES_NW121

#ifdef USES_NW122
  122,
#endif // ifdef USES_NW122

#ifdef USES_NW123
  123,
#endif // ifdef USES_NW123

#ifdef USES_NW124
  124,
#endif // ifdef USES_NW124

#ifdef USES_NW125
  125,
#endif // ifdef USES_NW125

#ifdef USES_NW126
  126,
#endif // ifdef USES_NW126

#ifdef USES_NW127
  127,
#endif // ifdef USES_NW127

#ifdef USES_NW128
  128,
#endif // ifdef USES_NW128

#ifdef USES_NW129
  129,
#endif // ifdef USES_NW129

#ifdef USES_NW130
  130,
#endif // ifdef USES_NW130

#ifdef USES_NW131
  131,
#endif // ifdef USES_NW131

#ifdef USES_NW132
  132,
#endif // ifdef USES_NW132

#ifdef USES_NW133
  133,
#endif // ifdef USES_NW133

#ifdef USES_NW134
  134,
#endif // ifdef USES_NW134

#ifdef USES_NW135
  135,
#endif // ifdef USES_NW135

#ifdef USES_NW136
  136,
#endif // ifdef USES_NW136

#ifdef USES_NW137
  137,
#endif // ifdef USES_NW137

#ifdef USES_NW138
  138,
#endif // ifdef USES_NW138

#ifdef USES_NW139
  139,
#endif // ifdef USES_NW139

#ifdef USES_NW140
  140,
#endif // ifdef USES_NW140

#ifdef USES_NW141
  141,
#endif // ifdef USES_NW141

#ifdef USES_NW142
  142,
#endif // ifdef USES_NW142

#ifdef USES_NW143
  143,
#endif // ifdef USES_NW143

#ifdef USES_NW144
  144,
#endif // ifdef USES_NW144

#ifdef USES_NW145
  145,
#endif // ifdef USES_NW145

#ifdef USES_NW146
  146,
#endif // ifdef USES_NW146

#ifdef USES_NW147
  147,
#endif // ifdef USES_NW147

#ifdef USES_NW148
  148,
#endif // ifdef USES_NW148

#ifdef USES_NW149
  149,
#endif // ifdef USES_NW149

#ifdef USES_NW150
  150,
#endif // ifdef USES_NW150

#ifdef USES_NW151
  151,
#endif // ifdef USES_NW151

#ifdef USES_NW152
  152,
#endif // ifdef USES_NW152

#ifdef USES_NW153
  153,
#endif // ifdef USES_NW153

#ifdef USES_NW154
  154,
#endif // ifdef USES_NW154

#ifdef USES_NW155
  155,
#endif // ifdef USES_NW155

#ifdef USES_NW156
  156,
#endif // ifdef USES_NW156

#ifdef USES_NW157
  157,
#endif // ifdef USES_NW157

#ifdef USES_NW158
  158,
#endif // ifdef USES_NW158

#ifdef USES_NW159
  159,
#endif // ifdef USES_NW159

#ifdef USES_NW160
  160,
#endif // ifdef USES_NW160

#ifdef USES_NW161
  161,
#endif // ifdef USES_NW161

#ifdef USES_NW162
  162,
#endif // ifdef USES_NW162

#ifdef USES_NW163
  163,
#endif // ifdef USES_NW163

#ifdef USES_NW164
  164,
#endif // ifdef USES_NW164

#ifdef USES_NW165
  165,
#endif // ifdef USES_NW165

#ifdef USES_NW166
  166,
#endif // ifdef USES_NW166

#ifdef USES_NW167
  167,
#endif // ifdef USES_NW167

#ifdef USES_NW168
  168,
#endif // ifdef USES_NW168

#ifdef USES_NW169
  169,
#endif // ifdef USES_NW169

#ifdef USES_NW170
  170,
#endif // ifdef USES_NW170

#ifdef USES_NW171
  171,
#endif // ifdef USES_NW171

#ifdef USES_NW172
  172,
#endif // ifdef USES_NW172

#ifdef USES_NW173
  173,
#endif // ifdef USES_NW173

#ifdef USES_NW174
  174,
#endif // ifdef USES_NW174

#ifdef USES_NW175
  175,
#endif // ifdef USES_NW175

#ifdef USES_NW176
  176,
#endif // ifdef USES_NW176

#ifdef USES_NW177
  177,
#endif // ifdef USES_NW177

#ifdef USES_NW178
  178,
#endif // ifdef USES_NW178

#ifdef USES_NW179
  179,
#endif // ifdef USES_NW179

#ifdef USES_NW180
  180,
#endif // ifdef USES_NW180

#ifdef USES_NW181
  181,
#endif // ifdef USES_NW181

#ifdef USES_NW182
  182,
#endif // ifdef USES_NW182

#ifdef USES_NW183
  183,
#endif // ifdef USES_NW183

#ifdef USES_NW184
  184,
#endif // ifdef USES_NW184

#ifdef USES_NW185
  185,
#endif // ifdef USES_NW185

#ifdef USES_NW186
  186,
#endif // ifdef USES_NW186

#ifdef USES_NW187
  187,
#endif // ifdef USES_NW187

#ifdef USES_NW188
  188,
#endif // ifdef USES_NW188

#ifdef USES_NW189
  189,
#endif // ifdef USES_NW189

#ifdef USES_NW190
  190,
#endif // ifdef USES_NW190

#ifdef USES_NW191
  191,
#endif // ifdef USES_NW191

#ifdef USES_NW192
  192,
#endif // ifdef USES_NW192

#ifdef USES_NW193
  193,
#endif // ifdef USES_NW193

#ifdef USES_NW194
  194,
#endif // ifdef USES_NW194

#ifdef USES_NW195
  195,
#endif // ifdef USES_NW195

#ifdef USES_NW196
  196,
#endif // ifdef USES_NW196

#ifdef USES_NW197
  197,
#endif // ifdef USES_NW197

#ifdef USES_NW198
  198,
#endif // ifdef USES_NW198

#ifdef USES_NW199
  199,
#endif // ifdef USES_NW199

#ifdef USES_NW200
  200,
#endif // ifdef USES_NW200

#ifdef USES_NW201
  201,
#endif // ifdef USES_NW201

#ifdef USES_NW202
  202,
#endif // ifdef USES_NW202

#ifdef USES_NW203
  203,
#endif // ifdef USES_NW203

#ifdef USES_NW204
  204,
#endif // ifdef USES_NW204

#ifdef USES_NW205
  205,
#endif // ifdef USES_NW205

#ifdef USES_NW206
  206,
#endif // ifdef USES_NW206

#ifdef USES_NW207
  207,
#endif // ifdef USES_NW207

#ifdef USES_NW208
  208,
#endif // ifdef USES_NW208

#ifdef USES_NW209
  209,
#endif // ifdef USES_NW209

#ifdef USES_NW210
  210,
#endif // ifdef USES_NW210

#ifdef USES_NW211
  211,
#endif // ifdef USES_NW211

#ifdef USES_NW212
  212,
#endif // ifdef USES_NW212

#ifdef USES_NW213
  213,
#endif // ifdef USES_NW213

#ifdef USES_NW214
  214,
#endif // ifdef USES_NW214

#ifdef USES_NW215
  215,
#endif // ifdef USES_NW215

#ifdef USES_NW216
  216,
#endif // ifdef USES_NW216

#ifdef USES_NW217
  217,
#endif // ifdef USES_NW217

#ifdef USES_NW218
  218,
#endif // ifdef USES_NW218

#ifdef USES_NW219
  219,
#endif // ifdef USES_NW219

#ifdef USES_NW220
  220,
#endif // ifdef USES_NW220

#ifdef USES_NW221
  221,
#endif // ifdef USES_NW221

#ifdef USES_NW222
  222,
#endif // ifdef USES_NW222

#ifdef USES_NW223
  223,
#endif // ifdef USES_NW223

#ifdef USES_NW224
  224,
#endif // ifdef USES_NW224

#ifdef USES_NW225
  225,
#endif // ifdef USES_NW225

#ifdef USES_NW226
  226,
#endif // ifdef USES_NW226

#ifdef USES_NW227
  227,
#endif // ifdef USES_NW227

#ifdef USES_NW228
  228,
#endif // ifdef USES_NW228

#ifdef USES_NW229
  229,
#endif // ifdef USES_NW229

#ifdef USES_NW230
  230,
#endif // ifdef USES_NW230

#ifdef USES_NW231
  231,
#endif // ifdef USES_NW231

#ifdef USES_NW232
  232,
#endif // ifdef USES_NW232

#ifdef USES_NW233
  233,
#endif // ifdef USES_NW233

#ifdef USES_NW234
  234,
#endif // ifdef USES_NW234

#ifdef USES_NW235
  235,
#endif // ifdef USES_NW235

#ifdef USES_NW236
  236,
#endif // ifdef USES_NW236

#ifdef USES_NW237
  237,
#endif // ifdef USES_NW237

#ifdef USES_NW238
  238,
#endif // ifdef USES_NW238

#ifdef USES_NW239
  239,
#endif // ifdef USES_NW239

#ifdef USES_NW240
  240,
#endif // ifdef USES_NW240

#ifdef USES_NW241
  241,
#endif // ifdef USES_NW241

#ifdef USES_NW242
  242,
#endif // ifdef USES_NW242

#ifdef USES_NW243
  243,
#endif // ifdef USES_NW243

#ifdef USES_NW244
  244,
#endif // ifdef USES_NW244

#ifdef USES_NW245
  245,
#endif // ifdef USES_NW245

#ifdef USES_NW246
  246,
#endif // ifdef USES_NW246

#ifdef USES_NW247
  247,
#endif // ifdef USES_NW247

#ifdef USES_NW248
  248,
#endif // ifdef USES_NW248

#ifdef USES_NW249
  249,
#endif // ifdef USES_NW249

#ifdef USES_NW250
  250,
#endif // ifdef USES_NW250

#ifdef USES_NW251
  251,
#endif // ifdef USES_NW251

#ifdef USES_NW252
  252,
#endif // ifdef USES_NW252

#ifdef USES_NW253
  253,
#endif // ifdef USES_NW253

#ifdef USES_NW254
  254,
#endif // ifdef USES_NW254

#ifdef USES_NW255
  255,
#endif // ifdef USES_NW255
};


typedef bool (*NWPlugin_ptr_t)(NWPlugin::Function,
                               EventStruct *,
                               String&);

const NWPlugin_ptr_t PROGMEM NWPlugin_ptr[] =
{
#ifdef USES_NW001
  &NWPlugin_001,
#endif // ifdef USES_NW001

#ifdef USES_NW002
  &NWPlugin_002,
#endif // ifdef USES_NW002

#ifdef USES_NW003
  &NWPlugin_003,
#endif // ifdef USES_NW003

#ifdef USES_NW004
  &NWPlugin_004,
#endif // ifdef USES_NW004

#ifdef USES_NW005
  &NWPlugin_005,
#endif // ifdef USES_NW005

#ifdef USES_NW006
  &NWPlugin_006,
#endif // ifdef USES_NW006

#ifdef USES_NW007
  &NWPlugin_007,
#endif // ifdef USES_NW007

#ifdef USES_NW008
  &NWPlugin_008,
#endif // ifdef USES_NW008

#ifdef USES_NW009
  &NWPlugin_009,
#endif // ifdef USES_NW009

#ifdef USES_NW010
  &NWPlugin_010,
#endif // ifdef USES_NW010

#ifdef USES_NW011
  &NWPlugin_011,
#endif // ifdef USES_NW011

#ifdef USES_NW012
  &NWPlugin_012,
#endif // ifdef USES_NW012

#ifdef USES_NW013
  &NWPlugin_013,
#endif // ifdef USES_NW013

#ifdef USES_NW014
  &NWPlugin_014,
#endif // ifdef USES_NW014

#ifdef USES_NW015
  &NWPlugin_015,
#endif // ifdef USES_NW015

#ifdef USES_NW016
  &NWPlugin_016,
#endif // ifdef USES_NW016

#ifdef USES_NW017
  &NWPlugin_017,
#endif // ifdef USES_NW017

#ifdef USES_NW018
  &NWPlugin_018,
#endif // ifdef USES_NW018

#ifdef USES_NW019
  &NWPlugin_019,
#endif // ifdef USES_NW019

#ifdef USES_NW020
  &NWPlugin_020,
#endif // ifdef USES_NW020

#ifdef USES_NW021
  &NWPlugin_021,
#endif // ifdef USES_NW021

#ifdef USES_NW022
  &NWPlugin_022,
#endif // ifdef USES_NW022

#ifdef USES_NW023
  &NWPlugin_023,
#endif // ifdef USES_NW023

#ifdef USES_NW024
  &NWPlugin_024,
#endif // ifdef USES_NW024

#ifdef USES_NW025
  &NWPlugin_025,
#endif // ifdef USES_NW025

#ifdef USES_NW026
  &NWPlugin_026,
#endif // ifdef USES_NW026

#ifdef USES_NW027
  &NWPlugin_027,
#endif // ifdef USES_NW027

#ifdef USES_NW028
  &NWPlugin_028,
#endif // ifdef USES_NW028

#ifdef USES_NW029
  &NWPlugin_029,
#endif // ifdef USES_NW029

#ifdef USES_NW030
  &NWPlugin_030,
#endif // ifdef USES_NW030

#ifdef USES_NW031
  &NWPlugin_031,
#endif // ifdef USES_NW031

#ifdef USES_NW032
  &NWPlugin_032,
#endif // ifdef USES_NW032

#ifdef USES_NW033
  &NWPlugin_033,
#endif // ifdef USES_NW033

#ifdef USES_NW034
  &NWPlugin_034,
#endif // ifdef USES_NW034

#ifdef USES_NW035
  &NWPlugin_035,
#endif // ifdef USES_NW035

#ifdef USES_NW036
  &NWPlugin_036,
#endif // ifdef USES_NW036

#ifdef USES_NW037
  &NWPlugin_037,
#endif // ifdef USES_NW037

#ifdef USES_NW038
  &NWPlugin_038,
#endif // ifdef USES_NW038

#ifdef USES_NW039
  &NWPlugin_039,
#endif // ifdef USES_NW039

#ifdef USES_NW040
  &NWPlugin_040,
#endif // ifdef USES_NW040

#ifdef USES_NW041
  &NWPlugin_041,
#endif // ifdef USES_NW041

#ifdef USES_NW042
  &NWPlugin_042,
#endif // ifdef USES_NW042

#ifdef USES_NW043
  &NWPlugin_043,
#endif // ifdef USES_NW043

#ifdef USES_NW044
  &NWPlugin_044,
#endif // ifdef USES_NW044

#ifdef USES_NW045
  &NWPlugin_045,
#endif // ifdef USES_NW045

#ifdef USES_NW046
  &NWPlugin_046,
#endif // ifdef USES_NW046

#ifdef USES_NW047
  &NWPlugin_047,
#endif // ifdef USES_NW047

#ifdef USES_NW048
  &NWPlugin_048,
#endif // ifdef USES_NW048

#ifdef USES_NW049
  &NWPlugin_049,
#endif // ifdef USES_NW049

#ifdef USES_NW050
  &NWPlugin_050,
#endif // ifdef USES_NW050

#ifdef USES_NW051
  &NWPlugin_051,
#endif // ifdef USES_NW051

#ifdef USES_NW052
  &NWPlugin_052,
#endif // ifdef USES_NW052

#ifdef USES_NW053
  &NWPlugin_053,
#endif // ifdef USES_NW053

#ifdef USES_NW054
  &NWPlugin_054,
#endif // ifdef USES_NW054

#ifdef USES_NW055
  &NWPlugin_055,
#endif // ifdef USES_NW055

#ifdef USES_NW056
  &NWPlugin_056,
#endif // ifdef USES_NW056

#ifdef USES_NW057
  &NWPlugin_057,
#endif // ifdef USES_NW057

#ifdef USES_NW058
  &NWPlugin_058,
#endif // ifdef USES_NW058

#ifdef USES_NW059
  &NWPlugin_059,
#endif // ifdef USES_NW059

#ifdef USES_NW060
  &NWPlugin_060,
#endif // ifdef USES_NW060

#ifdef USES_NW061
  &NWPlugin_061,
#endif // ifdef USES_NW061

#ifdef USES_NW062
  &NWPlugin_062,
#endif // ifdef USES_NW062

#ifdef USES_NW063
  &NWPlugin_063,
#endif // ifdef USES_NW063

#ifdef USES_NW064
  &NWPlugin_064,
#endif // ifdef USES_NW064

#ifdef USES_NW065
  &NWPlugin_065,
#endif // ifdef USES_NW065

#ifdef USES_NW066
  &NWPlugin_066,
#endif // ifdef USES_NW066

#ifdef USES_NW067
  &NWPlugin_067,
#endif // ifdef USES_NW067

#ifdef USES_NW068
  &NWPlugin_068,
#endif // ifdef USES_NW068

#ifdef USES_NW069
  &NWPlugin_069,
#endif // ifdef USES_NW069

#ifdef USES_NW070
  &NWPlugin_070,
#endif // ifdef USES_NW070

#ifdef USES_NW071
  &NWPlugin_071,
#endif // ifdef USES_NW071

#ifdef USES_NW072
  &NWPlugin_072,
#endif // ifdef USES_NW072

#ifdef USES_NW073
  &NWPlugin_073,
#endif // ifdef USES_NW073

#ifdef USES_NW074
  &NWPlugin_074,
#endif // ifdef USES_NW074

#ifdef USES_NW075
  &NWPlugin_075,
#endif // ifdef USES_NW075

#ifdef USES_NW076
  &NWPlugin_076,
#endif // ifdef USES_NW076

#ifdef USES_NW077
  &NWPlugin_077,
#endif // ifdef USES_NW077

#ifdef USES_NW078
  &NWPlugin_078,
#endif // ifdef USES_NW078

#ifdef USES_NW079
  &NWPlugin_079,
#endif // ifdef USES_NW079

#ifdef USES_NW080
  &NWPlugin_080,
#endif // ifdef USES_NW080

#ifdef USES_NW081
  &NWPlugin_081,
#endif // ifdef USES_NW081

#ifdef USES_NW082
  &NWPlugin_082,
#endif // ifdef USES_NW082

#ifdef USES_NW083
  &NWPlugin_083,
#endif // ifdef USES_NW083

#ifdef USES_NW084
  &NWPlugin_084,
#endif // ifdef USES_NW084

#ifdef USES_NW085
  &NWPlugin_085,
#endif // ifdef USES_NW085

#ifdef USES_NW086
  &NWPlugin_086,
#endif // ifdef USES_NW086

#ifdef USES_NW087
  &NWPlugin_087,
#endif // ifdef USES_NW087

#ifdef USES_NW088
  &NWPlugin_088,
#endif // ifdef USES_NW088

#ifdef USES_NW089
  &NWPlugin_089,
#endif // ifdef USES_NW089

#ifdef USES_NW090
  &NWPlugin_090,
#endif // ifdef USES_NW090

#ifdef USES_NW091
  &NWPlugin_091,
#endif // ifdef USES_NW091

#ifdef USES_NW092
  &NWPlugin_092,
#endif // ifdef USES_NW092

#ifdef USES_NW093
  &NWPlugin_093,
#endif // ifdef USES_NW093

#ifdef USES_NW094
  &NWPlugin_094,
#endif // ifdef USES_NW094

#ifdef USES_NW095
  &NWPlugin_095,
#endif // ifdef USES_NW095

#ifdef USES_NW096
  &NWPlugin_096,
#endif // ifdef USES_NW096

#ifdef USES_NW097
  &NWPlugin_097,
#endif // ifdef USES_NW097

#ifdef USES_NW098
  &NWPlugin_098,
#endif // ifdef USES_NW098

#ifdef USES_NW099
  &NWPlugin_099,
#endif // ifdef USES_NW099

#ifdef USES_NW100
  &NWPlugin_100,
#endif // ifdef USES_NW100

#ifdef USES_NW101
  &NWPlugin_101,
#endif // ifdef USES_NW101

#ifdef USES_NW102
  &NWPlugin_102,
#endif // ifdef USES_NW102

#ifdef USES_NW103
  &NWPlugin_103,
#endif // ifdef USES_NW103

#ifdef USES_NW104
  &NWPlugin_104,
#endif // ifdef USES_NW104

#ifdef USES_NW105
  &NWPlugin_105,
#endif // ifdef USES_NW105

#ifdef USES_NW106
  &NWPlugin_106,
#endif // ifdef USES_NW106

#ifdef USES_NW107
  &NWPlugin_107,
#endif // ifdef USES_NW107

#ifdef USES_NW108
  &NWPlugin_108,
#endif // ifdef USES_NW108

#ifdef USES_NW109
  &NWPlugin_109,
#endif // ifdef USES_NW109

#ifdef USES_NW110
  &NWPlugin_110,
#endif // ifdef USES_NW110

#ifdef USES_NW111
  &NWPlugin_111,
#endif // ifdef USES_NW111

#ifdef USES_NW112
  &NWPlugin_112,
#endif // ifdef USES_NW112

#ifdef USES_NW113
  &NWPlugin_113,
#endif // ifdef USES_NW113

#ifdef USES_NW114
  &NWPlugin_114,
#endif // ifdef USES_NW114

#ifdef USES_NW115
  &NWPlugin_115,
#endif // ifdef USES_NW115

#ifdef USES_NW116
  &NWPlugin_116,
#endif // ifdef USES_NW116

#ifdef USES_NW117
  &NWPlugin_117,
#endif // ifdef USES_NW117

#ifdef USES_NW118
  &NWPlugin_118,
#endif // ifdef USES_NW118

#ifdef USES_NW119
  &NWPlugin_119,
#endif // ifdef USES_NW119

#ifdef USES_NW120
  &NWPlugin_120,
#endif // ifdef USES_NW120

#ifdef USES_NW121
  &NWPlugin_121,
#endif // ifdef USES_NW121

#ifdef USES_NW122
  &NWPlugin_122,
#endif // ifdef USES_NW122

#ifdef USES_NW123
  &NWPlugin_123,
#endif // ifdef USES_NW123

#ifdef USES_NW124
  &NWPlugin_124,
#endif // ifdef USES_NW124

#ifdef USES_NW125
  &NWPlugin_125,
#endif // ifdef USES_NW125

#ifdef USES_NW126
  &NWPlugin_126,
#endif // ifdef USES_NW126

#ifdef USES_NW127
  &NWPlugin_127,
#endif // ifdef USES_NW127

#ifdef USES_NW128
  &NWPlugin_128,
#endif // ifdef USES_NW128

#ifdef USES_NW129
  &NWPlugin_129,
#endif // ifdef USES_NW129

#ifdef USES_NW130
  &NWPlugin_130,
#endif // ifdef USES_NW130

#ifdef USES_NW131
  &NWPlugin_131,
#endif // ifdef USES_NW131

#ifdef USES_NW132
  &NWPlugin_132,
#endif // ifdef USES_NW132

#ifdef USES_NW133
  &NWPlugin_133,
#endif // ifdef USES_NW133

#ifdef USES_NW134
  &NWPlugin_134,
#endif // ifdef USES_NW134

#ifdef USES_NW135
  &NWPlugin_135,
#endif // ifdef USES_NW135

#ifdef USES_NW136
  &NWPlugin_136,
#endif // ifdef USES_NW136

#ifdef USES_NW137
  &NWPlugin_137,
#endif // ifdef USES_NW137

#ifdef USES_NW138
  &NWPlugin_138,
#endif // ifdef USES_NW138

#ifdef USES_NW139
  &NWPlugin_139,
#endif // ifdef USES_NW139

#ifdef USES_NW140
  &NWPlugin_140,
#endif // ifdef USES_NW140

#ifdef USES_NW141
  &NWPlugin_141,
#endif // ifdef USES_NW141

#ifdef USES_NW142
  &NWPlugin_142,
#endif // ifdef USES_NW142

#ifdef USES_NW143
  &NWPlugin_143,
#endif // ifdef USES_NW143

#ifdef USES_NW144
  &NWPlugin_144,
#endif // ifdef USES_NW144

#ifdef USES_NW145
  &NWPlugin_145,
#endif // ifdef USES_NW145

#ifdef USES_NW146
  &NWPlugin_146,
#endif // ifdef USES_NW146

#ifdef USES_NW147
  &NWPlugin_147,
#endif // ifdef USES_NW147

#ifdef USES_NW148
  &NWPlugin_148,
#endif // ifdef USES_NW148

#ifdef USES_NW149
  &NWPlugin_149,
#endif // ifdef USES_NW149

#ifdef USES_NW150
  &NWPlugin_150,
#endif // ifdef USES_NW150

#ifdef USES_NW151
  &NWPlugin_151,
#endif // ifdef USES_NW151

#ifdef USES_NW152
  &NWPlugin_152,
#endif // ifdef USES_NW152

#ifdef USES_NW153
  &NWPlugin_153,
#endif // ifdef USES_NW153

#ifdef USES_NW154
  &NWPlugin_154,
#endif // ifdef USES_NW154

#ifdef USES_NW155
  &NWPlugin_155,
#endif // ifdef USES_NW155

#ifdef USES_NW156
  &NWPlugin_156,
#endif // ifdef USES_NW156

#ifdef USES_NW157
  &NWPlugin_157,
#endif // ifdef USES_NW157

#ifdef USES_NW158
  &NWPlugin_158,
#endif // ifdef USES_NW158

#ifdef USES_NW159
  &NWPlugin_159,
#endif // ifdef USES_NW159

#ifdef USES_NW160
  &NWPlugin_160,
#endif // ifdef USES_NW160

#ifdef USES_NW161
  &NWPlugin_161,
#endif // ifdef USES_NW161

#ifdef USES_NW162
  &NWPlugin_162,
#endif // ifdef USES_NW162

#ifdef USES_NW163
  &NWPlugin_163,
#endif // ifdef USES_NW163

#ifdef USES_NW164
  &NWPlugin_164,
#endif // ifdef USES_NW164

#ifdef USES_NW165
  &NWPlugin_165,
#endif // ifdef USES_NW165

#ifdef USES_NW166
  &NWPlugin_166,
#endif // ifdef USES_NW166

#ifdef USES_NW167
  &NWPlugin_167,
#endif // ifdef USES_NW167

#ifdef USES_NW168
  &NWPlugin_168,
#endif // ifdef USES_NW168

#ifdef USES_NW169
  &NWPlugin_169,
#endif // ifdef USES_NW169

#ifdef USES_NW170
  &NWPlugin_170,
#endif // ifdef USES_NW170

#ifdef USES_NW171
  &NWPlugin_171,
#endif // ifdef USES_NW171

#ifdef USES_NW172
  &NWPlugin_172,
#endif // ifdef USES_NW172

#ifdef USES_NW173
  &NWPlugin_173,
#endif // ifdef USES_NW173

#ifdef USES_NW174
  &NWPlugin_174,
#endif // ifdef USES_NW174

#ifdef USES_NW175
  &NWPlugin_175,
#endif // ifdef USES_NW175

#ifdef USES_NW176
  &NWPlugin_176,
#endif // ifdef USES_NW176

#ifdef USES_NW177
  &NWPlugin_177,
#endif // ifdef USES_NW177

#ifdef USES_NW178
  &NWPlugin_178,
#endif // ifdef USES_NW178

#ifdef USES_NW179
  &NWPlugin_179,
#endif // ifdef USES_NW179

#ifdef USES_NW180
  &NWPlugin_180,
#endif // ifdef USES_NW180

#ifdef USES_NW181
  &NWPlugin_181,
#endif // ifdef USES_NW181

#ifdef USES_NW182
  &NWPlugin_182,
#endif // ifdef USES_NW182

#ifdef USES_NW183
  &NWPlugin_183,
#endif // ifdef USES_NW183

#ifdef USES_NW184
  &NWPlugin_184,
#endif // ifdef USES_NW184

#ifdef USES_NW185
  &NWPlugin_185,
#endif // ifdef USES_NW185

#ifdef USES_NW186
  &NWPlugin_186,
#endif // ifdef USES_NW186

#ifdef USES_NW187
  &NWPlugin_187,
#endif // ifdef USES_NW187

#ifdef USES_NW188
  &NWPlugin_188,
#endif // ifdef USES_NW188

#ifdef USES_NW189
  &NWPlugin_189,
#endif // ifdef USES_NW189

#ifdef USES_NW190
  &NWPlugin_190,
#endif // ifdef USES_NW190

#ifdef USES_NW191
  &NWPlugin_191,
#endif // ifdef USES_NW191

#ifdef USES_NW192
  &NWPlugin_192,
#endif // ifdef USES_NW192

#ifdef USES_NW193
  &NWPlugin_193,
#endif // ifdef USES_NW193

#ifdef USES_NW194
  &NWPlugin_194,
#endif // ifdef USES_NW194

#ifdef USES_NW195
  &NWPlugin_195,
#endif // ifdef USES_NW195

#ifdef USES_NW196
  &NWPlugin_196,
#endif // ifdef USES_NW196

#ifdef USES_NW197
  &NWPlugin_197,
#endif // ifdef USES_NW197

#ifdef USES_NW198
  &NWPlugin_198,
#endif // ifdef USES_NW198

#ifdef USES_NW199
  &NWPlugin_199,
#endif // ifdef USES_NW199

#ifdef USES_NW200
  &NWPlugin_200,
#endif // ifdef USES_NW200

#ifdef USES_NW201
  &NWPlugin_201,
#endif // ifdef USES_NW201

#ifdef USES_NW202
  &NWPlugin_202,
#endif // ifdef USES_NW202

#ifdef USES_NW203
  &NWPlugin_203,
#endif // ifdef USES_NW203

#ifdef USES_NW204
  &NWPlugin_204,
#endif // ifdef USES_NW204

#ifdef USES_NW205
  &NWPlugin_205,
#endif // ifdef USES_NW205

#ifdef USES_NW206
  &NWPlugin_206,
#endif // ifdef USES_NW206

#ifdef USES_NW207
  &NWPlugin_207,
#endif // ifdef USES_NW207

#ifdef USES_NW208
  &NWPlugin_208,
#endif // ifdef USES_NW208

#ifdef USES_NW209
  &NWPlugin_209,
#endif // ifdef USES_NW209

#ifdef USES_NW210
  &NWPlugin_210,
#endif // ifdef USES_NW210

#ifdef USES_NW211
  &NWPlugin_211,
#endif // ifdef USES_NW211

#ifdef USES_NW212
  &NWPlugin_212,
#endif // ifdef USES_NW212

#ifdef USES_NW213
  &NWPlugin_213,
#endif // ifdef USES_NW213

#ifdef USES_NW214
  &NWPlugin_214,
#endif // ifdef USES_NW214

#ifdef USES_NW215
  &NWPlugin_215,
#endif // ifdef USES_NW215

#ifdef USES_NW216
  &NWPlugin_216,
#endif // ifdef USES_NW216

#ifdef USES_NW217
  &NWPlugin_217,
#endif // ifdef USES_NW217

#ifdef USES_NW218
  &NWPlugin_218,
#endif // ifdef USES_NW218

#ifdef USES_NW219
  &NWPlugin_219,
#endif // ifdef USES_NW219

#ifdef USES_NW220
  &NWPlugin_220,
#endif // ifdef USES_NW220

#ifdef USES_NW221
  &NWPlugin_221,
#endif // ifdef USES_NW221

#ifdef USES_NW222
  &NWPlugin_222,
#endif // ifdef USES_NW222

#ifdef USES_NW223
  &NWPlugin_223,
#endif // ifdef USES_NW223

#ifdef USES_NW224
  &NWPlugin_224,
#endif // ifdef USES_NW224

#ifdef USES_NW225
  &NWPlugin_225,
#endif // ifdef USES_NW225

#ifdef USES_NW226
  &NWPlugin_226,
#endif // ifdef USES_NW226

#ifdef USES_NW227
  &NWPlugin_227,
#endif // ifdef USES_NW227

#ifdef USES_NW228
  &NWPlugin_228,
#endif // ifdef USES_NW228

#ifdef USES_NW229
  &NWPlugin_229,
#endif // ifdef USES_NW229

#ifdef USES_NW230
  &NWPlugin_230,
#endif // ifdef USES_NW230

#ifdef USES_NW231
  &NWPlugin_231,
#endif // ifdef USES_NW231

#ifdef USES_NW232
  &NWPlugin_232,
#endif // ifdef USES_NW232

#ifdef USES_NW233
  &NWPlugin_233,
#endif // ifdef USES_NW233

#ifdef USES_NW234
  &NWPlugin_234,
#endif // ifdef USES_NW234

#ifdef USES_NW235
  &NWPlugin_235,
#endif // ifdef USES_NW235

#ifdef USES_NW236
  &NWPlugin_236,
#endif // ifdef USES_NW236

#ifdef USES_NW237
  &NWPlugin_237,
#endif // ifdef USES_NW237

#ifdef USES_NW238
  &NWPlugin_238,
#endif // ifdef USES_NW238

#ifdef USES_NW239
  &NWPlugin_239,
#endif // ifdef USES_NW239

#ifdef USES_NW240
  &NWPlugin_240,
#endif // ifdef USES_NW240

#ifdef USES_NW241
  &NWPlugin_241,
#endif // ifdef USES_NW241

#ifdef USES_NW242
  &NWPlugin_242,
#endif // ifdef USES_NW242

#ifdef USES_NW243
  &NWPlugin_243,
#endif // ifdef USES_NW243

#ifdef USES_NW244
  &NWPlugin_244,
#endif // ifdef USES_NW244

#ifdef USES_NW245
  &NWPlugin_245,
#endif // ifdef USES_NW245

#ifdef USES_NW246
  &NWPlugin_246,
#endif // ifdef USES_NW246

#ifdef USES_NW247
  &NWPlugin_247,
#endif // ifdef USES_NW247

#ifdef USES_NW248
  &NWPlugin_248,
#endif // ifdef USES_NW248

#ifdef USES_NW249
  &NWPlugin_249,
#endif // ifdef USES_NW249

#ifdef USES_NW250
  &NWPlugin_250,
#endif // ifdef USES_NW250

#ifdef USES_NW251
  &NWPlugin_251,
#endif // ifdef USES_NW251

#ifdef USES_NW252
  &NWPlugin_252,
#endif // ifdef USES_NW252

#ifdef USES_NW253
  &NWPlugin_253,
#endif // ifdef USES_NW253

#ifdef USES_NW254
  &NWPlugin_254,
#endif // ifdef USES_NW254

#ifdef USES_NW255
  &NWPlugin_255,
#endif // ifdef USES_NW255
};


constexpr size_t NetworkDriverIndex_to_NWPlugin_id_size = sizeof(NetworkDriverIndex_to_NWPlugin_id);

// Highest NWPlugin ID included in the build
constexpr size_t Highest_NWPlugin_id = NetworkDriverIndex_to_NWPlugin_id_size ==
                                       0 ? 0 : NetworkDriverIndex_to_NWPlugin_id[NetworkDriverIndex_to_NWPlugin_id_size - 1];

constexpr size_t NWPlugin_id_to_NetworkDriverIndex_size = Highest_NWPlugin_id + 1;

// Array filled during init.
// Valid index: 1 ... Highest_NWPlugin_id
// Returns index to the NetworkDriverIndex_to_NWPlugin_id array
networkDriverIndex_t NWPlugin_id_to_NetworkDriverIndex[NWPlugin_id_to_NetworkDriverIndex_size]{};


NetworkDriverStruct NetworkDriverArray[NetworkDriverIndex_to_NWPlugin_id_size + 1]{};

NetworkDriverStruct& getNetworkDriverStruct(networkDriverIndex_t networkDriverIndex)
{
  if (networkDriverIndex.value >= NetworkDriverIndex_to_NWPlugin_id_size) {
    networkDriverIndex = NetworkDriverIndex_to_NWPlugin_id_size;
  }
  return NetworkDriverArray[networkDriverIndex.value];
}

networkDriverIndex_t do_getNetworkDriverIndex_from_NWPluginID(nwpluginID_t nwpluginID)
{
  if (nwpluginID.value < NWPlugin_id_to_NetworkDriverIndex_size)
  {
    return static_cast<networkDriverIndex_t>(NWPlugin_id_to_NetworkDriverIndex[nwpluginID.value]);
  }
  return INVALID_NETWORKDRIVER_INDEX;
}

nwpluginID_t do_getNWPluginID_from_NetworkDriverIndex(networkDriverIndex_t networkDriverIndex)
{
  if (networkDriverIndex.value < NetworkDriverIndex_to_NWPlugin_id_size)
  {
    //    return static_cast<nwpluginID_t>(NetworkDriverIndex_to_NWPlugin_id[networkDriverIndex]);
    return nwpluginID_t::toPluginID(pgm_read_byte(NetworkDriverIndex_to_NWPlugin_id + networkDriverIndex.value));
  }
  return INVALID_NW_PLUGIN_ID;
}

bool do_check_validNetworkDriverIndex(networkDriverIndex_t networkDriverIndex)
{
  return networkDriverIndex.value < NetworkDriverIndex_to_NWPlugin_id_size;
}

nwpluginID_t getHighestIncludedNWPluginID() { return nwpluginID_t::toPluginID(Highest_NWPlugin_id); }

bool         do_NWPluginCall(networkDriverIndex_t networkDriverIndex, NWPlugin::Function Function, EventStruct *event, String& string)
{
  static uint32_t networkIndex_initialized{};

  if (networkDriverIndex.value < NetworkDriverIndex_to_NWPlugin_id_size)
  {
    if (Function == NWPlugin::Function::NWPLUGIN_INIT) {
      if (bitRead(networkIndex_initialized, event->NetworkIndex)) {
        // FIXME TD-er: What to do here? Was already initialized
        addLog(LOG_LEVEL_ERROR, strformat(F("Network %d was already initialized"), event->NetworkIndex + 1));
        return false;
      }
      bitSet(networkIndex_initialized, event->NetworkIndex);
    } else if (Function == NWPlugin::Function::NWPLUGIN_EXIT) {
      if (!bitRead(networkIndex_initialized, event->NetworkIndex)) {
        // FIXME TD-er: What to do here? Was not (yet) initialized
        //        addLog(LOG_LEVEL_ERROR, strformat(F("Network %d was not (yet) initialized"), event->NetworkIndex + 1));
        return false;
      }
      bitClear(networkIndex_initialized, event->NetworkIndex);
    }


    START_TIMER;
    NWPlugin_ptr_t nwplugin_call = (NWPlugin_ptr_t)pgm_read_ptr(NWPlugin_ptr + networkDriverIndex.value);
    const bool     res           = nwplugin_call(Function, event, string);
    STOP_TIMER_NETWORK(networkDriverIndex, Function);
    return res;
  }
  return false;
}

void NWPluginSetup()
{
  static bool setupDone = false;

  if (setupDone) { return; }

  for (size_t id = 0; id < NWPlugin_id_to_NetworkDriverIndex_size; ++id)
  {
    NWPlugin_id_to_NetworkDriverIndex[id] = INVALID_NETWORKDRIVER_INDEX;
  }

  networkDriverIndex_t networkDriverIndex{};

  for (; networkDriverIndex.value < NetworkDriverIndex_to_NWPlugin_id_size; ++networkDriverIndex)
  {
    const nwpluginID_t nwpluginID = getNWPluginID_from_NetworkDriverIndex(networkDriverIndex);

    if (nwpluginID) {
      NWPlugin_id_to_NetworkDriverIndex[nwpluginID.value] = networkDriverIndex;
      EventStruct TempEvent;
      TempEvent.idx = networkDriverIndex.value;
      String dummy;
      do_NWPluginCall(networkDriverIndex, NWPlugin::Function::NWPLUGIN_DRIVER_ADD, &TempEvent, dummy);
    }
  }
  setupDone = true;
}

void NWPluginInit()
{
  // Set all not supported nwplugins to disabled.
  for (ESPEasy::net::networkIndex_t network = 0; network < NETWORK_MAX; ++network) {
    if (!supportedNWPluginID(Settings.getNWPluginID_for_network(network))) {
      Settings.setNetworkEnabled(network, false);
    }
  }
  NWPluginCall(NWPlugin::Function::NWPLUGIN_INIT_ALL, 0);
}

void NWPlugin_Exit_Init(networkIndex_t networkIndex)
{
  const networkDriverIndex_t NetworkDriverIndex =
    getNetworkDriverIndex_from_NetworkIndex(networkIndex);

  if (validNetworkDriverIndex(NetworkDriverIndex)) {
    struct EventStruct TempEvent;
    TempEvent.NetworkIndex = networkIndex;
    String dummy;

    // May need to call init later, so make sure exit is called first
    NWPluginCall(NWPlugin::Function::NWPLUGIN_EXIT, &TempEvent, dummy);

    if (Settings.getNetworkEnabled(networkIndex)) {
      NWPluginCall(NWPlugin::Function::NWPLUGIN_INIT, &TempEvent, dummy);
    }
  }
}

} // namespace net
} // namespace ESPEasy

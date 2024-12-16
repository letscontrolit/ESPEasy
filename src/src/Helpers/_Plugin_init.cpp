#include "../Helpers/_Plugin_init.h"

#include "../../ESPEasy_common.h"

#include "../Globals/Device.h"
#include "../Globals/Settings.h"

#include "../Helpers/Misc.h"


// ********************************************************************************
// Initialize all plugins that where defined earlier
// and initialize the function call pointer into the plugin array
// ********************************************************************************



// Vector to match a "DeviceIndex" to a plugin ID.
constexpr /*pluginID_t*/ uint8_t DeviceIndex_to_Plugin_id[] PROGMEM =
{
#ifdef USES_P001
  1,
#endif // ifdef USES_P001

#ifdef USES_P002
  2,
#endif // ifdef USES_P002

#ifdef USES_P003
  3,
#endif // ifdef USES_P003

#ifdef USES_P004
  4,
#endif // ifdef USES_P004

#ifdef USES_P005
  5,
#endif // ifdef USES_P005

#ifdef USES_P006
  6,
#endif // ifdef USES_P006

#ifdef USES_P007
  7,
#endif // ifdef USES_P007

#ifdef USES_P008
  8,
#endif // ifdef USES_P008

#ifdef USES_P009
  9,
#endif // ifdef USES_P009

#ifdef USES_P010
  10,
#endif // ifdef USES_P010

#ifdef USES_P011
  11,
#endif // ifdef USES_P011

#ifdef USES_P012
  12,
#endif // ifdef USES_P012

#ifdef USES_P013
  13,
#endif // ifdef USES_P013

#ifdef USES_P014
  14,
#endif // ifdef USES_P014

#ifdef USES_P015
  15,
#endif // ifdef USES_P015

#ifdef USES_P016
  16,
#endif // ifdef USES_P016

#ifdef USES_P017
  17,
#endif // ifdef USES_P017

#ifdef USES_P018
  18,
#endif // ifdef USES_P018

#ifdef USES_P019
  19,
#endif // ifdef USES_P019

#ifdef USES_P020
  20,
#endif // ifdef USES_P020

#ifdef USES_P021
  21,
#endif // ifdef USES_P021

#ifdef USES_P022
  22,
#endif // ifdef USES_P022

#ifdef USES_P023
  23,
#endif // ifdef USES_P023

#ifdef USES_P024
  24,
#endif // ifdef USES_P024

#ifdef USES_P025
  25,
#endif // ifdef USES_P025

#ifdef USES_P026
  26,
#endif // ifdef USES_P026

#ifdef USES_P027
  27,
#endif // ifdef USES_P027

#ifdef USES_P028
  28,
#endif // ifdef USES_P028

#ifdef USES_P029
  29,
#endif // ifdef USES_P029

#ifdef USES_P030
  30,
#endif // ifdef USES_P030

#ifdef USES_P031
  31,
#endif // ifdef USES_P031

#ifdef USES_P032
  32,
#endif // ifdef USES_P032

#ifdef USES_P033
  33,
#endif // ifdef USES_P033

#ifdef USES_P034
  34,
#endif // ifdef USES_P034

#ifdef USES_P035
  35,
#endif // ifdef USES_P035

#ifdef USES_P036
  36,
#endif // ifdef USES_P036

#ifdef USES_P037
  37,
#endif // ifdef USES_P037

#ifdef USES_P038
  38,
#endif // ifdef USES_P038

#ifdef USES_P039
  39,
#endif // ifdef USES_P039

#ifdef USES_P040
  40,
#endif // ifdef USES_P040

#ifdef USES_P041
  41,
#endif // ifdef USES_P041

#ifdef USES_P042
  42,
#endif // ifdef USES_P042

#ifdef USES_P043
  43,
#endif // ifdef USES_P043

#ifdef USES_P044
  44,
#endif // ifdef USES_P044

#ifdef USES_P045
  45,
#endif // ifdef USES_P045

#ifdef USES_P046
  46,
#endif // ifdef USES_P046

#ifdef USES_P047
  47,
#endif // ifdef USES_P047

#ifdef USES_P048
  48,
#endif // ifdef USES_P048

#ifdef USES_P049
  49,
#endif // ifdef USES_P049

#ifdef USES_P050
  50,
#endif // ifdef USES_P050

#ifdef USES_P051
  51,
#endif // ifdef USES_P051

#ifdef USES_P052
  52,
#endif // ifdef USES_P052

#ifdef USES_P053
  53,
#endif // ifdef USES_P053

#ifdef USES_P054
  54,
#endif // ifdef USES_P054

#ifdef USES_P055
  55,
#endif // ifdef USES_P055

#ifdef USES_P056
  56,
#endif // ifdef USES_P056

#ifdef USES_P057
  57,
#endif // ifdef USES_P057

#ifdef USES_P058
  58,
#endif // ifdef USES_P058

#ifdef USES_P059
  59,
#endif // ifdef USES_P059

#ifdef USES_P060
  60,
#endif // ifdef USES_P060

#ifdef USES_P061
  61,
#endif // ifdef USES_P061

#ifdef USES_P062
  62,
#endif // ifdef USES_P062

#ifdef USES_P063
  63,
#endif // ifdef USES_P063

#ifdef USES_P064
  64,
#endif // ifdef USES_P064

#ifdef USES_P065
  65,
#endif // ifdef USES_P065

#ifdef USES_P066
  66,
#endif // ifdef USES_P066

#ifdef USES_P067
  67,
#endif // ifdef USES_P067

#ifdef USES_P068
  68,
#endif // ifdef USES_P068

#ifdef USES_P069
  69,
#endif // ifdef USES_P069

#ifdef USES_P070
  70,
#endif // ifdef USES_P070

#ifdef USES_P071
  71,
#endif // ifdef USES_P071

#ifdef USES_P072
  72,
#endif // ifdef USES_P072

#ifdef USES_P073
  73,
#endif // ifdef USES_P073

#ifdef USES_P074
  74,
#endif // ifdef USES_P074

#ifdef USES_P075
  75,
#endif // ifdef USES_P075

#ifdef USES_P076
  76,
#endif // ifdef USES_P076

#ifdef USES_P077
  77,
#endif // ifdef USES_P077

#ifdef USES_P078
  78,
#endif // ifdef USES_P078

#ifdef USES_P079
  79,
#endif // ifdef USES_P079

#ifdef USES_P080
  80,
#endif // ifdef USES_P080

#ifdef USES_P081
  81,
#endif // ifdef USES_P081

#ifdef USES_P082
  82,
#endif // ifdef USES_P082

#ifdef USES_P083
  83,
#endif // ifdef USES_P083

#ifdef USES_P084
  84,
#endif // ifdef USES_P084

#ifdef USES_P085
  85,
#endif // ifdef USES_P085

#ifdef USES_P086
  86,
#endif // ifdef USES_P086

#ifdef USES_P087
  87,
#endif // ifdef USES_P087

#ifdef USES_P088
  88,
#endif // ifdef USES_P088

#ifdef USES_P089
  # ifdef ESP8266

  // FIXME TD-er: Support Ping plugin for ESP32
  89,
  # endif // ifdef ESP8266
#endif // ifdef USES_P089

#ifdef USES_P090
  90,
#endif // ifdef USES_P090

#ifdef USES_P091
  91,
#endif // ifdef USES_P091

#ifdef USES_P092
  92,
#endif // ifdef USES_P092

#ifdef USES_P093
  93,
#endif // ifdef USES_P093

#ifdef USES_P094
  94,
#endif // ifdef USES_P094

#ifdef USES_P095
  95,
#endif // ifdef USES_P095

#ifdef USES_P096
  96,
#endif // ifdef USES_P096

#ifdef USES_P097
  # if defined(ESP32) && !defined(ESP32C2) && !defined(ESP32C3) && !defined(ESP32C6)

  // Touch (ESP32)
  97,
  # endif // if defined(ESP32) && !defined(ESP32Cxx)
#endif // ifdef USES_P097

#ifdef USES_P098
  98,
#endif // ifdef USES_P098

#ifdef USES_P099
  99,
#endif // ifdef USES_P099

#ifdef USES_P100
  100,
#endif // ifdef USES_P100

#ifdef USES_P101
  101,
#endif // ifdef USES_P101

#ifdef USES_P102
  102,
#endif // ifdef USES_P102

#ifdef USES_P103
  103,
#endif // ifdef USES_P103

#ifdef USES_P104
  104,
#endif // ifdef USES_P104

#ifdef USES_P105
  105,
#endif // ifdef USES_P105

#ifdef USES_P106
  106,
#endif // ifdef USES_P106

#ifdef USES_P107
  107,
#endif // ifdef USES_P107

#ifdef USES_P108
  108,
#endif // ifdef USES_P108

#ifdef USES_P109
  109,
#endif // ifdef USES_P109

#ifdef USES_P110
  110,
#endif // ifdef USES_P110

#ifdef USES_P111
  111,
#endif // ifdef USES_P111

#ifdef USES_P112
  112,
#endif // ifdef USES_P112

#ifdef USES_P113
  113,
#endif // ifdef USES_P113

#ifdef USES_P114
  114,
#endif // ifdef USES_P114

#ifdef USES_P115
  115,
#endif // ifdef USES_P115

#ifdef USES_P116
  116,
#endif // ifdef USES_P116

#ifdef USES_P117
  117,
#endif // ifdef USES_P117

#ifdef USES_P118
  118,
#endif // ifdef USES_P118

#ifdef USES_P119
  119,
#endif // ifdef USES_P119

#ifdef USES_P120
  120,
#endif // ifdef USES_P120

#ifdef USES_P121
  121,
#endif // ifdef USES_P121

#ifdef USES_P122
  122,
#endif // ifdef USES_P122

#ifdef USES_P123
  123,
#endif // ifdef USES_P123

#ifdef USES_P124
  124,
#endif // ifdef USES_P124

#ifdef USES_P125
  125,
#endif // ifdef USES_P125

#ifdef USES_P126
  126,
#endif // ifdef USES_P126

#ifdef USES_P127
  127,
#endif // ifdef USES_P127

#ifdef USES_P128
  128,
#endif // ifdef USES_P128

#ifdef USES_P129
  129,
#endif // ifdef USES_P129

#ifdef USES_P130
  130,
#endif // ifdef USES_P130

#ifdef USES_P131
  131,
#endif // ifdef USES_P131

#ifdef USES_P132
  132,
#endif // ifdef USES_P132

#ifdef USES_P133
  133,
#endif // ifdef USES_P133

#ifdef USES_P134
  134,
#endif // ifdef USES_P134

#ifdef USES_P135
  135,
#endif // ifdef USES_P135

#ifdef USES_P136
  136,
#endif // ifdef USES_P136

#ifdef USES_P137
  137,
#endif // ifdef USES_P137

#ifdef USES_P138
  138,
#endif // ifdef USES_P138

#ifdef USES_P139
  139,
#endif // ifdef USES_P139

#ifdef USES_P140
  140,
#endif // ifdef USES_P140

#ifdef USES_P141
  141,
#endif // ifdef USES_P141

#ifdef USES_P142
  142,
#endif // ifdef USES_P142

#ifdef USES_P143
  143,
#endif // ifdef USES_P143

#ifdef USES_P144
  144,
#endif // ifdef USES_P144

#ifdef USES_P145
  145,
#endif // ifdef USES_P145

#ifdef USES_P146
  146,
#endif // ifdef USES_P146

#ifdef USES_P147
  147,
#endif // ifdef USES_P147

#ifdef USES_P148
  148,
#endif // ifdef USES_P148

#ifdef USES_P149
  149,
#endif // ifdef USES_P149

#ifdef USES_P150
  150,
#endif // ifdef USES_P150

#ifdef USES_P151
  151,
#endif // ifdef USES_P151

#ifdef USES_P152
  152,
#endif // ifdef USES_P152

#ifdef USES_P153
  153,
#endif // ifdef USES_P153

#ifdef USES_P154
  154,
#endif // ifdef USES_P154

#ifdef USES_P155
  155,
#endif // ifdef USES_P155

#ifdef USES_P156
  156,
#endif // ifdef USES_P156

#ifdef USES_P157
  157,
#endif // ifdef USES_P157

#ifdef USES_P158
  158,
#endif // ifdef USES_P158

#ifdef USES_P159
  159,
#endif // ifdef USES_P159

#ifdef USES_P160
  160,
#endif // ifdef USES_P160

#ifdef USES_P161
  161,
#endif // ifdef USES_P161

#ifdef USES_P162
  162,
#endif // ifdef USES_P162

#ifdef USES_P163
  163,
#endif // ifdef USES_P163

#ifdef USES_P164
  164,
#endif // ifdef USES_P164

#ifdef USES_P165
  165,
#endif // ifdef USES_P165

#ifdef USES_P166
  166,
#endif // ifdef USES_P166

#ifdef USES_P167
  167,
#endif // ifdef USES_P167

#ifdef USES_P168
  168,
#endif // ifdef USES_P168

#ifdef USES_P169
  169,
#endif // ifdef USES_P169

#ifdef USES_P170
  170,
#endif // ifdef USES_P170

#ifdef USES_P171
  171,
#endif // ifdef USES_P171

#ifdef USES_P172
  172,
#endif // ifdef USES_P172

#ifdef USES_P173
  173,
#endif // ifdef USES_P173

#ifdef USES_P174
  174,
#endif // ifdef USES_P174

#ifdef USES_P175
  175,
#endif // ifdef USES_P175

#ifdef USES_P176
  176,
#endif // ifdef USES_P176

#ifdef USES_P177
  177,
#endif // ifdef USES_P177

#ifdef USES_P178
  178,
#endif // ifdef USES_P178

#ifdef USES_P179
  179,
#endif // ifdef USES_P179

#ifdef USES_P180
  180,
#endif // ifdef USES_P180

#ifdef USES_P181
  181,
#endif // ifdef USES_P181

#ifdef USES_P182
  182,
#endif // ifdef USES_P182

#ifdef USES_P183
  183,
#endif // ifdef USES_P183

#ifdef USES_P184
  184,
#endif // ifdef USES_P184

#ifdef USES_P185
  185,
#endif // ifdef USES_P185

#ifdef USES_P186
  186,
#endif // ifdef USES_P186

#ifdef USES_P187
  187,
#endif // ifdef USES_P187

#ifdef USES_P188
  188,
#endif // ifdef USES_P188

#ifdef USES_P189
  189,
#endif // ifdef USES_P189

#ifdef USES_P190
  190,
#endif // ifdef USES_P190

#ifdef USES_P191
  191,
#endif // ifdef USES_P191

#ifdef USES_P192
  192,
#endif // ifdef USES_P192

#ifdef USES_P193
  193,
#endif // ifdef USES_P193

#ifdef USES_P194
  194,
#endif // ifdef USES_P194

#ifdef USES_P195
  195,
#endif // ifdef USES_P195

#ifdef USES_P196
  196,
#endif // ifdef USES_P196

#ifdef USES_P197
  197,
#endif // ifdef USES_P197

#ifdef USES_P198
  198,
#endif // ifdef USES_P198

#ifdef USES_P199
  199,
#endif // ifdef USES_P199

#ifdef USES_P200
  200,
#endif // ifdef USES_P200

#ifdef USES_P201
  201,
#endif // ifdef USES_P201

#ifdef USES_P202
  202,
#endif // ifdef USES_P202

#ifdef USES_P203
  203,
#endif // ifdef USES_P203

#ifdef USES_P204
  204,
#endif // ifdef USES_P204

#ifdef USES_P205
  205,
#endif // ifdef USES_P205

#ifdef USES_P206
  206,
#endif // ifdef USES_P206

#ifdef USES_P207
  207,
#endif // ifdef USES_P207

#ifdef USES_P208
  208,
#endif // ifdef USES_P208

#ifdef USES_P209
  209,
#endif // ifdef USES_P209

#ifdef USES_P210
  210,
#endif // ifdef USES_P210

#ifdef USES_P211
  211,
#endif // ifdef USES_P211

#ifdef USES_P212
  212,
#endif // ifdef USES_P212

#ifdef USES_P213
  213,
#endif // ifdef USES_P213

#ifdef USES_P214
  214,
#endif // ifdef USES_P214

#ifdef USES_P215
  215,
#endif // ifdef USES_P215

#ifdef USES_P216
  216,
#endif // ifdef USES_P216

#ifdef USES_P217
  217,
#endif // ifdef USES_P217

#ifdef USES_P218
  218,
#endif // ifdef USES_P218

#ifdef USES_P219
  219,
#endif // ifdef USES_P219

#ifdef USES_P220
  220,
#endif // ifdef USES_P220

#ifdef USES_P221
  221,
#endif // ifdef USES_P221

#ifdef USES_P222
  222,
#endif // ifdef USES_P222

#ifdef USES_P223
  223,
#endif // ifdef USES_P223

#ifdef USES_P224
  224,
#endif // ifdef USES_P224

#ifdef USES_P225
  225,
#endif // ifdef USES_P225

#ifdef USES_P226
  226,
#endif // ifdef USES_P226

#ifdef USES_P227
  227,
#endif // ifdef USES_P227

#ifdef USES_P228
  228,
#endif // ifdef USES_P228

#ifdef USES_P229
  229,
#endif // ifdef USES_P229

#ifdef USES_P230
  230,
#endif // ifdef USES_P230

#ifdef USES_P231
  231,
#endif // ifdef USES_P231

#ifdef USES_P232
  232,
#endif // ifdef USES_P232

#ifdef USES_P233
  233,
#endif // ifdef USES_P233

#ifdef USES_P234
  234,
#endif // ifdef USES_P234

#ifdef USES_P235
  235,
#endif // ifdef USES_P235

#ifdef USES_P236
  236,
#endif // ifdef USES_P236

#ifdef USES_P237
  237,
#endif // ifdef USES_P237

#ifdef USES_P238
  238,
#endif // ifdef USES_P238

#ifdef USES_P239
  239,
#endif // ifdef USES_P239

#ifdef USES_P240
  240,
#endif // ifdef USES_P240

#ifdef USES_P241
  241,
#endif // ifdef USES_P241

#ifdef USES_P242
  242,
#endif // ifdef USES_P242

#ifdef USES_P243
  243,
#endif // ifdef USES_P243

#ifdef USES_P244
  244,
#endif // ifdef USES_P244

#ifdef USES_P245
  245,
#endif // ifdef USES_P245

#ifdef USES_P246
  246,
#endif // ifdef USES_P246

#ifdef USES_P247
  247,
#endif // ifdef USES_P247

#ifdef USES_P248
  248,
#endif // ifdef USES_P248

#ifdef USES_P249
  249,
#endif // ifdef USES_P249

#ifdef USES_P250
  250,
#endif // ifdef USES_P250

#ifdef USES_P251
  251,
#endif // ifdef USES_P251

#ifdef USES_P252
  252,
#endif // ifdef USES_P252

#ifdef USES_P253
  253,
#endif // ifdef USES_P253

#ifdef USES_P254
  254,
#endif // ifdef USES_P254

#ifdef USES_P255
  255,
#endif // ifdef USES_P255
};

typedef boolean (*Plugin_ptr_t)(uint8_t,
                        struct EventStruct *,
                        String&);

// Array of function pointers to call plugins.
constexpr const Plugin_ptr_t PROGMEM Plugin_ptr[] =
{
#ifdef USES_P001
  &Plugin_001,
#endif // ifdef USES_P001

#ifdef USES_P002
  &Plugin_002,
#endif // ifdef USES_P002

#ifdef USES_P003
  &Plugin_003,
#endif // ifdef USES_P003

#ifdef USES_P004
  &Plugin_004,
#endif // ifdef USES_P004

#ifdef USES_P005
  &Plugin_005,
#endif // ifdef USES_P005

#ifdef USES_P006
  &Plugin_006,
#endif // ifdef USES_P006

#ifdef USES_P007
  &Plugin_007,
#endif // ifdef USES_P007

#ifdef USES_P008
  &Plugin_008,
#endif // ifdef USES_P008

#ifdef USES_P009
  &Plugin_009,
#endif // ifdef USES_P009

#ifdef USES_P010
  &Plugin_010,
#endif // ifdef USES_P010

#ifdef USES_P011
  &Plugin_011,
#endif // ifdef USES_P011

#ifdef USES_P012
  &Plugin_012,
#endif // ifdef USES_P012

#ifdef USES_P013
  &Plugin_013,
#endif // ifdef USES_P013

#ifdef USES_P014
  &Plugin_014,
#endif // ifdef USES_P014

#ifdef USES_P015
  &Plugin_015,
#endif // ifdef USES_P015

#ifdef USES_P016
  &Plugin_016,
#endif // ifdef USES_P016

#ifdef USES_P017
  &Plugin_017,
#endif // ifdef USES_P017

#ifdef USES_P018
  &Plugin_018,
#endif // ifdef USES_P018

#ifdef USES_P019
  &Plugin_019,
#endif // ifdef USES_P019

#ifdef USES_P020
  &Plugin_020,
#endif // ifdef USES_P020

#ifdef USES_P021
  &Plugin_021,
#endif // ifdef USES_P021

#ifdef USES_P022
  &Plugin_022,
#endif // ifdef USES_P022

#ifdef USES_P023
  &Plugin_023,
#endif // ifdef USES_P023

#ifdef USES_P024
  &Plugin_024,
#endif // ifdef USES_P024

#ifdef USES_P025
  &Plugin_025,
#endif // ifdef USES_P025

#ifdef USES_P026
  &Plugin_026,
#endif // ifdef USES_P026

#ifdef USES_P027
  &Plugin_027,
#endif // ifdef USES_P027

#ifdef USES_P028
  &Plugin_028,
#endif // ifdef USES_P028

#ifdef USES_P029
  &Plugin_029,
#endif // ifdef USES_P029

#ifdef USES_P030
  &Plugin_030,
#endif // ifdef USES_P030

#ifdef USES_P031
  &Plugin_031,
#endif // ifdef USES_P031

#ifdef USES_P032
  &Plugin_032,
#endif // ifdef USES_P032

#ifdef USES_P033
  &Plugin_033,
#endif // ifdef USES_P033

#ifdef USES_P034
  &Plugin_034,
#endif // ifdef USES_P034

#ifdef USES_P035
  &Plugin_035,
#endif // ifdef USES_P035

#ifdef USES_P036
  &Plugin_036,
#endif // ifdef USES_P036

#ifdef USES_P037
  &Plugin_037,
#endif // ifdef USES_P037

#ifdef USES_P038
  &Plugin_038,
#endif // ifdef USES_P038

#ifdef USES_P039
  &Plugin_039,
#endif // ifdef USES_P039

#ifdef USES_P040
  &Plugin_040,
#endif // ifdef USES_P040

#ifdef USES_P041
  &Plugin_041,
#endif // ifdef USES_P041

#ifdef USES_P042
  &Plugin_042,
#endif // ifdef USES_P042

#ifdef USES_P043
  &Plugin_043,
#endif // ifdef USES_P043

#ifdef USES_P044
  &Plugin_044,
#endif // ifdef USES_P044

#ifdef USES_P045
  &Plugin_045,
#endif // ifdef USES_P045

#ifdef USES_P046
  &Plugin_046,
#endif // ifdef USES_P046

#ifdef USES_P047
  &Plugin_047,
#endif // ifdef USES_P047

#ifdef USES_P048
  &Plugin_048,
#endif // ifdef USES_P048

#ifdef USES_P049
  &Plugin_049,
#endif // ifdef USES_P049

#ifdef USES_P050
  &Plugin_050,
#endif // ifdef USES_P050

#ifdef USES_P051
  &Plugin_051,
#endif // ifdef USES_P051

#ifdef USES_P052
  &Plugin_052,
#endif // ifdef USES_P052

#ifdef USES_P053
  &Plugin_053,
#endif // ifdef USES_P053

#ifdef USES_P054
  &Plugin_054,
#endif // ifdef USES_P054

#ifdef USES_P055
  &Plugin_055,
#endif // ifdef USES_P055

#ifdef USES_P056
  &Plugin_056,
#endif // ifdef USES_P056

#ifdef USES_P057
  &Plugin_057,
#endif // ifdef USES_P057

#ifdef USES_P058
  &Plugin_058,
#endif // ifdef USES_P058

#ifdef USES_P059
  &Plugin_059,
#endif // ifdef USES_P059

#ifdef USES_P060
  &Plugin_060,
#endif // ifdef USES_P060

#ifdef USES_P061
  &Plugin_061,
#endif // ifdef USES_P061

#ifdef USES_P062
  &Plugin_062,
#endif // ifdef USES_P062

#ifdef USES_P063
  &Plugin_063,
#endif // ifdef USES_P063

#ifdef USES_P064
  &Plugin_064,
#endif // ifdef USES_P064

#ifdef USES_P065
  &Plugin_065,
#endif // ifdef USES_P065

#ifdef USES_P066
  &Plugin_066,
#endif // ifdef USES_P066

#ifdef USES_P067
  &Plugin_067,
#endif // ifdef USES_P067

#ifdef USES_P068
  &Plugin_068,
#endif // ifdef USES_P068

#ifdef USES_P069
  &Plugin_069,
#endif // ifdef USES_P069

#ifdef USES_P070
  &Plugin_070,
#endif // ifdef USES_P070

#ifdef USES_P071
  &Plugin_071,
#endif // ifdef USES_P071

#ifdef USES_P072
  &Plugin_072,
#endif // ifdef USES_P072

#ifdef USES_P073
  &Plugin_073,
#endif // ifdef USES_P073

#ifdef USES_P074
  &Plugin_074,
#endif // ifdef USES_P074

#ifdef USES_P075
  &Plugin_075,
#endif // ifdef USES_P075

#ifdef USES_P076
  &Plugin_076,
#endif // ifdef USES_P076

#ifdef USES_P077
  &Plugin_077,
#endif // ifdef USES_P077

#ifdef USES_P078
  &Plugin_078,
#endif // ifdef USES_P078

#ifdef USES_P079
  &Plugin_079,
#endif // ifdef USES_P079

#ifdef USES_P080
  &Plugin_080,
#endif // ifdef USES_P080

#ifdef USES_P081
  &Plugin_081,
#endif // ifdef USES_P081

#ifdef USES_P082
  &Plugin_082,
#endif // ifdef USES_P082

#ifdef USES_P083
  &Plugin_083,
#endif // ifdef USES_P083

#ifdef USES_P084
  &Plugin_084,
#endif // ifdef USES_P084

#ifdef USES_P085
  &Plugin_085,
#endif // ifdef USES_P085

#ifdef USES_P086
  &Plugin_086,
#endif // ifdef USES_P086

#ifdef USES_P087
  &Plugin_087,
#endif // ifdef USES_P087

#ifdef USES_P088
  &Plugin_088,
#endif // ifdef USES_P088

#ifdef USES_P089
  # ifdef ESP8266

  // FIXME TD-er: Support Ping plugin for ESP32
  &Plugin_089,
  # endif // ifdef ESP8266
#endif // ifdef USES_P089

#ifdef USES_P090
  &Plugin_090,
#endif // ifdef USES_P090

#ifdef USES_P091
  &Plugin_091,
#endif // ifdef USES_P091

#ifdef USES_P092
  &Plugin_092,
#endif // ifdef USES_P092

#ifdef USES_P093
  &Plugin_093,
#endif // ifdef USES_P093

#ifdef USES_P094
  &Plugin_094,
#endif // ifdef USES_P094

#ifdef USES_P095
  &Plugin_095,
#endif // ifdef USES_P095

#ifdef USES_P096
  &Plugin_096,
#endif // ifdef USES_P096

#ifdef USES_P097
  # if defined(ESP32) && !defined(ESP32C2) && !defined(ESP32C3) && !defined(ESP32C6)

  // Touch (ESP32)
  &Plugin_097,
  # endif // if defined(ESP32) && !defined(ESP32Cxx)
#endif // ifdef USES_P097

#ifdef USES_P098
  &Plugin_098,
#endif // ifdef USES_P098

#ifdef USES_P099
  &Plugin_099,
#endif // ifdef USES_P099

#ifdef USES_P100
  &Plugin_100,
#endif // ifdef USES_P100

#ifdef USES_P101
  &Plugin_101,
#endif // ifdef USES_P101

#ifdef USES_P102
  &Plugin_102,
#endif // ifdef USES_P102

#ifdef USES_P103
  &Plugin_103,
#endif // ifdef USES_P103

#ifdef USES_P104
  &Plugin_104,
#endif // ifdef USES_P104

#ifdef USES_P105
  &Plugin_105,
#endif // ifdef USES_P105

#ifdef USES_P106
  &Plugin_106,
#endif // ifdef USES_P106

#ifdef USES_P107
  &Plugin_107,
#endif // ifdef USES_P107

#ifdef USES_P108
  &Plugin_108,
#endif // ifdef USES_P108

#ifdef USES_P109
  &Plugin_109,
#endif // ifdef USES_P109

#ifdef USES_P110
  &Plugin_110,
#endif // ifdef USES_P110

#ifdef USES_P111
  &Plugin_111,
#endif // ifdef USES_P111

#ifdef USES_P112
  &Plugin_112,
#endif // ifdef USES_P112

#ifdef USES_P113
  &Plugin_113,
#endif // ifdef USES_P113

#ifdef USES_P114
  &Plugin_114,
#endif // ifdef USES_P114

#ifdef USES_P115
  &Plugin_115,
#endif // ifdef USES_P115

#ifdef USES_P116
  &Plugin_116,
#endif // ifdef USES_P116

#ifdef USES_P117
  &Plugin_117,
#endif // ifdef USES_P117

#ifdef USES_P118
  &Plugin_118,
#endif // ifdef USES_P118

#ifdef USES_P119
  &Plugin_119,
#endif // ifdef USES_P119

#ifdef USES_P120
  &Plugin_120,
#endif // ifdef USES_P120

#ifdef USES_P121
  &Plugin_121,
#endif // ifdef USES_P121

#ifdef USES_P122
  &Plugin_122,
#endif // ifdef USES_P122

#ifdef USES_P123
  &Plugin_123,
#endif // ifdef USES_P123

#ifdef USES_P124
  &Plugin_124,
#endif // ifdef USES_P124

#ifdef USES_P125
  &Plugin_125,
#endif // ifdef USES_P125

#ifdef USES_P126
  &Plugin_126,
#endif // ifdef USES_P126

#ifdef USES_P127
  &Plugin_127,
#endif // ifdef USES_P127

#ifdef USES_P128
  &Plugin_128,
#endif // ifdef USES_P128

#ifdef USES_P129
  &Plugin_129,
#endif // ifdef USES_P129

#ifdef USES_P130
  &Plugin_130,
#endif // ifdef USES_P130

#ifdef USES_P131
  &Plugin_131,
#endif // ifdef USES_P131

#ifdef USES_P132
  &Plugin_132,
#endif // ifdef USES_P132

#ifdef USES_P133
  &Plugin_133,
#endif // ifdef USES_P133

#ifdef USES_P134
  &Plugin_134,
#endif // ifdef USES_P134

#ifdef USES_P135
  &Plugin_135,
#endif // ifdef USES_P135

#ifdef USES_P136
  &Plugin_136,
#endif // ifdef USES_P136

#ifdef USES_P137
  &Plugin_137,
#endif // ifdef USES_P137

#ifdef USES_P138
  &Plugin_138,
#endif // ifdef USES_P138

#ifdef USES_P139
  &Plugin_139,
#endif // ifdef USES_P139

#ifdef USES_P140
  &Plugin_140,
#endif // ifdef USES_P140

#ifdef USES_P141
  &Plugin_141,
#endif // ifdef USES_P141

#ifdef USES_P142
  &Plugin_142,
#endif // ifdef USES_P142

#ifdef USES_P143
  &Plugin_143,
#endif // ifdef USES_P143

#ifdef USES_P144
  &Plugin_144,
#endif // ifdef USES_P144

#ifdef USES_P145
  &Plugin_145,
#endif // ifdef USES_P145

#ifdef USES_P146
  &Plugin_146,
#endif // ifdef USES_P146

#ifdef USES_P147
  &Plugin_147,
#endif // ifdef USES_P147

#ifdef USES_P148
  &Plugin_148,
#endif // ifdef USES_P148

#ifdef USES_P149
  &Plugin_149,
#endif // ifdef USES_P149

#ifdef USES_P150
  &Plugin_150,
#endif // ifdef USES_P150

#ifdef USES_P151
  &Plugin_151,
#endif // ifdef USES_P151

#ifdef USES_P152
  &Plugin_152,
#endif // ifdef USES_P152

#ifdef USES_P153
  &Plugin_153,
#endif // ifdef USES_P153

#ifdef USES_P154
  &Plugin_154,
#endif // ifdef USES_P154

#ifdef USES_P155
  &Plugin_155,
#endif // ifdef USES_P155

#ifdef USES_P156
  &Plugin_156,
#endif // ifdef USES_P156

#ifdef USES_P157
  &Plugin_157,
#endif // ifdef USES_P157

#ifdef USES_P158
  &Plugin_158,
#endif // ifdef USES_P158

#ifdef USES_P159
  &Plugin_159,
#endif // ifdef USES_P159

#ifdef USES_P160
  &Plugin_160,
#endif // ifdef USES_P160

#ifdef USES_P161
  &Plugin_161,
#endif // ifdef USES_P161

#ifdef USES_P162
  &Plugin_162,
#endif // ifdef USES_P162

#ifdef USES_P163
  &Plugin_163,
#endif // ifdef USES_P163

#ifdef USES_P164
  &Plugin_164,
#endif // ifdef USES_P164

#ifdef USES_P165
  &Plugin_165,
#endif // ifdef USES_P165

#ifdef USES_P166
  &Plugin_166,
#endif // ifdef USES_P166

#ifdef USES_P167
  &Plugin_167,
#endif // ifdef USES_P167

#ifdef USES_P168
  &Plugin_168,
#endif // ifdef USES_P168

#ifdef USES_P169
  &Plugin_169,
#endif // ifdef USES_P169

#ifdef USES_P170
  &Plugin_170,
#endif // ifdef USES_P170

#ifdef USES_P171
  &Plugin_171,
#endif // ifdef USES_P171

#ifdef USES_P172
  &Plugin_172,
#endif // ifdef USES_P172

#ifdef USES_P173
  &Plugin_173,
#endif // ifdef USES_P173

#ifdef USES_P174
  &Plugin_174,
#endif // ifdef USES_P174

#ifdef USES_P175
  &Plugin_175,
#endif // ifdef USES_P175

#ifdef USES_P176
  &Plugin_176,
#endif // ifdef USES_P176

#ifdef USES_P177
  &Plugin_177,
#endif // ifdef USES_P177

#ifdef USES_P178
  &Plugin_178,
#endif // ifdef USES_P178

#ifdef USES_P179
  &Plugin_179,
#endif // ifdef USES_P179

#ifdef USES_P180
  &Plugin_180,
#endif // ifdef USES_P180

#ifdef USES_P181
  &Plugin_181,
#endif // ifdef USES_P181

#ifdef USES_P182
  &Plugin_182,
#endif // ifdef USES_P182

#ifdef USES_P183
  &Plugin_183,
#endif // ifdef USES_P183

#ifdef USES_P184
  &Plugin_184,
#endif // ifdef USES_P184

#ifdef USES_P185
  &Plugin_185,
#endif // ifdef USES_P185

#ifdef USES_P186
  &Plugin_186,
#endif // ifdef USES_P186

#ifdef USES_P187
  &Plugin_187,
#endif // ifdef USES_P187

#ifdef USES_P188
  &Plugin_188,
#endif // ifdef USES_P188

#ifdef USES_P189
  &Plugin_189,
#endif // ifdef USES_P189

#ifdef USES_P190
  &Plugin_190,
#endif // ifdef USES_P190

#ifdef USES_P191
  &Plugin_191,
#endif // ifdef USES_P191

#ifdef USES_P192
  &Plugin_192,
#endif // ifdef USES_P192

#ifdef USES_P193
  &Plugin_193,
#endif // ifdef USES_P193

#ifdef USES_P194
  &Plugin_194,
#endif // ifdef USES_P194

#ifdef USES_P195
  &Plugin_195,
#endif // ifdef USES_P195

#ifdef USES_P196
  &Plugin_196,
#endif // ifdef USES_P196

#ifdef USES_P197
  &Plugin_197,
#endif // ifdef USES_P197

#ifdef USES_P198
  &Plugin_198,
#endif // ifdef USES_P198

#ifdef USES_P199
  &Plugin_199,
#endif // ifdef USES_P199

#ifdef USES_P200
  &Plugin_200,
#endif // ifdef USES_P200

#ifdef USES_P201
  &Plugin_201,
#endif // ifdef USES_P201

#ifdef USES_P202
  &Plugin_202,
#endif // ifdef USES_P202

#ifdef USES_P203
  &Plugin_203,
#endif // ifdef USES_P203

#ifdef USES_P204
  &Plugin_204,
#endif // ifdef USES_P204

#ifdef USES_P205
  &Plugin_205,
#endif // ifdef USES_P205

#ifdef USES_P206
  &Plugin_206,
#endif // ifdef USES_P206

#ifdef USES_P207
  &Plugin_207,
#endif // ifdef USES_P207

#ifdef USES_P208
  &Plugin_208,
#endif // ifdef USES_P208

#ifdef USES_P209
  &Plugin_209,
#endif // ifdef USES_P209

#ifdef USES_P210
  &Plugin_210,
#endif // ifdef USES_P210

#ifdef USES_P211
  &Plugin_211,
#endif // ifdef USES_P211

#ifdef USES_P212
  &Plugin_212,
#endif // ifdef USES_P212

#ifdef USES_P213
  &Plugin_213,
#endif // ifdef USES_P213

#ifdef USES_P214
  &Plugin_214,
#endif // ifdef USES_P214

#ifdef USES_P215
  &Plugin_215,
#endif // ifdef USES_P215

#ifdef USES_P216
  &Plugin_216,
#endif // ifdef USES_P216

#ifdef USES_P217
  &Plugin_217,
#endif // ifdef USES_P217

#ifdef USES_P218
  &Plugin_218,
#endif // ifdef USES_P218

#ifdef USES_P219
  &Plugin_219,
#endif // ifdef USES_P219

#ifdef USES_P220
  &Plugin_220,
#endif // ifdef USES_P220

#ifdef USES_P221
  &Plugin_221,
#endif // ifdef USES_P221

#ifdef USES_P222
  &Plugin_222,
#endif // ifdef USES_P222

#ifdef USES_P223
  &Plugin_223,
#endif // ifdef USES_P223

#ifdef USES_P224
  &Plugin_224,
#endif // ifdef USES_P224

#ifdef USES_P225
  &Plugin_225,
#endif // ifdef USES_P225

#ifdef USES_P226
  &Plugin_226,
#endif // ifdef USES_P226

#ifdef USES_P227
  &Plugin_227,
#endif // ifdef USES_P227

#ifdef USES_P228
  &Plugin_228,
#endif // ifdef USES_P228

#ifdef USES_P229
  &Plugin_229,
#endif // ifdef USES_P229

#ifdef USES_P230
  &Plugin_230,
#endif // ifdef USES_P230

#ifdef USES_P231
  &Plugin_231,
#endif // ifdef USES_P231

#ifdef USES_P232
  &Plugin_232,
#endif // ifdef USES_P232

#ifdef USES_P233
  &Plugin_233,
#endif // ifdef USES_P233

#ifdef USES_P234
  &Plugin_234,
#endif // ifdef USES_P234

#ifdef USES_P235
  &Plugin_235,
#endif // ifdef USES_P235

#ifdef USES_P236
  &Plugin_236,
#endif // ifdef USES_P236

#ifdef USES_P237
  &Plugin_237,
#endif // ifdef USES_P237

#ifdef USES_P238
  &Plugin_238,
#endif // ifdef USES_P238

#ifdef USES_P239
  &Plugin_239,
#endif // ifdef USES_P239

#ifdef USES_P240
  &Plugin_240,
#endif // ifdef USES_P240

#ifdef USES_P241
  &Plugin_241,
#endif // ifdef USES_P241

#ifdef USES_P242
  &Plugin_242,
#endif // ifdef USES_P242

#ifdef USES_P243
  &Plugin_243,
#endif // ifdef USES_P243

#ifdef USES_P244
  &Plugin_244,
#endif // ifdef USES_P244

#ifdef USES_P245
  &Plugin_245,
#endif // ifdef USES_P245

#ifdef USES_P246
  &Plugin_246,
#endif // ifdef USES_P246

#ifdef USES_P247
  &Plugin_247,
#endif // ifdef USES_P247

#ifdef USES_P248
  &Plugin_248,
#endif // ifdef USES_P248

#ifdef USES_P249
  &Plugin_249,
#endif // ifdef USES_P249

#ifdef USES_P250
  &Plugin_250,
#endif // ifdef USES_P250

#ifdef USES_P251
  &Plugin_251,
#endif // ifdef USES_P251

#ifdef USES_P252
  &Plugin_252,
#endif // ifdef USES_P252

#ifdef USES_P253
  &Plugin_253,
#endif // ifdef USES_P253

#ifdef USES_P254
  &Plugin_254,
#endif // ifdef USES_P254

#ifdef USES_P255
  &Plugin_255,
#endif // ifdef USES_P255
};

bool _Plugin_init_setupDone = false;


constexpr size_t DeviceIndex_to_Plugin_id_size = NR_ELEMENTS(DeviceIndex_to_Plugin_id);

// Lowest plugin ID included in the build
constexpr size_t Lowest_Plugin_id = DeviceIndex_to_Plugin_id_size == 0 ? 0 : DeviceIndex_to_Plugin_id[0];

// Highest plugin ID included in the build
constexpr size_t Highest_Plugin_id = DeviceIndex_to_Plugin_id_size > 0 ? DeviceIndex_to_Plugin_id[DeviceIndex_to_Plugin_id_size - 1] : 0;

// Array size including index of highest plugin ID.
constexpr size_t Plugin_id_to_DeviceIndex_size = Highest_Plugin_id + 1 - Lowest_Plugin_id;

// Array filled during init.
// Valid index: 1 ... Highest_Plugin_id
// Returns index to the DeviceIndex_to_Plugin_id array
//
// Vector size should is lowest pluginID ... highest pluginID
deviceIndex_t Plugin_id_to_DeviceIndex[Plugin_id_to_DeviceIndex_size]{};

// Used as lookup for getting an alfabetically sorted deviceIndex
deviceIndex_t DeviceIndex_sorted[DeviceIndex_to_Plugin_id_size];


size_t get_Plugin_id_to_DeviceIndex_arrayIndex(pluginID_t pluginID)
{
  if (pluginID.value >= Lowest_Plugin_id)
  {
    const size_t arrIndex = static_cast<size_t>(pluginID.value) - Lowest_Plugin_id;
    if (arrIndex < Plugin_id_to_DeviceIndex_size) {
      return arrIndex;
    }
  }
  return Plugin_id_to_DeviceIndex_size;
}


/*
// TD-er: Test to make constexpr array Plugin_id_to_DeviceIndex
// Have to postpone, since std::integer_sequence is not available in Espressif SDK

// Constexpr functions to create Plugin_id_to_DeviceIndex at compile time
constexpr uint8_t getDevId_from_PId(unsigned p_id, unsigned d_id) {
  return (p_id == 0 || d_id == DeviceIndex_to_Plugin_id_size) ? DEVICE_INDEX_MAX
  :  (p_id == DeviceIndex_to_Plugin_id[d_id]) ? d_id : getDevId_from_PId(p_id, d_id+1);
}

constexpr uint8_t getDevId_from_PId(unsigned p_id) {
  return getDevId_from_PId(p_id, 0);
}
constexpr auto test = getDevId_from_PId(30);


#include <array>
#include <utility>
#include <type_traits>


template <unsigned... Is>
constexpr auto get_DevId_from_PId_array(std::integer_sequence<unsigned, Is...> a) -> std::array<uint8_t, a.size()> {
    std::array<uint8_t, a.size()> vals{};
    ((vals[Is] = getDevId_from_PId(Is)), ...);
    return vals;
}

constexpr auto x = get_DevId_from_PId_array(std::make_integer_sequence<unsigned, Plugin_id_to_DeviceIndex_size>{});
*/

unsigned getNrBitsDeviceIndex()
{
  // FIXME TD-er: Must somehow make this a constexpr function
  constexpr unsigned nrBits = NR_BITS(DeviceIndex_to_Plugin_id_size);
  return nrBits;
}

unsigned getNrBuiltInDeviceIndex()
{
  return DeviceIndex_to_Plugin_id_size;
}

deviceIndex_t getDeviceIndex_from_PluginID(pluginID_t pluginID)
{
  if (validPluginID(pluginID)) {
    const size_t arrayIndex = get_Plugin_id_to_DeviceIndex_arrayIndex(pluginID);
    if (arrayIndex < Plugin_id_to_DeviceIndex_size)
    {
      return Plugin_id_to_DeviceIndex[arrayIndex];
    }
  }
  return INVALID_DEVICE_INDEX;
}

pluginID_t getPluginID_from_DeviceIndex(deviceIndex_t deviceIndex)
{
  if (validDeviceIndex_init(deviceIndex))
  {
    return pluginID_t::toPluginID(pgm_read_byte(DeviceIndex_to_Plugin_id + deviceIndex.value));
  }
  return INVALID_PLUGIN_ID;
}

bool validDeviceIndex_init(deviceIndex_t deviceIndex)
{
  if (_Plugin_init_setupDone) {
    return deviceIndex < DeviceIndex_to_Plugin_id_size;
  }
  return false;
}

// Array containing "DeviceIndex" alfabetically sorted.
deviceIndex_t getDeviceIndex_sorted(deviceIndex_t deviceIndex)
{
  if (validDeviceIndex_init(deviceIndex)) {
    return DeviceIndex_sorted[deviceIndex.value];
  }
  return INVALID_DEVICE_INDEX;
}


boolean PluginCall(deviceIndex_t deviceIndex, uint8_t function, struct EventStruct *event, String& string)
{
  if (validDeviceIndex_init(deviceIndex))
  {
    Plugin_ptr_t plugin_call = (Plugin_ptr_t)pgm_read_ptr(Plugin_ptr + deviceIndex.value);
    return plugin_call(function, event, string);
  }
  return false;
}

void PluginSetup()
{
  if (_Plugin_init_setupDone) return;

  _Plugin_init_setupDone = true;

  for (size_t id = 0; id < Plugin_id_to_DeviceIndex_size; ++id)
  {
    Plugin_id_to_DeviceIndex[id] = INVALID_DEVICE_INDEX;
  }
  #ifdef ESP8266
  Device = new (std::nothrow) DeviceStruct[DeviceIndex_to_Plugin_id_size];
  #else
  Device.resize(DeviceIndex_to_Plugin_id_size);
  #endif

  for (deviceIndex_t deviceIndex; deviceIndex < DeviceIndex_to_Plugin_id_size; ++deviceIndex)
  {
    const pluginID_t pluginID = getPluginID_from_DeviceIndex(deviceIndex);

    if (validPluginID(pluginID)) { 
      const size_t arrayIndex = get_Plugin_id_to_DeviceIndex_arrayIndex(pluginID);
      if (arrayIndex < Plugin_id_to_DeviceIndex_size) {
        // Should never be outside these limits.
        Plugin_id_to_DeviceIndex[arrayIndex] = deviceIndex;
        struct EventStruct TempEvent;
        TempEvent.idx = deviceIndex.value;
        String dummy;
        PluginCall(deviceIndex, PLUGIN_DEVICE_ADD, &TempEvent, dummy);
      }
    }
  }
#ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("PLUGIN_DEVICE_ADD"));
#endif

  // ********************************************************************************
  // Device Sort routine, actual sorting alfabetically by plugin name.
  // Sorting does happen case sensitive.
  // Used in device selector dropdown.
  // ********************************************************************************

  // First fill the existing number of the DeviceIndex.
  for (deviceIndex_t x; x < DeviceIndex_to_Plugin_id_size; ++x) {
    DeviceIndex_sorted[x.value] = x;
  }

  struct
  {
    bool operator()(deviceIndex_t a, deviceIndex_t b) const { 
      return getPluginNameFromDeviceIndex(a) < 
             getPluginNameFromDeviceIndex(b); 
    }
  }
  customLess;
  std::sort(DeviceIndex_sorted, DeviceIndex_sorted + DeviceIndex_to_Plugin_id_size, customLess);
}

void PluginInit(bool priorityOnly)
{

  // Set all not supported plugins to disabled.
  for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; ++taskIndex) {
    if (!supportedPluginID(Settings.getPluginID_for_task(taskIndex))) {
      Settings.TaskDeviceEnabled[taskIndex] = false;
    }
  }


  if (!priorityOnly) {
    String dummy;
    PluginCall(PLUGIN_INIT_ALL, nullptr, dummy);
    #ifndef BUILD_NO_RAM_TRACKER
    logMemUsageAfter(F("PLUGIN_INIT_ALL"));
    #endif
  }
}

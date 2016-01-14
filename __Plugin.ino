//********************************************************************************
// Initialize all plugins that where defined earlier
// and initialize the function call pointer into the plugin array
//********************************************************************************
void PluginInit(void)
{
  byte x;

  // Clear pointer table for all plugins
  for (x = 0; x < PLUGIN_MAX; x++)
  {
    Plugin_ptr[x] = 0;
    Plugin_id[x] = 0;
  }

  x = 0;

#ifdef PLUGIN_001
  Plugin_id[x] = 1; Plugin_ptr[x++] = &Plugin_001;
#endif

#ifdef PLUGIN_002
  Plugin_id[x] = 2; Plugin_ptr[x++] = &Plugin_002;
#endif

#ifdef PLUGIN_003
  Plugin_id[x] = 3; Plugin_ptr[x++] = &Plugin_003;
#endif

#ifdef PLUGIN_004
  Plugin_id[x] = 4; Plugin_ptr[x++] = &Plugin_004;
#endif

#ifdef PLUGIN_005
  Plugin_id[x] = 5; Plugin_ptr[x++] = &Plugin_005;
#endif

#ifdef PLUGIN_006
  Plugin_id[x] = 6; Plugin_ptr[x++] = &Plugin_006;
#endif

#ifdef PLUGIN_007
  Plugin_id[x] = 7; Plugin_ptr[x++] = &Plugin_007;
#endif

#ifdef PLUGIN_008
  Plugin_id[x] = 8; Plugin_ptr[x++] = &Plugin_008;
#endif

#ifdef PLUGIN_009
  Plugin_id[x] = 9; Plugin_ptr[x++] = &Plugin_009;
#endif

#ifdef PLUGIN_010
  Plugin_id[x] = 10; Plugin_ptr[x++] = &Plugin_010;
#endif

#ifdef PLUGIN_011
  Plugin_id[x] = 11; Plugin_ptr[x++] = &Plugin_011;
#endif

#ifdef PLUGIN_012
  Plugin_id[x] = 12; Plugin_ptr[x++] = &Plugin_012;
#endif

#ifdef PLUGIN_013
  Plugin_id[x] = 13; Plugin_ptr[x++] = &Plugin_013;
#endif

#ifdef PLUGIN_014
  Plugin_id[x] = 14; Plugin_ptr[x++] = &Plugin_014;
#endif

#ifdef PLUGIN_015
  Plugin_id[x] = 15; Plugin_ptr[x++] = &Plugin_015;
#endif

#ifdef PLUGIN_016
  Plugin_id[x] = 16; Plugin_ptr[x++] = &Plugin_016;
#endif

#ifdef PLUGIN_017
  Plugin_id[x] = 17; Plugin_ptr[x++] = &Plugin_017;
#endif

#ifdef PLUGIN_018
  Plugin_id[x] = 18; Plugin_ptr[x++] = &Plugin_018;
#endif

#ifdef PLUGIN_019
  Plugin_id[x] = 19; Plugin_ptr[x++] = &Plugin_019;
#endif

#ifdef PLUGIN_020
  Plugin_id[x] = 20; Plugin_ptr[x++] = &Plugin_020;
#endif

#ifdef PLUGIN_021
  Plugin_id[x] = 21; Plugin_ptr[x++] = &Plugin_021;
#endif

#ifdef PLUGIN_022
  Plugin_id[x] = 22; Plugin_ptr[x++] = &Plugin_022;
#endif

#ifdef PLUGIN_023
  Plugin_id[x] = 23; Plugin_ptr[x++] = &Plugin_023;
#endif

#ifdef PLUGIN_024
  Plugin_id[x] = 24; Plugin_ptr[x++] = &Plugin_024;
#endif

#ifdef PLUGIN_025
  Plugin_id[x] = 25; Plugin_ptr[x++] = &Plugin_025;
#endif

#ifdef PLUGIN_026
  Plugin_id[x] = 26; Plugin_ptr[x++] = &Plugin_026;
#endif

#ifdef PLUGIN_027
  Plugin_id[x] = 27; Plugin_ptr[x++] = &Plugin_027;
#endif

#ifdef PLUGIN_028
  Plugin_id[x] = 28; Plugin_ptr[x++] = &Plugin_028;
#endif

#ifdef PLUGIN_029
  Plugin_id[x] = 29; Plugin_ptr[x++] = &Plugin_029;
#endif

#ifdef PLUGIN_030
  Plugin_id[x] = 30; Plugin_ptr[x++] = &Plugin_030;
#endif

#ifdef PLUGIN_031
  Plugin_id[x] = 31; Plugin_ptr[x++] = &Plugin_031;
#endif

#ifdef PLUGIN_032
  Plugin_id[x] = 32; Plugin_ptr[x++] = &Plugin_032;
#endif

#ifdef PLUGIN_033
  Plugin_id[x] = 33; Plugin_ptr[x++] = &Plugin_033;
#endif

#ifdef PLUGIN_034
  Plugin_id[x] = 34; Plugin_ptr[x++] = &Plugin_034;
#endif

#ifdef PLUGIN_035
  Plugin_id[x] = 35; Plugin_ptr[x++] = &Plugin_035;
#endif

#ifdef PLUGIN_036
  Plugin_id[x] = 36; Plugin_ptr[x++] = &Plugin_036;
#endif

#ifdef PLUGIN_037
  Plugin_id[x] = 37; Plugin_ptr[x++] = &Plugin_037;
#endif

#ifdef PLUGIN_038
  Plugin_id[x] = 38; Plugin_ptr[x++] = &Plugin_038;
#endif

#ifdef PLUGIN_039
  Plugin_id[x] = 39; Plugin_ptr[x++] = &Plugin_039;
#endif

#ifdef PLUGIN_040
  Plugin_id[x] = 40; Plugin_ptr[x++] = &Plugin_040;
#endif

#ifdef PLUGIN_041
  Plugin_id[x] = 41; Plugin_ptr[x++] = &Plugin_041;
#endif

#ifdef PLUGIN_042
  Plugin_id[x] = 42; Plugin_ptr[x++] = &Plugin_042;
#endif

#ifdef PLUGIN_043
  Plugin_id[x] = 43; Plugin_ptr[x++] = &Plugin_043;
#endif

#ifdef PLUGIN_044
  Plugin_id[x] = 44; Plugin_ptr[x++] = &Plugin_044;
#endif

#ifdef PLUGIN_045
  Plugin_id[x] = 45; Plugin_ptr[x++] = &Plugin_045;
#endif

#ifdef PLUGIN_046
  Plugin_id[x] = 46; Plugin_ptr[x++] = &Plugin_046;
#endif

#ifdef PLUGIN_047
  Plugin_id[x] = 47; Plugin_ptr[x++] = &Plugin_047;
#endif

#ifdef PLUGIN_048
  Plugin_id[x] = 48; Plugin_ptr[x++] = &Plugin_048;
#endif

#ifdef PLUGIN_049
  Plugin_id[x] = 49; Plugin_ptr[x++] = &Plugin_049;
#endif

#ifdef PLUGIN_050
  Plugin_id[x] = 50; Plugin_ptr[x++] = &Plugin_050;
#endif

#ifdef PLUGIN_051
  Plugin_id[x] = 51; Plugin_ptr[x++] = &Plugin_051;
#endif

#ifdef PLUGIN_052
  Plugin_id[x] = 52; Plugin_ptr[x++] = &Plugin_052;
#endif

#ifdef PLUGIN_053
  Plugin_id[x] = 53; Plugin_ptr[x++] = &Plugin_053;
#endif

#ifdef PLUGIN_054
  Plugin_id[x] = 54; Plugin_ptr[x++] = &Plugin_054;
#endif

#ifdef PLUGIN_055
  Plugin_id[x] = 55; Plugin_ptr[x++] = &Plugin_055;
#endif

#ifdef PLUGIN_056
  Plugin_id[x] = 56; Plugin_ptr[x++] = &Plugin_056;
#endif

#ifdef PLUGIN_057
  Plugin_id[x] = 57; Plugin_ptr[x++] = &Plugin_057;
#endif

#ifdef PLUGIN_058
  Plugin_id[x] = 58; Plugin_ptr[x++] = &Plugin_058;
#endif

#ifdef PLUGIN_059
  Plugin_id[x] = 59; Plugin_ptr[x++] = &Plugin_059;
#endif

#ifdef PLUGIN_060
  Plugin_id[x] = 60; Plugin_ptr[x++] = &Plugin_060;
#endif

#ifdef PLUGIN_061
  Plugin_id[x] = 61; Plugin_ptr[x++] = &Plugin_061;
#endif

#ifdef PLUGIN_062
  Plugin_id[x] = 62; Plugin_ptr[x++] = &Plugin_062;
#endif

#ifdef PLUGIN_063
  Plugin_id[x] = 63; Plugin_ptr[x++] = &Plugin_063;
#endif

#ifdef PLUGIN_064
  Plugin_id[x] = 64; Plugin_ptr[x++] = &Plugin_064;
#endif

#ifdef PLUGIN_065
  Plugin_id[x] = 65; Plugin_ptr[x++] = &Plugin_065;
#endif

#ifdef PLUGIN_066
  Plugin_id[x] = 66; Plugin_ptr[x++] = &Plugin_066;
#endif

#ifdef PLUGIN_067
  Plugin_id[x] = 67; Plugin_ptr[x++] = &Plugin_067;
#endif

#ifdef PLUGIN_068
  Plugin_id[x] = 68; Plugin_ptr[x++] = &Plugin_068;
#endif

#ifdef PLUGIN_069
  Plugin_id[x] = 69; Plugin_ptr[x++] = &Plugin_069;
#endif

#ifdef PLUGIN_070
  Plugin_id[x] = 70; Plugin_ptr[x++] = &Plugin_070;
#endif

#ifdef PLUGIN_071
  Plugin_id[x] = 71; Plugin_ptr[x++] = &Plugin_071;
#endif

#ifdef PLUGIN_072
  Plugin_id[x] = 72; Plugin_ptr[x++] = &Plugin_072;
#endif

#ifdef PLUGIN_073
  Plugin_id[x] = 73; Plugin_ptr[x++] = &Plugin_073;
#endif

#ifdef PLUGIN_074
  Plugin_id[x] = 74; Plugin_ptr[x++] = &Plugin_074;
#endif

#ifdef PLUGIN_075
  Plugin_id[x] = 75; Plugin_ptr[x++] = &Plugin_075;
#endif

#ifdef PLUGIN_076
  Plugin_id[x] = 76; Plugin_ptr[x++] = &Plugin_076;
#endif

#ifdef PLUGIN_077
  Plugin_id[x] = 77; Plugin_ptr[x++] = &Plugin_077;
#endif

#ifdef PLUGIN_078
  Plugin_id[x] = 78; Plugin_ptr[x++] = &Plugin_078;
#endif

#ifdef PLUGIN_079
  Plugin_id[x] = 79; Plugin_ptr[x++] = &Plugin_079;
#endif

#ifdef PLUGIN_080
  Plugin_id[x] = 80; Plugin_ptr[x++] = &Plugin_080;
#endif

#ifdef PLUGIN_081
  Plugin_id[x] = 81; Plugin_ptr[x++] = &Plugin_081;
#endif

#ifdef PLUGIN_082
  Plugin_id[x] = 82; Plugin_ptr[x++] = &Plugin_082;
#endif

#ifdef PLUGIN_083
  Plugin_id[x] = 83; Plugin_ptr[x++] = &Plugin_083;
#endif

#ifdef PLUGIN_084
  Plugin_id[x] = 84; Plugin_ptr[x++] = &Plugin_084;
#endif

#ifdef PLUGIN_085
  Plugin_id[x] = 85; Plugin_ptr[x++] = &Plugin_085;
#endif

#ifdef PLUGIN_086
  Plugin_id[x] = 86; Plugin_ptr[x++] = &Plugin_086;
#endif

#ifdef PLUGIN_087
  Plugin_id[x] = 87; Plugin_ptr[x++] = &Plugin_087;
#endif

#ifdef PLUGIN_088
  Plugin_id[x] = 88; Plugin_ptr[x++] = &Plugin_088;
#endif

#ifdef PLUGIN_089
  Plugin_id[x] = 89; Plugin_ptr[x++] = &Plugin_089;
#endif

#ifdef PLUGIN_090
  Plugin_id[x] = 90; Plugin_ptr[x++] = &Plugin_090;
#endif

#ifdef PLUGIN_091
  Plugin_id[x] = 91; Plugin_ptr[x++] = &Plugin_091;
#endif

#ifdef PLUGIN_092
  Plugin_id[x] = 92; Plugin_ptr[x++] = &Plugin_092;
#endif

#ifdef PLUGIN_093
  Plugin_id[x] = 93; Plugin_ptr[x++] = &Plugin_093;
#endif

#ifdef PLUGIN_094
  Plugin_id[x] = 94; Plugin_ptr[x++] = &Plugin_094;
#endif

#ifdef PLUGIN_095
  Plugin_id[x] = 95; Plugin_ptr[x++] = &Plugin_095;
#endif

#ifdef PLUGIN_096
  Plugin_id[x] = 96; Plugin_ptr[x++] = &Plugin_096;
#endif

#ifdef PLUGIN_097
  Plugin_id[x] = 97; Plugin_ptr[x++] = &Plugin_097;
#endif

#ifdef PLUGIN_098
  Plugin_id[x] = 98; Plugin_ptr[x++] = &Plugin_098;
#endif

#ifdef PLUGIN_099
  Plugin_id[x] = 99; Plugin_ptr[x++] = &Plugin_099;
#endif

#ifdef PLUGIN_100
  Plugin_id[x] = 100; Plugin_ptr[x++] = &Plugin_100;
#endif

#ifdef PLUGIN_101
  Plugin_id[x] = 101; Plugin_ptr[x++] = &Plugin_101;
#endif

#ifdef PLUGIN_102
  Plugin_id[x] = 102; Plugin_ptr[x++] = &Plugin_102;
#endif

#ifdef PLUGIN_103
  Plugin_id[x] = 103; Plugin_ptr[x++] = &Plugin_103;
#endif

#ifdef PLUGIN_104
  Plugin_id[x] = 104; Plugin_ptr[x++] = &Plugin_104;
#endif

#ifdef PLUGIN_105
  Plugin_id[x] = 105; Plugin_ptr[x++] = &Plugin_105;
#endif

#ifdef PLUGIN_106
  Plugin_id[x] = 106; Plugin_ptr[x++] = &Plugin_106;
#endif

#ifdef PLUGIN_107
  Plugin_id[x] = 107; Plugin_ptr[x++] = &Plugin_107;
#endif

#ifdef PLUGIN_108
  Plugin_id[x] = 108; Plugin_ptr[x++] = &Plugin_108;
#endif

#ifdef PLUGIN_109
  Plugin_id[x] = 109; Plugin_ptr[x++] = &Plugin_109;
#endif

#ifdef PLUGIN_110
  Plugin_id[x] = 110; Plugin_ptr[x++] = &Plugin_110;
#endif

#ifdef PLUGIN_111
  Plugin_id[x] = 111; Plugin_ptr[x++] = &Plugin_111;
#endif

#ifdef PLUGIN_112
  Plugin_id[x] = 112; Plugin_ptr[x++] = &Plugin_112;
#endif

#ifdef PLUGIN_113
  Plugin_id[x] = 113; Plugin_ptr[x++] = &Plugin_113;
#endif

#ifdef PLUGIN_114
  Plugin_id[x] = 114; Plugin_ptr[x++] = &Plugin_114;
#endif

#ifdef PLUGIN_115
  Plugin_id[x] = 115; Plugin_ptr[x++] = &Plugin_115;
#endif

#ifdef PLUGIN_116
  Plugin_id[x] = 116; Plugin_ptr[x++] = &Plugin_116;
#endif

#ifdef PLUGIN_117
  Plugin_id[x] = 117; Plugin_ptr[x++] = &Plugin_117;
#endif

#ifdef PLUGIN_118
  Plugin_id[x] = 118; Plugin_ptr[x++] = &Plugin_118;
#endif

#ifdef PLUGIN_119
  Plugin_id[x] = 119; Plugin_ptr[x++] = &Plugin_119;
#endif

#ifdef PLUGIN_120
  Plugin_id[x] = 120; Plugin_ptr[x++] = &Plugin_120;
#endif

#ifdef PLUGIN_121
  Plugin_id[x] = 121; Plugin_ptr[x++] = &Plugin_121;
#endif

#ifdef PLUGIN_122
  Plugin_id[x] = 122; Plugin_ptr[x++] = &Plugin_122;
#endif

#ifdef PLUGIN_123
  Plugin_id[x] = 123; Plugin_ptr[x++] = &Plugin_123;
#endif

#ifdef PLUGIN_124
  Plugin_id[x] = 124; Plugin_ptr[x++] = &Plugin_124;
#endif

#ifdef PLUGIN_125
  Plugin_id[x] = 125; Plugin_ptr[x++] = &Plugin_125;
#endif

#ifdef PLUGIN_126
  Plugin_id[x] = 126; Plugin_ptr[x++] = &Plugin_126;
#endif

#ifdef PLUGIN_127
  Plugin_id[x] = 127; Plugin_ptr[x++] = &Plugin_127;
#endif

#ifdef PLUGIN_128
  Plugin_id[x] = 128; Plugin_ptr[x++] = &Plugin_128;
#endif

#ifdef PLUGIN_129
  Plugin_id[x] = 129; Plugin_ptr[x++] = &Plugin_129;
#endif

#ifdef PLUGIN_130
  Plugin_id[x] = 130; Plugin_ptr[x++] = &Plugin_130;
#endif

#ifdef PLUGIN_131
  Plugin_id[x] = 131; Plugin_ptr[x++] = &Plugin_131;
#endif

#ifdef PLUGIN_132
  Plugin_id[x] = 132; Plugin_ptr[x++] = &Plugin_132;
#endif

#ifdef PLUGIN_133
  Plugin_id[x] = 133; Plugin_ptr[x++] = &Plugin_133;
#endif

#ifdef PLUGIN_134
  Plugin_id[x] = 134; Plugin_ptr[x++] = &Plugin_134;
#endif

#ifdef PLUGIN_135
  Plugin_id[x] = 135; Plugin_ptr[x++] = &Plugin_135;
#endif

#ifdef PLUGIN_136
  Plugin_id[x] = 136; Plugin_ptr[x++] = &Plugin_136;
#endif

#ifdef PLUGIN_137
  Plugin_id[x] = 137; Plugin_ptr[x++] = &Plugin_137;
#endif

#ifdef PLUGIN_138
  Plugin_id[x] = 138; Plugin_ptr[x++] = &Plugin_138;
#endif

#ifdef PLUGIN_139
  Plugin_id[x] = 139; Plugin_ptr[x++] = &Plugin_139;
#endif

#ifdef PLUGIN_140
  Plugin_id[x] = 140; Plugin_ptr[x++] = &Plugin_140;
#endif

#ifdef PLUGIN_141
  Plugin_id[x] = 141; Plugin_ptr[x++] = &Plugin_141;
#endif

#ifdef PLUGIN_142
  Plugin_id[x] = 142; Plugin_ptr[x++] = &Plugin_142;
#endif

#ifdef PLUGIN_143
  Plugin_id[x] = 143; Plugin_ptr[x++] = &Plugin_143;
#endif

#ifdef PLUGIN_144
  Plugin_id[x] = 144; Plugin_ptr[x++] = &Plugin_144;
#endif

#ifdef PLUGIN_145
  Plugin_id[x] = 145; Plugin_ptr[x++] = &Plugin_145;
#endif

#ifdef PLUGIN_146
  Plugin_id[x] = 146; Plugin_ptr[x++] = &Plugin_146;
#endif

#ifdef PLUGIN_147
  Plugin_id[x] = 147; Plugin_ptr[x++] = &Plugin_147;
#endif

#ifdef PLUGIN_148
  Plugin_id[x] = 148; Plugin_ptr[x++] = &Plugin_148;
#endif

#ifdef PLUGIN_149
  Plugin_id[x] = 149; Plugin_ptr[x++] = &Plugin_149;
#endif

#ifdef PLUGIN_150
  Plugin_id[x] = 150; Plugin_ptr[x++] = &Plugin_150;
#endif

#ifdef PLUGIN_151
  Plugin_id[x] = 151; Plugin_ptr[x++] = &Plugin_151;
#endif

#ifdef PLUGIN_152
  Plugin_id[x] = 152; Plugin_ptr[x++] = &Plugin_152;
#endif

#ifdef PLUGIN_153
  Plugin_id[x] = 153; Plugin_ptr[x++] = &Plugin_153;
#endif

#ifdef PLUGIN_154
  Plugin_id[x] = 154; Plugin_ptr[x++] = &Plugin_154;
#endif

#ifdef PLUGIN_155
  Plugin_id[x] = 155; Plugin_ptr[x++] = &Plugin_155;
#endif

#ifdef PLUGIN_156
  Plugin_id[x] = 156; Plugin_ptr[x++] = &Plugin_156;
#endif

#ifdef PLUGIN_157
  Plugin_id[x] = 157; Plugin_ptr[x++] = &Plugin_157;
#endif

#ifdef PLUGIN_158
  Plugin_id[x] = 158; Plugin_ptr[x++] = &Plugin_158;
#endif

#ifdef PLUGIN_159
  Plugin_id[x] = 159; Plugin_ptr[x++] = &Plugin_159;
#endif

#ifdef PLUGIN_160
  Plugin_id[x] = 160; Plugin_ptr[x++] = &Plugin_160;
#endif

#ifdef PLUGIN_161
  Plugin_id[x] = 161; Plugin_ptr[x++] = &Plugin_161;
#endif

#ifdef PLUGIN_162
  Plugin_id[x] = 162; Plugin_ptr[x++] = &Plugin_162;
#endif

#ifdef PLUGIN_163
  Plugin_id[x] = 163; Plugin_ptr[x++] = &Plugin_163;
#endif

#ifdef PLUGIN_164
  Plugin_id[x] = 164; Plugin_ptr[x++] = &Plugin_164;
#endif

#ifdef PLUGIN_165
  Plugin_id[x] = 165; Plugin_ptr[x++] = &Plugin_165;
#endif

#ifdef PLUGIN_166
  Plugin_id[x] = 166; Plugin_ptr[x++] = &Plugin_166;
#endif

#ifdef PLUGIN_167
  Plugin_id[x] = 167; Plugin_ptr[x++] = &Plugin_167;
#endif

#ifdef PLUGIN_168
  Plugin_id[x] = 168; Plugin_ptr[x++] = &Plugin_168;
#endif

#ifdef PLUGIN_169
  Plugin_id[x] = 169; Plugin_ptr[x++] = &Plugin_169;
#endif

#ifdef PLUGIN_170
  Plugin_id[x] = 170; Plugin_ptr[x++] = &Plugin_170;
#endif

#ifdef PLUGIN_171
  Plugin_id[x] = 171; Plugin_ptr[x++] = &Plugin_171;
#endif

#ifdef PLUGIN_172
  Plugin_id[x] = 172; Plugin_ptr[x++] = &Plugin_172;
#endif

#ifdef PLUGIN_173
  Plugin_id[x] = 173; Plugin_ptr[x++] = &Plugin_173;
#endif

#ifdef PLUGIN_174
  Plugin_id[x] = 174; Plugin_ptr[x++] = &Plugin_174;
#endif

#ifdef PLUGIN_175
  Plugin_id[x] = 175; Plugin_ptr[x++] = &Plugin_175;
#endif

#ifdef PLUGIN_176
  Plugin_id[x] = 176; Plugin_ptr[x++] = &Plugin_176;
#endif

#ifdef PLUGIN_177
  Plugin_id[x] = 177; Plugin_ptr[x++] = &Plugin_177;
#endif

#ifdef PLUGIN_178
  Plugin_id[x] = 178; Plugin_ptr[x++] = &Plugin_178;
#endif

#ifdef PLUGIN_179
  Plugin_id[x] = 179; Plugin_ptr[x++] = &Plugin_179;
#endif

#ifdef PLUGIN_180
  Plugin_id[x] = 180; Plugin_ptr[x++] = &Plugin_180;
#endif

#ifdef PLUGIN_181
  Plugin_id[x] = 181; Plugin_ptr[x++] = &Plugin_181;
#endif

#ifdef PLUGIN_182
  Plugin_id[x] = 182; Plugin_ptr[x++] = &Plugin_182;
#endif

#ifdef PLUGIN_183
  Plugin_id[x] = 183; Plugin_ptr[x++] = &Plugin_183;
#endif

#ifdef PLUGIN_184
  Plugin_id[x] = 184; Plugin_ptr[x++] = &Plugin_184;
#endif

#ifdef PLUGIN_185
  Plugin_id[x] = 185; Plugin_ptr[x++] = &Plugin_185;
#endif

#ifdef PLUGIN_186
  Plugin_id[x] = 186; Plugin_ptr[x++] = &Plugin_186;
#endif

#ifdef PLUGIN_187
  Plugin_id[x] = 187; Plugin_ptr[x++] = &Plugin_187;
#endif

#ifdef PLUGIN_188
  Plugin_id[x] = 188; Plugin_ptr[x++] = &Plugin_188;
#endif

#ifdef PLUGIN_189
  Plugin_id[x] = 189; Plugin_ptr[x++] = &Plugin_189;
#endif

#ifdef PLUGIN_190
  Plugin_id[x] = 190; Plugin_ptr[x++] = &Plugin_190;
#endif

#ifdef PLUGIN_191
  Plugin_id[x] = 191; Plugin_ptr[x++] = &Plugin_191;
#endif

#ifdef PLUGIN_192
  Plugin_id[x] = 192; Plugin_ptr[x++] = &Plugin_192;
#endif

#ifdef PLUGIN_193
  Plugin_id[x] = 193; Plugin_ptr[x++] = &Plugin_193;
#endif

#ifdef PLUGIN_194
  Plugin_id[x] = 194; Plugin_ptr[x++] = &Plugin_194;
#endif

#ifdef PLUGIN_195
  Plugin_id[x] = 195; Plugin_ptr[x++] = &Plugin_195;
#endif

#ifdef PLUGIN_196
  Plugin_id[x] = 196; Plugin_ptr[x++] = &Plugin_196;
#endif

#ifdef PLUGIN_197
  Plugin_id[x] = 197; Plugin_ptr[x++] = &Plugin_197;
#endif

#ifdef PLUGIN_198
  Plugin_id[x] = 198; Plugin_ptr[x++] = &Plugin_198;
#endif

#ifdef PLUGIN_199
  Plugin_id[x] = 199; Plugin_ptr[x++] = &Plugin_199;
#endif

#ifdef PLUGIN_200
  Plugin_id[x] = 200; Plugin_ptr[x++] = &Plugin_200;
#endif

#ifdef PLUGIN_201
  Plugin_id[x] = 201; Plugin_ptr[x++] = &Plugin_201;
#endif

#ifdef PLUGIN_202
  Plugin_id[x] = 202; Plugin_ptr[x++] = &Plugin_202;
#endif

#ifdef PLUGIN_203
  Plugin_id[x] = 203; Plugin_ptr[x++] = &Plugin_203;
#endif

#ifdef PLUGIN_204
  Plugin_id[x] = 204; Plugin_ptr[x++] = &Plugin_204;
#endif

#ifdef PLUGIN_205
  Plugin_id[x] = 205; Plugin_ptr[x++] = &Plugin_205;
#endif

#ifdef PLUGIN_206
  Plugin_id[x] = 206; Plugin_ptr[x++] = &Plugin_206;
#endif

#ifdef PLUGIN_207
  Plugin_id[x] = 207; Plugin_ptr[x++] = &Plugin_207;
#endif

#ifdef PLUGIN_208
  Plugin_id[x] = 208; Plugin_ptr[x++] = &Plugin_208;
#endif

#ifdef PLUGIN_209
  Plugin_id[x] = 209; Plugin_ptr[x++] = &Plugin_209;
#endif

#ifdef PLUGIN_210
  Plugin_id[x] = 210; Plugin_ptr[x++] = &Plugin_210;
#endif

#ifdef PLUGIN_211
  Plugin_id[x] = 211; Plugin_ptr[x++] = &Plugin_211;
#endif

#ifdef PLUGIN_212
  Plugin_id[x] = 212; Plugin_ptr[x++] = &Plugin_212;
#endif

#ifdef PLUGIN_213
  Plugin_id[x] = 213; Plugin_ptr[x++] = &Plugin_213;
#endif

#ifdef PLUGIN_214
  Plugin_id[x] = 214; Plugin_ptr[x++] = &Plugin_214;
#endif

#ifdef PLUGIN_215
  Plugin_id[x] = 215; Plugin_ptr[x++] = &Plugin_215;
#endif

#ifdef PLUGIN_216
  Plugin_id[x] = 216; Plugin_ptr[x++] = &Plugin_216;
#endif

#ifdef PLUGIN_217
  Plugin_id[x] = 217; Plugin_ptr[x++] = &Plugin_217;
#endif

#ifdef PLUGIN_218
  Plugin_id[x] = 218; Plugin_ptr[x++] = &Plugin_218;
#endif

#ifdef PLUGIN_219
  Plugin_id[x] = 219; Plugin_ptr[x++] = &Plugin_219;
#endif

#ifdef PLUGIN_220
  Plugin_id[x] = 220; Plugin_ptr[x++] = &Plugin_220;
#endif

#ifdef PLUGIN_221
  Plugin_id[x] = 221; Plugin_ptr[x++] = &Plugin_221;
#endif

#ifdef PLUGIN_222
  Plugin_id[x] = 222; Plugin_ptr[x++] = &Plugin_222;
#endif

#ifdef PLUGIN_223
  Plugin_id[x] = 223; Plugin_ptr[x++] = &Plugin_223;
#endif

#ifdef PLUGIN_224
  Plugin_id[x] = 224; Plugin_ptr[x++] = &Plugin_224;
#endif

#ifdef PLUGIN_225
  Plugin_id[x] = 225; Plugin_ptr[x++] = &Plugin_225;
#endif

#ifdef PLUGIN_226
  Plugin_id[x] = 226; Plugin_ptr[x++] = &Plugin_226;
#endif

#ifdef PLUGIN_227
  Plugin_id[x] = 227; Plugin_ptr[x++] = &Plugin_227;
#endif

#ifdef PLUGIN_228
  Plugin_id[x] = 228; Plugin_ptr[x++] = &Plugin_228;
#endif

#ifdef PLUGIN_229
  Plugin_id[x] = 229; Plugin_ptr[x++] = &Plugin_229;
#endif

#ifdef PLUGIN_230
  Plugin_id[x] = 230; Plugin_ptr[x++] = &Plugin_230;
#endif

#ifdef PLUGIN_231
  Plugin_id[x] = 231; Plugin_ptr[x++] = &Plugin_231;
#endif

#ifdef PLUGIN_232
  Plugin_id[x] = 232; Plugin_ptr[x++] = &Plugin_232;
#endif

#ifdef PLUGIN_233
  Plugin_id[x] = 233; Plugin_ptr[x++] = &Plugin_233;
#endif

#ifdef PLUGIN_234
  Plugin_id[x] = 234; Plugin_ptr[x++] = &Plugin_234;
#endif

#ifdef PLUGIN_235
  Plugin_id[x] = 235; Plugin_ptr[x++] = &Plugin_235;
#endif

#ifdef PLUGIN_236
  Plugin_id[x] = 236; Plugin_ptr[x++] = &Plugin_236;
#endif

#ifdef PLUGIN_237
  Plugin_id[x] = 237; Plugin_ptr[x++] = &Plugin_237;
#endif

#ifdef PLUGIN_238
  Plugin_id[x] = 238; Plugin_ptr[x++] = &Plugin_238;
#endif

#ifdef PLUGIN_239
  Plugin_id[x] = 239; Plugin_ptr[x++] = &Plugin_239;
#endif

#ifdef PLUGIN_240
  Plugin_id[x] = 240; Plugin_ptr[x++] = &Plugin_240;
#endif

#ifdef PLUGIN_241
  Plugin_id[x] = 241; Plugin_ptr[x++] = &Plugin_241;
#endif

#ifdef PLUGIN_242
  Plugin_id[x] = 242; Plugin_ptr[x++] = &Plugin_242;
#endif

#ifdef PLUGIN_243
  Plugin_id[x] = 243; Plugin_ptr[x++] = &Plugin_243;
#endif

#ifdef PLUGIN_244
  Plugin_id[x] = 244; Plugin_ptr[x++] = &Plugin_244;
#endif

#ifdef PLUGIN_245
  Plugin_id[x] = 245; Plugin_ptr[x++] = &Plugin_245;
#endif

#ifdef PLUGIN_246
  Plugin_id[x] = 246; Plugin_ptr[x++] = &Plugin_246;
#endif

#ifdef PLUGIN_247
  Plugin_id[x] = 247; Plugin_ptr[x++] = &Plugin_247;
#endif

#ifdef PLUGIN_248
  Plugin_id[x] = 248; Plugin_ptr[x++] = &Plugin_248;
#endif

#ifdef PLUGIN_249
  Plugin_id[x] = 249; Plugin_ptr[x++] = &Plugin_249;
#endif

#ifdef PLUGIN_250
  Plugin_id[x] = 250; Plugin_ptr[x++] = &Plugin_250;
#endif

#ifdef PLUGIN_251
  Plugin_id[x] = 251; Plugin_ptr[x++] = &Plugin_251;
#endif

#ifdef PLUGIN_252
  Plugin_id[x] = 252; Plugin_ptr[x++] = &Plugin_252;
#endif

#ifdef PLUGIN_253
  Plugin_id[x] = 253; Plugin_ptr[x++] = &Plugin_253;
#endif

#ifdef PLUGIN_254
  Plugin_id[x] = 254; Plugin_ptr[x++] = &Plugin_254;
#endif

#ifdef PLUGIN_255
  Plugin_id[x] = 255; Plugin_ptr[x++] = &Plugin_255;
#endif

  PluginCall(PLUGIN_DEVICE_ADD, 0, dummyString);
  PluginCall(PLUGIN_INIT_ALL, 0, dummyString);

}


/*********************************************************************************************\
* Function call to all or specific plugins
\*********************************************************************************************/
byte PluginCall(byte Function, struct EventStruct *event, String& str)
{
  int x;
  struct EventStruct TempEvent;

  if (event == 0)
    event = &TempEvent;

  switch (Function)
  {
    // Unconditional calls to all plugins
    case PLUGIN_DEVICE_ADD:
      for (x = 0; x < PLUGIN_MAX; x++)
        if (Plugin_id[x] != 0)
          Plugin_ptr[x](Function, event, str);
      return true;
      break;

    // Call to all plugins. Return at first match
    case PLUGIN_WRITE:
      for (x = 0; x < PLUGIN_MAX; x++)
        if (Plugin_id[x] != 0)
          if (Plugin_ptr[x](Function, event, str))
            return true;
      break;

    // Call to all plugins used in a task. Return at first match
    case PLUGIN_SERIAL_IN:
    case PLUGIN_UDP_IN:
      {
        for (byte y = 0; y < TASKS_MAX; y++)
        {
          if (Settings.TaskDeviceNumber[y] != 0)
          {
            for (x = 0; x < PLUGIN_MAX; x++)
            {
              if (Plugin_id[x] == Settings.TaskDeviceNumber[y])
              {
                if (Plugin_ptr[x](Function, event, str))
                  return true;
              }
            }
          }
        }
        return false;
        break;
      }

    // Call to all plugins that are used in a task
    case PLUGIN_ONCE_A_SECOND:
    case PLUGIN_TEN_PER_SECOND:
    case PLUGIN_INIT_ALL:
    case PLUGIN_CLOCK_IN:
    case PLUGIN_EVENT_OUT:
      {
        if (Function == PLUGIN_INIT_ALL)
          Function = PLUGIN_INIT;
        for (byte y = 0; y < TASKS_MAX; y++)
        {
          if (Settings.TaskDeviceNumber[y] != 0)
          {
            if (Settings.TaskDeviceDataFeed[y] == 0) // these calls only to tasks with local feed
            {
              byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[y]);
              TempEvent.TaskIndex = y;
              TempEvent.BaseVarIndex = y * VARS_PER_TASK;
              TempEvent.idx = Settings.TaskDeviceID[y];
              TempEvent.sensorType = Device[DeviceIndex].VType;
              TempEvent.OriginTaskIndex = event->TaskIndex;
              for (x = 0; x < PLUGIN_MAX; x++)
              {
                if (Plugin_id[x] == Settings.TaskDeviceNumber[y])
                {
                  Plugin_ptr[x](Function, &TempEvent, str);
                }
              }
            }
          }
        }
        return true;
        break;
      }

    // Call to specific plugin that is used for current task
    case PLUGIN_INIT:
    case PLUGIN_WEBFORM_LOAD:
    case PLUGIN_WEBFORM_SAVE:
    case PLUGIN_WEBFORM_SHOW_VALUES:
    case PLUGIN_WEBFORM_SHOW_CONFIG:
    case PLUGIN_GET_DEVICEVALUENAMES:
    case PLUGIN_READ:
      for (x = 0; x < PLUGIN_MAX; x++)
      {
        if ((Plugin_id[x] != 0 ) && (Plugin_id[x] == Settings.TaskDeviceNumber[event->TaskIndex]))
        {
          event->BaseVarIndex = event->TaskIndex * VARS_PER_TASK;
          return Plugin_ptr[x](Function, event, str);
        }
      }
      return false;
      break;

  }// case
  return false;
}

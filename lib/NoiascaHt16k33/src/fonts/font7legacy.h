/* *******************************************************************
  HT16K33 Library - Fontfile
  copyright (c) noiasca
  
  Version 2021-03-14
  
  This is the original 7 segment font as of library version 1.0.2 from 2020-06-07
  
 * ******************************************************************/
 
#pragma once
#define FONTFILE7 legacy               // include that to each fontfile as additional identifier
#ifndef FONTFILE_WAS_INCLUDED
#define FONTFILE_WAS_INCLUDED
#endif
#define FONTFILE7_MAX_CHAR   130      // 129 + 1
 
 /* *******************************************************************
         Characterset for 7 segment
 * ******************************************************************/

const uint8_t charTable [] PROGMEM  = {
  0,                                                       //     32   space
  SEG_B | SEG_C | SEG_DP,                                  // !   33
  SEG_B | SEG_F,                                           // "   34
  0,                                                       // #   35
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                   // $   36
  SEG_A | SEG_B | SEG_F | SEG_G,                           // %   37
  0,                                                       // &   38
  SEG_B,                                                   // '   39
  SEG_A | SEG_D | SEG_E | SEG_F,                           // (   40
  SEG_A | SEG_B | SEG_C | SEG_D,                           // )   41
  0,                                                       // *   42   no character on 7segment
  0,                                                       // +   43   no character on 7segment
  0,                                                       // ,   44   will be handled in the write methode
  SEG_G,                                                   // -   45
  0,                                                       // .   46   will be handled in the write methode
  SEG_B | SEG_E | SEG_G ,                                  // /   47
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,           // 0   48
  SEG_B | SEG_C,                                           // 1   49
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                   // 2   50
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,                   // 3   51
  SEG_B | SEG_C | SEG_F | SEG_G,                           // 4   52
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                   // 5   53
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,           // 6   54
  SEG_A | SEG_B | SEG_C,                                   // 7   55
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,   // 8   56
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,           // 9   57
  0,                                                       // :   58   will be handled in the write methode
  0,                                                       // ;   59   will be handled in the write methode
  SEG_D | SEG_E | SEG_G,                                   // <   60
  SEG_G,                                                   // =   61
  SEG_C | SEG_D | SEG_G,                                   // >   62
  SEG_A | SEG_B | SEG_E | SEG_G,                           // ?   63
  0,                                                       // @   64
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,           // A   65
  SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,                   // B
  SEG_A | SEG_D | SEG_E | SEG_F,                           // C
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,                   // D
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,                   // E
  SEG_A | SEG_E | SEG_F | SEG_G,                           // F
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F,                   // G
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,                   // H
  SEG_B | SEG_C,                                           // I
  SEG_B | SEG_C | SEG_D | SEG_E,                           // J
  SEG_A | SEG_C | SEG_E | SEG_F | SEG_G,                   // K
  SEG_D | SEG_E | SEG_F,                                   // L
  SEG_A | SEG_C | SEG_E,                                   // M
  SEG_C | SEG_E | SEG_G,                                   // N
  SEG_C | SEG_D | SEG_E | SEG_G,                           // O
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,                   // P
  SEG_A | SEG_B | SEG_C | SEG_F | SEG_G,                   // Q
  SEG_E | SEG_G,                                           // R
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                   // S
  SEG_D | SEG_E | SEG_F | SEG_G,                           // T
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,                   // U
  SEG_C | SEG_D | SEG_E,                                   // V
  SEG_B | SEG_D | SEG_F,                                   // W
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,                   // X
  SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,                   // Y
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                   // Z   90
  SEG_A | SEG_D | SEG_E | SEG_F,                           // [   91
  SEG_C | SEG_F | SEG_G,                                   /* \   92 backslash*/
  SEG_A | SEG_B | SEG_C | SEG_D,                           // ]   93
  SEG_A | SEG_B | SEG_G | SEG_F,                           // ^   94 ESPEasy change: ° degree symbol, like P073 7-digit fonts
  SEG_D,                                                   // _   95 underscore
  SEG_B,                                                   // `   96
  SEG_C | SEG_D | SEG_E | SEG_G | SEG_DP,                  // a   97
  SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,                   // b   98
  SEG_D | SEG_E | SEG_G,                                   // c   99
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,                   // d   100
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,                   // e   101
  SEG_A | SEG_E | SEG_F | SEG_G,                           // f   102
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F,                   // g G 103 capital letter will be used
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,                   // h   104
  SEG_C,                                                   // i   105
  SEG_C | SEG_D,                                           // j   106
  SEG_A | SEG_C | SEG_E | SEG_F | SEG_G,                   // k   107
  SEG_E | SEG_F,                                           // l   108
  SEG_A | SEG_C | SEG_E,                                   // m n 109 n will be used
  SEG_C | SEG_E | SEG_G,                                   // n   110
  SEG_C | SEG_D | SEG_E | SEG_G,                           // o   111
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,                   // p P 112
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_DP,          // q Q 113
  SEG_E | SEG_G,                                           // r   114
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                   // s S 115
  SEG_D | SEG_E | SEG_F | SEG_G,                           // t   116
  SEG_C | SEG_D | SEG_E,                                   // u   117
  SEG_C | SEG_D | SEG_E,                                   // v u 118 u will be used
  SEG_C | SEG_D | SEG_E,                                   // w u 119 u will be used
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,                   // x   120
  SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,                   // y Y 121
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                   // z Z 122
  SEG_A | SEG_D | SEG_E | SEG_F,                           // {   123
  SEG_B | SEG_C,                                           // |   124
  SEG_A | SEG_B | SEG_C | SEG_D,                           // }   125
  SEG_G,                                                   // ~   126
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_DP,  //   127 all segments (255)
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,                   // €   128 Euro (121) added for ESPEasy
  SEG_A | SEG_B | SEG_G | SEG_F                            // °   129 degree (99) added for ESPEasy
};
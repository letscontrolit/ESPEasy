/* *******************************************************************
  HT16K33 Library - Fontfile
  copyright (c) noiasca
  
  Version 2021-03-14
  
  This is the original 14 segment font as of library Version 1.0.2 from 2020-06-07
  
 * ******************************************************************/
 
#pragma once
#define FONTFILE14 legacy              // include that to each fontfile as additional identifier
#ifndef FONTFILE_WAS_INCLUDED
#define FONTFILE_WAS_INCLUDED
#endif
 
/* *******************************************************************
   Characterset for 14 segment
 * ******************************************************************/

const uint16_t charTable14 [] PROGMEM  = {
  0,                                                                                     //     32   space
  SEG14_B | SEG14_C | SEG14_DP,                                                          // !   33
  SEG14_B | SEG14_L,                                                                     // "   34
  0,                                                                                     // #   35
  SEG14_A | SEG14_C | SEG14_D | SEG14_F | SEG14_G,                                       // $   36
  SEG14_A | SEG14_B | SEG14_F | SEG14_G | SEG14_H,                                       // %   37
  0,                                                                                     // &   38
  SEG14_L,                                                                               // '   39
  SEG14_M | SEG14_P,                                                                     // (   40
  SEG14_K | SEG14_N,                                                                     // )   41
  SEG14_G | SEG14_H | SEG14_M | SEG14_P | SEG14_N | SEG14_K,                             // *   42
  SEG14_G | SEG14_H | SEG14_O | SEG14_L,                                                 // +   43
  0,                                                                                     // ,   44   will be handled in the write methode
  SEG14_H,                                                                               // -   45
  0,                                                                                     // .   46   will be handled in the write methode
  SEG14_M | SEG14_N,                                                                     // /   47
  SEG14_A | SEG14_B | SEG14_C | SEG14_D | SEG14_E | SEG14_F,                             // 0   48
  SEG14_B | SEG14_C | SEG14_M,                                                           // 1   49
  SEG14_A | SEG14_B | SEG14_D | SEG14_E | SEG14_G | SEG14_H,                             // 2   50
  SEG14_A | SEG14_B | SEG14_C | SEG14_D | SEG14_H,                                       // 3   51
  SEG14_B | SEG14_C | SEG14_F | SEG14_G | SEG14_H,                                       // 4   52
  SEG14_A | SEG14_C | SEG14_D | SEG14_F | SEG14_G | SEG14_H,                             // 5   53
  SEG14_A | SEG14_C | SEG14_D | SEG14_E | SEG14_F | SEG14_G | SEG14_H,                   // 6   54
  SEG14_A | SEG14_M | SEG14_N,                                                           // 7   55
  SEG14_A | SEG14_B | SEG14_C | SEG14_D | SEG14_E | SEG14_F | SEG14_G | SEG14_H,         // 8   56
  SEG14_A | SEG14_B | SEG14_C | SEG14_D | SEG14_F | SEG14_G | SEG14_H,                   // 9   57
  0,                                                                                     // :   58   will be handled in the write methode
  0,                                                                                     // ;   59   will be handled in the write methode
  SEG14_M | SEG14_P,                                                                     // <   60
  SEG14_G | SEG14_H,                                                                     // =   61
  SEG14_K | SEG14_N,                                                                     // >   62
  SEG14_A | SEG14_B | SEG14_H | SEG14_O | SEG14_DP,                                      // ?   63
  SEG14_D | SEG14_E | SEG14_G | SEG14_O | SEG14_N,                                       // @   64
  SEG14_A | SEG14_B | SEG14_C | SEG14_E | SEG14_F | SEG14_G | SEG14_H,                   // A   65
  SEG14_A | SEG14_B | SEG14_C | SEG14_D | SEG14_H | SEG14_L | SEG14_O,                   // B
  SEG14_A | SEG14_D | SEG14_E | SEG14_F,                                                 // C
  SEG14_A | SEG14_B | SEG14_C | SEG14_D | SEG14_L | SEG14_O,                             // D
  SEG14_A | SEG14_D | SEG14_E | SEG14_F | SEG14_G,                                       // E
  SEG14_A | SEG14_E | SEG14_F | SEG14_G,                                                 // F
  SEG14_A | SEG14_C | SEG14_D | SEG14_E | SEG14_F  | SEG14_H,                            // G
  SEG14_B | SEG14_C | SEG14_E | SEG14_F | SEG14_G  | SEG14_H,                            // H
  SEG14_A | SEG14_D | SEG14_L | SEG14_O,                                                 // I
  SEG14_B | SEG14_C | SEG14_D | SEG14_E,                                                 // J
  SEG14_E | SEG14_F | SEG14_G | SEG14_M | SEG14_P,                                       // K
  SEG14_D | SEG14_E | SEG14_F,                                                           // L
  SEG14_B | SEG14_C | SEG14_E | SEG14_F | SEG14_K | SEG14_M,                             // M
  SEG14_B | SEG14_C | SEG14_E | SEG14_F | SEG14_K | SEG14_P,                             // N
  SEG14_A | SEG14_B | SEG14_C | SEG14_D | SEG14_E | SEG14_F,                             // O
  SEG14_A | SEG14_B | SEG14_E | SEG14_F | SEG14_G | SEG14_H,                             // P
  SEG14_A | SEG14_B | SEG14_C | SEG14_D | SEG14_E | SEG14_F | SEG14_P,                   // Q
  SEG14_A | SEG14_B | SEG14_E | SEG14_F | SEG14_G | SEG14_H | SEG14_P,                   // R
  SEG14_A | SEG14_C | SEG14_D | SEG14_H | SEG14_K,                                       // S
  SEG14_A | SEG14_L | SEG14_O,                                                           // T
  SEG14_B | SEG14_C | SEG14_D | SEG14_E | SEG14_F,                                       // U
  SEG14_E | SEG14_F | SEG14_M | SEG14_N,                                                 // V
  SEG14_B | SEG14_C | SEG14_E | SEG14_F | SEG14_N | SEG14_P,                             // W
  SEG14_K | SEG14_M | SEG14_N | SEG14_P,                                                 // X
  SEG14_B | SEG14_F | SEG14_G | SEG14_H | SEG14_O,                                       // Y
  SEG14_A | SEG14_D | SEG14_M | SEG14_N,                                                 // Z   90
  SEG14_A | SEG14_D | SEG14_E | SEG14_F,                                                 // [   91
  SEG14_K | SEG14_P,                                                                     /* \   92 backslash*/
  SEG14_A | SEG14_B | SEG14_C | SEG14_D,                                                 // ]   93
  SEG14_F | SEG14_K,                                                                     // ^   94
  SEG14_D,                                                                               // _   95 underscore
  SEG14_K,                                                                               // `   96
  SEG14_D | SEG14_E | SEG14_G | SEG14_O,                                                 // a   97
  SEG14_D | SEG14_E | SEG14_F | SEG14_G | SEG14_P,                                       // b
  SEG14_D | SEG14_E | SEG14_G | SEG14_H,                                                 // c
  SEG14_B | SEG14_C | SEG14_D | SEG14_H | SEG14_N,                                       // d
  SEG14_D | SEG14_E | SEG14_G | SEG14_N,                                                 // e
  SEG14_A | SEG14_E | SEG14_F | SEG14_G,                                                 // f
  SEG14_B | SEG14_C | SEG14_D | SEG14_H | SEG14_M,                                       // g
  SEG14_E | SEG14_F | SEG14_G | SEG14_O,                                                 // h
  SEG14_O,                                                                               // i
  SEG14_B | SEG14_C | SEG14_D,                                                           // j
  SEG14_L | SEG14_M | SEG14_O | SEG14_P,                                                 // k
  SEG14_L | SEG14_O,                                                                     // l
  SEG14_C | SEG14_E | SEG14_G | SEG14_H | SEG14_O,                                       // m
  SEG14_E | SEG14_G | SEG14_O,                                                           // n
  SEG14_C | SEG14_D | SEG14_E | SEG14_G | SEG14_H,                                       // o
  SEG14_A | SEG14_E | SEG14_F | SEG14_G | SEG14_M,                                       // p
  SEG14_A | SEG14_B | SEG14_C | SEG14_H | SEG14_K,                                       // q
  SEG14_E | SEG14_G,                                                                     // r
  SEG14_D | SEG14_H | SEG14_P,                                                           // s
  SEG14_D | SEG14_E | SEG14_F | SEG14_G,                                                 // t
  SEG14_C | SEG14_D | SEG14_E,                                                           // u
  SEG14_E | SEG14_N,                                                                     // v
  SEG14_C | SEG14_E | SEG14_N | SEG14_P,                                                 // w
  SEG14_K | SEG14_M | SEG14_N | SEG14_P,                                                 // x
  SEG14_B | SEG14_C | SEG14_D | SEG14_H | SEG14_L,                                       // y Y
  SEG14_D | SEG14_G | SEG14_N,                                                           // z   122
  SEG14_A | SEG14_D | SEG14_G | SEG14_N | SEG14_K,                                       // {   123  
  SEG14_B | SEG14_C,                                                                     // |   124
  SEG14_A | SEG14_D | SEG14_H | SEG14_P | SEG14_M,                                       // }   125  
  SEG14_G,                                                                               // ~   126
  SEG14_P | SEG14_O | SEG14_N | SEG14_M | SEG14_L | SEG14_K | SEG14_H | SEG14_G | SEG14_F | SEG14_E | SEG14_D | SEG14_C | SEG14_B | SEG14_A | SEG14_DP// 127 all Segments
};

 
 
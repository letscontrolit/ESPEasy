// ---------------------------------------------------------------------------
// AUTHOR/LICENSE:
//  The following code was written by Antoine Beauchamp. For other authors, see AUTHORS file.
//  The code & updates for the library can be found at https://github.com/end2endzone/AnyRtttl
//  MIT License: http://www.opensource.org/licenses/mit-license.php
// ---------------------------------------------------------------------------

#ifndef PITCHES_H
#define PITCHES_H

/*************************************************
 * Notes Constants
 *************************************************/
#define NOTE_REST   0
#define NOTE_SILENT 0

#define NOTE_C0  16
#define NOTE_CS0 17
#define NOTE_D0  18
#define NOTE_DS0 19
#define NOTE_E0  21
#define NOTE_F0  22
#define NOTE_FS0 23
#define NOTE_G0  24
#define NOTE_GS0 26
#define NOTE_A0  28
#define NOTE_AS0 29
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define NOTE_E8  5274
#define NOTE_F8  5588
#define NOTE_FS8 5920
#define NOTE_G8  6272
#define NOTE_GS8 6645
#define NOTE_A8  7040
#define NOTE_AS8 7459
#define NOTE_B8  7902
#define NOTE_C9  8372
#define NOTE_CS9 8870
#define NOTE_D9  9397
#define NOTE_DS9 9956
#define NOTE_E9  10548
#define NOTE_F9  11175
#define NOTE_FS9 11840
#define NOTE_G9  12544
#define NOTE_GS9 13290
#define NOTE_A9   ?
#define NOTE_AS9  ?
#define NOTE_B9   ?

//Duplicated note with same frequency.
//Obtained from the following code:
//  for(char o='0'; o<='9'; o++)
//  {
//    for(char letter='A'; letter<='G'; letter++)
//    {
//      //     #define NOTE_DB7 NOTE_CS7
//      printf("#define NOTE_%cB%c NOTE_%cS%c\n", (letter+1 == 'H'? 'A' : letter+1), o, letter, o);
//    }
//  }
#define NOTE_BB0 NOTE_AS0
#define NOTE_CB0 NOTE_BS0
#define NOTE_DB0 NOTE_CS0
#define NOTE_EB0 NOTE_DS0
#define NOTE_FB0 NOTE_ES0
#define NOTE_GB0 NOTE_FS0
#define NOTE_AB0 NOTE_GS0
#define NOTE_BB1 NOTE_AS1
#define NOTE_CB1 NOTE_BS1
#define NOTE_DB1 NOTE_CS1
#define NOTE_EB1 NOTE_DS1
#define NOTE_FB1 NOTE_ES1
#define NOTE_GB1 NOTE_FS1
#define NOTE_AB1 NOTE_GS1
#define NOTE_BB2 NOTE_AS2
#define NOTE_CB2 NOTE_BS2
#define NOTE_DB2 NOTE_CS2
#define NOTE_EB2 NOTE_DS2
#define NOTE_FB2 NOTE_ES2
#define NOTE_GB2 NOTE_FS2
#define NOTE_AB2 NOTE_GS2
#define NOTE_BB3 NOTE_AS3
#define NOTE_CB3 NOTE_BS3
#define NOTE_DB3 NOTE_CS3
#define NOTE_EB3 NOTE_DS3
#define NOTE_FB3 NOTE_ES3
#define NOTE_GB3 NOTE_FS3
#define NOTE_AB3 NOTE_GS3
#define NOTE_BB4 NOTE_AS4
#define NOTE_CB4 NOTE_BS4
#define NOTE_DB4 NOTE_CS4
#define NOTE_EB4 NOTE_DS4
#define NOTE_FB4 NOTE_ES4
#define NOTE_GB4 NOTE_FS4
#define NOTE_AB4 NOTE_GS4
#define NOTE_BB5 NOTE_AS5
#define NOTE_CB5 NOTE_BS5
#define NOTE_DB5 NOTE_CS5
#define NOTE_EB5 NOTE_DS5
#define NOTE_FB5 NOTE_ES5
#define NOTE_GB5 NOTE_FS5
#define NOTE_AB5 NOTE_GS5
#define NOTE_BB6 NOTE_AS6
#define NOTE_CB6 NOTE_BS6
#define NOTE_DB6 NOTE_CS6
#define NOTE_EB6 NOTE_DS6
#define NOTE_FB6 NOTE_ES6
#define NOTE_GB6 NOTE_FS6
#define NOTE_AB6 NOTE_GS6
#define NOTE_BB7 NOTE_AS7
#define NOTE_CB7 NOTE_BS7
#define NOTE_DB7 NOTE_CS7
#define NOTE_EB7 NOTE_DS7
#define NOTE_FB7 NOTE_ES7
#define NOTE_GB7 NOTE_FS7
#define NOTE_AB7 NOTE_GS7
#define NOTE_BB8 NOTE_AS8
#define NOTE_CB8 NOTE_BS8
#define NOTE_DB8 NOTE_CS8
#define NOTE_EB8 NOTE_DS8
#define NOTE_FB8 NOTE_ES8
#define NOTE_GB8 NOTE_FS8
#define NOTE_AB8 NOTE_GS8
#define NOTE_BB9 NOTE_AS9
#define NOTE_CB9 NOTE_BS9
#define NOTE_DB9 NOTE_CS9
#define NOTE_EB9 NOTE_DS9
#define NOTE_FB9 NOTE_ES9
#define NOTE_GB9 NOTE_FS9
#define NOTE_AB9 NOTE_GS9

#endif //PITCHES_H

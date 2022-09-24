#ifndef __SAM_TABS_H__
#define __SAM_TABS_H__

// tab40672
extern unsigned char stressInputTable[];
extern unsigned char signInputTable1[];
extern unsigned char signInputTable2[];
extern unsigned short flags[];
// loc_9F8C
enum
{
	FLAG_PLOSIVE = 0x0001,
	FLAG_STOPCONS = 0x0002, /* stop consonant */
	FLAG_VOICED = 0x0004,
	/* 0x08 */
	FLAG_DIPTHONG = 0x0010,
	FLAG_DIP_YX = 0x0020, /* dipthong ending with YX */
	FLAG_CONSONANT = 0x0040,
	FLAG_VOWEL = 0x0080,
	FLAG_PUNCT = 0x0100,
	/* 0x200 */
	FLAG_ALVEOLAR = 0x0400,
	FLAG_NASAL = 0x0800,
	FLAG_LIQUIC = 0x1000, /* liquic consonant */
	FLAG_FRICATIVE = 0x2000
};
extern int timetable[5][5];
// tab45616???
extern unsigned char phonemeStressedLengthTable[];
// tab45536???
extern unsigned char phonemeLengthTable[];

extern unsigned char inBlendLength[];
extern unsigned char outBlendLength[];
extern unsigned char blendRank[];

/*

Ind  | phoneme |  flags   |
-----|---------|----------|
0    |   *     | 00000000 |
1    |  .*     | 00000000 |
2    |  ?*     | 00000000 |
3    |  ,*     | 00000000 |
4    |  -*     | 00000000 |

VOWELS
5    |  IY     | 10100100 |
6    |  IH     | 10100100 |
7    |  EH     | 10100100 |
8    |  AE     | 10100100 |
9    |  AA     | 10100100 |
10   |  AH     | 10100100 |
11   |  AO     | 10000100 |
17   |  OH     | 10000100 |
12   |  UH     | 10000100 |
16   |  UX     | 10000100 |
15   |  ER     | 10000100 |
13   |  AX     | 10100100 |
14   |  IX     | 10100100 |

DIPTHONGS
48   |  EY     | 10110100 |
49   |  AY     | 10110100 |
50   |  OY     | 10110100 |
51   |  AW     | 10010100 |
52   |  OW     | 10010100 |
53   |  UW     | 10010100 |


21   |  YX     | 10000100 |
20   |  WX     | 10000100 |
18   |  RX     | 10000100 |
19   |  LX     | 10000100 |
37   |  /X     | 01000000 |
30   |  DX     | 01001000 |


22   |  WH     | 01000100 |


VOICED CONSONANTS
23   |  R*     | 01000100 |
24   |  L*     | 01000100 |
25   |  W*     | 01000100 |
26   |  Y*     | 01000100 |
27   |  M*     | 01001100 |
28   |  N*     | 01001100 |
29   |  NX     | 01001100 |
54   |  B*     | 01001110 |
57   |  D*     | 01001110 |
60   |  G*     | 01001110 |
44   |  J*     | 01001100 |
38   |  Z*     | 01000100 |
39   |  ZH     | 01000100 |
40   |  V*     | 01000100 |
41   |  DH     | 01000100 |

unvoiced CONSONANTS
32   |  S*     | 01000000 |
33   |  SH     | 01000000 |
34   |  F*     | 01000000 |
35   |  TH     | 01000000 |
66   |  P*     | 01001011 |
69   |  T*     | 01001011 |
72   |  K*     | 01001011 |
42   |  CH     | 01001000 |
36   |  /H     | 01000000 |

43   |  **     | 01000000 |
45   |  **     | 01000100 |
46   |  **     | 00000000 |
47   |  **     | 00000000 |


55   |  **     | 01001110 |
56   |  **     | 01001110 |
58   |  **     | 01001110 |
59   |  **     | 01001110 |
61   |  **     | 01001110 |
62   |  **     | 01001110 |
63   |  GX     | 01001110 |
64   |  **     | 01001110 |
65   |  **     | 01001110 |
67   |  **     | 01001011 |
68   |  **     | 01001011 |
70   |  **     | 01001011 |
71   |  **     | 01001011 |
73   |  **     | 01001011 |
74   |  **     | 01001011 |
75   |  KX     | 01001011 |
76   |  **     | 01001011 |
77   |  **     | 01001011 |


SPECIAL
78   |  UL     | 10000000 |
79   |  UM     | 11000001 |
80   |  UN     | 11000001 |
31   |  Q*     | 01001100 |

*/

#endif // __SAM_TABS_H__
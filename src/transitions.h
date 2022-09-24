#ifndef __TRANSITIONS_H__
#define __TRANSITIONS_H__

#include "sam.h"
unsigned char ReadTableValue(SAMContext *ctx, unsigned char p, unsigned char Y);
void WriteTableValue(SAMContext *ctx, unsigned char p, unsigned char Y, unsigned char value);
void Interpolate(SAMContext *ctx, unsigned char width, unsigned char table, unsigned char frame, char diff);
unsigned char CreateTransitions(SAMContext *ctx);


enum
{
    TBL_PITCH = 168,
    TBL_FREQ1 = 169,
    TBL_FREQ2 = 170,
    TBL_FREQ3 = 171,
    TBL_AMP1 = 172,
    TBL_AMP2 = 173,
    TBL_AMP3 = 174,
} WhichTable;

#endif // __TRANSITIONS_H__
#ifndef __RECITER_H__
#define __RECITER_H__
#include "sam.h"
unsigned int IsNextInput(SAMContext *ctx, const char *str);
unsigned char Code37055(SAMContext *ctx, unsigned char npos, unsigned char mask);
int handle_ch(SAMContext *ctx, unsigned char ch, unsigned char mem);
int handle_ch2(SAMContext *ctx, unsigned char ch, unsigned char mem);
int TextToPhonemes(SAMContext *ctx, unsigned char *input);
#endif // __RECITER_H__
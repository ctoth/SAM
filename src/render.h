#ifndef __RENDER_H__
#define __RENDER_H__

#include "sam.h"

#define PHONEME_PERIOD (1)
#define PHONEME_QUESTION (2)

#define RISING_INFLECTION (1)
#define FALLING_INFLECTION (256)

unsigned char MultiplyAndShift(unsigned char a, unsigned char b);

void SAMSetMouthThroat(SAMContext *ctx);
void AddInflection(SAMContext *ctx, unsigned char inflection, unsigned char pos);
void RenderSample(SAMContext *ctx, unsigned char *out, unsigned char consonantFlag, unsigned char phonemeIndex);
void WriteToBuf(SAMContext *ctx, int index, unsigned char A);
int RenderAudio(SAMContext *ctx);
// audio buffer struct

typedef struct
{
    unsigned char *buffer;
    int bufferpos;
} AudioBuffer;

#endif // __RENDER_H__
#ifndef __RECITER_H__
#define __RECITER_H__

typedef struct SAMReciterContext
{
    unsigned char reciterInput[256];
    unsigned char workingChar; // The current char which is often mutated
    unsigned char reciterIndex;
} SAMReciterContext;

unsigned int IsNextInput(SAMReciterContext *ctx, const char *str);
unsigned char Code37055(SAMReciterContext *ctx, unsigned char npos, unsigned char mask);
int handle_ch(SAMReciterContext *ctx, unsigned char ch, unsigned char mem);
int handle_ch2(SAMReciterContext *ctx, unsigned char ch, unsigned char mem);
int TextToPhonemes(unsigned char *input);

#endif // __RECITER_H__
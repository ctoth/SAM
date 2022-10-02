#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>

#include "phoneme_parser.h"
#include "reciter.h"
#include "render.h"
#include "rules.h"
#include "sam.h"
#include "sam_tabs.h"

void PrintPhonemes(SAMContext *ctx)
{
    int i = 0;
    printf("===========================================\n");

    printf("Internal Phoneme presentation:\n\n");
    printf(" idx    phoneme  length  stress\n");
    printf("------------------------------\n");

    while ((ctx->phonemeindex[i] != END) && (i < 255))
    {
        if (ctx->phonemeindex[i] < 81)
        {
            printf(" %3i      %c%c      %3i       %i\n",
                   ctx->phonemeindex[i],
                   signInputTable1[ctx->phonemeindex[i]],
                   signInputTable2[ctx->phonemeindex[i]],
                   ctx->phonemeLength[i],
                   ctx->stress[i]);
        }
        else
        {
            printf(" %3i      ??      %3i       %i\n", ctx->phonemeindex[i], ctx->phonemeLength[i], ctx->stress[i]);
        }
        i++;
    }
    printf("===========================================\n");
    printf("\n");
}

void SAMInit(SAMContext *ctx)
{
    int i;
    SAMSetMouthThroat(ctx);

    ctx->bufferpos = 0;

    ctx->buffer = malloc(22050 * 10);

    for (i = 0; i < 256; i++)
    {
        ctx->stress[i] = 0;
        ctx->phonemeLength[i] = 0;
    }

    for (i = 0; i < 60; i++)
    {
        ctx->phonemeIndexOutput[i] = 0;
        ctx->stressOutput[i] = 0;
        ctx->phonemeLengthOutput[i] = 0;
    }
    ctx->phonemeindex[255] = END; // to prevent buffer overflow // ML : changed from 32 to 255 to stop freezing with long inputs
    // ctx->phonemeindex[255] = 32;  // to prevent buffer overflow
}

void SAMSpeak(SAMUtterance *toSpeak)
{
    printf("input: %s\n", toSpeak->input);
    SAMContext ctx;
    ctx.toSpeak = *toSpeak;
    // make a copy of the input string to pass to get phonemes out of
    // this is because the input string is modified by the phoneme parser
    char inputCopy[256];
    memset(inputCopy, 155, 256);
    strcpy(inputCopy, toSpeak->input);
    // uppercase the copy
    for (int i = 0; i < strlen(inputCopy); i++)
    {
        inputCopy[i] = toupper(inputCopy[i]);
    }
    TextToPhonemes(&ctx, inputCopy);
    ctx.toSpeak.input = inputCopy;
    SAMInit(&ctx);
    ParsePhonemes(&ctx);
    ApplyRules(&ctx);
    CopyStressToPrevVowel(&ctx);
    SetPhonemeLengths(&ctx);
    AdjustPhonemeLengths(&ctx);
    AddVoicedStopConsonants(&ctx);
    CheckPhonemes(&ctx);
    InsertBreath(&ctx);
    // PrintPhonemes(&ctx);
    PrepareOutput(&ctx);

    // if we were passed a callback on toSpeak, call it now with our buffer-
    if (toSpeak->finished_callback)
        toSpeak->finished_callback(toSpeak->userdata, ctx.buffer, ctx.bufferpos);

    SAMFree(&ctx);
}

void SAMFree(SAMContext *ctx)
{
    free(ctx->buffer);
}

void PrepareOutput(SAMContext *ctx)
{
    unsigned char srcpos = 0;  // Position in source
    unsigned char destpos = 0; // Position in output

    while (1)
    {
        unsigned char A = ctx->phonemeindex[srcpos];
        ctx->phonemeIndexOutput[destpos] = A;
        switch (A)
        {
        case END:
            RenderAudio(ctx);
            return;
        case BREAK:
            ctx->phonemeIndexOutput[destpos] = END;
            RenderAudio(ctx);
            destpos = 0;
            break;
        case 0:
            break;
        default:
            ctx->phonemeLengthOutput[destpos] = ctx->phonemeLength[srcpos];
            ctx->stressOutput[destpos] = ctx->stress[srcpos];
            ++destpos;
            break;
        }
        ++srcpos;
    }
}

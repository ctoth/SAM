#include "sam.h"
#include "sam_tabs.h"
#include "transitions.h"

#include <math.h>

unsigned char ReadTableValue(SAMContext *ctx, unsigned char whichTable, unsigned char Y)
{
    switch (whichTable)
    {
    case TBL_PITCH:
        return ctx->pitches[Y];
    case TBL_FREQ1:
        return ctx->frequency1[Y];
    case TBL_FREQ2:
        return ctx->frequency2[Y];
    case TBL_FREQ3:
        return ctx->frequency3[Y];
    case TBL_AMP1:
        return ctx->amplitude1[Y];
    case TBL_AMP2:
        return ctx->amplitude2[Y];
    case TBL_AMP3:
        return ctx->amplitude3[Y];
    default:
        return 0;
    }
}

void WriteTableValue(SAMContext *ctx, unsigned char whichTable, unsigned char Y, unsigned char value)
{
    switch (whichTable)
    {
    case TBL_PITCH:
        ctx->pitches[Y] = value;
        return;
    case TBL_FREQ1:
        ctx->frequency1[Y] = value;
        return;
    case TBL_FREQ2:
        ctx->frequency2[Y] = value;
        return;
    case TBL_FREQ3:
        ctx->frequency3[Y] = value;
        return;
    case TBL_AMP1:
        ctx->amplitude1[Y] = value;
        return;
    case TBL_AMP2:
        ctx->amplitude2[Y] = value;
        return;
    case TBL_AMP3:
        ctx->amplitude3[Y] = value;
        return;
    default:

        return;
    }
}

// linearly interpolate values
void Interpolate(SAMContext *ctx, unsigned char width, unsigned char table, unsigned char frame, char diff)
{
    unsigned char sign = (diff < 0);
    unsigned char remainder = abs(diff) % width;
    unsigned char div = diff / width;

    unsigned char error = 0;
    unsigned char pos = width;
    unsigned char val = ReadTableValue(ctx, table, frame) + div;

    while (--pos)
    {
        error += remainder;
        if (error >= width)
        { // accumulated a whole integer error, so adjust output
            error -= width;
            if (sign)
                val--;
            else if (val)
                val++; // if input is 0, we always leave it alone
        }
        WriteTableValue(ctx, table, ++frame, val); // Write updated value back to next frame.
        val += div;
    }
}

void InterpolatePitch(SAMContext *ctx, unsigned char pos, unsigned char mem49, unsigned char frame)
{
    // unlike the other values, the pitches[] interpolates from
    // the middle of the current phoneme to the middle of the
    // next phoneme

    // half the width of the current and next phoneme
    unsigned char cur_width = ctx->phonemeLengthOutput[pos] / 2;
    unsigned char next_width = ctx->phonemeLengthOutput[pos + 1] / 2;
    // sum the values
    unsigned char width = cur_width + next_width;
    char pitch = ctx->pitches[next_width + mem49] - ctx->pitches[mem49 - cur_width];
    Interpolate(ctx, width, TBL_PITCH, frame, pitch);
}

unsigned char CreateTransitions(SAMContext *ctx)
{
    unsigned char mem49 = 0;
    unsigned char pos = 0;
    while (1)
    {
        unsigned char next_rank;
        unsigned char rank;
        unsigned char speedcounter;
        unsigned char phase1;
        unsigned char phase2;
        unsigned char phase3;
        unsigned char transition;

        unsigned char phoneme = ctx->phonemeIndexOutput[pos];
        unsigned char next_phoneme = ctx->phonemeIndexOutput[pos + 1];

        if (next_phoneme == 255)
            break; // 255 == end_token

        // get the ranking of each phoneme
        next_rank = blendRank[next_phoneme];
        rank = blendRank[phoneme];

        // compare the rank - lower rank value is stronger
        if (rank == next_rank)
        {
            // same rank, so use out blend lengths from each phoneme
            phase1 = outBlendLength[phoneme];
            phase2 = outBlendLength[next_phoneme];
        }
        else if (rank < next_rank)
        {
            // next phoneme is stronger, so us its blend lengths
            phase1 = inBlendLength[next_phoneme];
            phase2 = outBlendLength[next_phoneme];
        }
        else
        {
            // current phoneme is stronger, so use its blend lengths
            // note the out/in are swapped
            phase1 = outBlendLength[phoneme];
            phase2 = inBlendLength[phoneme];
        }

        mem49 += ctx->phonemeLengthOutput[pos];

        speedcounter = mem49 + phase2;
        phase3 = mem49 - phase1;
        transition = phase1 + phase2; // total transition?

        if (((transition - 2) & 128) == 0)
        {
            unsigned char table = 169;
            InterpolatePitch(ctx, pos, mem49, phase3);
            while (table < 175)
            {
                // tables:
                // 168  pitches[]
                // 169  frequency1
                // 170  frequency2
                // 171  frequency3
                // 172  amplitude1
                // 173  amplitude2
                // 174  amplitude3

                char value = ReadTableValue(ctx, table, speedcounter) - ReadTableValue(ctx, table, phase3);
                Interpolate(ctx, transition, table, phase3, value);
                table++;
            }
        }
        ++pos;
    }

    // add the length of this phoneme
    return mem49 + ctx->phonemeLengthOutput[pos];
}

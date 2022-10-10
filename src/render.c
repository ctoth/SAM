#include "render.h"
#include "sam.h"
#include "render_tabs.h"
#include "sam_tabs.h"
#include "transitions.h"
#include <string.h>

void WriteToBuf(SAMContext *ctx, int index, unsigned char A)
{

    int k;
    ctx->bufferpos += timetable[ctx->oldtimetableindex][index];
    ctx->oldtimetableindex = index;
    // write a little bit in advance
    for (k = 0; k < 5; k++)
        ctx->buffer[ctx->bufferpos / 50 + k] = (A & 15) * 16;
}

unsigned char MultiplyAndShift(unsigned char a, unsigned char b)
{
    return (((unsigned int)a * b) >> 8) << 1;
}

void SAMSetMouthThroat(SAMContext *ctx)
{
    // mouth formants (F1) 5..29
    static const unsigned char mouthFormants5_29[30] = {
        0, 0, 0, 0, 0, 10,
        14, 19, 24, 27, 23, 21, 16, 20, 14, 18, 14, 18, 18,
        16, 13, 15, 11, 18, 14, 11, 9, 6, 6, 6};

    // throat formants (F2) 5..29
    static const unsigned char throatFormants5_29[30] = {
        255,
        255,
        255,
        255,
        255,
        84,
        73,
        67,
        63,
        40,
        44,
        31,
        37,
        45,
        73,
        49,
        36,
        30,
        51,
        37,
        29,
        69,
        24,
        50,
        30,
        24,
        83,
        46,
        54,
        86,
    };

    // there must be no zeros in these 2 tables
    // formant 1 frequencies (mouth) 48..53
    static const unsigned char mouthFormants48_53[6] = {19, 27, 21, 27, 18, 13};

    // formant 2 frequencies (throat) 48..53
    static const unsigned char throatFormants48_53[6] = {72, 39, 31, 43, 30, 34};

    // copy default values into our context
    // freq1data and freq2data
    memcpy(&ctx->freq1data, freq1data, sizeof(freq1data));
    memcpy(&ctx->freq2data, freq2data, sizeof(freq2data));
    unsigned char newFrequency = 0;
    unsigned char pos = 5;

    // recalculate formant frequencies 5..29 for the mouth (F1) and throat (F2)
    while (pos < 30)
    {
        // recalculate mouth frequency
        unsigned char initialFrequency = mouthFormants5_29[pos];
        if (initialFrequency != 0)
        {
            newFrequency = MultiplyAndShift(ctx->toSpeak.mouth, initialFrequency);
        }
        ctx->freq1data[pos] = newFrequency;

        // recalculate throat frequency
        initialFrequency = throatFormants5_29[pos];
        if (initialFrequency != 0)
        {
            newFrequency = MultiplyAndShift(ctx->toSpeak.throat, initialFrequency);
        }
        ctx->freq2data[pos] = newFrequency;
        pos++;
    }

    // recalculate formant frequencies 48..53
    pos = 0;
    while (pos < 6)
    {
        // recalculate F1 (mouth formant)
        unsigned char initialFrequency = mouthFormants48_53[pos];
        ctx->freq1data[pos + 48] = MultiplyAndShift(ctx->toSpeak.mouth, initialFrequency);

        // recalculate F2 (throat formant)
        initialFrequency = throatFormants48_53[pos];
        ctx->freq2data[pos + 48] = MultiplyAndShift(ctx->toSpeak.throat, initialFrequency);
        pos++;
    }
}

// Create a rising or falling inflection 30 frames prior to
// index X. A rising inflection is used for questions, and
// a falling inflection is used for statements.

void AddInflection(SAMContext *ctx, unsigned char inflection, unsigned char pos)
{
    unsigned char A;
    // store the location of the punctuation
    unsigned char end = pos;

    if (pos < 30)
        pos = 0;
    else
        pos -= 30;

    // FIXME: Explain this fix better, it's not obvious
    // ML : A =, fixes a problem with invalid pitch with '.'
    while ((A = ctx->pitches[pos]) == 127)
        ++pos;

    while (pos != end)
    {
        // add the inflection direction
        A += inflection;

        // set the inflection
        ctx->pitches[pos] = A;

        while ((++pos != end) && ctx->pitches[pos] == 255)
            ;
    }
}

// CREATE FRAMES
//
// The length parameter in the list corresponds to the number of frames
// to expand the phoneme to. Each frame represents 10 milliseconds of time.
// So a phoneme with a length of 7 = 7 frames = 70 milliseconds duration.
//
// The parameters are copied from the phoneme to the frame verbatim.
//

void CreateFrames(SAMContext *ctx)
{
    unsigned char X = 0;
    unsigned int index = 0;
    while (index < 256)
    {
        // get the phoneme at the index
        unsigned char phoneme = ctx->phonemeIndexOutput[index];
        unsigned char stressPitch;
        signed frameCount;

        // if terminal phoneme, exit the loop
        if (phoneme == 255)
            break;

        if (phoneme == PHONEME_PERIOD)
            AddInflection(ctx, RISING_INFLECTION, X);
        else if (phoneme == PHONEME_QUESTION)
            AddInflection(ctx, FALLING_INFLECTION, X);

        // get the stress amount (more stress = higher pitch)
        stressPitch = stressPitchesTable[ctx->stressOutput[index] + 1];

        // get number of frames to write
        frameCount = ctx->phonemeLengthOutput[index];

        // copy from the source to the frames list

        do
        {
            ctx->frequency1[X] = freq1data[phoneme];                       // F1 frequency
            ctx->frequency2[X] = freq2data[phoneme];                       // F2 frequency
            ctx->frequency3[X] = freq3data[phoneme];                       // F3 frequency
            ctx->amplitude1[X] = ampl1data[phoneme];                       // F1 amplitude
            ctx->amplitude2[X] = ampl2data[phoneme];                       // F2 amplitude
            ctx->amplitude3[X] = ampl3data[phoneme];                       // F3 amplitude
            ctx->sampledConsonantFlag[X] = sampledConsonantFlags[phoneme]; // phoneme data for sampled consonants
            ctx->pitches[X] = ctx->toSpeak.pitch + stressPitch;            // pitch
            ++X;
        } while (--frameCount != 0);
        ++index;
    }
}

// ASSIGN PITCH CONTOUR
//
// This subtracts the F1 frequency from the pitch to create a
// pitch contour. Without this, the output would be at a single
// pitch level (monotone).

void AssignPitchContour(SAMContext *ctx)
{
    int i;
    for (i = 0; i < 256; i++)
    {
        // subtract half of the frequency of the formant 1.
        // this adds variety to the voice
        ctx->pitches[i] -= (ctx->frequency1[i] >> 1);
        // ctx->pitches[i] -= (ctx->frequency1[i] - (ctx->frequency1[i] >> 2));
    }
}

// RESCALE AMPLITUDE
//
// Rescale volume from a linear scale to decibels.
//
void RescaleAmplitude(SAMContext *ctx)
{
    int i;
    for (i = 255; i >= 0; i--)
    {
        ctx->amplitude1[i] = amplitudeRescale[ctx->amplitude1[i]];
        ctx->amplitude2[i] = amplitudeRescale[ctx->amplitude2[i]];
        ctx->amplitude3[i] = amplitudeRescale[ctx->amplitude3[i]];
    }
}

void CombineGlottalAndFormants(SAMContext *ctx, unsigned char phase1, unsigned char phase2, unsigned char phase3, unsigned char Y)
{
    unsigned int tmp;

    tmp = multtable[sinus[phase1] | ctx->amplitude1[Y]];      //     F1
    tmp += multtable[sinus[phase2] | ctx->amplitude2[Y]];     // +   F2
    tmp += tmp > 255 ? 1 : 0;                                 // if addition above overflows, we for some reason add one;   + 1
    tmp += multtable[rectangle[phase3] | ctx->amplitude3[Y]]; // +   F3
    tmp += 136;
    tmp >>= 4; // Scale down to 0..15 range of C64 audio.

    WriteToBuf(ctx, 0, tmp & 0xf);
}

// PROCESS THE FRAMES
//
// In traditional vocal synthesis, the glottal pulse drives filters, which
// are attenuated to the frequencies of the formants.
//
// SAM generates these formants directly with sin and rectangular waves.
// To simulate them being driven by the glottal pulse, the waveforms are
// reset at the beginning of each glottal pulse.
//

void ProcessFrames(SAMContext *ctx, unsigned char frameCount)
{
    unsigned char speedcounter = 72;
    unsigned char phase1 = 0;
    unsigned char phase2 = 0;
    unsigned char phase3 = 0;
    unsigned char mem66 = 0; //!! was not initialized

    unsigned char Y = 0;

    unsigned char glottal_pulse = ctx->pitches[0];
    unsigned char count = glottal_pulse - (glottal_pulse >> 2); // mem44 * 0.75

    while (frameCount)
    {
        unsigned char flags = ctx->sampledConsonantFlag[Y];

        // unvoiced sampled phoneme?
        if (flags & 248)
        {
            RenderSample(ctx, &mem66, flags, Y);
            // skip ahead two in the phoneme buffer
            Y += 2;
            frameCount -= 2;
            speedcounter = ctx->toSpeak.speed;
        }
        else
        {
            CombineGlottalAndFormants(ctx, phase1, phase2, phase3, Y);

            speedcounter--;
            if (speedcounter == 0)
            {
                Y++; // go to next amplitude
                // decrement the frame count
                frameCount--;
                if (frameCount == 0)
                    return;
                speedcounter = ctx->toSpeak.speed;
            }

            --glottal_pulse;

            if (glottal_pulse != 0)
            {
                // not finished with a glottal pulse

                --count;
                // within the first 75% of the glottal pulse?
                // is the count non-zero and the sampled flag is zero?
                if ((count != 0) || (flags == 0))
                {
                    // reset the phase of the formants to match the pulse
                    phase1 += ctx->frequency1[Y]; // F1
                    phase2 += ctx->frequency2[Y]; // F2
                    phase3 += ctx->frequency3[Y]; // F3
                    continue;
                }

                // voiced sampled phonemes interleave the sample with the
                // glottal pulse. The sample flag is non-zero, so render
                // the sample for the phoneme.
                RenderSample(ctx, &mem66, flags, Y);
            }
        }

        glottal_pulse = ctx->pitches[Y];
        count = glottal_pulse - (glottal_pulse >> 2); // mem44 * 0.75

        // reset the formant wave generators to keep them in
        // sync with the glottal pulse
        phase1 = 0;
        phase2 = 0;
        phase3 = 0;
    }
}

static unsigned char RenderVoicedSample(SAMContext *ctx, unsigned short hi, unsigned char offset, unsigned char phase1)
{
    do
    {
        unsigned char bit = 8;
        unsigned char sample = sampleTable[hi + offset];
        do
        {
            if ((sample & 128) != 0) // bit 7 set?
                WriteToBuf(ctx, 3, 26);
            else
                WriteToBuf(ctx, 4, 6);
            sample <<= 1;
        } while (--bit != 0);
        offset++;
    } while (++phase1 != 0);
    return offset;
}

void RenderUnvoicedSample(SAMContext *ctx, unsigned short hi, unsigned char off, unsigned char mem53)
{
    do
    {
        unsigned char bit = 8;
        unsigned char sample = sampleTable[hi + off];
        do
        {
            if ((sample & 128) != 0)
                WriteToBuf(ctx, 2, 5);
            else
                WriteToBuf(ctx, 1, mem53);
            sample <<= 1;
        } while (--bit != 0);
    } while (++off != 0);
}

// Render a sampled sound from the sampleTable.
//
//   Phoneme   Sample Start   Sample End
//   32: S*    15             255
//   33: SH    257            511
//   34: F*    559            767
//   35: TH    583            767
//   36: /H    903            1023
//   37: /X    1135           1279
//   38: Z*    84             119
//   39: ZH    340            375
//   40: V*    596            639
//   41: DH    596            631
//
//   42: CH
//   43: **    399            511
//
//   44: J*
//   45: **    257            276
//   46: **
//
//   66: P*
//   67: **    743            767
//   68: **
//
//   69: T*
//   70: **    231            255
//   71: **
//
// The SampledPhonemesTable[] holds flags indicating if a phoneme is
// voiced or not. If the upper 5 bits are zero, the sample is voiced.
//
// Samples in the sampleTable are compressed, with bits being converted to
// bytes from high bit to low, as follows:
//
//   unvoiced 0 bit   -> X
//   unvoiced 1 bit   -> 5
//
//   voiced 0 bit     -> 6
//   voiced 1 bit     -> 24
//
// Where X is a value from the table:
//
//   { 0x18, 0x1A, 0x17, 0x17, 0x17 };
//
// The index into this table is determined by masking off the lower
// 3 bits from the SampledPhonemesTable:
//
//        index = (SampledPhonemesTable[i] & 7) - 1;
//
// For voices samples, samples are interleaved between voiced output.

void RenderSample(SAMContext *ctx, unsigned char *out, unsigned char consonantFlag, unsigned char phonemeIndex)
{

    // mask low three bits and subtract 1 get value to
    // convert 0 bits on unvoiced samples.
    unsigned char hibyte = (consonantFlag & 7) - 1;

    // determine which offset to use from table { 0x18, 0x1A, 0x17, 0x17, 0x17 }
    // T, S, Z                0          0x18
    // CH, J, SH, ZH          1          0x1A
    // P, F*, V, TH, DH       2          0x17
    // /H                     3          0x17
    // /X                     4          0x17

    unsigned short hi = hibyte * 256;
    // voiced sample?
    unsigned char pitchl = consonantFlag & 248;
    if (pitchl == 0)
    {
        // voiced phoneme: Z*, ZH, V*, DH
        pitchl = ctx->pitches[phonemeIndex] >> 4;
        *out = RenderVoicedSample(ctx, hi, *out, pitchl ^ 255);
    }
    else
        RenderUnvoicedSample(ctx, hi, pitchl ^ 255, tab48426[hibyte]);
}

// RENDER THE PHONEMES IN THE LIST
//
// The phoneme list is converted into sound through the steps:
//
// 1. Copy each phoneme <length> number of times into the frames list,
//    where each frame represents 10 milliseconds of sound.
//
// 2. Determine the transitions lengths between phonemes, and linearly
//    interpolate the values across the frames.
//
// 3. Offset the pitches by the fundamental frequency.
//
// 4. Render each frame.

int RenderAudio(SAMContext *ctx)
{
    unsigned char totalFrameCount = 0;

    if (ctx->phonemeIndexOutput[0] == 255)
        return 0; // exit if no data

    CreateFrames(ctx);
    totalFrameCount = CreateTransitions(ctx);

    if (!ctx->toSpeak.singmode)
        AssignPitchContour(ctx);
    RescaleAmplitude(ctx);
    ProcessFrames(ctx, totalFrameCount);
    return totalFrameCount;
}

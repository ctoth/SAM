#include "sam.h"
#include "rules.h"
#include "sam_tabs.h"

void Insert(SAMContext *ctx, unsigned char position, unsigned char phonemeIndex, unsigned char phonemeLength, unsigned char stress)
{
    int i;
    for (i = 253; i >= position; i--) // ML : always keep last safe-guarding 255
    {
        ctx->phonemeindex[i + 1] = ctx->phonemeindex[i];
        ctx->stress[i + 1] = ctx->stress[i];
        ctx->phonemeLength[i + 1] = ctx->phonemeLength[i];
    }

    ctx->phonemeindex[position] = phonemeIndex;
    ctx->phonemeLength[position] = phonemeLength;
    ctx->stress[position] = stress;
}

void ChangeRule(SAMContext *ctx, unsigned char position, unsigned char phonemeIndex, const char *descr)
{
    ctx->phonemeindex[position] = 13; // rule;
    Insert(ctx, position + 1, phonemeIndex, 0, ctx->stress[position]);
}

void rule_j(SAMContext *ctx, unsigned char pos)
{
    Insert(ctx, pos + 1, 45, 0, ctx->stress[pos]);
}

void rule_g(SAMContext *ctx, unsigned char pos)
{
    // G <VOWEL OR DIPTHONG NOT ENDING WITH IY> -> GX <VOWEL OR DIPTHONG NOT ENDING WITH IY>
    // Example: GO

    unsigned char index = ctx->phonemeindex[pos + 1];

    // If dipthong ending with YX, move continue processing next phoneme
    if ((index != 255) && ((flags[index] & FLAG_DIP_YX) == 0))
    {
        // replace G with GX and continue processing next phoneme

        ctx->phonemeindex[pos] = 63; // 'GX'
    }
}

void rule_alveolar_uw(SAMContext *ctx, unsigned char X)
{
    // ALVEOLAR flag set?
    if (flags[ctx->phonemeindex[X - 1]] & FLAG_ALVEOLAR)
    {
        ctx->phonemeindex[X] = 16;
    }
}

void rule_ch(SAMContext *ctx, unsigned char pos)
{
    Insert(ctx, pos + 1, 43, 0, ctx->stress[pos]);
}

void rule_dipthong(SAMContext *ctx, unsigned char p, unsigned short pf, unsigned char pos)
{
    // <DIPTHONG ENDING WITH WX> -> <DIPTHONG ENDING WITH WX> WX
    // <DIPTHONG NOT ENDING WITH WX> -> <DIPTHONG NOT ENDING WITH WX> YX
    // Example: OIL, COW

    // If ends with IY, use YX, else use WX
    unsigned char A = (pf & FLAG_DIP_YX) ? 21 : 20; // 'WX' = 20 'YX' = 21

    // Insert at WX or YX following, copying the stress

    Insert(ctx, pos + 1, A, 0, ctx->stress[pos]);

    if (p == 53)
        rule_alveolar_uw(ctx, pos); // Example: NEW, DEW, SUE, ZOO, THOO, TOO
    else if (p == 42)
        rule_ch(ctx, pos); // Example: CHEW
    else if (p == 44)
        rule_j(ctx, pos); // Example: JAY
}

void ApplyRules(SAMContext *ctx)
{
    unsigned char pos = 0;
    unsigned char p;

    while ((p = ctx->phonemeindex[pos]) != END)
    {
        unsigned short pf;
        unsigned char prior;

        if (p == 0)
        { // Is phoneme pause?
            ++pos;
            continue;
        }

        pf = flags[p];
        prior = ctx->phonemeindex[pos - 1];

        if ((pf & FLAG_DIPTHONG))
            rule_dipthong(ctx, p, pf, pos);
        else if (p == 78)
            ChangeRule(ctx, pos, 24, "UL -> AX L"); // Example: MEDDLE
        else if (p == 79)
            ChangeRule(ctx, pos, 27, "UM -> AX M"); // Example: ASTRONOMY
        else if (p == 80)
            ChangeRule(ctx, pos, 28, "UN -> AX N"); // Example: FUNCTION
        else if ((pf & FLAG_VOWEL) && ctx->stress[pos])
        {
            // RULE:
            //       <STRESSED VOWEL> <SILENCE> <STRESSED VOWEL> -> <STRESSED VOWEL> <SILENCE> Q <VOWEL>
            // EXAMPLE: AWAY EIGHT
            if (!ctx->phonemeindex[pos + 1])
            { // If following phoneme is a pause, get next
                p = ctx->phonemeindex[pos + 2];
                if (p != END && (flags[p] & FLAG_VOWEL) && ctx->stress[pos + 2])
                {

                    Insert(ctx, pos + 2, 31, 0, 0); // 31 = 'Q'
                }
            }
        }
        else if (p == pR)
        { // RULES FOR PHONEMES BEFORE R
            if (prior == pT)
                ctx->phonemeindex[pos - 1] = 42; // "T R -> CH R"); Example: TRACK
            else if (prior == pD)
                ctx->phonemeindex[pos - 1] = 44; // "D R -> J R"); Example: DRY
            else if (flags[prior] & FLAG_VOWEL)
                ctx->phonemeindex[pos] = 18; // "<VOWEL> R -> <VOWEL> RX"); Example: ART
        }
        else if (p == 24 && (flags[prior] & FLAG_VOWEL))
            ctx->phonemeindex[pos] = 19; // "<VOWEL> L -> <VOWEL> LX" // Example: ALL
        else if (prior == 60 && p == 32)
        { // 'G' 'S'
            // Can't get to fire -
            //       1. The G -> GX rule intervenes
            //       2. Reciter already replaces GS -> GZ
            ctx->phonemeindex[pos] = 38; // "G S -> G Z"
        }
        else if (p == 60)
            rule_g(ctx, pos);
        else
        {
            if (p == 72)
            { // 'K'
                // K <VOWEL OR DIPTHONG NOT ENDING WITH IY> -> KX <VOWEL OR DIPTHONG NOT ENDING WITH IY>
                // Example: COW
                unsigned char Y = ctx->phonemeindex[pos + 1];
                // If at end, replace current phoneme with KX
                if ((flags[Y] & FLAG_DIP_YX) == 0 || Y == END)
                { // VOWELS AND DIPTHONGS ENDING WITH IY SOUND flag set?
                    ctx->phonemeindex[pos] = 7;
                    p = 75;
                    pf = flags[p];
                }
            }

            // Replace with softer version?
            if ((flags[p] & FLAG_PLOSIVE) && (prior == 32))
            { // 'S'
                // RULE:
                //      S P -> S B
                //      S T -> S D
                //      S K -> S G
                //      S KX -> S GX
                // Examples: SPY, STY, SKY, SCOWL

                ctx->phonemeindex[pos] = p - 12;
            }
            else if (!(pf & FLAG_PLOSIVE))
            {
                p = ctx->phonemeindex[pos];
                if (p == 53)
                    rule_alveolar_uw(ctx, pos); // Example: NEW, DEW, SUE, ZOO, THOO, TOO
                else if (p == 42)
                    rule_ch(ctx, pos); // Example: CHEW
                else if (p == 44)
                    rule_j(ctx, pos); // Example: JAY
            }

            if (p == 69 || p == 57)
            { // 'T', 'D'
                // RULE: Soften T following vowel
                // NOTE: This rule fails for cases such as "ODD"
                //       <UNSTRESSED VOWEL> T <PAUSE> -> <UNSTRESSED VOWEL> DX <PAUSE>
                //       <UNSTRESSED VOWEL> D <PAUSE>  -> <UNSTRESSED VOWEL> DX <PAUSE>
                // Example: PARTY, TARDY
                if (flags[ctx->phonemeindex[pos - 1]] & FLAG_VOWEL)
                {
                    p = ctx->phonemeindex[pos + 1];
                    if (!p)
                        p = ctx->phonemeindex[pos + 2];
                    if ((flags[p] & FLAG_VOWEL) && !ctx->stress[pos + 1])
                        ctx->phonemeindex[pos] = 30; //  "Soften T or D following vowel or ER and preceding a pause -> DX"
                }
            }
        }
        pos++;
    } // while
}

// Iterates through the phoneme buffer, copying the stress value from
// the following phoneme under the following circumstance:

//     1. The current phoneme is voiced, excluding plosives and fricatives
//     2. The following phoneme is voiced, excluding plosives and fricatives, and
//     3. The following phoneme is stressed
//
//  In those cases, the stress value+1 from the following phoneme is copied.
//
// For example, the word LOITER is represented as LOY5TER, with as stress
// of 5 on the dipthong OY. This routine will copy the stress value of 6 (5+1)
// to the L that precedes it.

void CopyStressToPrevVowel(SAMContext *ctx)
{
    // loop through all the phonemes to be output
    unsigned char pos = 0;
    unsigned char Y;
    while ((Y = ctx->phonemeindex[pos]) != END)
    {
        // if CONSONANT_FLAG set, skip - only vowels get stress
        if (flags[Y] & 64)
        {
            Y = ctx->phonemeindex[pos + 1];

            // if the following phoneme is the end, or a vowel, skip
            if (Y != END && (flags[Y] & 128) != 0)
            {
                // get the stress value at the next position
                Y = ctx->stress[pos + 1];
                if (Y && !(Y & 128))
                {
                    // if next phoneme is stressed, and a VOWEL OR ER
                    // copy stress from next phoneme to this one
                    ctx->stress[pos] = Y + 1;
                }
            }
        }

        ++pos;
    }
}

/*
This function calculates the length of each phoneme in the input string, based on whether or not the phoneme is stressed.

If a phoneme is not stressed, the function looks up the length of that phoneme in the 'phonemeLengthTable'.
If a phoneme is stressed, the function looks up the length of that phoneme in the 'phonemeStressedLengthTable'.

*/
void SetPhonemeLengths(SAMContext *ctx)
{
    int position = 0;
    while (ctx->phonemeindex[position] != 255)
    {
        unsigned char stress = ctx->stress[position];
        if ((stress == 0) || ((stress & 128) != 0))
        {
            ctx->phonemeLength[position] = phonemeLengthTable[ctx->phonemeindex[position]];
        }
        else
        {
            ctx->phonemeLength[position] = phonemeStressedLengthTable[ctx->phonemeindex[position]];
        }
        position++;
    }
}

// Applies various rules that adjust the lengths of phonemes
//
//         Lengthen <FRICATIVE> or <VOICED> between <VOWEL> and <PUNCTUATION> by 1.5
//         <VOWEL> <RX | LX> <CONSONANT> - decrease <VOWEL> length by 1
//         <VOWEL> <UNVOICED PLOSIVE> - decrease vowel by 1/8th
//         <VOWEL> <UNVOICED CONSONANT> - increase vowel by 1/2 + 1
//         <NASAL> <STOP CONSONANT> - set nasal = 5, consonant = 6
//         <VOICED STOP CONSONANT> {optional silence} <STOP CONSONANT> - shorten both to 1/2 + 1
//         <LIQUID CONSONANT> <DIPTHONG> - decrease by 2
//
void AdjustPhonemeLengths(SAMContext *ctx)
{
    // LENGTHEN VOWELS PRECEDING PUNCTUATION
    //
    // Search for punctuation. If found, back up to the first vowel, then
    // process all phonemes between there and up to (but not including) the punctuation.
    // If any phoneme is found that is a either a fricative or voiced, the duration is
    // increased by (length * 1.5) + 1

    // loop index
    {
        unsigned char X = 0;
        unsigned char index;

        while ((index = ctx->phonemeindex[X]) != END)
        {
            unsigned char loopIndex;

            // not punctuation?
            if ((flags[index] & FLAG_PUNCT) == 0)
            {
                ++X;
                continue;
            }

            loopIndex = X;

            while (--X && !(flags[ctx->phonemeindex[X]] & FLAG_VOWEL))
                ; // back up while not a vowel
            if (X == 0)
                break;

            do
            {
                // test for vowel
                index = ctx->phonemeindex[X];

                // test for fricative/unvoiced or not voiced
                if (!(flags[index] & FLAG_FRICATIVE) || (flags[index] & FLAG_VOICED))
                { // nochmal überprüfen
                    unsigned char A = ctx->phonemeLength[X];
                    // change phoneme length to (length * 1.5) + 1
                    ctx->phonemeLength[X] = (A >> 1) + A + 1;
                }
            } while (++X != loopIndex);
            X++;
        } // while
    }

    // Similar to the above routine, but shorten vowels under some circumstances

    // Loop through all phonemes
    unsigned char loopIndex = 0;
    unsigned char index;

    while ((index = ctx->phonemeindex[loopIndex]) != END)
    {
        unsigned char X = loopIndex;

        if (flags[index] & FLAG_VOWEL)
        {
            index = ctx->phonemeindex[loopIndex + 1];
            if (!(flags[index] & FLAG_CONSONANT))
            {
                if ((index == 18) || (index == 19))
                { // 'RX', 'LX'
                    index = ctx->phonemeindex[loopIndex + 2];
                    if ((flags[index] & FLAG_CONSONANT))
                    {
                        ctx->phonemeLength[loopIndex]--;
                    }
                }
            }
            else
            {                                                             // Got here if not <VOWEL>
                unsigned short flag = (index == END) ? 65 : flags[index]; // 65 if end marker

                if (!(flag & FLAG_VOICED))
                { // Unvoiced
                    // *, .*, ?*, ,*, -*, DX, S*, SH, F*, TH, /H, /X, CH, P*, T*, K*, KX
                    if ((flag & FLAG_PLOSIVE))
                    { // unvoiced plosive
                        // RULE: <VOWEL> <UNVOICED PLOSIVE>
                        // <VOWEL> <P*, T*, K*, KX>
                        ctx->phonemeLength[loopIndex] -= (ctx->phonemeLength[loopIndex] >> 3);
                    }
                }
                else
                {
                    unsigned char A;
                    // decrease length
                    A = ctx->phonemeLength[loopIndex];
                    ctx->phonemeLength[loopIndex] = (A >> 2) + A + 1; // 5/4*A + 1
                }
            }
        }
        else if ((flags[index] & FLAG_NASAL) != 0)
        { // nasal?
            // RULE: <NASAL> <STOP CONSONANT>
            //       Set punctuation length to 6
            //       Set stop consonant length to 5
            index = ctx->phonemeindex[++X];
            if (index != END && (flags[index] & FLAG_STOPCONS))
            {
                ctx->phonemeLength[X] = 6;     // set stop consonant length to 6
                ctx->phonemeLength[X - 1] = 5; // set nasal length to 5
            }
        }
        else if ((flags[index] & FLAG_STOPCONS))
        { // (voiced) stop consonant?
            // RULE: <VOICED STOP CONSONANT> {optional silence} <STOP CONSONANT>
            //       Shorten both to (length/2 + 1)

            // move past silence
            while ((index = ctx->phonemeindex[++X]) == 0)
                ;

            if (index != END && (flags[index] & FLAG_STOPCONS))
            {
                // FIXME, this looks wrong?
                // RULE: <UNVOICED STOP CONSONANT> {optional silence} <STOP CONSONANT>
                ctx->phonemeLength[X] = (ctx->phonemeLength[X] >> 1) + 1;
                ctx->phonemeLength[loopIndex] = (ctx->phonemeLength[loopIndex] >> 1) + 1;
                X = loopIndex;
            }
        }
        else if ((flags[index] & FLAG_LIQUIC))
        { // liquic consonant?
            // RULE: <VOICED NON-VOWEL> <DIPTHONG>
            //       Decrease <DIPTHONG> by 2
            index = ctx->phonemeindex[X - 1]; // prior phoneme;

            // FIXME: The debug code here breaks the rule.
            // prior phoneme a stop consonant>
            if ((flags[index] & FLAG_STOPCONS) != 0)

                ctx->phonemeLength[X] -= 2; // 20ms
        }

        ++loopIndex;
    }
}

void AddVoicedStopConsonants(SAMContext *ctx)
{
    unsigned char pos = 0;

    while (ctx->phonemeindex[pos] != END)
    {
        unsigned char index = ctx->phonemeindex[pos];

        if ((flags[index] & FLAG_STOPCONS))
        {
            if ((flags[index] & FLAG_PLOSIVE))
            {
                unsigned char A;
                unsigned char X = pos;
                while (!ctx->phonemeindex[++X])
                    ; /* Skip pause */
                A = ctx->phonemeindex[X];
                if (A != END)
                {
                    if ((flags[A] & 8) || (A == 36) || (A == 37))
                    {
                        ++pos;
                        continue;
                    } // '/H' '/X'
                }
            }
            Insert(ctx, pos + 1, index + 1, phonemeLengthTable[index + 1], ctx->stress[pos]);
            Insert(ctx, pos + 2, index + 2, phonemeLengthTable[index + 2], ctx->stress[pos]);
            pos += 2;
        }
        ++pos;
        if (pos >= 0xf)
            break;
    }
}

// this is code which appears to be a sanity check which was originally part of SAMMain
// If any phonemes are too high it ends the speaking there.
void CheckPhonemes(SAMContext *ctx)
{
    unsigned char pos = 0;
    while (ctx->phonemeindex[pos] != END)
    {
        if (ctx->phonemeindex[pos] > 80)
        {
            ctx->phonemeindex[pos] = END;
            break; // error: delete all behind it
        }
        ++pos;
    }
}

void InsertBreath(SAMContext *ctx)
{
    unsigned char lastNonPunctPos = 255;
    unsigned char len = 0;
    unsigned char index; // variable Y

    unsigned char pos = 0;

    while ((index = ctx->phonemeindex[pos]) != END)
    {
        len += ctx->phonemeLength[pos];
        if (len < 232)
        {
            if (index == BREAK)
            {
            }
            else if (!(flags[index] & FLAG_PUNCT))
            {
                if (index == 0)
                    lastNonPunctPos = pos;
            }
            else
            {
                len = 0;
                Insert(ctx, ++pos, BREAK, 0, 0);
            }
        }
        else
        {
            pos = lastNonPunctPos;
            ctx->phonemeindex[pos] = 31; // 'Q*' glottal stop
            ctx->phonemeLength[pos] = 4;
            ctx->stress[pos] = 0;

            len = 0;
            Insert(ctx, ++pos, BREAK, 0, 0);
        }
        ++pos;
    }
}

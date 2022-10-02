#include "phoneme_parser.h"
#include "sam.h"
#include "sam_tabs.h"

#include <string.h>
// The input[] buffer contains a string of phonemes and stress markers along
// the lines of:
//
//     DHAX KAET IHZ AH5GLIY. <0x9B>
//
// The byte 0x9B marks the end of the buffer. Some phonemes are 2 bytes
// long, such as "DH" and "AX". Others are 1 byte long, such as "T" and "Z".
// There are also stress markers, such as "5" and ".".
//
// The first character of the phonemes are stored in the table signInputTable1[].
// The second character of the phonemes are stored in the table signInputTable2[].
// The stress characters are arranged in low to high stress order in stressInputTable[].
//
// The following process is used to parse the input[] buffer:
//
// Repeat until the <0x9B> character is reached:
//
//        First, a search is made for a 2 character match for phonemes that do not
//        end with the '*' (wildcard) character. On a match, the index of the phoneme
//        is added to phonemeIndex[] and the buffer position is advanced 2 bytes.
//
//        If this fails, a search is made for a 1 character match against all
//        phoneme names ending with a '*' (wildcard). If this succeeds, the
//        phoneme is added to phonemeIndex[] and the buffer position is advanced
//        1 byte.
//
//        If this fails, search for a 1 character match in the stressInputTable[].
//        If this succeeds, the stress value is placed in the last stress[] table
//        at the same index of the last added phoneme, and the buffer position is
//        advanced by 1 byte.
//
//        If this fails, return a 0.
//
// On success:
//
//    1. phonemeIndex[] will contain the index of all the phonemes.
//    2. The last index in phonemeIndex[] will be 255.
//    3. stress[] will contain the stress value for each phoneme

// input[] holds the string of phonemes, each two bytes wide
// signInputTable1[] holds the first character of each phoneme
// signInputTable2[] holds the second character of each phoneme
// phonemeIndex[] holds the indexes of the phonemes after parsing input[]
//
// The parser scans through the input[], finding the names of the phonemes
// by searching signInputTable1[] and signInputTable2[]. On a match, it
// copies the index of the phoneme into the phonemeIndexTable[].
//
// The character <0x9B> marks the end of text in input[]. When it is reached,
// the index 255 is placed at the end of the phonemeIndexTable[], and the
// function returns with a 1 indicating success.

int ParsePhonemes(SAMContext *ctx)
{
    unsigned char sign1;
    unsigned char position = 0;
    unsigned char srcpos = 0;

    memset(ctx->stress, 0, 256); // Clear the stress table.

    while ((sign1 = ctx->toSpeak.input[srcpos]) != 155)
    
    { // 155 (\233) is end of line marker
        signed int match;
        unsigned char sign2 = ctx->toSpeak.input[++srcpos];
        if ((match = full_match(sign1, sign2)) != -1)
        {
            // Matched both characters (no wildcards)
            ctx->phonemeindex[position++] = (unsigned char)match;
            ++srcpos; // Skip the second character of the input as we've matched it
        }
        else if ((match = wild_match(sign1)) != -1)
        {
            // Matched just the first character (with second character matching '*'
            ctx->phonemeindex[position++] = (unsigned char)match;
        }
        else
        {
            // Should be a stress character. Search through the
            // stress table backwards.
            match = 8; // End of stress table. FIXME: Don't hardcode.
            while ((sign1 != stressInputTable[match]) && (match > 0))
                --match;

            if (match == 0)
                return 0; // failure

            ctx->stress[position - 1] = (unsigned char)match; // Set stress for prior phoneme
        }
    } // while

    ctx->phonemeindex[position] = END;
    return 1;
}

signed int full_match(unsigned char sign1, unsigned char sign2)
{
    unsigned char Y = 0;
    do
    {
        // GET FIRST CHARACTER AT POSITION Y IN signInputTable
        // --> should change name to PhonemeNameTable1
        unsigned char A = signInputTable1[Y];

        if (A == sign1)
        {
            A = signInputTable2[Y];
            // NOT A SPECIAL AND MATCHES SECOND CHARACTER?
            if ((A != '*') && (A == sign2))
                return Y;
        }
    } while (++Y != 81);
    return -1;
}

signed int wild_match(unsigned char sign1)
{
    signed int Y = 0;
    do
    {
        if (signInputTable2[Y] == '*')
        {
            if (signInputTable1[Y] == sign1)
                return Y;
        }
    } while (++Y != 81);
    return -1;
}

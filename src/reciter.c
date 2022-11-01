#include "sam.h"
#include "reciter.h"
#include "reciter_tabs.h"
#include <malloc.h>

unsigned char GetRuleByte(unsigned short address, unsigned char Y)
{
    if (address >= 37541)
    {
        address -= 37541;
        return rules2[address + Y];
    }
    address -= 32000;
    return rules[address + Y];
}

int TextToPhonemes(unsigned char *input)
{
    unsigned char phonemeOutputPos; // output position for phonemes
    unsigned char mem57;
    unsigned char mem58;
    unsigned char mem59;
    unsigned char mem60;
    unsigned char mem61;
    unsigned short curRulePos; // memory position of current rule

    unsigned char mem64;     // position of '=' or current character
    unsigned char rparenPos; // position of ')'
    unsigned char lparemPos; // position of '('

    unsigned char Y;

    int r;
    // create SAMReciterContext
    SAMReciterContext *ctx = (SAMReciterContext *)malloc(sizeof(SAMReciterContext));

    ctx->reciterInput[0] = ' ';

    // secure copy of input
    // because input will be overwritten by phonemes
    ctx->reciterIndex = 0;
    do
    {
        ctx->workingChar = input[ctx->reciterIndex] & 127;
        ctx->reciterInput[++ctx->reciterIndex] = ctx->workingChar;
    } while (ctx->reciterIndex < 255);
    ctx->reciterInput[255] = 27;

    phonemeOutputPos = mem61 = 255;

pos36554:
    while (1)
    {
        while (1)
        {
            ctx->reciterIndex = ++mem61;
            mem64 = ctx->reciterInput[ctx->reciterIndex];
            if (mem64 == '[')
            {
                ctx->reciterIndex = ++phonemeOutputPos;
                input[ctx->reciterIndex] = 155;
                return 1;
            }

            if (mem64 != '.')
                break;
            ctx->reciterIndex++;
            ctx->workingChar = tab36376[ctx->reciterInput[ctx->reciterIndex]] & 1;
            if (ctx->workingChar != 0)
                break;
            phonemeOutputPos++;
            ctx->reciterIndex = phonemeOutputPos;
            ctx->workingChar = '.';
            input[ctx->reciterIndex] = '.';
        }
        mem57 = tab36376[mem64];
        if ((mem57 & 2) != 0)
        {
            curRulePos = 37541;
            goto pos36700;
        }

        if (mem57 != 0)
            break;
        ctx->reciterInput[ctx->reciterIndex] = ' ';
        ctx->reciterIndex = ++phonemeOutputPos;
        if (ctx->reciterIndex > 120)
        {
            input[ctx->reciterIndex] = 155;
            return 1;
        }
        input[ctx->reciterIndex] = 32;
    }

    if (!(mem57 & 128))
        return 0;

    // go to the right rules for this character.
    ctx->reciterIndex = mem64 - 'A';
    curRulePos = tab37489[ctx->reciterIndex] | (tab37515[ctx->reciterIndex] << 8);

pos36700:
    // find next rule
    while ((GetRuleByte(++curRulePos, 0) & 128) == 0)
        ;
    Y = 0;
    while (GetRuleByte(curRulePos, ++Y) != '(')
        ;
    lparemPos = Y;
    while (GetRuleByte(curRulePos, ++Y) != ')')
        ;
    rparenPos = Y;
    while ((GetRuleByte(curRulePos, ++Y) & 127) != '=')
        ;
    mem64 = Y;

    mem60 = ctx->reciterIndex = mem61;
    // compare the string within the bracket
    Y = lparemPos + 1;

    while (1)
    {
        if (GetRuleByte(curRulePos, Y) != ctx->reciterInput[ctx->reciterIndex])
            goto pos36700;
        if (++Y == rparenPos)
            break;
        mem60 = ++ctx->reciterIndex;
    }

    // the string in the bracket is correct

    mem59 = mem61;

    while (1)
    {
        unsigned char ch;
        while (1)
        {
            lparemPos--;
            mem57 = GetRuleByte(curRulePos, lparemPos);
            if ((mem57 & 128) != 0)
            {
                mem58 = mem60;
                goto pos37184;
            }
            ctx->reciterIndex = mem57 & 127;
            if ((tab36376[ctx->reciterIndex] & 128) == 0)
                break;
            if (ctx->reciterInput[mem59 - 1] != mem57)
                goto pos36700;
            --mem59;
        }

        ch = mem57;

        r = handle_ch2(ctx, ch, mem59 - 1);
        if (r == -1)
        {
            switch (ch)
            {
            case '&':
                if (!Code37055(ctx, mem59 - 1, 16))
                {
                    if (ctx->reciterInput[ctx->reciterIndex] != 'H')
                        r = 1;
                    else
                    {
                        ctx->workingChar = ctx->reciterInput[--ctx->reciterIndex];
                        if ((ctx->workingChar != 'C') && (ctx->workingChar != 'S'))
                            r = 1;
                    }
                }
                break;

            case '@':
                if (!Code37055(ctx, mem59 - 1, 4))
                {
                    ctx->workingChar = ctx->reciterInput[ctx->reciterIndex];
                    if (ctx->workingChar != 72)
                        r = 1;
                    if ((ctx->workingChar != 84) && (ctx->workingChar != 67) && (ctx->workingChar != 83))
                        r = 1;
                }
                break;
            case '+':
                ctx->reciterIndex = mem59;
                ctx->workingChar = ctx->reciterInput[--ctx->reciterIndex];
                if ((ctx->workingChar != 'E') && (ctx->workingChar != 'I') && (ctx->workingChar != 'Y'))
                    r = 1;
                break;
            case ':':
                while (Code37055(ctx, mem59 - 1, 32))
                    --mem59;
                continue;
            default:
                return 0;
            }
        }

        if (r == 1)
            goto pos36700;

        mem59 = ctx->reciterIndex;
    }

    do
    {
        ctx->reciterIndex = mem58 + 1;
        if (ctx->reciterInput[ctx->reciterIndex] == 'E')
        {
            if ((tab36376[ctx->reciterInput[ctx->reciterIndex + 1]] & 128) != 0)
            {
                ctx->workingChar = ctx->reciterInput[++ctx->reciterIndex];
                if (ctx->workingChar == 'L')
                {
                    if (ctx->reciterInput[++ctx->reciterIndex] != 'Y')
                        goto pos36700;
                }
                else if ((ctx->workingChar != 'R') && (ctx->workingChar != 'S') && (ctx->workingChar != 'D') && !IsNextInput(ctx, "FUL"))
                    goto pos36700;
            }
        }
        else
        {
            if (!IsNextInput(ctx, "ING"))
                goto pos36700;
            mem58 = ctx->reciterIndex;
        }

    pos37184:
        r = 0;
        do
        {
            while (1)
            {
                Y = rparenPos + 1;
                if (Y == mem64)
                {
                    mem61 = mem60;

                    while (1)
                    {
                        mem57 = ctx->workingChar = GetRuleByte(curRulePos, Y);
                        ctx->workingChar = ctx->workingChar & 127;
                        if (ctx->workingChar != '=')
                            input[++phonemeOutputPos] = ctx->workingChar;
                        if ((mem57 & 128) != 0)
                            goto pos36554;
                        Y++;
                    }
                }
                rparenPos = Y;
                mem57 = GetRuleByte(curRulePos, Y);
                if ((tab36376[mem57] & 128) == 0)
                    break;
                if (ctx->reciterInput[mem58 + 1] != mem57)
                {
                    r = 1;
                    break;
                }
                ++mem58;
            }

            if (r == 0)
            {
                ctx->workingChar = mem57;
                if (ctx->workingChar == '@')
                {
                    if (Code37055(ctx, mem58 + 1, 4) == 0)
                    {
                        ctx->workingChar = ctx->reciterInput[ctx->reciterIndex];
                        if ((ctx->workingChar != 82) && (ctx->workingChar != 84) &&
                            (ctx->workingChar != 67) && (ctx->workingChar != 83))
                            r = 1;
                    }
                    else
                    {
                        r = -2;
                    }
                }
                else if (ctx->workingChar == ':')
                {
                    while (Code37055(ctx, mem58 + 1, 32))
                        mem58 = ctx->reciterIndex;
                    r = -2;
                }
                else
                    r = handle_ch(ctx, ctx->workingChar, mem58 + 1);
            }

            if (r == 1)
                goto pos36700;
            if (r == -2)
            {
                r = 0;
                continue;
            }
            if (r == 0)
                mem58 = ctx->reciterIndex;
        } while (r == 0);
    } while (ctx->workingChar == '%');
    return 0;
}

unsigned int IsNextInput(SAMReciterContext *ctx, const char *str)
{
    while (*str)
    {
        unsigned char ch = *str;
        ctx->workingChar = ctx->reciterInput[ctx->reciterIndex++];
        if (ctx->workingChar != ch)
            return 0;
        ++str;
    }
    return 1;
}

/* Retrieve flags for character at mem59-1 */
unsigned char Code37055(SAMReciterContext *ctx, unsigned char npos, unsigned char mask)
{
    ctx->reciterIndex = npos;
    return tab36376[ctx->reciterInput[ctx->reciterIndex]] & mask;
}

int handle_ch(SAMReciterContext *ctx, unsigned char ch, unsigned char mem)
{
    unsigned char tmp;
    ctx->reciterIndex = mem;
    tmp = tab36376[ctx->reciterInput[ctx->reciterIndex]];
    if (ch == ' ')
    {
        if ((tmp & 128) != 0)
            return 1;
    }
    else if (ch == '#')
    {
        if ((tmp & 64) == 0)
            return 1;
    }
    else if (ch == '.')
    {
        if ((tmp & 8) == 0)
            return 1;
    }
    else if (ch == '&')
    {
        if ((tmp & 16) == 0)
        {
            if (ctx->reciterInput[ctx->reciterIndex] != 72)
                return 1;
            ++ctx->reciterIndex;
        }
    }
    else if (ch == '^')
    {
        if ((tmp & 32) == 0)
            return 1;
    }
    else if (ch == '+')
    {
        ctx->reciterIndex = mem;
        ch = ctx->reciterInput[ctx->reciterIndex];
        if ((ch != 69) && (ch != 73) && (ch != 89))
            return 1;
    }
    else
        return -1;
    return 0;
}

int handle_ch2(SAMReciterContext *ctx, unsigned char ch, unsigned char mem)
{
    unsigned char tmp;
    ctx->reciterIndex = mem;
    tmp = tab36376[ctx->reciterInput[mem]];
    if (ch == ' ')
    {
        if (tmp & 128)
            return 1;
    }
    else if (ch == '#')
    {
        if (!(tmp & 64))
            return 1;
    }
    else if (ch == '.')
    {
        if (!(tmp & 8))
            return 1;
    }
    else if (ch == '^')
    {
        if (!(tmp & 32))
            return 1;
    }
    else
        return -1;
    return 0;
}

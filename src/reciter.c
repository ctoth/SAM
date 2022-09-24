#include "sam.h"
#include "reciter.h"
#include "reciter_tabs.h"

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

int TextToPhonemes(SAMContext *ctx, unsigned char *input)
{
    unsigned char phonemeOutputPos; // output position for phonemes
    unsigned char mem57;
    unsigned char mem58;
    unsigned char mem59;
    unsigned char mem60;
    unsigned char mem61;
    unsigned short curRulePos; // memory position of current rule

    unsigned char mem64; // position of '=' or current character
    unsigned char mem65; // position of ')'
    unsigned char mem66; // position of '('

    unsigned char Y;

    int r;

    ctx->inputtemp[0] = ' ';

    // secure copy of input
    // because input will be overwritten by phonemes
    ctx->X = 0;
    do
    {
        ctx->A = input[ctx->X] & 127;
        if (ctx->A >= 112)
            ctx->A = ctx->A & 95;
        else if (ctx->A >= 96)
            ctx->A = ctx->A & 79;
        ctx->inputtemp[++ctx->X] = ctx->A;
    } while (ctx->X < 255);
    ctx->inputtemp[255] = 27;
    phonemeOutputPos = mem61 = 255;

pos36554:
    while (1)
    {
        while (1)
        {
            ctx->X = ++mem61;
            mem64 = ctx->inputtemp[ctx->X];
            if (mem64 == '[')
            {
                ctx->X = ++phonemeOutputPos;
                input[ctx->X] = 155;
                return 1;
            }

            if (mem64 != '.')
                break;
            ctx->X++;
            ctx->A = tab36376[ctx->inputtemp[ctx->X]] & 1;
            if (ctx->A != 0)
                break;
            phonemeOutputPos++;
            ctx->X = phonemeOutputPos;
            ctx->A = '.';
            input[ctx->X] = '.';
        }
        mem57 = tab36376[mem64];
        if ((mem57 & 2) != 0)
        {
            curRulePos = 37541;
            goto pos36700;
        }

        if (mem57 != 0)
            break;
        ctx->inputtemp[ctx->X] = ' ';
        ctx->X = ++phonemeOutputPos;
        if (ctx->X > 120)
        {
            input[ctx->X] = 155;
            return 1;
        }
        input[ctx->X] = 32;
    }

    if (!(mem57 & 128))
        return 0;

    // go to the right rules for this character.
    ctx->X = mem64 - 'A';
    curRulePos = tab37489[ctx->X] | (tab37515[ctx->X] << 8);

pos36700:
    // find next rule
    while ((GetRuleByte(++curRulePos, 0) & 128) == 0)
        ;
    Y = 0;
    while (GetRuleByte(curRulePos, ++Y) != '(')
        ;
    mem66 = Y;
    while (GetRuleByte(curRulePos, ++Y) != ')')
        ;
    mem65 = Y;
    while ((GetRuleByte(curRulePos, ++Y) & 127) != '=')
        ;
    mem64 = Y;

    mem60 = ctx->X = mem61;
    // compare the string within the bracket
    Y = mem66 + 1;

    while (1)
    {
        if (GetRuleByte(curRulePos, Y) != ctx->inputtemp[ctx->X])
            goto pos36700;
        if (++Y == mem65)
            break;
        mem60 = ++ctx->X;
    }

    // the string in the bracket is correct

    mem59 = mem61;

    while (1)
    {
        unsigned char ch;
        while (1)
        {
            mem66--;
            mem57 = GetRuleByte(curRulePos, mem66);
            if ((mem57 & 128) != 0)
            {
                mem58 = mem60;
                goto pos37184;
            }
            ctx->X = mem57 & 127;
            if ((tab36376[ctx->X] & 128) == 0)
                break;
            if (ctx->inputtemp[mem59 - 1] != mem57)
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
                    if (ctx->inputtemp[ctx->X] != 'H')
                        r = 1;
                    else
                    {
                        ctx->A = ctx->inputtemp[--ctx->X];
                        if ((ctx->A != 'C') && (ctx->A != 'S'))
                            r = 1;
                    }
                }
                break;

            case '@':
                if (!Code37055(ctx, mem59 - 1, 4))
                {
                    ctx->A = ctx->inputtemp[ctx->X];
                    if (ctx->A != 72)
                        r = 1;
                    if ((ctx->A != 84) && (ctx->A != 67) && (ctx->A != 83))
                        r = 1;
                }
                break;
            case '+':
                ctx->X = mem59;
                ctx->A = ctx->inputtemp[--ctx->X];
                if ((ctx->A != 'E') && (ctx->A != 'I') && (ctx->A != 'Y'))
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

        mem59 = ctx->X;
    }

    do
    {
        ctx->X = mem58 + 1;
        if (ctx->inputtemp[ctx->X] == 'E')
        {
            if ((tab36376[ctx->inputtemp[ctx->X + 1]] & 128) != 0)
            {
                ctx->A = ctx->inputtemp[++ctx->X];
                if (ctx->A == 'L')
                {
                    if (ctx->inputtemp[++ctx->X] != 'Y')
                        goto pos36700;
                }
                else if ((ctx->A != 'R') && (ctx->A != 'S') && (ctx->A != 'D') && !IsNextInput(ctx, "FUL"))
                    goto pos36700;
            }
        }
        else
        {
            if (!IsNextInput(ctx, "ING"))
                goto pos36700;
            mem58 = ctx->X;
        }

    pos37184:
        r = 0;
        do
        {
            while (1)
            {
                Y = mem65 + 1;
                if (Y == mem64)
                {
                    mem61 = mem60;

                    while (1)
                    {
                        mem57 = ctx->A = GetRuleByte(curRulePos, Y);
                        ctx->A = ctx->A & 127;
                        if (ctx->A != '=')
                            input[++phonemeOutputPos] = ctx->A;
                        if ((mem57 & 128) != 0)
                            goto pos36554;
                        Y++;
                    }
                }
                mem65 = Y;
                mem57 = GetRuleByte(curRulePos, Y);
                if ((tab36376[mem57] & 128) == 0)
                    break;
                if (ctx->inputtemp[mem58 + 1] != mem57)
                {
                    r = 1;
                    break;
                }
                ++mem58;
            }

            if (r == 0)
            {
                ctx->A = mem57;
                if (ctx->A == '@')
                {
                    if (Code37055(ctx, mem58 + 1, 4) == 0)
                    {
                        ctx->A = ctx->inputtemp[ctx->X];
                        if ((ctx->A != 82) && (ctx->A != 84) &&
                            (ctx->A != 67) && (ctx->A != 83))
                            r = 1;
                    }
                    else
                    {
                        r = -2;
                    }
                }
                else if (ctx->A == ':')
                {
                    while (Code37055(ctx, mem58 + 1, 32))
                        mem58 = ctx->X;
                    r = -2;
                }
                else
                    r = handle_ch(ctx, ctx->A, mem58 + 1);
            }

            if (r == 1)
                goto pos36700;
            if (r == -2)
            {
                r = 0;
                continue;
            }
            if (r == 0)
                mem58 = ctx->X;
        } while (r == 0);
    } while (ctx->A == '%');
    return 0;
}

unsigned int IsNextInput(SAMContext *ctx, const char *str)
{
    while (*str)
    {
        unsigned char ch = *str;
        ctx->A = ctx->inputtemp[ctx->X++];
        if (ctx->A != ch)
            return 0;
        ++str;
    }
    return 1;
}

/* Retrieve flags for character at mem59-1 */
unsigned char Code37055(SAMContext *ctx, unsigned char npos, unsigned char mask)
{
    ctx->X = npos;
    return tab36376[ctx->inputtemp[ctx->X]] & mask;
}

int handle_ch(SAMContext *ctx, unsigned char ch, unsigned char mem)
{
    unsigned char tmp;
    ctx->X = mem;
    tmp = tab36376[ctx->inputtemp[ctx->X]];
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
            if (ctx->inputtemp[ctx->X] != 72)
                return 1;
            ++ctx->X;
        }
    }
    else if (ch == '^')
    {
        if ((tmp & 32) == 0)
            return 1;
    }
    else if (ch == '+')
    {
        ctx->X = mem;
        ch = ctx->inputtemp[ctx->X];
        if ((ch != 69) && (ch != 73) && (ch != 89))
            return 1;
    }
    else
        return -1;
    return 0;
}

int handle_ch2(SAMContext *ctx, unsigned char ch, unsigned char mem)
{
    unsigned char tmp;
    ctx->X = mem;
    tmp = tab36376[ctx->inputtemp[mem]];
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

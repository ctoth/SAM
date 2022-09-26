#ifndef __PHONEME_PARSER_H__
#define __PHONEME_PARSER_H__
#include "sam.h"
signed int full_match(unsigned char sign1, unsigned char sign2);
signed int wild_match(unsigned char sign1);
int ParsePhonemes(SAMContext *ctx);

#endif // __PHONEME_PARSER_H__
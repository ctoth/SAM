#ifndef __RULES_H__
#define __RULES_H__

#include "sam.h"

void Insert(SAMContext *ctx, unsigned char position, unsigned char phonemeIndex, unsigned char phonemeLength, unsigned char stress);
void ChangeRule(SAMContext *ctx, unsigned char position, unsigned char phonemeIndex, const char *descr);
void rule_j(SAMContext *ctx, unsigned char pos);
void rule_g(SAMContext *ctx, unsigned char pos);
void rule_alveolar_uw(SAMContext *ctx, unsigned char X);
void rule_ch(SAMContext *ctx, unsigned char pos);
void rule_dipthong(SAMContext *ctx, unsigned char p, unsigned short pf, unsigned char pos);
void ApplyRules(SAMContext *ctx);
void CopyStressToPrevVowel(SAMContext *ctx);
void SetPhonemeLengths(SAMContext *ctx);
void AdjustPhonemeLengths(SAMContext *ctx);
void AddVoicedStopConsonants(SAMContext *ctx);
void CheckPhonemes(SAMContext *ctx);
void InsertBreath(SAMContext *ctx);
#endif // __RULES_H__
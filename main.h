#ifndef __EP2__main__
#define __EP2__main__

// Variáveis globais
extern mpf_t x;
extern mpf_t error;
extern mpf_t cosine;

extern bool shouldCompareTerms;
extern unsigned long numberOfRounds;

void sequencialCalc();
void mostre_uso(const char *nome_prog);

#endif /* defined(__EP2__main__) */
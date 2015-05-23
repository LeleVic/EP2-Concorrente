#include <iostream>
#include "math_2.h"

using namespace std;

/* Calcula o fatorial de x e retorna o valor em result.
*/
void factorial(mpf_t result, unsigned long x) {
	
	mpf_t aux;
	mpf_init(aux);
	mpf_init(result);	
	mpf_set_d(result, 1.0);
	mpf_set_ui(aux, x);

	if (x == 0) mpf_set_d(result, 1.0);

	while( mpf_cmp_ui(aux, 0) > 0 ) {
		mpf_mul(result, result, aux);
		mpf_sub_ui(aux, aux, 1);
	}
}

/* Calcula 'b' elevado a 'e' e retorna o valor em result.
*/
void power(mpf_t result, mpf_t b, double e) {

	int i;
	mpf_init(result);
	mpf_set_d(result, 1.0);	

	for (i = 0; i < e; i++) {
		mpf_mul(result, result, b);
	}
}
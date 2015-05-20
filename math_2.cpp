#include <iostream>
#include "math_2.h"

using namespace std;


void factorial(mpf_t result, unsigned long x) {
	
	mpf_t aux;
	mpf_init(aux);
	mpf_init(result);	
	mpf_set_d(result, 1.0);
	mpf_set_ui(aux, x);

	if (x == 0) mpf_set_d(result, 1.0);

	//for (aux; aux > 0; aux--) {
	while( mpf_cmp_ui(aux, 0) > 0 ) {
		mpf_mul(result, result, aux);
		mpf_sub_ui(aux, aux, 1);
		//result = result * aux;
	}
}


void power(mpf_t result, mpf_t b, double e) {

	int i;
	mpf_init(result);
	mpf_set_d(result, 1.0);	
	//double result = 1;

	for (i = 0; i < e; i++) {
		//result = result * b;
		mpf_mul(result, result, b);
	}
}
#include <iostream>
#include <cmath>
#include <thread>
#include <cstdlib>
#include <gmpxx.h>

#include "thread.h"
#include "main.h"
#include "math_2.h"


using namespace std;

// TODO: Análise de desempenho

int main(int argc, const char *argv[]) {

    int numThreads;
    int precisao; // A precisao do erro a ser usado. (10 elevado a -precisao)
    bool sequencial; // Indica se é para calcular usando threads ou de forma sequencial.
    unsigned long precision = 700000;
    mpf_set_default_prec(precision);

    if (argc < 5 || argc > 6)
    {
        mostre_uso(argv[0]);
        return -1;
    }

    if (atoi(argv[1]) == 0)
        numThreads = thread::hardware_concurrency(); // obtem a quantidade de núcleos do compuratador.
    else
        numThreads = atoi(argv[1]);

    if (argv[2][0] == 'f')
        setShouldCompareTerms(false);    
    else if (argv[2][0] == 'm')
        setShouldCompareTerms(true);

    precisao = atoi(argv[3]);

    if (argc == 6) {
        if (argv[5][0] == 'd') {
            setshouldPrintArrival(true);
            setShouldPrintCosine(true);
        }
        else if (argv[5][0] == 's') {
            setshouldPrintArrival(false);
            setShouldPrintCosine(true);
            sequencial = true;
        }
    }
    else {
        setshouldPrintArrival(false);
        setShouldPrintCosine(false);
    }

    setQ(numThreads); // Seta a quantidade de threads que serão criadas
    mpf_init(x); 
    mpf_init(error);
    setX(argv[4]); // Seta o valer de x que é para ser calculado.
    setError(pow(10, -precisao)); // Seta o valor do error.

    if (!sequencial) {
        initializeSemaphores();
        initializeBarrier();
        initializeEnvironment();
        startThreads();
        joinThreads();
        printInformation();
    }
    else {
        sequencialCalc();
        printInformation();
    }

    return 0;
}



/* Calcula o cosseno de forma sequencial. Sem uso de threads e
   programacao concorrente.
*/
void sequencialCalc()
{   
    mpf_t s;
    mpf_t numerator;
    mpf_t denominator;
    mpf_t d;
    mpf_t aux;
    mpf_t termo;
    mpf_init(s);    
    mpf_init(numerator);
    mpf_init(denominator);
    mpf_init(d);
    mpf_init(aux);
    mpf_init(termo);
    mpf_init(cosine);

    mpf_t lastCosine;
    mpf_init(lastCosine);

    unsigned long n = 0;
    double sign;
    bool done = false;

    while (!done) {

        sign = (!(n % 2)) * 2 - 1;
        power(numerator, x, 2*n);
        factorial(denominator, 2*n);
        mpf_div(d, numerator, denominator);
        mpf_set_d(s, sign);
        mpf_mul(termo, s, d);
        mpf_add(cosine, cosine, termo);

        mpf_abs(aux, termo);
        if (shouldCompareTerms && mpf_cmp(aux, error) < 0) {
            done = true;
        }
        else if (!shouldCompareTerms) {
            mpf_sub (aux, cosine, lastCosine);
            mpf_abs(aux, aux);
            if (mpf_cmp(aux, error) < 0)
                done = true;

            mpf_set(lastCosine, cosine);
        }

        cout << "Iteracao: " << n+1 << "\n";
        cout << "cosine: ";
        mpf_out_str(stdout, 10, 1000, cosine);
        cout << "\n\n";

        n++;
    }

    numberOfRounds = n+1;

    mpf_clear(s);
    mpf_clear(numerator);
    mpf_clear(denominator);
    mpf_clear(d);
    mpf_clear(aux);
    mpf_clear(termo);
    mpf_clear(lastCosine);
}



void mostre_uso(const char *nome_prog)
{
    fprintf(stdout, " Uso: %s quant_threads (f | m) presisao x [d | s]"
                    "\n", nome_prog);
}



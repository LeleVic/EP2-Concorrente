#include <cstdlib>
#include <cmath>
#include <pthread.h>

#include <iostream>
#include <condition_variable>

#include "math_2.h"
#include "thread.h"

using namespace std;


unsigned long q = 1;
vector<pthread_t> threads;

mpf_t x;
mpf_t error;
mpf_t cosine;

pthread_barrier_t barrier_term; // Sincroniza todas as threads apos o calculo do termo.
vector<mutex *> arrive;
condition_variable goCV;
mutex goMutex;
bool goReady;
bool goRound;

bool done; // indica que acabou de calcular o cosseno

mutex *cosineMutex;
mutex *printArrivalMutex;

bool isFirstArrival = true;
bool shouldPrintArrival = false;
bool shouldCompareTerms = false;
bool shouldPrintCosine = false;
unsigned long numberOfRounds;

void *threadFunction(void *id);
void printArrival(unsigned long i);

unsigned long getQ() {
    return q;
}

void setQ(unsigned long newValue) {
    q = newValue;
}

/*double getX() {
    return x;
}*/

void setX(double newValue) {
    mpf_set_d(x, newValue);
} 

/*double getError() {
    return error;
}*/

void setError(double newValue) {
    mpf_set_d(error, newValue);
    /*cout << "\nERROR: ";
    mpf_out_str(stdout, 10, 1000, error);
    cout << "\n"; */
}

void setshouldPrintArrival(bool newValue) {
    shouldPrintArrival = newValue;
}

void setShouldCompareTerms(bool newValue) {
    shouldCompareTerms = newValue;
}

void setShouldPrintCosine(bool newValue) {
    shouldPrintCosine = newValue;
}

void initializeEnvironment() {
    threads.clear();

    mpf_init(cosine);
    numberOfRounds = 0;

    goRound = true;
    goReady = true;

    done = false;

    isFirstArrival = true;
}

void startThreads() {
    for (unsigned long i = 0; i < q; i++) {
        pthread_t thread;

        if (pthread_create(&thread, nullptr, threadFunction, (void *)i))
            printf("\n ERROR creating thread %ld\n", i + 1);

        threads.push_back(thread);
    }
}

void initializeBarrier() {
	pthread_barrier_init(&barrier_term, NULL, q);
}

void initializeSemaphores() {

    // Cria um semaforo para cada thread que será utilizado para 
    // a implementação da barreira de sincronização das threads.
    for (unsigned long i = 0; i < q; i++) {

        mutex *a = new mutex();
        a->lock();
        arrive.push_back(a);
    }

    cosineMutex = new mutex(); 
    printArrivalMutex = new mutex();
}


void *threadFunction(void *id) {

    unsigned long i = (unsigned long)id;

    double sign = -1.0;
    mpf_t s;
    mpf_t numerator;
    mpf_t denominator;
    mpf_t d;
    mpf_t aux;
    mpf_t termo;
    mpf_init(s);
    mpf_set_d(s, sign);
    mpf_init(numerator);
    mpf_init(denominator);
    mpf_init(d);
    mpf_init(aux);
    mpf_init(termo);

    mpf_t lastCosine;
    mpf_init(lastCosine);

    unsigned long n;

    for (n = i; true; n += q) {

    	sign = (!(n % 2)) * 2 - 1;

        power(numerator, x, 2*n);
        factorial(denominator, 2*n);
        mpf_div(d, numerator, denominator);
        mpf_set_d(s, sign);
        mpf_mul(termo, s, d);
 
        mpf_abs(aux, termo);
 
        if (shouldCompareTerms && mpf_cmp(aux, error) < 0) {
            //cout << "DONE " << i << "\n";
            done = true;
        }

        // Parece que está funcionando com essa barreira. Mas acho que deu problema uma vez.
        //cout << "VALUE DONE = " << done << "\n";
        //cout <<  "N = " << n << "\n";
        pthread_barrier_wait(&barrier_term);

        //cout << "VALUE DONE = " << done << "\n";

        // POR QUE AS VEZES NAO ENTRA NESSE IF ???????
        // Por que ele ainda está na volta anterior!!!

       	if (done) {
        	//cout << "DONE T: " << i << "\n";
          	break;
        }

        // Região critica, atualizar o valor do cosseno.
        cosineMutex->lock();
        mpf_add(cosine, cosine, termo);
        cosineMutex->unlock();

        // É a ultima thread da lista.
        if (i == q - 1) {    

            if (shouldPrintArrival) {
                printArrival(i);
            }

            // Faz P(arrive) para todas as threads.
            for (unsigned long i = 0; i < q - 1; i++)
                arrive[i]->lock();

            // Imprime as informacoes do encontro na barreira, 
            // se estivers setada essa opcao.
            if (shouldPrintArrival) {
                isFirstArrival = true;                
                cout << "\n";
            }

            // Se o termino do programa eh para ser comparando os dois
            // ultimos conessos calculados, então apenas uma thread compara, na
            // barreira de sincronizacao
            if (!shouldCompareTerms) {
                mpf_sub (aux, cosine, lastCosine);
                mpf_abs(aux, aux);
                if (mpf_cmp(aux, error) < 0)
                    done = true;

                mpf_set(lastCosine, cosine);
            }

            if (shouldPrintCosine) {
            	cout << "cosine: ";
                mpf_out_str(stdout, 10, 1000, cosine); // FIXME: 100000
                cout << "\n\n";
            }

            goRound = !goRound;
            goReady = goRound;
            goCV.notify_all();

            //cout << "GO :" << i << "\n";

            //if (done) {
            //    cout << "DONE: " << i << "\n";
            //    cout <<  "N = " << n << "\n";
            //	break;
            //}

        } else {
            unique_lock<mutex> lock(goMutex);

            if (shouldPrintArrival)
                printArrival(i);

            // Organizacao da barreria.
            if (goRound) {
                arrive[i]->unlock();
                goCV.wait(lock, [] { return !goReady; });
            } else {
                arrive[i]->unlock();
                goCV.wait(lock, [] { return goReady; });
            }

            //cout << "GO :" << i << "\n";

            //if (done) {
            //	cout << "DONE: " << i << "\n";
            //	cout <<  "N = " << n << "\n";
            //   	break;
            //}
        }

        //cout << "GO :" << i << "\n";
    }

    if (i == q - 1) {
        numberOfRounds = (int) (n + 1)/q;
    }

    mpf_clear(s);
    mpf_clear(numerator);
    mpf_clear(denominator);
    mpf_clear(d);
    mpf_clear(aux);
    mpf_clear(termo);
    mpf_clear(lastCosine);

    return 0;
}

void printArrival(unsigned long i) {
    printArrivalMutex->lock();
    if (isFirstArrival) {
        isFirstArrival = false;
        cout << "Ordem de chegada das threads: ";
    }

    cout << i << " ";
    printArrivalMutex->unlock();
}

void joinThreads() {
    for (unsigned long i = 0; i < q; i++) {
        pthread_join(threads[i], NULL);
    }
}

void printInformation() {
    cout << "=========== Results ==========\n";

    cout << "Cosine = ";
    mpf_out_str(stdout, 10, 1000, cosine); // FIX ME: 100000. O numero fica muito grande!!!!!
    cout << "\n\n";

    cout << "Number of rounds = " << numberOfRounds << "\n";
}
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
// Variaveis utilizadas na barreira para sincronizar a comparacao dos valores do cosseno
vector<mutex *> arrive; 
condition_variable goCV;
mutex goMutex;
bool goReady;
bool goRound;

bool done; // indica que acabou de calcular o cosseno

mutex *cosineMutex; // Semaforo para escrever na variável cosseno.
mutex *printArrivalMutex; // Semaforo para escrever na tela.

bool isFirstArrival = true;
bool shouldPrintArrival = false;
bool shouldCompareTerms = false;
bool shouldPrintCosine = false;
unsigned long numberOfRounds; // salva a quantidade de vezes que as threads se encontraram
                              // na barreira.

void *threadFunction(void *id);
void printArrival(unsigned long i);

unsigned long getQ() {
    return q;
}

void setQ(unsigned long newValue) {
    q = newValue;
}

void setX(const char *newValue) {
    mpf_set_str(x, newValue, 10);
}

void setError(double newValue) {
    mpf_set_d(error, newValue);
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
        // Compara o termo calculado com erro, se a opcao passada for a m.
        if (shouldCompareTerms && (mpf_cmp(aux, error) < 0)) {
            done = true;
        }

        // Barreira para sincronizar o calculo dos termos.
        pthread_barrier_wait(&barrier_term);  

        // Região critica, atualizar o valor do cosseno.
        cosineMutex->lock();
        mpf_add(cosine, cosine, termo);
        cosineMutex->unlock();

        // Termina a execucao da thread.
        if (done && shouldCompareTerms) {
            break;
        }

        if (i == q - 1) {    

            if (shouldPrintArrival) {
                printArrival(i);
            }

            // Espera todas as outras threads (threads 0 a q-2),
            // avisarem que chegaram na barreira.
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
            // barreira de sincronizacao.
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

            // Libera as outras threads para continuar.
            goRound = !goRound;
            goReady = goRound;
            goCV.notify_all();

        } else {
            unique_lock<mutex> lock(goMutex);

            if (shouldPrintArrival)
                printArrival(i);
            
            // Espera mudar o valor de goRound.
            // A thread q-1, ira mudar o valor de goRound quando liberar 
            // as outras threads para continuar sua execucao.
            if (goRound) {
                // Avisa que chegou na barreira.
                arrive[i]->unlock();
                // Espera poder continuar.
                goCV.wait(lock, [] { return !goReady; });
            } else {
                // Avisa que chegou na barreira.
                arrive[i]->unlock();
                // Espera poder continuar.
                goCV.wait(lock, [] { return goReady; });
            }
        }

        // Termina a execucao da thread.
        if (done && !shouldCompareTerms) {
            break;
        }        
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
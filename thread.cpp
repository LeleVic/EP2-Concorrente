#include <cstdlib>
#include <cmath>
#include <pthread.h>

#include <iostream>
#include <condition_variable>

#include "math_2.h"
#include "thread.h"

using namespace std;


unsigned long q = 1;
mpf_t x;
mpf_t error;

vector<pthread_t> threads;

//vector<mpf_t> terms;
mpf_t *terms;

mpf_t cosine;

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
    //x = newValue;
    mpf_init(x);
    mpf_set_d(x, newValue);
} 

/*double getError() {
    return error;
}*/

void setError(double newValue) {
    //error = newValue;
    mpf_init(error);
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

    //initializeMathWithPowerBase(x);

    //terms.clear();
    // Faz um vetor com malloc, pois se utilizar a classe vector dá erro no compilador!
    terms = (mpf_t *) malloc(q * sizeof(mpf_t));
    for (unsigned long i = 0; i < q; i++) {
        mpf_t a;
        mpf_init(a);
        mpf_set(terms[i], a);
        //terms.push_back(a);
        //terms.push_back(0.0); // inicializa o termo que cada thread vai calcular com 0.
    }

    //cosine = 0;
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
    //double numerator;
    //double denominator;
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
    //double lastCosine = 0;

    unsigned long n;

    for (n = i; true; n += q) {
        sign = (!(n % 2)) * 2 - 1;

        power(numerator, x, 2*n);
        factorial(denominator, 2*n);
        mpf_div(d, numerator, denominator);
        mpf_set_d(s, sign);
        mpf_mul(termo, s, d);
        //terms[i] = sign * numerator / denominator;
        //mpf_set(terms[i], aux);

        //mpf_abs(aux, terms[i]);

        // Estou usando a variável termo, pois o vetor terms[i], não estava funcionando.
        mpf_abs(aux, termo);
        // Não está funcionando muito bem. As vezes não termina!
        // impreme esse done, mas não vai pra frente!!
        if (shouldCompareTerms && mpf_cmp(aux, error) < 0) {
            //cout << "\n AUX";
            //mpf_out_str(stdout, 10, 0, cosine);
            //cout << "\n\n";
            cout << "DONE\n";
            done = true;
        }

        // Região critica, atualizar o valor do cosseno.
        cosineMutex->lock();
        //mpf_add(cosine, cosine, terms[i]);
        mpf_add(cosine, cosine, termo);
        //cosine += terms[i];
        cosineMutex->unlock();

        // É a ultima thread da lista.
        if (i == q - 1) {    

            if (shouldPrintArrival) {
                printArrival(i);
            }

            // Faz P(arrive) para todas as threads.
            for (unsigned long i = 0; i < q - 1; i++)
                arrive[i]->lock();

            //updateMath();

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
                //cout << "cosine: " << cosine << "\n";
                mpf_out_str(stdout, 10, 0, cosine);
                cout << "\n\n";
            }

            // Nao entendi esse round e ready!!!!
            goRound = !goRound;
            goReady = goRound;
            goCV.notify_all();

            if (done)
                break;

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

            if (done)
                break;
        }
    }

    if (i == q - 1) {
        numberOfRounds = (int) (n + 1)/4;
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

    //cout << "Cosine = " << cosine << "\n";
    mpf_out_str(stdout, 10, 100000, cosine);
    cout << "\n\n";

    cout << "Number of rounds = " << numberOfRounds << "\n";
}
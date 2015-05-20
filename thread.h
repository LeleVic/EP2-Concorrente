#ifndef __EP2__thread__
#define __EP2__thread__

#include <gmpxx.h>
#include <vector>
#include <mutex>

unsigned long getQ();
void setQ(unsigned long newValue);
double getX();
void setX(double newValue);
double getError();
void setError(double newValue);
void setshouldPrintArrival(bool newValue);
void setShouldCompareTerms(bool newValue);
void setShouldPrintCosine(bool newValue);

void initializeSemaphores();
void initializeEnvironment();
void startThreads();
void *threadFunction(void *id);
void joinThreads();
void printInformation();


#endif  /* defined(__EP2__thread__) */
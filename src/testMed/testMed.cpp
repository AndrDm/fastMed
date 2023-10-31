#include <windows.h>
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "..\\libMed\libMed.h"

using namespace std;
using namespace std::chrono;

int main()
{
    double qsMed=0.0, fastMed=0.0;

    printf("Median Test\n");
    srand(GetTickCount());

    for (short n = 1024; n <= 4096; n *= 2) {
        double* arrDBL = (double*)malloc(n * n * sizeof(double));
        if (arrDBL) for (int i = 0; i < n * n; i++) arrDBL[i] = ((double)rand()/RAND_MAX); //RAND_MAX
        cout << n * n << " Elelemnts:" << endl;
        auto start = system_clock::now();
        MedianDBL(arrDBL, n * n, &fastMed);
        auto end = system_clock::now();
        cout << "myMed - "<< duration_cast<milliseconds>(end - start).count() << " ms; ";

        start = system_clock::now();
        QuickSelectDBL(arrDBL, n * n, &qsMed);
        end = system_clock::now();
        cout << "qsMed - " << duration_cast<milliseconds>(end - start).count() << " ms" << endl;
 
        printf("Quick Select Med %f, Fast Med %f", qsMed, fastMed);
        if (qsMed == fastMed) printf(" - passed\n");

        unsigned int* arrU32 = (unsigned int*)malloc(n * n * sizeof(unsigned int));
        if (arrU32) for (int i = 0; i < n * n; i++) arrU32[i] = ((unsigned int)rand());

        start = system_clock::now();
        MedianU32(arrU32, n * n, &fastMed);
        end = system_clock::now();
        cout << "myMed U32 - " << duration_cast<milliseconds>(end - start).count() << " ms; " << endl;

        unsigned short* arrU16 = (unsigned short*)malloc(n * n * sizeof(unsigned short));
        if (arrU16) for (int i = 0; i < n * n; i++) arrU16[i] = ((unsigned short)rand());

        start = system_clock::now();
        MedianU16(arrU16, n * n, &fastMed);
        end = system_clock::now();
        cout << "myMed U16 - " << duration_cast<milliseconds>(end - start).count() << " ms; " << endl;

        free(arrDBL);
        free(arrU32);
        free(arrU16);
    }
}

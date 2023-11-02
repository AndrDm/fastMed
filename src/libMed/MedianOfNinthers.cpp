// MedianOfNinthers.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "MedianOfNinthers.h"
#include "libMed.h"

template <class T>
static void quickselect(T* beg, T* mid, T* end)
{
    if (beg == end || mid >= end) return;
    assert(beg <= mid && mid < end);
    adaptiveQuickselect(beg, mid - beg, end - beg);
}

void (*computeSelection)(double*, double*, double*) = &quickselect<double>;


// This is exported function.
LIBMED_API int fnMedianOfNinthers(double* arr, int n, double *med)
{
    computeSelection(arr, arr + n/2, arr + n);
    if (n&1) *med = arr[(n / 2)];
    else *med = (arr[(n / 2) - 1] + arr[(n / 2)]) / 2.0; //this doesn't work well
    return 0;
}

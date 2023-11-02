#include "framework.h"
#include "libMed.h"

#define xFFFF 0xFFFFFFFFFFFFFFFF
#define x8000 0x8000000000000000
#define x0000 0x0000000000000000
#define xF000 0xFFFF000000000000
#define x0F00 0x0000FFFF00000000
#define x00F0 0x00000000FFFF0000
#define x000F 0x000000000000FFFF

//0 - LSW; 3 - MSW
#define	W3 (ptr_U64[i] & 0xFFFF000000000000)
#define	W2 (ptr_U64[i] & 0x0000FFFF00000000)
#define	W1 (ptr_U64[i] & 0x00000000FFFF0000)
#define	W0 (ptr_U64[i] & 0x000000000000FFFF)
#define	W32 (ptr_U64[i] & 0xFFFFFFFF00000000)
#define	W321 (ptr_U64[i] & 0xFFFFFFFFFFFF0000)

#define NANh  0x7FFF000000000000
#define NANl  0x0000FFFFFFFFFFFF

#define ENCODE(var) if (!((var) & x8000)) (var) ^= x8000; else (var) ^= xFFFF
#define DECODE(var) if (((var) & x8000)) (var) ^= x8000; else (var) ^= xFFFF

LIBMED_API int MedianDBL(double* ptr, size_t Length, double* med)
{
	if (!ptr || !Length) return -1;
	size_t* histo = (size_t*)calloc(USHRT_MAX + 1, sizeof(size_t));
	if (!histo) return -2;
	uint64_t count, i, medianPos;
	uint64_t min0, min1, min2, max0, max1, max2;
	uint64_t medValW0 = 0, medValW1 = 0, medValW2 = 0, medValW3 = 0; 
	uint64_t medianLeft, medianRight, medVal, nextVal, med_;


	uint64_t* ptr_U64 = (uint64_t*)ptr; //Cast

	for (i = 0; i < Length; i++) {
		if (((ptr_U64[i] & NANh) == NANh) && ((ptr_U64[i] & NANl) != 0)) {
			*med = ptr[i];
			for (int j = 0; j < i; j++) DECODE(ptr_U64[j]); //revert back
			free(histo);
			return 0;
		}
		ENCODE(ptr_U64[i]);
		histo[W3 >> 48]++;
	}

	// Find the initial median position;
	medianPos = (Length + 1) / 2;
	for (count = 0, i = 0; i <= USHRT_MAX; i++) { // Iterate over histogram
		count += histo[i];
		if (count >= medianPos) { medValW3 = i; if (count > medianPos) count-= histo[i]; break; } // >> single Median at MSW
	}

	if (!(Length & 1) && count == medianPos) { //W3
		nextVal = medValW3;
		while (!histo[++nextVal]);
		medValW3 <<= 48; nextVal <<= 48;

		for (max2 = 0, min2 = _UI64_MAX, i = 0; i < Length; i++) { //1.2 pass
			if (medValW3 == W3 && W2 > max2) max2 = W2;
			if (W3 == nextVal && W2 < min2) min2 = W2;
		}

		for (max1 = 0, min1 = _UI64_MAX, i = 0; i < Length; i++) { //1.3 pass
			if (medValW3 == W3 && max2 == W2 && W1 > max1) max1 = W1;
			if (W3 == nextVal && min2 == W2 && W1 < min1) min1 = W1;
		}

		for (max0 = 0, min0 = _UI64_MAX, i = 0; i < Length; i++) { //1.4 pass, final
			if (medValW3 == W3 && max2 == W2 && max1 == W1 && W0 > max0) max0 = W0;
			if (W3 == nextVal && min2 == W2 && min1 == W1 && W0 < min0) min0 = W0;
			DECODE(ptr_U64[i]);
		} // 1.2 - 1.4

		medianLeft = medValW3 | max2 | max1 | max0;
		DECODE(medianLeft);
		medianRight = nextVal | min2 | min1 | min0;
		DECODE(medianRight);
		*med = (*(double*)&medianLeft + *(double*)&medianRight) / 2.0;
	} else {
		memset(histo, 0, (USHRT_MAX + 1) * sizeof(size_t));
		med_ = (medValW3 <<= 48);
		//count = 0;
		for (i = 0; i < Length; i++) { //2.2 pass
			if (medValW3 == W3) histo[W2 >> 32]++;
			//if (ptr_U64[i] < med_) count++;
		} // 2.2

		for (int i = 0; i <= USHRT_MAX; i++) {
			count += histo[i]; //continue
			if (count >= medianPos) { medValW2 = i; break; }
		}

		if (!(Length & 1) && count == medianPos) { // W2
			nextVal = medValW2;
			while (!histo[++nextVal]);
			medValW2 <<= 32;
			nextVal <<= 32;

			for (max1 = 0, min1 = _UI64_MAX, i = 0; i < Length; i++) { //2.3 pass
				if (medValW3 == W3 && medValW2 == W2 && W1 > max1) max1 = W1;
				if (W3 == medValW3 && nextVal == W2 && W1 < min1) min1 = W1;
			}

			for (max0 = 0, min0 = _UI64_MAX, i = 0; i < Length; i++) { //2.4 pass, final
				if (medValW3 == W3 && medValW2 == W2 && max1 == W1 && W0 > max0) max0 = W0;
				if (W3 == medValW3 && nextVal == W2 && min1 == W1 && W0 < min0) min0 = W0;
				DECODE(ptr_U64[i]);
			} //2.3-2.4

			medianLeft = medValW3 | medValW2 | max1 | max0;
			DECODE(medianLeft);
			medianRight = medValW3 | nextVal | min1 | min0;
			DECODE(medianRight);
			*med = (*(double*)&medianLeft + *(double*)&medianRight) / 2.0;
		} else {
			memset(histo, 0, (USHRT_MAX + 1) * sizeof(size_t));
			med_ = medValW3 | (medValW2 <<= 32);
			for (count = 0, i = 0; i < Length; i++) { //3.3 pass
				//if (medValW3 == W3 && medValW2 == W2) histo[W1 >> 16]++;
				if (med_ == W32) histo[W1 >> 16]++;
				if (ptr_U64[i] < med_) count++;
			} //3.3

			for (int i = 0; i <= USHRT_MAX; i++) {
				count += histo[i];
				if (count >= medianPos) { medValW1 = i; break; }
			}

			if (!(Length & 1) && count == medianPos) { //W1
				nextVal = medValW1;
				while (!histo[++nextVal]);
				medValW1 <<= 16;
				nextVal <<= 16;

				for (max0 = 0, min0 = _UI64_MAX, i = 0; i < Length; i++) { //3.4 pass, final
					if ((medValW3 == W3) && (medValW2 == W2) && (medValW1 == W1) && W0 > max0) max0 = W0;
					if ((W3 == medValW3) && (medValW2 == W2) && (nextVal == W1) && W0 < min0) min0 = W0;
					DECODE(ptr_U64[i]);
				} //3.4

				medianLeft = medValW3 | medValW2 | medValW1 | max0;
				DECODE(medianLeft);
				medianRight = medValW3 | medValW2 | nextVal | min0;
				DECODE(medianRight);
				*med = (*(double*)&medianLeft + *(double*)&medianRight) / 2.0;
			} else {
				memset(histo, 0, (USHRT_MAX + 1) * sizeof(size_t));
				med_ = medValW3 | medValW2 | (medValW1 <<= 16);
				for (count = 0, i = 0; i < Length; i++) {//!! Spaziergang !!
					//if (medValW3 == W3 && medValW2 == W2 && medValW1 == W1) histo[W0]++;
					if (med_ == W321) histo[W0]++;
					if (ptr_U64[i] < med_) count++;
					DECODE(ptr_U64[i]); //restore back
				} //4.4, final

				for (int i = 0; i <= USHRT_MAX; i++) {
					count += histo[i];
					if (count >= medianPos) { medValW0 = i; break; }
				}

				if (!(Length & 1) && count == medianPos) {
					nextVal = medValW0;
					while (!histo[++nextVal]);
					medianLeft = medValW3 | medValW2 | medValW1 | medValW0;
					DECODE(medianLeft);
					medianRight = medValW3 | medValW2 | medValW1 | nextVal;
					DECODE(medianRight);
					*med = (*(double*)&medianLeft + *(double*)&medianRight) / 2.0;
				} else {
					medVal = medValW3 | medValW2 | medValW1 | medValW0;
					DECODE(medVal);
					*med = *(double*)&medVal;
				}
			}
		}
	}
	free(histo);
	return 0;
} //MedianDBL


#define SWAP(a,b) {temp=a;a=b;b=temp;}
//https://www.stat.cmu.edu/~ryantibs/median/
//Numerical Recipes, 3rd Edition, Chapter 8.5 Selecting the Mth Largest (page 457(433))
LIBMED_API int QuickSelectDBL(double* src, unsigned int n, double* med) {
    unsigned long k = n / 2; double temp;
    unsigned long l = 0;
    unsigned long ir = n - 1;
	double* arr;
	if (!src || !n) return -1;
	arr = (double*)malloc(n * sizeof(double));
	if (!arr) return -2;
	memcpy(arr, src, n * sizeof(double));
	double med1, med2;
    for (;;) {
        if (ir <= l + 1) {
            if (ir == l + 1 && arr[ir] < arr[l]) SWAP(arr[l], arr[ir]);
            med1 = arr[k];
            break;
        } else {
            unsigned long mid = (l + ir) >> 1;
            SWAP(arr[mid], arr[l + 1]);
            if (arr[l] > arr[ir]) SWAP(arr[l], arr[ir]);
            if (arr[l + 1] > arr[ir]) SWAP(arr[l + 1], arr[ir]);
            if (arr[l] > arr[l + 1]) SWAP(arr[l], arr[l + 1]);
            unsigned long i = l + 1;
            unsigned long j = ir;
            double a = arr[l + 1];
            for (;;) {
                do i++; while (arr[i] < a);
                do j--; while (arr[j] > a);
                if (j < i) break;
                SWAP(arr[i], arr[j]);
            }
            arr[l + 1] = arr[j];
            arr[j] = a;
            if (j >= k) ir = j - 1;
            if (j <= k) l = i;
        }
    }

	if (!(n & 1)) {
		memcpy(arr, src, n * sizeof(double));
		k--;
		l = 0;
		ir = n - 1;
		for (;;) {
			if (ir <= l + 1) {
				if (ir == l + 1 && arr[ir] < arr[l]) SWAP(arr[l], arr[ir]);
				med2 = arr[k];
				break;
			} else {
				unsigned long mid = (l + ir) >> 1;
				SWAP(arr[mid], arr[l + 1]);
				if (arr[l] > arr[ir]) SWAP(arr[l], arr[ir]);
				if (arr[l + 1] > arr[ir]) SWAP(arr[l + 1], arr[ir]);
				if (arr[l] > arr[l + 1]) SWAP(arr[l], arr[l + 1]);
				unsigned long i = l + 1;
				unsigned long j = ir;
				double a = arr[l + 1];
				for (;;) {
					do i++; while (arr[i] < a);
					do j--; while (arr[j] > a);
					if (j < i) break;
					SWAP(arr[i], arr[j]);
				}
				arr[l + 1] = arr[j];
				arr[j] = a;
				if (j >= k) ir = j - 1;
				if (j <= k) l = i;
			}
		}
		*med = (med1 + med2) / 2.0;
	} else {
		*med = med1;
	}
	free(arr);
	return 0;
}

LIBMED_API int MedianU32(unsigned int* ptr, size_t Length, double* med)
{
	size_t* histo;
	int i;
	uint64_t medianLeft, medianRight, indexMSW, nextVal, med_;
	uint64_t medianValI32, medianValLSW, medianValMSW = 0, min, max;
	
	if (!ptr || !Length) return -1;
	histo = (size_t*)calloc(USHRT_MAX + 1, sizeof(size_t));
	if (!histo) return -2;

	//MSW HISTO:
	for (int i = 0; i < Length; i++) histo[(ptr[i] & 0xFFFF0000) >> 16]++;

	// Find the median position;
	uint64_t medianPos = (Length + 1) / 2, count = 0;

	for (int i = 0; i <= USHRT_MAX; i++) { // Iterate over histogram to find the med
		count += histo[i];
		if (count >= medianPos) { medianValMSW = i; break; }
	}
	if (!(Length % 2 || count > medianPos)) { // If the size is even, find the next
		nextVal = medianValMSW;
		while (!histo[++nextVal]);

		for (max = 0, min = 65535, i = 0; i < Length; i++) {
			indexMSW = (ptr[i] & 0xFFFF0000) >> 16;
			if (medianValMSW == indexMSW && (ptr[i] & 0x0000FFFF) > max) max = (ptr[i] & 0x0000FFFF);
			if (indexMSW == nextVal && (ptr[i] & 0x0000FFFF) < min) min = (ptr[i] & 0x0000FFFF);
		}

		medianLeft = (medianValMSW << 16) | max;
		medianRight = (nextVal << 16) | min;
		*med = ((double)medianLeft + (double)medianRight) / 2.0;
	} else { //LSW HISTO
		ZeroMemory(histo, (USHRT_MAX + 1) * sizeof(size_t));
		med_ = medianValMSW << 16;
		for (count = 0, i = 0; i < Length; i++) {
			indexMSW = (ptr[i] & 0xFFFF0000) >> 16;
			if (medianValMSW == indexMSW) histo[(ptr[i] & 0x0000FFFF)]++;
			if (ptr[i] < med_) count++;
		}

		// Find the median position;
		medianValLSW = 0;
		for (int i = 0; i <= USHRT_MAX; i++) { // Iterate over histogram 
			count += histo[i];
			if (count >= medianPos) { medianValLSW = i; break; }
		}

		if (!(Length % 2 || count > medianPos)) {
			nextVal = medianValLSW;
			while (!histo[++nextVal]);

			medianLeft = (medianValMSW << 16) | medianValLSW;
			medianRight = (medianValMSW << 16) | nextVal;
			*med = ((double)medianLeft + (double)medianRight) / 2.0;
		}
		else {
			medianValI32 = (medianValMSW << 16) | medianValLSW;
			*med = (double)medianValI32;
		}
	}
	free(histo);
	return 0;
} //MedianU32

LIBMED_API int MedianU16(unsigned short* ptr, size_t Length, double* med)
{
	size_t* histogram;
	if (!ptr || !Length) return -1;
	histogram = (size_t*)calloc(USHRT_MAX + 1, sizeof(size_t));  // calloc init to zero
	if (!histogram) return -2;

	for (int i = 0; i < Length; i++) histogram[ptr[i]]++;

	// Iterate over histogram to find the median position;
	size_t medianPos = (Length + 1) / 2, count = 0, medianVal = 0;
	for (int i = 0; i <= USHRT_MAX; i++) {
		count += histogram[i];
		if (count >= medianPos) { medianVal = i; break; }
	}
	if (!(Length % 2 || count > medianPos)) { // If the size is even, find the next
		size_t nextVal = medianVal;
		while (!histogram[++nextVal]);
		*med = (medianVal + nextVal) / 2.0; // Middle
	}
	else *med = (double)medianVal;

	free(histogram);
	return 0;
}//MedianU16

//ChatGPT version, slightly modified:
double computeMedian(unsigned short* arr, size_t size)
{
	size_t* histogram;
	double Median;
	if (!arr || !size) return -1;
	if (!( histogram = (size_t*)calloc(USHRT_MAX + 1, sizeof(size_t)))) return -2;

	for (int i = 0; i < size; i++) histogram[arr[i]]++;    // Populate histogram
	size_t medianPos = (size + 1) / 2, count = 0, medianVal; // Find the median position
	for (medianVal = 0; medianVal <= USHRT_MAX; medianVal++) { // Iterate over histogram
		count += histogram[medianVal];
		if (count >= medianPos) break;
	}

	if (!(size % 2 || count > medianPos)) { // If the size is even, find the next
		size_t nextVal = medianVal;
		while (!histogram[++nextVal]);
		Median = (medianVal + nextVal) / 2.0; // Middle
	}
	else Median = (double)medianVal;

	free(histogram);
	return Median;
}


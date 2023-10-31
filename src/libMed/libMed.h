// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the LIBMED_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
#ifdef LIBMED_EXPORTS
#define LIBMED_API extern "C" __declspec(dllexport) 
#else
#define LIBMED_API extern "C" __declspec(dllimport)

#ifdef _DEBUG
#pragma comment(lib, "..\\\\x64\\Debug\\libmed")
#else
#pragma comment(lib, "..\\\\x64\\Release\\libmed")
#endif

#endif

#pragma comment(linker, "/EXPORT:fastMedian=MedianDBL") //for Python

LIBMED_API int  __cdecl QuickSelectDBL(double* arr, unsigned int n, double* med);
LIBMED_API int  __cdecl MedianDBL(double* ptr, size_t Length, double* med);
LIBMED_API int  __cdecl MedianU32(unsigned int* ptr, size_t Length, double* med);
LIBMED_API int  __cdecl MedianU16(unsigned short* ptr, size_t Length, double* med);

/* Error Codes */
typedef enum {
    NoErr = 0,	// No Error.
    SamplesErr = -1,	// Number of samples must be greater than zero.
} MedianLibErrType;

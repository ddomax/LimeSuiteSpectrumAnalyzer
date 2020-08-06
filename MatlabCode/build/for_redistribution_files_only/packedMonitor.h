/*
 * MATLAB Compiler: 6.4 (R2017a)
 * Date: Thu Aug 06 01:57:34 2020
 * Arguments:
 * "-B""macro_default""-W""lib:packedMonitor""-T""link:lib""-d""D:\Qt_workspace\
 * LimeSuiteSpectrumAnalyzer\MatlabCode\build\for_testing""-v""D:\Qt_workspace\L
 * imeSuiteSpectrumAnalyzer\MatlabCode\packedMonitor.m"
 */

#ifndef __packedMonitor_h
#define __packedMonitor_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_packedMonitor
#define PUBLIC_packedMonitor_C_API __global
#else
#define PUBLIC_packedMonitor_C_API /* No import statement needed. */
#endif

#define LIB_packedMonitor_C_API PUBLIC_packedMonitor_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_packedMonitor
#define PUBLIC_packedMonitor_C_API __declspec(dllexport)
#else
#define PUBLIC_packedMonitor_C_API __declspec(dllimport)
#endif

#define LIB_packedMonitor_C_API PUBLIC_packedMonitor_C_API


#else

#define LIB_packedMonitor_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_packedMonitor_C_API 
#define LIB_packedMonitor_C_API /* No special import/export declaration */
#endif

extern LIB_packedMonitor_C_API 
bool MW_CALL_CONV packedMonitorInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_packedMonitor_C_API 
bool MW_CALL_CONV packedMonitorInitialize(void);

extern LIB_packedMonitor_C_API 
void MW_CALL_CONV packedMonitorTerminate(void);



extern LIB_packedMonitor_C_API 
void MW_CALL_CONV packedMonitorPrintStackTrace(void);

extern LIB_packedMonitor_C_API 
bool MW_CALL_CONV mlxPackedMonitor(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);



extern LIB_packedMonitor_C_API bool MW_CALL_CONV mlfPackedMonitor(mxArray* iqBuffer, mxArray* sampleNum, mxArray* NFFT);

#ifdef __cplusplus
}
#endif
#endif

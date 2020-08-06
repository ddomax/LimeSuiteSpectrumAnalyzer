#include "spectrummonitor.h"
#include <chrono>
#include "packedMonitor.h"
#include <QDebug>

using namespace std;

SpectrumMonitor::SpectrumMonitor(QObject *parent) : QObject(parent)
{
    packedMonitorInitialize();
    qDebug() << "packedMonitorInitialize";
}

void SpectrumMonitor::runMonitor()
{
    mxArray *iqBuffer = mxCreateNumericMatrix(1,paramSampleNumMax*2,mxINT16_CLASS,mxREAL);
    mxArray *sampleNum = mxCreateDoubleMatrix(1,1,mxREAL);
    mxArray *NFFT = mxCreateDoubleMatrix(1,1,mxREAL);
    isRunning = true;
    while (isRunning)
    {
//        auto tic = chrono::high_resolution_clock::now();
//        while (chrono::high_resolution_clock::now() - tic < chrono::milliseconds(100));
        if (!*processDone)
        {
            memcpy((void*)mxGetPr(iqBuffer), (void*)inBuffer, 2*paramSampleNum*sizeof(int16_t));
            memcpy((void*)mxGetPr(sampleNum), (void*)&paramSampleNum, sizeof(paramSampleNum));
            memcpy((void*)mxGetPr(NFFT), (void*)&paramNFFT, sizeof(paramNFFT));

//            auto toc = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - tic);
//            qDebug() << "Matrix Creation: " << toc.count();
            mlfPackedMonitor(iqBuffer, sampleNum, NFFT);
//            toc = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - tic);
//            qDebug() << "Processing Done: " << toc.count();
            *processDone = true;
        }
    }
}

void SpectrumMonitor::stopMonitor()
{
    isRunning = false;
//    packedMonitorTerminate();
}

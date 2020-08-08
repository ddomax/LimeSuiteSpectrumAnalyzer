#include "spectrummonitor.h"
#include <chrono>
#include "packedMonitor.h"
#include <QDebug>

using namespace std;

SpectrumMonitor::SpectrumMonitor(QObject *parent) : QObject(parent)
{
    packedMonitorInitialize();
//    settingsDB[settingList::detector] = 2;
//    *saveDone = true;
    qDebug() << "packedMonitorInitialize";
}

void SpectrumMonitor::runMonitor()
{
    static mxArray *iqBuffer = mxCreateNumericMatrix(1,paramSampleNumMax*2,mxINT16_CLASS,mxREAL);
    static mxArray *settings = mxCreateDoubleMatrix(1,6,mxREAL);
    static mxArray *freqOut = NULL;
    static mxArray *fftOut = NULL;
    static double *freqDB,*fftDB;
//  auto tic = chrono::high_resolution_clock::now();
    memcpy((void*)mxGetPr(iqBuffer), (void*)inBuffer, 2*settingsDB[settingList::sampleNum]*sizeof(int16_t));
    memcpy((void*)mxGetPr(settings), (void*)settingsDB, sizeof(settingsDB));

//  auto toc = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - tic);
//  qDebug() << "Matrix Creation: " << toc.count();
    mlfPackedMonitor(2, &freqOut, &fftOut, iqBuffer, settings);
//  toc = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - tic);
//  qDebug() << "Processing Done: " << toc.count();
    freqDB = mxGetPr(freqOut);
    fftDB = mxGetPr(fftOut);
    if (*drawDone)
    {
        memcpy(freqOutBuffer, freqDB, FFTOUTNUM*sizeof(double));
        memcpy(fftOutBuffer, fftDB, FFTOUTNUM*sizeof (double));
        *drawDone = false;
        emit rePlot();
    }
    else
    {
        qDebug() << "Loss Plot!";
    }

    if (*saveDone)
    {
        memcpy(freqSaveBuffer, freqDB, FFTOUTNUM*sizeof(double));
        memcpy(fftSaveBuffer, fftDB, FFTOUTNUM*sizeof (double));
        *saveDone = false;
        emit saveFrame(FFTOUTNUM);
    }
    else
    {
        qDebug() << "Loss Save!";
    }

    *processDone = true;
}

void SpectrumMonitor::setNFFT(double nfft)
{
    settingsDB[settingList::NFFT] = nfft;
}

void SpectrumMonitor::setSpan(double span)
{
    settingsDB[settingList::Fs] = span;
}

void SpectrumMonitor::setSampleNum(double sampleNum)
{
    settingsDB[settingList::sampleNum] = sampleNum;
}

void SpectrumMonitor::setCenterFreq(double freq)
{
    settingsDB[settingList::centerFreq] = freq;
}

void SpectrumMonitor::setGain(double gain)
{
    settingsDB[settingList::gain] = gain;
}

void SpectrumMonitor::setDetector(const QString &detector)
{
    qDebug() << "Setting Detecotor";
//    settingsDB[5] = 2.0;
    if (detector == "Sample")
        settingsDB[settingList::detector] = 1;
    else if(detector == "Peak")
        settingsDB[settingList::detector] = 2;
    else
        qDebug() << "Invalid Detecor Type!";
}

void SpectrumMonitor::setSaving(bool enable)
{
    isSaving = enable;
}

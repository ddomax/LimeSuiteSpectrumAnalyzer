#ifndef SPECTRUMMONITOR_H
#define SPECTRUMMONITOR_H

#include <QObject>
#include "packedMonitor.h"

class SpectrumMonitor : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumMonitor(QObject *parent = nullptr);
    int16_t *inBuffer;
    double *freqOutBuffer;
    double *fftOutBuffer;
    double *freqSaveBuffer;
    double *fftSaveBuffer;
    bool *processDone;
    bool *drawDone;
    bool *saveDone;
    const int paramSampleNumMax = 1<<22;
    void setSampleNum(double sampleNum);
    void setNFFT(double nfft);
    void setCenterFreq(double freq);
    void setSpan(double span);
    void setGain(double gain);
    void setDetector(const QString& detector);
    void setSaving(bool enable);
    static const int FFTOUTNUM = 16384;

public slots:
    void runMonitor();

signals:
    void rePlot();
    void saveFrame(int frameSize);

private:
    bool isRunning = false;
    bool isSaving = false;
    double settingsDB[6];
    enum settingList
    {
        sampleNum, NFFT, centerFreq, gain, Fs, detector
    };

};

#endif // SPECTRUMMONITOR_H

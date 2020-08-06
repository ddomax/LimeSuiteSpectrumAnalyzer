#ifndef SPECTRUMMONITOR_H
#define SPECTRUMMONITOR_H

#include <QObject>
#include "packedMonitor.h"

class SpectrumMonitor : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumMonitor(QObject *parent = nullptr);
    void stopMonitor();
    int16_t *inBuffer;
    bool *processDone;
    const int paramSampleNumMax = 1<<22;
    double paramSampleNum = 1024;
    double paramNFFT = 256;

public slots:
    void runMonitor();

private:
    bool isRunning = false;

};

#endif // SPECTRUMMONITOR_H

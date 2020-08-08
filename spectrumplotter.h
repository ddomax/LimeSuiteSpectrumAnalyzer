#ifndef SPECTRUMPLOTTER_H
#define SPECTRUMPLOTTER_H

#include <QObject>
#include <qcustomplot.h>
#include <QMouseEvent>
#include <QUdpSocket>

class SpectrumPlotter : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumPlotter(QObject *parent = nullptr);
    ~SpectrumPlotter();
    bool *drawDone;
    double *freqInBuffer;
    double *fftInBuffer;
    void setupPlot(QCustomPlot *plot);
    bool isRunning = false;
    void setSpan(double center_MHz, double span_MHz);
    void setRefLevel(double refLevel_dBm);
    void setLowLevel(double lowLevel_dBm);
    bool replayMode = false;

private:
    QCustomPlot *specPlot;
    void Sleep(int msec = 1);
    double disFreqMin = 100,disFreqMax = 200;
    double refLevel = -60,lowLevel = -140;
    QCPItemTracer *tracer;
    QCPItemText *label;
    QVector<double> xData,yData;
    static const int FFTOUTNUM = 16384;
    int  middle = FFTOUTNUM/2;
    bool tracerLocked = false;
    QUdpSocket* socket;

public slots:
    void runPlotter();
    void showTracer(QMouseEvent *event);
    void lockTracer(QMouseEvent *event);
    void sendUDP();

signals:
    void readFrame(int frameSize);

};

#endif // SPECTRUMPLOTTER_H

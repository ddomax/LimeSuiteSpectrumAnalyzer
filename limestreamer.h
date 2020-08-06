#ifndef LIMESTREAMER_H
#define LIMESTREAMER_H

#include <QObject>
#include "LimeSuite.h"
#include <QtNetwork>

class limeStreamer : public QObject
{
    Q_OBJECT
public:
    explicit limeStreamer(QObject *parent = nullptr);
    //Device structure, should be initialize to NULL
    lms_device_t* device = NULL;
    void stopStreaming();
    void setUdpPacketNum(int num);
    void setUdpPacketLen(int len);
    void pauseStreaming();
    void resumeStreaming();
    int waitPauseHandlerDone(int time_out_ms = 10000);
    int setGaindB(int gain);
    int setCenterFreq(double freq);
    int setSampleRate(double rate);
    void reqReCalibrate();
    int setMinReadDuration(int time);
    bool skipWaitPause = false;
    void clearSendCnt();
    int16_t *outBuffer;
    bool *processDone;

private:
    int error(QString errorStr = "Undefined Error!");
    QUdpSocket *sender;
    static void handler(int log_level , char *msg)
    {
        if(log_level <= LMS_LOG_DEBUG)
            qDebug() << msg;
    }
    void checkLock(lms_device_t *device);
    void checkDrop(lms_stream_t *streamId);
    void pauseHandler(lms_device_t *device, lms_stream_t *streamId);
    lms_stream_t* pausedStreamId = NULL;
    bool isRunning = false,isPaused = false;
    int udpPackNum = 1;
    int udpPackLen = 8192;
    int paramGaindB = 73;
    double paramCenterFreq = 150e6;
    double paramSampleRate = 10e6;
    bool requsetCal = false;
    int ctrMinReadDuration = 200;
    bool pauseHandlerDone = false;
    int sendPackCnt = 0;

public slots:
    int streaming();

signals:
    void resultReady(const QString &str);
    void stopRunning();
    void startRunning();
    void startStreaming();
};

#endif // LIMESTREAMER_H

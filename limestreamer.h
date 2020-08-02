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
    int streaming();

private:
    int error();
    QUdpSocket *sender;
    static void handler(int log_level , char *msg)
    {
        if(log_level <= LMS_LOG_DEBUG)
            qDebug() << msg;
    }
    void checkLock(lms_device_t *device);
    void checkDrop(lms_stream_t* streamId);
signals:

};

#endif // LIMESTREAMER_H

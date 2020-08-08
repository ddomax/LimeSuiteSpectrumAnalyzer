#ifndef FILESOURCE_H
#define FILESOURCE_H

#include <QObject>
#include <QFile>

class FileSource : public QObject
{
    Q_OBJECT
public:
    explicit FileSource(QObject *parent = nullptr);
    ~FileSource();
    void closeFile();
    int openFile(QWidget *widget);
    void setInterval(int msec);
    double *keyOutBuffer;
    double *valueOutBuffer;

private:
    QFile* sourceFile;
    int interval_msec = 30;
    void Sleep(int msec);

public slots:
    void readFrame(int frameSize);

signals:
    void rePlot();

};

#endif // FILESOURCE_H

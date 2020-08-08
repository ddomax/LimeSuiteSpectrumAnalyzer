#ifndef FILESINK_H
#define FILESINK_H

#include <QObject>
#include <QFile>
#include <QFileDialog>

class FileSink : public QObject
{
    Q_OBJECT
public:
    explicit FileSink(QObject *parent = nullptr);
    ~FileSink();
    void closeFile();
    void setFileName(QWidget *widget);
    bool *saveDone;
    double *keyInBuffer;
    double *valueInBuffer;

private:
    QFile *sinkFile;

signals:

public slots:
    void writeFrame(int frameSize);
};

#endif // FILESINK_H

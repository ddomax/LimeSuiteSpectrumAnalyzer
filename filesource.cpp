#include "filesource.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTime>

FileSource::FileSource(QObject *parent) : QObject(parent)
{
    sourceFile = new QFile();
}

FileSource::~FileSource()
{
    sourceFile->deleteLater();
}

void FileSource::closeFile()
{
    sourceFile->close();
}

int FileSource::openFile(QWidget *widget)
{
    QString fileName = QFileDialog::getOpenFileName(widget, tr("Open File"),
                                ".", //default open location: current directory
                                tr("Spectrum Records (*.sr)"));
    if (!fileName.isEmpty())
        QMessageBox::information(NULL, tr("Path"), tr("You selected ") + fileName);
    else
        QMessageBox::information(NULL, tr("Path"), tr("No file is selected!"));

    sourceFile->setFileName(fileName);
    if (!sourceFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(NULL, tr("File Open"), tr("Can not open the file!"));
        return -1;
    }
    return 0;
}

void FileSource::readFrame(int frameSize)
{
    if (sourceFile->openMode() == QIODevice::OpenModeFlag::NotOpen)
    {
        qDebug() << "FileSouce: File Not Open!";
        return;
    }
    int readKeyBytes = sourceFile->read((char*)keyOutBuffer, frameSize*sizeof(double));
    int readValueBytes = sourceFile->read((char*)valueOutBuffer, frameSize*sizeof(double));
    if (readKeyBytes<frameSize || readValueBytes<frameSize )
    {
        qDebug() << "FileSouce: File End!";
        return;
    }
    Sleep(interval_msec);
    emit rePlot();
}

void FileSource::Sleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime ); //Block current thread
}

void FileSource::setInterval(int msec)
{
    interval_msec = msec;
}

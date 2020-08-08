#include "filesink.h"
#include <QMessageBox>

FileSink::FileSink(QObject *parent) : QObject(parent)
{
    sinkFile = new QFile();
}

FileSink::~FileSink()
{
    sinkFile->deleteLater();
}

void FileSink::writeFrame(int frameSize)
{
    if (sinkFile->openMode() != QIODevice::OpenModeFlag::NotOpen)
    {
        sinkFile->write((char*)keyInBuffer, frameSize*sizeof(double));
        sinkFile->write((char*)valueInBuffer, frameSize*sizeof(double));
    }
    *saveDone = true;
}

void FileSink::closeFile()
{
    sinkFile->close();
}

void FileSink::setFileName(QWidget *widget)
{
    QString fileName = QFileDialog::getSaveFileName(widget, tr("Save File"),
                                ".", //default save location: current directory
                                tr("Spectrum Records (*.sr)"));
    if (!fileName.isEmpty())
        QMessageBox::information(NULL, tr("Path"), tr("You selected ") + fileName);
    else
        QMessageBox::information(NULL, tr("Path"), tr("No file is selected!"));

    sinkFile->setFileName(fileName);
    if (!sinkFile->open(QIODevice::WriteOnly))
        QMessageBox::warning(NULL, tr("File Open"), tr("Can not open the file!"));
}

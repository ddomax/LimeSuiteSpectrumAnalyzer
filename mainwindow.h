#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "limestreamer.h"
#include <QThread>
#include "spectrummonitor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QUdpSocket* sender;
    QThread devThread,monitorThread;
    limeStreamer* dev;
    SpectrumMonitor* monitor;
    bool isStreaming = false;
    void Sleep(int msec);
    void connectSliderAndBox();
    int16_t swapBuffer[8*1024*1024];

private slots:
    void on_fftLenSlider_sliderReleased();
    void on_gainSlider_sliderReleased();
    void on_centerSlider_sliderReleased();
    void on_spanSlider_sliderReleased();
    void on_streamButton_clicked();
    void on_stopButton_clicked();
    void on_reCalButton_clicked();
    void on_monitorButton_clicked();

signals:
    void startRunning();
    void startMonitor();

public slots:
    void on_receiveResult(const QString &str);
    void on_stopRunning();
    void on_startRunning();
    void on_startStreaming();
};
#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
//    this->setStyleSheet("background-color:black;");
    ui->setupUi(this);
    connectSliderAndBox();

    sender = new QUdpSocket();

    dev = new limeStreamer();
    dev->moveToThread(&devThread);

    connect(this, &MainWindow::startRunning, dev, &limeStreamer::streaming);
    connect(dev, &limeStreamer::resultReady, this, &MainWindow::on_receiveResult);
    connect(&devThread, &QThread::finished, dev, &QObject::deleteLater);
    connect(dev, &limeStreamer::startRunning, this, &MainWindow::on_startRunning);
    connect(dev, &limeStreamer::stopRunning, this, &MainWindow::on_stopRunning);
    connect(dev, &limeStreamer::startStreaming, this, &MainWindow::on_startStreaming);

    devThread.start();

    size_t stackSize = 256000000;
    monitorThread.setStackSize(static_cast<uint>(stackSize));
    monitor = new SpectrumMonitor();
    monitor->moveToThread(&monitorThread);
    connect(dev, &limeStreamer::startProcess, monitor, &SpectrumMonitor::runMonitor);
    connect(&monitorThread, &QThread::finished, monitor, &QObject::deleteLater);

    monitorThread.start();

    sink = new FileSink();
    sink->moveToThread(&sinkThread);
    connect(monitor, SIGNAL(saveFrame(int)), sink, SLOT(writeFrame(int)));
    connect(&sinkThread, &QThread::finished, sink, &QObject::deleteLater);

    sinkThread.start();

    plotter = new SpectrumPlotter();
    plotter->setupPlot(ui->specPlotWidget);
    connect(monitor, &SpectrumMonitor::rePlot, plotter, &SpectrumPlotter::runPlotter);
    connect(ui->specPlotWidget, SIGNAL(mouseMove(QMouseEvent*)), plotter, SLOT(showTracer(QMouseEvent*)));
    connect(ui->specPlotWidget, SIGNAL(mousePress(QMouseEvent*)), plotter, SLOT(lockTracer(QMouseEvent*)));

    source = new FileSource();
    source->moveToThread(&sourceThread);
    connect(source, SIGNAL(rePlot()), plotter, SLOT(runPlotter()));
    connect(plotter, SIGNAL(readFrame(int)), source, SLOT(readFrame(int)));
    connect(&sourceThread, &QThread::finished, source, &QObject::deleteLater);

    sourceThread.start();

    dev->outBuffer = swapBuffer;
    monitor->inBuffer = swapBuffer;

    monitor->fftOutBuffer = fftSwapBuffer;
    monitor->freqOutBuffer = freqSwapBuffer;
    source->valueOutBuffer = fftSwapBuffer;
    source->keyOutBuffer = freqSwapBuffer;
    plotter->fftInBuffer = fftSwapBuffer;
    plotter->freqInBuffer = freqSwapBuffer;

    monitor->fftSaveBuffer = fftSinkBuffer;
    monitor->freqSaveBuffer = freqSinkBuffer;
    sink->valueInBuffer = fftSinkBuffer;
    sink->keyInBuffer = freqSinkBuffer;

    bool processDone = true;
    dev->processDone = &processDone;
    monitor->processDone = &processDone;

    bool drawDone = true;
    monitor->drawDone = &drawDone;
    plotter->drawDone = &drawDone;

    bool saveDone = true;
    monitor->saveDone = &saveDone;
    sink->saveDone = &saveDone;

    setUI(); // setUI will trig indexChanged(), must be called after monitor is initialized!
}

MainWindow::~MainWindow()
{
    on_stopButton_clicked();
    QString ia = QString("Exit:0;");
    if (sender->writeDatagram(ia.toLocal8Bit(),ia.size(),QHostAddress::Broadcast,45456) == -1)
    {
        exit(-2);
    }
    devThread.quit();
    devThread.wait();
    monitorThread.quit();
    monitorThread.wait();
    sinkThread.quit();
    sinkThread.wait();
    sourceThread.quit();
    sourceThread.wait();
    plotter->deleteLater();
    sender->deleteLater();
    sink->deleteLater();
    source->deleteLater();
    delete ui;
}

void MainWindow::Sleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::connectSliderAndBox()
{
    ui->gainSpinBox->setValue(ui->gainSlider->value());
    ui->fftLenSpinBox->setValue(ui->fftLenSlider->value());
    ui->spanSpinBox->setValue(ui->spanSlider->value());
    ui->centerSpinBox->setValue(ui->centerSlider->value());
    ui->refSpinBox->setValue(ui->refSpinBox->value());
    connect(ui->gainSpinBox, SIGNAL(valueChanged(int)), ui->gainSlider, SLOT(setValue(int)));
    connect(ui->fftLenSpinBox, SIGNAL(valueChanged(int)), ui->fftLenSlider, SLOT(setValue(int)));
    connect(ui->spanSpinBox, SIGNAL(valueChanged(int)), ui->spanSlider, SLOT(setValue(int)));
    connect(ui->centerSpinBox, SIGNAL(valueChanged(int)), ui->centerSlider, SLOT(setValue(int)));
    connect(ui->refSpinBox, SIGNAL(valueChanged(int)), ui->refSlider, SLOT(setValue(int)));
    connect(ui->divSpinBox, SIGNAL(valueChanged(int)), ui->divSlider, SLOT(setValue(int)));
    connect(ui->gainSlider, SIGNAL(valueChanged(int)), ui->gainSpinBox, SLOT(setValue(int)));
    connect(ui->fftLenSlider, SIGNAL(valueChanged(int)), ui->fftLenSpinBox, SLOT(setValue(int)));
    connect(ui->spanSlider, SIGNAL(valueChanged(int)), ui->spanSpinBox, SLOT(setValue(int)));
    connect(ui->centerSlider, SIGNAL(valueChanged(int)), ui->centerSpinBox, SLOT(setValue(int)));
    connect(ui->refSlider, SIGNAL(valueChanged(int)), ui->refSpinBox, SLOT(setValue(int)));
    connect(ui->divSlider, SIGNAL(valueChanged(int)), ui->divSpinBox, SLOT(setValue(int)));
}

void MainWindow::setUI()
{
    QStringList detectorBoxList;
    detectorBoxList << "Sample" << "Peak";
    ui->detectorBox->addItems(detectorBoxList);

    QStringList replaySpeedBoxList;
    replaySpeedBoxList << "0.1x" << "1x" << "10x" << "30x" << "Fastest";
    ui->replaySpeedBox->addItems(replaySpeedBoxList);
}

void MainWindow::on_streamButton_clicked()
{
    ui->streamButton->setText("Initialzing...");
    ui->openButton->setDisabled(true);
    QString ia = QString("clearBuffer:0;");
    if (sender->writeDatagram(ia.toLocal8Bit(),ia.size(),QHostAddress::Broadcast,45456) == -1)
    {
        exit(-2);
    }
    Sleep(10); //Wait Matlab to restart dsp.UDPReceiver
    dev->clearSendCnt();
    dev->skipWaitPause = true;
    on_fftLenSlider_sliderReleased();
    on_gainSlider_sliderReleased();
    on_centerSlider_sliderReleased();
    on_spanSlider_sliderReleased();
    on_detectorBox_currentIndexChanged(ui->detectorBox->currentText());
    on_refSlider_sliderReleased();
    on_divSlider_sliderReleased();
    dev->skipWaitPause = false;
    emit startRunning();
}

void MainWindow::on_stopButton_clicked()
{
    if (plotter->replayMode)
    {
        ui->openButton->setDisabled(false);
        ui->streamButton->setDisabled(false);
        ui->saveButton->setDisabled(false);
        ui->fftLenSlider->setDisabled(false);
        ui->gainSlider->setDisabled(false);
        ui->centerSlider->setDisabled(false);
        plotter->replayMode = false;
        source->closeFile();
        return;
    }
    dev->stopStreaming();
    Sleep(3000);
    if (isStreaming && ui->streamButton->isEnabled()) //streamButton not clicked but the thread is still running, kill it!
    {
        qDebug() << "Wait streaming thread to quit...";
        devThread.quit();
        devThread.wait();
        devThread.start();
        on_stopRunning();
        qDebug() << "Force Quit devThread!";
    }
    sink->closeFile();
    ui->openButton->setDisabled(false);
}

void MainWindow::on_saveButton_clicked()
{
    sink->setFileName(this);
}

void MainWindow::on_openButton_clicked()
{
    if (source->openFile(this) != 0)
    {
        return;
        qDebug() << "MainWindow: cannot open file!";
    }
    ui->openButton->setDisabled(true);
    ui->streamButton->setDisabled(true);
    ui->saveButton->setDisabled(true);
    ui->fftLenSlider->setDisabled(true);
    ui->gainSlider->setDisabled(true);
    ui->centerSlider->setDisabled(true);
    plotter->replayMode = true;
    source->readFrame(FFTOUTNUM);

}

void MainWindow::on_fftLenSlider_sliderReleased()
{
    int sampleCnt = 1 << (ui->fftLenSlider->value());
    int udpPackLen = 8192;
    int udpPackNum = sampleCnt/udpPackLen;
    if (udpPackNum<1)
    {
        udpPackNum = 1;
        udpPackLen = sampleCnt;
    }
    dev->pauseStreaming();
    dev->waitPauseHandlerDone();
//    if (udpPackNum>7)
//    {
//        dev->setMinReadDuration(udpPackNum*25);
//    }
//    else
//    {
//        dev->setMinReadDuration(100);
//    }

    udpPackNum = 1;
    double sampleRate_Hz = 1e3 * ui->spanSlider->value();
    double sampleTime_Seconds = 0.05;
    int sampleNum = 1<<((int)(log2(sampleTime_Seconds*sampleRate_Hz))+1);
    sampleNum = sampleNum < (0.25*1024*1024) ? sampleNum : 0.25*1024*1024;
    sampleNum = sampleNum < sampleCnt ? sampleCnt : sampleNum;
    udpPackLen = sampleNum;

    dev->setUdpPacketLen(udpPackLen);
    dev->setUdpPacketNum(udpPackNum);
    QString ia = QString("udpComPackLen:%1;").arg(sampleCnt);
    if (sender->writeDatagram(ia.toLocal8Bit(),ia.size(),QHostAddress::Broadcast,45456) == -1)
    {
        exit(-2);
    }
    monitor->setSampleNum(udpPackLen*udpPackNum);
    monitor->setNFFT(sampleCnt);
    Sleep(100); // Wait UDP
    dev->resumeStreaming();
}

void MainWindow::on_detectorBox_currentIndexChanged(const QString& currentText)
{
    monitor->setDetector(currentText);
}

void MainWindow::on_replaySpeedBox_currentIndexChanged(const QString& currentText)
{
    if (currentText=="0.1x")
        source->setInterval(300);
    else if (currentText=="1x")
        source->setInterval(30);
    else if(currentText=="10x")
        source->setInterval(3);
    else if(currentText=="30x")
        source->setInterval(1);
    else if(currentText=="Fastest")
        source->setInterval(0);
    else
        qDebug() << "Unknown replay speed!";
}

void MainWindow::on_receiveResult(const QString &str)
{
    qDebug() << str;
}

void MainWindow::on_stopRunning()
{
    isStreaming = false;
    ui->streamButton->setText("start");
    ui->stopButton->setEnabled(false);
    ui->streamButton->setEnabled(true);
}

void MainWindow::on_startRunning()
{
    isStreaming = true;
    ui->streamButton->setText("Connecting Device...");
    ui->streamButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
}

void MainWindow::on_startStreaming()
{
    ui->streamButton->setText("Streaming started");
    Sleep(100);
    monitor->runMonitor(); // Workaround for 'done' keeps false at the beginning
    plotter->runPlotter(); // Workaround for 'done' keeps false at the beginning
//    sink->writeFrame(FFTOUTNUM); // 'done' fixed in monitor's constructor
}

void MainWindow::on_gainSlider_sliderReleased()
{
    int gaindB = ui->gainSlider->value();
    dev->setGaindB(gaindB);
    dev->pauseStreaming();
    dev->waitPauseHandlerDone();
    dev->resumeStreaming();
    QString ia = QString("Gain:%1;").arg(gaindB);
    if (sender->writeDatagram(ia.toLocal8Bit(),ia.size(),QHostAddress::Broadcast,45456) == -1)
    {
        exit(-2);
    }
    monitor->setGain(gaindB);
}

void MainWindow::on_centerSlider_sliderReleased()
{
    int centerFreq_kHz = ui->centerSlider->value();
    double centerFreq_Hz = (double)centerFreq_kHz * 1e3;
    double centerFreq_MHz = (double)centerFreq_kHz * 1e-3;
    dev->setCenterFreq(centerFreq_Hz);
    dev->pauseStreaming();
    dev->waitPauseHandlerDone();
    dev->resumeStreaming();
    QString ia = QString("centerFreq:%1;").arg(centerFreq_kHz);
    if (sender->writeDatagram(ia.toLocal8Bit(),ia.size(),QHostAddress::Broadcast,45456) == -1)
    {
        exit(-2);
    }
    monitor->setCenterFreq(centerFreq_MHz);
    int Fs_kHz = ui->spanSlider->value();
    double Fs_MHz = Fs_kHz * 1e-3;
    plotter->setSpan(centerFreq_MHz,Fs_MHz);
}

void MainWindow::on_spanSlider_sliderReleased()
{
    int Fs_kHz = ui->spanSlider->value();
    double Fs_Hz = Fs_kHz * 1e3; // MHz to Hz
    double Fs_MHz = Fs_kHz * 1e-3;
    if (Fs_MHz < 2)
        dev->setSampleRate(2e6);
    else
        dev->setSampleRate(Fs_Hz);
    dev->pauseStreaming();
    dev->waitPauseHandlerDone();
    dev->resumeStreaming();
    QString ia = QString("Fs:%1;").arg(Fs_MHz);
    if (sender->writeDatagram(ia.toLocal8Bit(),ia.size(),QHostAddress::Broadcast,45456) == -1)
    {
        exit(-2);
    }
    if (Fs_MHz < 2)
    {
        monitor->setSpan(2);
    }
    else
    {
        monitor->setSpan(Fs_MHz);
    }
    int centerFreq_kHz = ui->centerSlider->value();
    double centerFreq_MHz = (double)centerFreq_kHz * 1e-3;
    plotter->setSpan(centerFreq_MHz,Fs_MHz);
    on_fftLenSlider_sliderReleased();
}

void MainWindow::on_refSlider_sliderReleased()
{
    int refLevel = ui->refSlider->value();
    plotter->setRefLevel(refLevel);
}

void MainWindow::on_divSlider_sliderReleased()
{
    int lowLevel = ui->divSlider->value();
    plotter->setLowLevel(lowLevel);
}

void MainWindow::on_reCalButton_clicked()
{
    dev->reqReCalibrate();
    dev->pauseStreaming();
    dev->waitPauseHandlerDone();
    dev->resumeStreaming();
}

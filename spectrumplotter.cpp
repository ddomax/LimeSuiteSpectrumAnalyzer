#include "spectrumplotter.h"
#include <QVector>
#include <algorithm>
#include <QUdpSocket>

SpectrumPlotter::SpectrumPlotter (QObject *parent)
    : QObject(parent)
{
    xData.resize(FFTOUTNUM);
    yData.resize(FFTOUTNUM);
    socket = new QUdpSocket();
    socket->bind(QHostAddress::LocalHost, 4012);
    connect(socket, &QUdpSocket::readyRead, this, &SpectrumPlotter::sendUDP);
}

SpectrumPlotter::~SpectrumPlotter()
{
    socket->deleteLater();
}

void SpectrumPlotter::showTracer(QMouseEvent *event)
{
    if (tracerLocked)
        return;
//    qDebug() << "(" << (event->pos().x()) << "," << (event->pos().y()) << ")";
    double x = specPlot->xAxis->pixelToCoord(event->pos().x());
//    double y = specPlot->yAxis->pixelToCoord(event->pos().y());

    double y = 0;
    QSharedPointer<QCPGraphDataContainer> tmpContainer;
    tmpContainer = specPlot->graph(0)->data();
    //Searching data point
    int low = 0, high = tmpContainer->size();
    while(high > low)
    {
      middle = (low + high) / 2;
      if(x < tmpContainer->constBegin()->mainKey() ||
              x > (tmpContainer->constEnd()-1)->mainKey())
          break;

      if(x == (tmpContainer->constBegin() + middle)->mainKey())
      {
          y = (tmpContainer->constBegin() + middle)->mainValue();
          break;
      }
      if(x > (tmpContainer->constBegin() + middle)->mainKey())
      {
          low = middle;
      }
      else if(x < (tmpContainer->constBegin() + middle)->mainKey())
      {
          high = middle;
      }
      if(high - low <= 1)
      {   //interpolation
          y = (tmpContainer->constBegin()+low)->mainValue() + ( (x - (tmpContainer->constBegin() + low)->mainKey()) *
              ((tmpContainer->constBegin()+high)->mainValue() - (tmpContainer->constBegin()+low)->mainValue()) ) /
              ((tmpContainer->constBegin()+high)->mainKey() - (tmpContainer->constBegin()+low)->mainKey());
          break;
      }

    }

//    auto max = std::max_element(std::begin(yData),std::end(yData));
    auto max = std::max_element(&yData.at(middle-100),&yData.at(middle+100));
    tracer->position->setCoords(x,*max);
    specPlot->replot();
}

void SpectrumPlotter::lockTracer(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton)
        tracerLocked = !tracerLocked;
}

void SpectrumPlotter::setupPlot(QCustomPlot *plot)
{
    specPlot = plot;

    // Setup tracer
    tracer = new QCPItemTracer(plot);
    tracer->setPen(QPen(Qt::green));
    tracer->setBrush(QPen(Qt::green).color());
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setSize(12);

    // Setup label
    label = new QCPItemText(plot);
    label->position->setType(QCPItemPosition::ptAxisRectRatio);
    label->setPositionAlignment(Qt::AlignRight|Qt::AlignBottom);
    label->position->setCoords(0.95, 0.15); // upper right corner of axis rect
//    label->setPen(QPen(Qt::green)); //out boarder
//    label->setBrush(Qt::green);
    label->setFont(QFont("SimSun",12));
    label->setColor(Qt::green);
    label->setText("--- MHz\n--- dBm");

    // create graph and assign data to it:
    specPlot->addGraph();

    // set dark background gradient:
//    QLinearGradient gradient(0, 0, 0, 400);
//    gradient.setColorAt(0, QColor(40, 40, 40));
//    gradient.setColorAt(0.38, QColor(80, 80, 80));
//    gradient.setColorAt(1, QColor(30, 30, 30));
//    specPlot->setBackground(QBrush(gradient));
    specPlot->setBackground(QBrush(Qt::black));

    // give the axes some labels:
    specPlot->xAxis->setLabel("Frequency (MHz)");
    specPlot->yAxis->setLabel("Power (dBm)");

    // Set axis color
    specPlot->xAxis->setBasePen(QPen(Qt::white));
    specPlot->xAxis2->setBasePen(QPen(Qt::white));
    specPlot->xAxis->setTickPen(QPen(Qt::white));
    specPlot->xAxis2->setTickPen(QPen(Qt::white));
    specPlot->xAxis->setLabelColor(Qt::white);
    specPlot->xAxis->setTickLabelColor(Qt::white);

    // Set grid
    specPlot->xAxis->grid()->setSubGridVisible(false);
    specPlot->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
    specPlot->xAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));

    // Set axis color
    specPlot->yAxis->setBasePen(QPen(Qt::white));
    specPlot->yAxis2->setBasePen(QPen(Qt::white));
    specPlot->yAxis->setTickPen(QPen(Qt::white));
    specPlot->yAxis2->setTickPen(QPen(Qt::white));
    specPlot->yAxis->setLabelColor(Qt::white);
    specPlot->yAxis->setTickLabelColor(Qt::white);

    // configure right and top axis to show ticks but no labels:
    // (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
    specPlot->xAxis2->setVisible(true);
    specPlot->xAxis2->setTickLabels(false);
    specPlot->yAxis2->setVisible(true);
    specPlot->yAxis2->setTickLabels(false);
    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(specPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), specPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(specPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), specPlot->yAxis2, SLOT(setRange(QCPRange)));

    // Set grid
    specPlot->yAxis->grid()->setSubGridVisible(false);
    specPlot->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
    specPlot->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));

    QPen curPen = QPen(Qt::yellow);
    curPen.setWidth(1);
    specPlot->graph(0)->setPen(curPen);
}

void SpectrumPlotter::runPlotter()
{
    // Get and set display Data
    memcpy(&xData[0], freqInBuffer, FFTOUTNUM*sizeof(double));
    memcpy(&yData[0], fftInBuffer, FFTOUTNUM*sizeof (double));
    specPlot->graph(0)->setData(xData,yData,true); // Already sorted -> true

    // Set Display Range
    specPlot->xAxis->setRange(disFreqMin,disFreqMax);
//    specPlot->xAxis->setRange(xData[0],xData[FFTOUTNUM-1]);
    specPlot->yAxis->setRange(lowLevel,refLevel);

    // Update Peak Cursor
    auto max = std::max_element(&yData.at(middle-300),&yData.at(middle+300));
    auto positionmax = std::distance<std::vector<double>::const_pointer>(std::begin(yData), max);
    tracer->position->setCoords(xData.at(positionmax),*max);

    // Update Cursor label
    label->setText(QString("%1 MHz\n%2 dBm").arg(QString::number(xData.at(positionmax), 'f', 4))
                   .arg(QString::number(yData.at(positionmax), 'f', 3)));

    specPlot->replot();
    *drawDone = true;
    if (replayMode)
        emit readFrame(FFTOUTNUM);
}


void SpectrumPlotter::Sleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void SpectrumPlotter::setSpan(double center_MHz, double span_MHz)
{
    disFreqMin = center_MHz - span_MHz / 2;
    disFreqMax = center_MHz + span_MHz / 2;
}

void SpectrumPlotter::setLowLevel(double lowLevel_dBm)
{
    lowLevel = lowLevel_dBm;
}

void SpectrumPlotter::setRefLevel(double refLevel_dBm)
{
    refLevel = refLevel_dBm;
}

void SpectrumPlotter::sendUDP()
{
    char recData[64];
    socket->readDatagram(recData, 64);
    QUdpSocket tmp;
//    qDebug() << tmp.writeDatagram((char*)&xData[0], xData.size(), QHostAddress::LocalHost, 4013);
//    qDebug() << tmp.writeDatagram((char*)&yData[0], yData.size(), QHostAddress::LocalHost, 4013);
//    qDebug() << tmp.writeDatagram((char*)&yData[0], 8, QHostAddress::LocalHost, 4013);
    double tracerVal = tracer->position->value();
    qDebug() << tmp.writeDatagram((char*)&tracerVal, sizeof (double), QHostAddress::LocalHost, 4013);
}

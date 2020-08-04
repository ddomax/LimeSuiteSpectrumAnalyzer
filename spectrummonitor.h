#ifndef SPECTRUMMONITOR_H
#define SPECTRUMMONITOR_H

#include <QObject>
#include "packedMonitor.h"

class SpectrumMonitor : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumMonitor(QObject *parent = nullptr);
    void stopMonitor();

public slots:
    void runMonitor();

};

#endif // SPECTRUMMONITOR_H

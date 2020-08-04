#include "spectrummonitor.h"

SpectrumMonitor::SpectrumMonitor(QObject *parent) : QObject(parent)
{
    packedMonitorInitialize();
}

void SpectrumMonitor::runMonitor()
{
    packedMonitor();
}

void SpectrumMonitor::stopMonitor()
{
    packedMonitorTerminate();
}

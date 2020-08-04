#include "limestreamer.h"
#include <iostream>
#include <chrono>
#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

limeStreamer::limeStreamer(QObject *parent) : QObject(parent)
{
    qDebug() << "limeStreamer Created!";
    sender = new QUdpSocket(this);
    LMS_RegisterLogHandler((LMS_LogHandler)handler);
}

/**
    @file   basicRX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  minimal RX example
 */


using namespace std;



int limeStreamer::error(QString errorStr)
{
    isRunning = false;
    if (device != NULL)
        LMS_Close(device);
    qDebug() << errorStr;
    emit stopRunning();
    return -1;
}

int limeStreamer::streaming()
{
    isRunning = true;
    emit startRunning();
    qDebug() << "limeStreamer: Streaming!";
    //Find devices
    int n;
    lms_info_str_t list[8]; //should be large enough to hold all detected devices
    if ((n = LMS_GetDeviceList(list)) < 0) //NULL can be passed to only get number of devices
    {
        error();
        return -1;
    }

    cout << "Devices found: " << n << endl; //print number of devices
    if (n < 1)
    {
        error();
        return -1;
    }

    //open the first device
    if (LMS_Open(&device, list[0], NULL))
    {
        error();
        return -1;
    }

    //Initialize device with default configuration
    //Do not use if you want to keep existing configuration
    //Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
    if (LMS_Init(device) != 0)
    {
        error();
        return -1;
    }

    //Enable RX channel
    //Channels are numbered starting at 0
    if (LMS_EnableChannel(device, LMS_CH_RX, 0, true) != 0)
    {
        error();
        return -1;
    }

    //Select RX Antenna
    //Channels are numbered starting at 0
    if (LMS_SetAntenna(device, LMS_CH_RX, 0, LMS_PATH_LNAW) != 0)
    {
        error();
        return -1;
    }

    //Set RX Combined Gain(dB) [0-73dB]
    //Channels are numbered starting at 0
    if (LMS_SetGaindB(device, LMS_CH_RX, 0, paramGaindB) != 0)
    {
        error();
        return -1;
    }

    //Set RX Combined Gain [0-1]
    //Channels are numbered starting at 0
//    if (LMS_SetNormalizedGain(device, LMS_CH_RX, 0, 1) != 0)
//    {
//        error();
//        return -1;
//    }

    //Set CP Current Scale (0-->63) (default:12)
    if (LMS_WriteParam(device,LMS7_IOFFSET_CP,38) != 0)
    {
        error();
        return -1;
    }

    //Set center frequency to 800 MHz
    if (LMS_SetLOFrequency(device, LMS_CH_RX, 0, paramCenterFreq) != 0)
    {
        error();
        return -1;
    }

    //Set sample rate to 8 MHz, ask to use 2x oversampling in RF
    //This set sampling rate for all channels
    if (LMS_SetSampleRate(device, paramSampleRate, 1) != 0)
    {
        error();
        return -1;
    }

    if (paramSampleRate<50e6)
    {
        if (LMS_SetLPFBW(device, LMS_CH_RX, 0, paramSampleRate) != 0)
        {
            error("LMS_SetLPFBW Failed!");
            return -1;
        }
    }
    else
    {
        if (LMS_SetLPF(device, LMS_CH_RX, 0, false) != 0)
        {
            error("LMS_SetLPF Failed!");
            return -1;
        }
    }

    //Perform automatic calibration
    if (LMS_Calibrate(device, LMS_CH_RX, 0, paramSampleRate, 0) != 0)
    {
        error();
        return -1;
    }

    //Enable test signal generation
    //To receive data from RF, remove this line or change signal to LMS_TESTSIG_NONE
//    if (LMS_SetTestSignal(device, LMS_CH_RX, 0, LMS_TESTSIG_NCODIV4F, 0, 0) != 0)
//    {
//        error();
//        return -1;
//    }

    //Streaming Setup

    //Initialize stream
    lms_stream_t streamId; //stream structure
    streamId.channel = 0; //channel number
    streamId.fifoSize = 10 * 1024 * 1024; //fifo size in samples
    streamId.throughputVsLatency = 1.0; //optimize for max throughput
    streamId.isTx = false; //RX channel
    streamId.dataFmt = lms_stream_t::LMS_FMT_I12; //12-bit integers
    if (LMS_SetupStream(device, &streamId) != 0)
    {
        error();
        return -1;
    }

    //Initialize data buffers
    static const int udpPackLenMax = 8192; //complex samples counts
    static const int udpPackNumMax = 128; //complex samples counts
    static const int sampleCntMax = udpPackLenMax*udpPackNumMax; //max complex samples counts
    static int16_t buffer[sampleCntMax*2]; //buffer to hold complex values (2*samples))

    //Start streaming
    LMS_StartStream(&streamId);
    emit startStreaming();

    //Streaming
#ifdef USE_GNU_PLOT
    GNUPlotPipe gp;
    gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
#endif
    while(1)
    {
    auto t1 = chrono::high_resolution_clock::now();
    int samplesRead = 0,readReturn = 0;
    int sampleCnt = udpPackLen * udpPackNum;
    int loopCnt = 0;
    while (chrono::high_resolution_clock::now() - t1 < chrono::milliseconds(ctrMinReadDuration)) //run for 20 seconds
    {
        //Receive samples
        readReturn = LMS_RecvStream(&streamId, buffer, sampleCnt, NULL, 2000);
        if (readReturn < 0)
        {
            error("Falied to read device!");
            return -1;
        }
        else
        {
            samplesRead += readReturn;
        }
        loopCnt++;
    //I and Q samples are interleaved in buffer: IQIQIQ...
    /*
        CODE FOR PROCESSING RECEIVED SAMPLES
    */
    }
    const int retryMax = 10;
    int retryCnt = 0;
    for (int i=0;i<udpPackNum;i++)
    {
        char* udpPackStart = (char*)(&buffer[i*udpPackLen*2]); // '*2' since one sample contains I,Q
        while (1)
        {
            if (sender->writeDatagram(udpPackStart,udpPackLen*2*sizeof(int16_t),QHostAddress::Broadcast,45454) == -1)
                retryCnt++;
            else
                break;
            if (retryCnt>retryMax)
            {
                error("UDP Send Failed!");
                return -1;
            }
        }
    }
    sendPackCnt++;
    auto captureTime = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t1);
    qDebug() << "send:" <<sendPackCnt <<"rd:" << samplesRead << "sps:" << sampleCnt << "time:" << captureTime.count() << "loop:" << loopCnt;
//    while(chrono::high_resolution_clock::now() - t1 < chrono::milliseconds(1100));
    checkLock(device);
    checkDrop(&streamId);
    if (!isRunning)
    {
        break;
    }
    if (isPaused)
    {
        pauseHandler(device, &streamId);
    }
    }
    //Stop streaming
    LMS_StopStream(&streamId); //stream is stopped but can be started again with LMS_StartStream()
    LMS_DestroyStream(device, &streamId); //stream is deallocated and can no longer be used

    //Close device
    LMS_Close(device);
    sender->close();

    emit resultReady("limeStreamer: Return!");
    emit stopRunning();
    return 0;
}

void limeStreamer::checkLock(lms_device_t *device)
{
    static int unlock_cnt = 0;
    uint16_t cmphl;
    LMS_ReadParam(device,LMS7_VCO_CMPHO, &cmphl);
    if (cmphl != 1)
    {
        unlock_cnt++;
        qDebug() << unlock_cnt << "LMS7_VCO_CMPHO" << cmphl;
    }

    LMS_ReadParam(device,LMS7_VCO_CMPLO, &cmphl);
    if (cmphl != 0)
    {
        unlock_cnt++;
        qDebug() << unlock_cnt << "LMS7_VCO_CMPLO" << cmphl;
    }

    LMS_ReadParam(device,LMS7_VCO_CMPHO_CGEN, &cmphl);
    if (cmphl != 1)
    {
        unlock_cnt++;
        qDebug() << unlock_cnt << "LMS7_VCO_CMPHO_CGEN" << cmphl;
    }

    LMS_ReadParam(device,LMS7_VCO_CMPLO_CGEN, &cmphl);
    if (cmphl != 0)
    {
        unlock_cnt++;
        qDebug() << unlock_cnt << "LMS7_VCO_CMPLO_CGEN" << cmphl;
    }
}

void limeStreamer::checkDrop(lms_stream_t* streamId)
{
    static lms_stream_status_t streamStatus;
    LMS_GetStreamStatus(streamId, &streamStatus);
    if (streamStatus.droppedPackets != 0)
        qDebug() << "DroppedPackets: "<< streamStatus.droppedPackets;
//    qDebug() << "linkRate: "<< streamStatus.linkRate/1e6;
    if (streamStatus.overrun != 0)
        qDebug() << "overrun: "<< streamStatus.overrun;
    if (streamStatus.underrun != 0)
        qDebug() << "underrun: "<< streamStatus.underrun;
//    qDebug() << "fifoSize: "<< streamStatus.fifoSize;
//    qDebug() << "fifoFilled: "<< streamStatus.fifoFilledCount;
}

void limeStreamer::stopStreaming()
{
    isRunning = false;
}

void limeStreamer::pauseStreaming()
{
    isPaused = true;
}

void limeStreamer::setUdpPacketNum(int num)
{
    udpPackNum = num;
}

void limeStreamer::setUdpPacketLen(int len)
{
    udpPackLen = len;
}

void limeStreamer::pauseHandler(lms_device_t *device, lms_stream_t *streamId)
{
    //Stop streaming
    LMS_StopStream(streamId); //stream is stopped but can be started again with LMS_StartStream()
    pausedStreamId = streamId;

    static int last_paramGaindB = 0;
    static double last_paramCenterFreq = 0;
    static double last_paramSampleRate = 0;

    //Set RX Combined Gain(dB) [0-73dB]
    //Channels are numbered starting at 0
    if (last_paramGaindB != paramGaindB)
        if (LMS_SetGaindB(device, LMS_CH_RX, 0, paramGaindB) != 0)
        {
            qDebug() << "LMS_SetGaindB Failed!";
        }

    //Set LO Frequency
    if (last_paramCenterFreq != paramCenterFreq)
    {
        if (LMS_SetLOFrequency(device, LMS_CH_RX, 0, paramCenterFreq) != 0)
        {
            qDebug() << "LMS_SetLOFrequency Failed!";
        }
    }

    //Set sample rate
    //This set sampling rate for all channels
    if (last_paramSampleRate != paramSampleRate)
    {
        if (LMS_SetSampleRate(device, paramSampleRate, 1) != 0)
        {
            qDebug() << "LMS_SetSampleRate Failed!";
        }
        if (paramSampleRate<50e6)
        {
            if (LMS_SetLPFBW(device, LMS_CH_RX, 0, paramSampleRate) != 0)
            {
                qDebug() << "LMS_SetLPFBW Failed!";
            }
        }
        else
        {
            if (LMS_SetLPF(device, LMS_CH_RX, 0, false) != 0)
            {
                qDebug() << "LMS_SetLPF Failed!";
            }
        }
    }
    if (requsetCal)
    {
        if (LMS_Calibrate(device, LMS_CH_RX, 0, 2e6, 0) != 0)
        {
            qDebug() << "LMS_Calibrate Failed!";
        }
        requsetCal = false;
    }

    last_paramGaindB = paramGaindB;
    last_paramCenterFreq = paramCenterFreq;
    last_paramSampleRate = paramSampleRate;

    pauseHandlerDone = true;
}

int limeStreamer::waitPauseHandlerDone(int time_out_ms)
{
    auto tic = chrono::high_resolution_clock::now();
    while (!pauseHandlerDone && !skipWaitPause)
    {
        if(chrono::high_resolution_clock::now() - tic > chrono::milliseconds(time_out_ms))
            return -1;
    }
    pauseHandlerDone = false;
    return 0;
}

void limeStreamer::resumeStreaming()
{
    //Start streaming
    if (pausedStreamId != NULL)
        LMS_StartStream(pausedStreamId);
    pausedStreamId = NULL;
    isPaused = false;
}

int limeStreamer::setGaindB(int gain)
{
    if (gain<0 || gain > 73)
    {
        qDebug() << "Gain(dB) out of range[0,73]!";
        return -1;
    }

    paramGaindB = gain;
    return 0;
}

int limeStreamer::setCenterFreq(double freq)
{
    if (freq<30e6 || freq > 3800e6)
    {
        qDebug() << QString("Freq(Hz):%1 out of range[30e6,3800e6]!").arg(freq);
        return -1;
    }

    paramCenterFreq = freq;
    return 0;
}

int limeStreamer::setSampleRate(double rate)
{
    if (rate<2e6 || rate > 50e6)
    {
        qDebug() << "Rate(Hz) out of range[2e6,50e6]!";
        return -1;
    }

    paramSampleRate = rate;
    return 0;
}

void limeStreamer::reqReCalibrate()
{
    requsetCal = true;
}

int limeStreamer::setMinReadDuration(int time_ms)
{
    if (!isPaused)
        return -1;
    ctrMinReadDuration = time_ms;
}

void limeStreamer::clearSendCnt()
{
    sendPackCnt = 0;
}

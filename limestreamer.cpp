#include "limestreamer.h"
#include <iostream>
#include <chrono>
#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

limeStreamer::limeStreamer(QObject *parent) : QObject(parent)
{
    sender = new QUdpSocket(this);
    LMS_RegisterLogHandler((LMS_LogHandler)handler);
}

/**
    @file   basicRX.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief  minimal RX example
 */


using namespace std;



int limeStreamer::error()
{
    if (device != NULL)
        LMS_Close(device);
    exit(-1);
}

int limeStreamer::streaming()
{
    //Find devices
    int n;
    lms_info_str_t list[8]; //should be large enough to hold all detected devices
    if ((n = LMS_GetDeviceList(list)) < 0) //NULL can be passed to only get number of devices
        error();

    cout << "Devices found: " << n << endl; //print number of devices
    if (n < 1)
        return -1;

    //open the first device
    if (LMS_Open(&device, list[0], NULL))
        error();

    //Initialize device with default configuration
    //Do not use if you want to keep existing configuration
    //Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI
    if (LMS_Init(device) != 0)
        error();

    //Enable RX channel
    //Channels are numbered starting at 0
    if (LMS_EnableChannel(device, LMS_CH_RX, 0, true) != 0)
        error();

    //Select RX Antenna
    //Channels are numbered starting at 0
    if (LMS_SetAntenna(device, LMS_CH_RX, 0, LMS_PATH_LNAW) != 0)
        error();

    //Set RX Combined Gain(dB) [0-73dB]
    //Channels are numbered starting at 0
    if (LMS_SetGaindB(device, LMS_CH_RX, 0, 73) != 0)
        error();

    //Set RX Combined Gain [0-1]
    //Channels are numbered starting at 0
//    if (LMS_SetNormalizedGain(device, LMS_CH_RX, 0, 1) != 0)
//        error();

    //Set CP Current Scale (0-->63) (default:12)
    if (LMS_WriteParam(device,LMS7_IOFFSET_CP,38) != 0)
        error();

    //Set center frequency to 800 MHz
    if (LMS_SetLOFrequency(device, LMS_CH_RX, 0, 105e6) != 0)
        error();

    //Set sample rate to 8 MHz, ask to use 2x oversampling in RF
    //This set sampling rate for all channels
    if (LMS_SetSampleRate(device, 4e6, 1) != 0)
        error();

    //Set Analog LPF BW (Min 1.4MHz)
    if (LMS_SetLPFBW(device, LMS_CH_RX, 0, 4e6) != 0)
        error();

    //Disable Analog LPF without reconfigure it
//    if (LMS_SetLPF(device, LMS_CH_RX, 0, false) != 0)
//        error();

    //Perform automatic calibration
    if (LMS_Calibrate(device, LMS_CH_RX, 0, 4e6, 0) != 0)
        error();

    //Enable test signal generation
    //To receive data from RF, remove this line or change signal to LMS_TESTSIG_NONE
//    if (LMS_SetTestSignal(device, LMS_CH_RX, 0, LMS_TESTSIG_NCODIV4F, 0, 0) != 0)
//        error();

    //Streaming Setup

    //Initialize stream
    lms_stream_t streamId; //stream structure
    streamId.channel = 0; //channel number
    streamId.fifoSize = 10 * 1024 * 1024; //fifo size in samples
    streamId.throughputVsLatency = 1.0; //optimize for max throughput
    streamId.isTx = false; //RX channel
    streamId.dataFmt = lms_stream_t::LMS_FMT_I12; //12-bit integers
    if (LMS_SetupStream(device, &streamId) != 0)
        error();

    //Initialize data buffers
    static const int udpPackLen = 8192; //complex samples counts
    static const int udpPackNum = 8; //complex samples counts
    static const int sampleCnt = udpPackLen*udpPackNum; //complex samples counts
    static int16_t buffer[sampleCnt*2]; //buffer to hold complex values (2*samples))

    //Start streaming
    LMS_StartStream(&streamId);

    //Streaming
#ifdef USE_GNU_PLOT
    GNUPlotPipe gp;
    gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
#endif
    for(int i=0;i<10000;i++)
    {
    auto t1 = chrono::high_resolution_clock::now();
    int samplesRead = 0;
//    while (chrono::high_resolution_clock::now() - t1 < chrono::milliseconds(200)) //run for 20 seconds
//    for(int i=0;i<2;i++)
    {
        //Receive samples
        samplesRead += LMS_RecvStream(&streamId, buffer, sampleCnt, NULL, 2000);
    //I and Q samples are interleaved in buffer: IQIQIQ...
    /*
        CODE FOR PROCESSING RECEIVED SAMPLES
    */
#ifdef USE_GNU_PLOT
        //Plot samples
        gp.write("plot '-' with points\n");
        for (int j = 0; j < samplesRead; ++j)
            gp.writef("%i %i\n", buffer[2 * j], buffer[2 * j + 1]);
        gp.write("e\n");
        gp.flush();
#endif
    }
    for (int i=0;i<udpPackNum;i++)
    {
        char* udpPackStart = (char*)(&buffer[i*udpPackLen*2]); // '*2' since one sample contains I,Q
        if( sender->writeDatagram(udpPackStart,udpPackLen*2*sizeof(int16_t),QHostAddress::Broadcast,45454) == -1)
            error();
    }
    static int sendPackCnt = 0;
    sendPackCnt++;
//    qDebug() << "sendPackCnt:" <<sendPackCnt;
//    qDebug() << QString("Received %1 samples").arg(samplesRead);
//    while(chrono::high_resolution_clock::now() - t1 < chrono::milliseconds(1100));
    checkLock(device);
    checkDrop(&streamId);
    }
    //Stop streaming
    LMS_StopStream(&streamId); //stream is stopped but can be started again with LMS_StartStream()
    LMS_DestroyStream(device, &streamId); //stream is deallocated and can no longer be used

    //Close device
    LMS_Close(device);
    sender->close();

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

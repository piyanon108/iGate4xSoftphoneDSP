#include "roip_ed137.h"

RoIP_ED137 *RoIP_ED137::theInstance_;
#define THIS_FILE "roip_ed137.cpp"
#define PI 3.141592653589793238462643383279

#define REAL 0
#define IMAG 1
#define PJ_AUTOCONF  1

#define SWVERSION "7.6 09122024"
#define HWVERSION "1.0 12082022"

static void on_reg_state(pjsua_acc_id acc_id);
static void on_call_state(pjsua_call_id call_id, pjsip_event *e);
static void on_call_media_state(pjsua_call_id call_id);
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,pjsip_rx_data *rdata);
static void on_incoming_subscribe(pjsua_acc_id acc_id,pjsua_srv_pres *srv_pres,pjsua_buddy_id buddy_id,const pj_str_t *from,pjsip_rx_data *rdata,pjsip_status_code *code,pj_str_t *reason,pjsua_msg_data *msg_data);
pjmedia_transport* on_create_media_transport(pjsua_call_id call_id, unsigned media_idx,pjmedia_transport *base_tp,unsigned flags);
void on_stream_created(pjsua_call_id call_id, pjmedia_stream *strm, unsigned stream_idx,pjmedia_port **p_port);
void on_stream_destroyed(pjsua_call_id call_id, pjmedia_stream *strm, unsigned stream_idx);

pj_pool_t           *pool;
pjmedia_snd_port    *sndPlayback,*sndCapture;
pjmedia_port	    *sc, *sc_ch1,*sc_ch2,*sc_ch3,*sc_ch4,*sc_ch5;
pjsua_conf_port_id  sc_ch1_slot,sc_ch2_slot,sc_ch3_slot,sc_ch4_slot,sc_ch5_slot;
pjmedia_port        *conf_port;
char log_buffer[250];
using namespace std;


bool validateIpAddress(const string &ipAddress, int Networktimeout = 300)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    float networktimeout = Networktimeout/1000.0;
    if (result != 0)
    {
        QString command = QString("timeout %1 ping -c1 %2").arg(networktimeout).arg(QString::fromStdString(ipAddress));
        int exitCode = QProcess::execute(command);
        QString nc_command = QString("nc -zvn %1 80").arg(QString::fromStdString(ipAddress));
        int nc_exitCode = QProcess::execute(nc_command);
        qDebug() << "Check host" << (exitCode == 0) << (nc_exitCode == 0);
        return  (exitCode == 0) || (nc_exitCode == 0);
    }
    return (result != 0);
}

void RoIP_ED137::onKeyPress(int keyNum, bool val)
{
    if (val == 0)
    {

    }
}

void RoIP_ED137::inviteModeUpdate()
{
    displayTimer->stop();
    scankeyTimer->stop();
    detectR2SPacketAndReconnTimer->stop();
    pjHangupAll();
    Line1 = "   Reload Config    ";
    Line2 = "                    ";
    exit(1);
}

RoIP_ED137::RoIP_ED137(int softphoneID, QObject *parent) :
    QObject(parent)
{
    qDebug() << "load main windows" << "softphoneID" << softphoneID;

    QString startGroupMng = "";

    startGroupMng = QString("systemctl restart iGateGroupCH%1.service").arg(softphoneID);
    system(startGroupMng.toStdString().c_str());
//    CARD_CAPTURE_NORMAL_NOREC = QString("voipIn%1").arg(softphoneID);
//    CARD_PLAYOUT = QString("voipOut%1").arg(softphoneID);

    CARD_CAPTURE_NORMAL_NOREC = QString("in%1").arg(softphoneID);
    CARD_PLAYOUT = QString("out%1").arg(softphoneID);
    switch (softphoneID)
    {
        case 1:
            gpioIn = QString::number(PTT_SQL_INPUT1);
            gpioOut = QString::number(PTT_SQL_OUTPUT1);
        break;
        case 2:
            gpioIn = QString::number(PTT_SQL_INPUT2);
            gpioOut =QString::number(PTT_SQL_OUTPUT2);
        break;
        case 3:
            gpioIn = QString::number(PTT_SQL_INPUT3);
            gpioOut = QString::number(PTT_SQL_OUTPUT3);
        break;
        case 4:
            gpioIn = QString::number(PTT_SQL_INPUT4);
            gpioOut = QString::number(PTT_SQL_OUTPUT4);
        break;
    }

    m_softPhoneID = softphoneID;

    FILESETTING = QString(QString("/home/orinnx/.config/ed137converter/settings%1.ini").arg(softphoneID));

    (void)getRadioType;
    (void)getRadioName;
    (void)getRadiogrOpStat;
    (void)getFrequency;
    (void)getVSWR;
    (void)SqlchThrhCarrier;
    (void)getRadioType2;
    (void)getRadioName2;
    (void)getRadiogrOpStat2;
    (void)getFrequency2;
    (void)getVSWR2;
    (void)SqlchThrhCarrier2;


    connect(this,SIGNAL(cppCommand(QString)),this,SLOT(onTextMessageToSend(QString)));
    connect(this,SIGNAL(onUpdateDisplay()),this,SLOT(sendChannelMessage()));

    SwVersion = SWVERSION;
    HwVersion = HWVERSION;
    scankeyTimer = new QTimer();
    displayTimer = new QTimer();
    checkInputTimer  = new QTimer();
    detectR2SPacketAndReconnTimer = new QTimer();

    trx1 = new channel;
    trx2 = new channel;
    trx1->radio1 = new trx;
    trx1->radio2 = new trx;
    trx2->radio1 = new trx;
    trx2->radio2 = new trx;
    trx1->radio1->radioNodeID = 1;
    trx1->radio2->radioNodeID = 2;
    trx2->radio1->radioNodeID = 3;
    trx2->radio2->radioNodeID = 4;

    for(int i=0; i<32; i++)
    {
        trx_incall.append(new trx);
        trx_incall.at(i)->callIndexName = QString("trx_incall%1").arg(i);
        trx_incall.at(i)->trxFailed = false;
        trx_incall.at(i)->call_id = PJSUA_INVALID_ID;
        trx_incall.at(i)->trxmode = TRXMODE_TRX;
        trx_incall.at(i)->m_PttPressed = false;
        trx_incall.at(i)->radioNodeID = i;
    }

    networking = new NetWorkMng(this);
//    captureLevel = new AudioMeter("capture");
//    playoutLevel = new AudioMeter("playout");
    timeLocation = networking->getTimezone();

    lcdBacklight = 50;
    clock_rate = CLOCK_RATE;

    trxStatus="--";

    myDatabase = new Database("iGatePlus","userData","Ifz8zean6868**","127.0.0.1",this);
    system("rm -rf /tmp/update");
    system("rm -rf /var/www/html/uploads/*");

    pttSQLInputPin = new GPIOClass(gpioIn.toStdString().c_str());
    pttSQLOutputPin = new GPIOClass(gpioOut.toStdString().c_str());

    usleep(10000);

    pttSQLInputPin->setdir_gpio("in");
    pttSQLOutputPin->setdir_gpio("out");
    Line1 = "IFZ Technologies";
    Line2 = "ATC Radio Gateway";

    while (!myDatabase->database_createConnection())
    {
        Line1 = "Database";
        Line2 = "Loading.....";
        system("/etc/init.d/mysql stop");
        system("/etc/init.d/mysql start");
    }

    QThread::msleep(2500);
    trx1->callLastState = DISCONNECTED;
    trx2->callLastState = DISCONNECTED;

    currentmenuIndex = HOMEDISPALY;
    txScheduler = SCH_ONLY_A;

    QThread::msleep(2500);
    theInstance_ = this;

    connect(detectR2SPacketAndReconnTimer,SIGNAL(timeout()),this,SLOT(detectR2SPacketAndReconn()));

    //connect(this,SIGNAL(onPttPressed(bool)),this, SLOT(setPttOutput(bool)));

    usleep(1000);


    acc_id = PJSUA_INVALID_ID;
    m_Register = false;
    m_AutoCall = false;
    m_SendPackets = false;
    m_PttPressed1 = false;
    m_PttPressed2 = false;
    m_RegTimeout = 5;
    m_isTimerRunning = false;
    m_NullSlot = PJ_INVALID_SOCKET;
    SLOT_VOLUME = 2.0f;

    udpActive = false;
    tcpActive = false;

    sampleRate = DEFAULT_SAMPLE_RATE;
    sampleCount = 0;
    isSipCall = false;

    socketClient = new ChatClient(1234);
    connect(socketClient,SIGNAL(connected()), this, SLOT(onConnected()));
    connect(socketClient,SIGNAL(textMessageReceived(QString)), this, SLOT(commandProcess(QString)));
    connect(this,SIGNAL(sendTextMessage(QString)), socketClient, SLOT(sendTextMessage(QString)));

    iGateGroupMNGClient = new ChatClient(8000+quint16(softphoneID));
}

void RoIP_ED137::initSoftPhone(string _SipUser, int _clock_rate, quint16 _sip_port, quint16 _rtp_cfg_port, quint16 _keepAlivePeroid, quint8 _txScheduler, quint8 _numConnection, quint8 _inviteMode, float _sidetone, bool _connToRadio, bool _sqlAlwayOn, QString _defaultEthernet, bool _sqlActiveHigh, bool mainRadioTxUsed, bool mainRadioRxUsed, bool radioautoinactive, int radioMainStandby, bool _rxBestSignalEnable, float _localSidetone, uint8_t _groupMute, uint8_t _pttDelay)
{
    if (_clock_rate == 0) _clock_rate = CLOCK_RATE;
    SipUser             = _SipUser;
    sip_port            = _sip_port;
    keepAlivePeroid     = _keepAlivePeroid;
    txScheduler         = _txScheduler;
    inviteMode          = _inviteMode;
    numConnection       = _numConnection;
    sidetone            = _sidetone;
    connToRadio         = _connToRadio;
    sqlAlwayOn          = _sqlAlwayOn;
    defaultEthernet     = _defaultEthernet;
    sqlActiveHigh       = _sqlActiveHigh;
    clock_rate          = _clock_rate;
    rtp_cfg_port        = _rtp_cfg_port;
    localSidetone       = _localSidetone;
//    mainRadioTransmitterUsed            = mainRadioTxUsed;
//    mainRadioReceiverUsed            = mainRadioRxUsed;
    m_radioAutoInactive = radioautoinactive;
    m_radioMainStandby = radioMainStandby;
    rxBestSignalEnable = _rxBestSignalEnable;
    groupMute = _groupMute;
    pttDelay_set = _pttDelay;
    qDebug() << "pttDelay_set" << pttDelay_set  << "_pttDelay" << _pttDelay;
    if (inviteMode != SERVER)
    {
        QString startGroupMng = QString("systemctl stop iGateGroupCH%1.service").arg(m_softPhoneID);
        system(startGroupMng.toStdString().c_str());
    }
}

void RoIP_ED137::initRadio(quint8 trxMainSTB, QString channelName, QString trxmode, bool enable,bool alwaysConnect, QString radio1callName, QString radio1ipAddress,
                           QString radio1portAddr, QString radio2callName, QString radio2ipAddress, QString radio2portAddr)
{
    qDebug() << "initRadio" << trxMainSTB << trxmode << enable;
    if (trxMainSTB == 1) // Main
    {
        trx1->channelName           = channelName;
        trx1->trxmode               = trxmode;
        trx1->enable                = enable;
        trx1->alwaysConnect         = alwaysConnect;

        trx1->radio1->callName      = radio1callName;
        trx1->radio1->ipAddress     = radio1ipAddress;
        trx1->radio1->portAddr      = radio1portAddr;

        trx1->radio2->callName      = radio2callName;
        trx1->radio2->ipAddress     = radio2ipAddress;
        trx1->radio2->portAddr      = radio2portAddr;        
    }
    else if(trxMainSTB == 2) //Standby
    {
        trx2->channelName           = channelName;
        trx2->trxmode               = trxmode;
        trx2->enable                = enable;
        trx2->alwaysConnect         = alwaysConnect;

        trx2->radio1->callName      = radio1callName;
        trx2->radio1->ipAddress     = radio1ipAddress;
        trx2->radio1->portAddr      = radio1portAddr;

        trx2->radio2->callName      = radio2callName;
        trx2->radio2->ipAddress     = radio2ipAddress;
        trx2->radio2->portAddr      = radio2portAddr;
    }

    if (inviteMode == SERVER)
    {
        qDebug() << "trx1->trxmode << trx1->enable << trx2->trxmode << trx2->enable" <<  trx1->trxmode << trx1->enable << trx2->trxmode << trx2->enable;

        if (trx1->trxmode == TRXMODE_SEPARATE)
        {
            trx1->radio1->trxmode = TRXMODE_TX;
            trx1->radio2->trxmode = TRXMODE_RX;
            trx1->radio1->enable = true;
            trx1->radio2->enable = true;
        }else if (trx1->enable & (trx1->trxmode != "")){
            trx1->radio1->trxmode = trx1->trxmode;
            trx1->radio1->enable = true;
            trx1->radio2->enable = false;
        }
        if (trx1->enable == false){
            trx1->radio1->enable = false;
            trx1->radio2->enable = false;
        }

        if (trx2->trxmode == TRXMODE_SEPARATE){
            trx2->radio1->trxmode = TRXMODE_TX;
            trx2->radio2->trxmode = TRXMODE_RX;
            trx2->radio1->enable = true;
            trx2->radio2->enable = true;
        }else if (trx2->enable & (trx2->trxmode != "")){
            trx2->radio1->trxmode = trx2->trxmode;
            trx2->radio1->enable = true;
            trx2->radio2->enable = false;
        }
        if (trx2->enable == false){
            trx2->radio1->enable = false;
            trx2->radio2->enable = false;
        }
    }
}
void RoIP_ED137::initUriNameAllowList(QStringList uriAllowList)
{
    uriNameAllowList = uriAllowList;
}

void RoIP_ED137::startSoftPhone()
{

//      myConfigurate();
//    connect(chatCtrl,SIGNAL(newConnection(QString, QWebSocket*)),this,SLOT(updateWebData(QString, QWebSocket*)));
//    connect(chatCtrl,SIGNAL(updateTRxConfig(int,QString,QString,QString,QString,QString,QString,QString,QString,QString,QString)),this,SLOT(updateTrxConfig(int,QString,QString,QString,QString,QString,QString,QString,QString,QString,QString)));
//    connect(chatCtrl,SIGNAL(updateNetwork(uint8_t,QString,QString,QString,QString,QString,QString)),this,SLOT(slotupdateNetwork(uint8_t,QString,QString,QString,QString,QString,QString)));
//    connect(chatCtrl,SIGNAL(restartnetwork()),this,SLOT(restartnetwork()));
//    connect(chatCtrl,SIGNAL(updateHostConfig(QString,int,int,QString)),this,SLOT(updateHostConfig(QString,int,int,QString)));
//    connect(chatCtrl,SIGNAL(reconnectTRx(int)),this,SLOT(reconnectTrx(int)));
//    connect(chatCtrl,SIGNAL(updateInputGain(int)),this,SLOT(updateInputGain(int)));
//    connect(chatCtrl,SIGNAL(updateswitchInviteMode(int)),this,SLOT(updateswitchInviteMode(int)));
//    connect(chatCtrl,SIGNAL(updateOutputGain(int)),this,SLOT(updateOutputGain(int)));
//    connect(chatCtrl,SIGNAL(updateSiteTone(float)),this,SLOT(updateSiteTone(float)));
//    connect(chatCtrl,SIGNAL(updatePortInterface(int)),this,SLOT(updatePortInterface(int)));
//    connect(chatCtrl,SIGNAL(updatetxScheduler(int)),this,SLOT(updatetxScheduler(int)));
//    connect(chatCtrl,SIGNAL(updateFirmware()),this,SLOT(updateFirmware()));
//    connect(chatCtrl,SIGNAL(updateNTPServer(QString)),this,SLOT(updateNTPServer(QString)));
//    connect(chatCtrl,SIGNAL(updateDeviceName(QString)),this,SLOT(updateDeviceName(QString)));
//    connect(chatCtrl,SIGNAL(setcurrentDatetime(QString)),this,SLOT(setcurrentDatetime(QString)));
//    connect(chatCtrl,SIGNAL(setLocation(QString)),this,SLOT(setLocation(QString)));
//    connect(chatCtrl,SIGNAL(disconnectUri(QString)),this,SLOT(disconnectUri(QString)));
//    connect(chatCtrl,SIGNAL(updateURILits(QStringList)),this,SLOT(updateURILits(QStringList)));
//    connect(chatCtrl,SIGNAL(updateNumconn(int)),this,SLOT(updateNumconn(int)));
//    connect(chatCtrl,SIGNAL(updateSQLDefeat(bool)),this,SLOT(updateSQLDefeat(bool)));
//    connect(chatCtrl,SIGNAL(updateSQLActiveHigh(bool)),this,SLOT(updateSQLActiveHigh(bool)));
//    connect(chatCtrl,SIGNAL(onNumClientChanged(int)),this,SLOT(onNumClientChanged(int)));
//    connect(chatCtrl,SIGNAL(updateTestmode(int)),this,SLOT(updateTestmode(int)));
//    connect(chatCtrl,SIGNAL(toggleGpioOut(int ,int )),this,SLOT(toggleGpioOut(int ,int )));
//    connect(chatCtrl,SIGNAL(updateCATISCONF(QString, int ,int, int ,int)),this,SLOT(updateCATISCONF(QString, int ,int, int ,int)));
    connect(displayTimer,SIGNAL(timeout()),this,SLOT(display_show()));



    m_EnableRecording = false;

    if (inviteMode == SERVER_RPT)
    {
        inviteMode = SERVER;
        connToRadio = true;
    }

    if (inviteMode == SERVER)
    {
        qDebug() << "trx1->trxmode << trx1->enable << trx2->trxmode << trx2->enable" <<  trx1->trxmode << trx1->enable << trx2->trxmode << trx2->enable ;
        trx_call_hangup(trx1->radio1->call_id,NULL,NULL);
        trx_call_hangup(trx1->radio2->call_id,NULL,NULL);
        trx_call_hangup(trx2->radio1->call_id,NULL,NULL);
        trx_call_hangup(trx2->radio2->call_id,NULL,NULL);
        Line1 = "   Hardware Mode    ";
        Line2 = "  4-Wires to ED-137 ";
        // display->printLine(Line1,LCD_LINE_2,ALIGNCENTER);
        // display->printLine(Line2,LCD_LINE_3,ALIGNCENTER);
//        system("cp /var/www/html/index_server.php /var/www/html/index.php");
//        system("chown -R nano2g:www-data /var/www/html/index.php");

        if (connToRadio)
        {
            pttWithPayload = false;
            m_EnableRecording = false;
        }
        else{
            pttWithPayload = true;
        }

        if (trx1->trxmode == TRXMODE_SEPARATE)
        {
            trx1->radio1->trxmode = TRXMODE_TX;
            trx1->radio2->trxmode = TRXMODE_RX;
            trx1->radio1->enable = true;
            trx1->radio2->enable = true;
        }else if (trx1->enable & (trx1->trxmode != "")){
            trx1->radio1->trxmode = trx1->trxmode;
            trx1->radio1->enable = true;
            trx1->radio2->enable = false;
        }
        if (trx1->enable == false){
            trx1->radio1->enable = false;
            trx1->radio2->enable = false;
        }

        if (trx2->trxmode == TRXMODE_SEPARATE){
            trx2->radio1->trxmode = TRXMODE_TX;
            trx2->radio2->trxmode = TRXMODE_RX;
            trx2->radio1->enable = true;
            trx2->radio2->enable = true;
        }else if (trx2->enable & (trx2->trxmode != "")){
            trx2->radio1->trxmode = trx2->trxmode;
            trx2->radio1->enable = true;
            trx2->radio2->enable = false;
        }
        if (trx2->enable == false){
            trx2->radio1->enable = false;
            trx2->radio2->enable = false;
        }
    }
    else if (inviteMode == CLIENT)
    {
        Line1 = "   Hardware Mode    ";
        Line2 = "  ED-137 to 4-Wires ";
        // display->printLine(Line1,LCD_LINE_2,ALIGNCENTER);
        // display->printLine(Line2,LCD_LINE_3,ALIGNCENTER);
//        system("cp /var/www/html/index_client.php /var/www/html/index.php");
//        system("chown -R nano:www-data /var/www/html/index.php");
        pttWithPayload = false;
        connToRadio = true;
    }
    else if (inviteMode == CLIENT_RPT)
    {
        Line1 = "   Hardware Mode    ";
        Line2 = " Repeater Controler ";
        // display->printLine(Line1,LCD_LINE_2,ALIGNCENTER);
        // display->printLine(Line2,LCD_LINE_3,ALIGNCENTER);
//        system("cp /var/www/html/index_client.php /var/www/html/index.php");
//        system("chown -R nano:www-data /var/www/html/index.php");
        pttWithPayload = false;
        connToRadio = true;
    }
    usleep(3000000);
    trx1->radio1->autoConnectCount = 0;
    trx1->radio2->autoConnectCount = 0;
    trx2->radio1->autoConnectCount = 0;
    trx2->radio2->autoConnectCount = 0;
    if (inviteMode == SERVER)
    {
        Line1 = TRX1 + trx1->mainStandby + trx1->callLastState + trx1->trxStatus;
        Line2 = TRX2 + trx2->mainStandby + trx2->callLastState + trx2->trxStatus;
    }
    else if (inviteMode == CLIENT)
    {
        Line1 = " ";
        Line2 = " ";
        trx1->enable = false;
        trx2->enable = false;
    }
    else if (inviteMode == CLIENT_RPT)
    {
        Line1 = " ";
        Line2 = " ";
        trx1->enable = false;
        trx2->enable = false;
    }
    updateUri();
    qDebug() << "inviteMode" << inviteMode;


    displayTimer->start(78);
    detectR2SPacketAndReconnTimer->start(40);


//    StartSip();
//    setAudioDevInfo();
//    setCodecPriority();
//    listConfs();
    startSipByUser();

    QObject::connect(this, SIGNAL(pttStatusChange(bool)),this, SLOT(onPttStatusChange(bool)));
    QObject::connect(this, SIGNAL(sqlStatusChange(bool)),this, SLOT(onSqlOnChange(bool)));
    QObject::connect(this, SIGNAL(keyPress(int,bool)),this, SLOT(onKeyPress(int,bool)));
    //QObject::connect(this, SIGNAL(newfrequency(int , QString , QString ,QString, int )), this, SLOT(updatefrequency(int , QString , QString ,QString, int )));

    if (inviteMode == SERVER){
        pttSQLOutputPin->resetGpio();

        m_IsSNMPThreadRunning = true;
    }
    else if (inviteMode == CLIENT)
    {
        pttSQLOutputPin->resetGpio();
        m_IsSNMPThreadRunning = false;
    }
    else if (inviteMode == CLIENT_RPT)
    {
        pttSQLOutputPin->resetGpio();
        m_IsSNMPThreadRunning = false;
    }

    m_IsSNMPThreadRunning = true; // TEST.........................
//    qDebug()  << "TEST......................... m_IsSNMPThreadRunning " << m_IsSNMPThreadRunning;
    m_SNMPThreadInstance = new boost::shared_ptr<boost::thread>;
    m_SNMPThreadInstance->reset(new boost::thread(boost::bind(&RoIP_ED137::getSnmp, this)));

//    qDebug()  << "TEST......................... m_ScanKeyThreadRunning " << m_ScanKeyThreadRunning;
//    m_ScanKeyThreadRunning = false; // TEST.........................
    m_ScanKeyThreadRunning = true;
    m_ScanKeyThreadInstance = new boost::shared_ptr<boost::thread>;
    m_ScanKeyThreadInstance->reset(new boost::thread(boost::bind(&RoIP_ED137::scanKeyPin, this)));

//    m_GetInputThreadRunning = true;
//    m_GetInputThreadInstance = new boost::shared_ptr<boost::thread>;
//    m_GetInputThreadInstance->reset(new boost::thread(boost::bind(&RoIP_ED137::getAudioInputLevel, this)));

//    m_GetOutputThreadRunning = false;
//    m_GetOutputThreadInstance = new boost::shared_ptr<boost::thread>;
//    m_GetOutputThreadInstance->reset(new boost::thread(boost::bind(&RoIP_ED137::getAudioOutputLevel, this)));

    int ret;
    ret=pthread_create(&idThread, nullptr, ThreadFunc, this);
    if(ret==0){
        qDebug() <<("Thread1 created successfully.\n");
    }
    else{
        qDebug() <<("Thread1 not created.\n");
    }

//    connect(captureLevel,SIGNAL(onValueChanged(int)),this,SLOT(updateInputLevel(int)));
//    connect(playoutLevel,SIGNAL(onValueChanged(int)),this,SLOT(updateOutputLevel(int)));

}

void RoIP_ED137::startSipByUser()
{
    if (sipStarted == false){


        StartSip();
        setAudioDevInfo();
        setCodecPriority();
        listConfs();

        sipStarted = true;
        //system("systemctl restart aloop.service");
    }

    updateUri();
}

void RoIP_ED137::getAudioInputLevel()
{
    qDebug() << "start getAudioInputLevel thread";
    while (m_GetInputThreadRunning)
    {
        ::usleep(25000);
        boost::unique_lock<boost::mutex> scoped_lock(getInput_mutex);
        captureLevel->getAudioLevel();
    }
}

void RoIP_ED137::getAudioOutputLevel()
{
    qDebug() << "start getAudioOutputLevel thread";
    while (m_GetOutputThreadRunning)
    {
        ::usleep(25000);
        boost::unique_lock<boost::mutex> scoped_lock(getOutput_mutex);
        playoutLevel->getAudioLevel();
    }
}

void RoIP_ED137::updateInputLevel(int level)
{
    if (level > 0)
        m_input_level = float(level/100.0);
    else
        m_input_level = 0;

//    qDebug() << m_input_level;
}

void RoIP_ED137::updateOutputLevel(int level)
{
    if (level > 0)
        m_output_level = float(level/100.0);
    else m_output_level = 0;
}

void RoIP_ED137::updateUri()
{
    trx1->radio1->url = QString("");
    trx1->radio2->url = QString("");
    trx2->radio1->url = QString("");
    trx2->radio2->url = QString("");

    if (trx1->radio1->callName != "")
        trx1->radio1->url = QString("<sip:%1@%2:%3>").arg(trx1->radio1->callName).arg(trx1->radio1->ipAddress).arg(trx1->radio1->portAddr);
    if (trx1->radio2->callName != "")
        trx1->radio2->url = QString("<sip:%1@%2:%3>").arg(trx1->radio2->callName).arg(trx1->radio2->ipAddress).arg(trx1->radio2->portAddr);
    if (trx2->radio1->callName != "")
        trx2->radio1->url = QString("<sip:%1@%2:%3>").arg(trx2->radio1->callName).arg(trx2->radio1->ipAddress).arg(trx2->radio1->portAddr);
    if (trx2->radio2->callName != "")
        trx2->radio2->url = QString("<sip:%1@%2:%3>").arg(trx2->radio2->callName).arg(trx2->radio2->ipAddress).arg(trx2->radio2->portAddr);

//    qDebug() << trx1->radio1->url;
//    qDebug() << trx1->radio2->url;
//    qDebug() << trx2->radio1->url;
//    qDebug() << trx2->radio2->url;
}

QStringList RoIP_ED137::findFile()
{
    QStringList listfilename;
    QString ss="/var/www/html/uploads/";
    const char *sss ; sss = ss.toStdString().c_str();
    QDir dir1("/var/www/html/uploads/");
    QString filepath;
    QString filename;
    QFileInfoList fi1List( dir1.entryInfoList( QDir::Files, QDir::Name) );
    foreach( const QFileInfo & fi1, fi1List ) {
        filepath = QString::fromUtf8(fi1.absoluteFilePath().toLocal8Bit());
        filename = QString::fromUtf8(fi1.fileName().toLocal8Bit());
        listfilename << filepath;
        qDebug() << filepath;// << filepath.toUtf8().toHex();
    }
    return listfilename;
}
void RoIP_ED137::pjHangupAll()
{
    trx1->radio1->callState= false;
    trx1->radio2->callState= false;
    trx2->radio1->callState= false;
    trx2->radio2->callState= false;
//    trx1->radio1->call_id= PJSUA_INVALID_ID;
//    trx1->radio2->call_id= PJSUA_INVALID_ID;
//    trx2->radio1->call_id= PJSUA_INVALID_ID;
//    trx2->radio2->call_id= PJSUA_INVALID_ID;
    for (int i=0; i<trx_incall.length();i++)
    {
        trx_incall.at(i)->callState = false ;
        trx_incall.at(i)->call_id = PJSUA_INVALID_ID;
    }
    pjsua_call_hangup_all();

}
void RoIP_ED137::updateFirmware(){
    QStringList fileupdate;
    trx1->enable = false;
    trx2->enable = false;
    pjHangupAll();
    trx1->radio1->call_id= PJSUA_INVALID_ID;
    trx1->radio2->call_id= PJSUA_INVALID_ID;
    trx2->radio1->call_id= PJSUA_INVALID_ID;
    trx2->radio2->call_id= PJSUA_INVALID_ID;

    trx1->radio1->callState= false;
    trx1->radio2->callState= false;
    trx2->radio1->callState= false;
    trx2->radio2->callState= false;

//    removeDefault();
    currentmenuIndex = SOFTWAREUPDATE_UPDATEING;
    Line1 = strSystemSwUpdate;
    Line2 = strUpdating;
    fileupdate = findFile();
    system("mkdir /tmp/update");
    if (fileupdate.size() > 0){
        QString commandCopyFile = "cp " + QString(fileupdate.at(0)) + " /tmp/update/update.tar";
        system(commandCopyFile.toStdString().c_str());
        system("tar -xf /tmp/update/update.tar -C /tmp/update/");
        system("sh /tmp/update/update.sh");
        system("rm -r /tmp/update");
        system("rm -r /var/www/html/uploads/*");
        currentmenuIndex = SOFTWAREUPDATE_UPDATED;
        Line1 = strSystemSwUpdate;
        Line2 = strUpdateed;
    }else{
//        system("reboot");
    }

}
void RoIP_ED137::toggleGpioOut(int gpioNum, int gpioVal)
{

}
void RoIP_ED137::updateWebData(QString menuIndex,  QWebSocket *webClient)
{
    uint8_t trx1mode = 0, trx2mode = 0, trx1active = 0, trx2active = 0;
    uint8_t dhcpmethodInt;
    if (trx1->enable) trx1active = 1;
    if (trx2->enable) trx2active = 1;

    if (trx1->trxmode == TRXMODE_TRX) trx1mode = 0;
    else if (trx1->trxmode == TRXMODE_TX) trx1mode = 1;
    else if (trx1->trxmode == TRXMODE_RX) trx1mode = 2;
    else if (trx1->trxmode == TRXMODE_SEPARATE) trx1mode = 3;

    if (trx2->trxmode == TRXMODE_TRX) trx2mode = 0;
    else if (trx2->trxmode == TRXMODE_TX) trx2mode = 1;
    else if (trx2->trxmode == TRXMODE_RX) trx2mode = 2;
    else if (trx2->trxmode == TRXMODE_SEPARATE) trx2mode = 3;


    int defaultEthernetInt = 0;
    if (defaultEthernet == "eth1") defaultEthernetInt = 1;

    int converterMode = inviteMode;
    if((inviteMode == SERVER)&(connToRadio))
    {
        converterMode = SERVER_RPT;
    }


}
void RoIP_ED137::updateTRxStatus()
{
    if (trx1->trxmode == TRXMODE_SEPARATE)
    {
        if ((!trx1->radio1->trxFailed)&&(!trx1->radio2->trxFailed)){
            if ((trx1->radio1->callState)&&(trx1->radio2->callState)){
                trx1->callLastState = CONNECTED;
            }
            else if ((trx1->radio1->callState)&&(trx1->radio2->callState == false)){
                trx1->callLastState = "Tx Only   ";
            }
            else if ((trx1->radio1->callState == false)&&(trx1->radio2->callState)){
                trx1->callLastState = "Rx Only   ";
            }
        }
        else if ((trx1->radio1->trxFailed)&&(!trx1->radio2->trxFailed)){
            if (trx1->trxStatus == STANDBY)
                trx1->callLastState = "Tx:" + trx1->radio1->ByeReason;
            else if(trx1->radio2->callState)
                trx1->callLastState = "Rx Only   ";
            else
                trx1->callLastState = "Tx:" + trx1->radio1->ByeReason;
        }
        else if ((!trx1->radio1->trxFailed)&&(trx1->radio2->trxFailed)){

            if (trx1->trxStatus == STANDBY)
                trx1->callLastState = "Rx:" + trx1->radio2->ByeReason;
            else if(trx1->radio1->callState)
                trx1->callLastState = "Tx Only   ";
            else
                trx1->callLastState = "Rx:" + trx1->radio2->ByeReason;
        }
        else if ((trx1->radio1->trxFailed)&&(trx1->radio2->trxFailed)){
            trx1->callLastState =  "TxRx Failed";
        }
        if ((trx1->radio1->callState == false)&&(trx1->radio2->callState == false)){
            trx1->callLastState = DISCONNECTED;
        }
    }else{
        if (!trx1->radio1->trxFailed){
            trx1->trxStatus = trx1->radio1->trxStatus;
            trx1->callLastState = trx1->radio1->callLastState;
        }
        else {
            trx1->trxStatus = "";
            trx1->callLastState = trx1->radio1->ByeReason;
        }
    }

    if (trx2->trxmode == TRXMODE_SEPARATE){

        if ((!trx2->radio1->trxFailed)&&(!trx2->radio2->trxFailed)){
            if ((trx2->radio1->callState)&&(trx2->radio2->callState)){
                trx2->callLastState = CONNECTED;
            }
            else if ((trx2->radio1->callState)&&(trx2->radio2->callState == false)){
                trx2->callLastState = "Tx Only   ";
            }
            else if ((trx2->radio1->callState == false)&&(trx2->radio2->callState)){
                trx2->callLastState = "Rx Only   ";
            }
        }
        else if ((trx2->radio1->trxFailed)&&(!trx2->radio2->trxFailed)){
            if (trx2->trxStatus == STANDBY)
                trx2->callLastState = "Tx:" + trx2->radio1->ByeReason;
            else if(trx2->radio2->callState)
                trx2->callLastState = "Rx Only   ";
            else
                trx2->callLastState = "Tx:" + trx2->radio1->ByeReason;
        }
        else if ((!trx2->radio1->trxFailed)&&(trx2->radio2->trxFailed)){

            if (trx2->trxStatus == STANDBY)
                trx2->callLastState = "Rx:" + trx2->radio2->ByeReason;
            else if(trx2->radio1->callState)
                trx2->callLastState = "Tx Only   ";
            else
                trx2->callLastState = "Rx:" + trx2->radio2->ByeReason;
        }
        else if ((trx2->radio1->trxFailed)&&(trx2->radio2->trxFailed)){
            trx2->callLastState =  "TxRx Failed";
        }
        if ((trx2->radio1->callState == false)&&(trx2->radio2->callState == false)){
            trx2->callLastState = DISCONNECTED;
        }
    }else{
        if (!trx2->radio1->trxFailed){
            trx2->trxStatus = trx2->radio1->trxStatus;
            trx2->callLastState = trx2->radio1->callLastState;
        }
        else {
            trx2->trxStatus = "";
            trx2->callLastState = trx2->radio1->ByeReason;
        }
    }
//    qDebug() << "updateTRxStatus";
}

void RoIP_ED137::updateHomeDisplay(QString trxid, QString connState, QString trxState, pjsua_call_id call_id, int radioNodeID)
{
    qDebug() << "updateHomeDisplay" << trxid << connState << trxState << radioNodeID;// << trx1->radio2->url;

    if (inviteMode == SERVER)
    {
        if ((connState == DISCONNECTED) & (radioNodeID == trx1->radio1->radioNodeID) & (trxid == trx1->radio1->url))
        {
            if (trx1->radio1->lastTx)
            {
                trx1->radio1->lastTx = false;
                m_PttPressed1 = false;
                if (localSidetoneLoopbackOn & (trx2->radio1->lastTx == false)){
                    localSidetoneLoopbackOn = false;
                }
                if (onLocalSidetoneLoopbackChanged != localSidetoneLoopbackOn)
                {
                    onLocalSidetoneLoopbackChanged = localSidetoneLoopbackOn;
                    QString message = QString("{\"menuID\":\"onLocalSidetoneLoopbackChanged\", \"localSidetoneLoopbackOn\":%1, \"softPhoneID\":%2}")
                            .arg(localSidetoneLoopbackOn).arg(m_softPhoneID);
                    emit sendTextMessage(message);
                }
            }
        }
        else if ((connState == DISCONNECTED) & (radioNodeID == trx2->radio1->radioNodeID) & (trxid == trx2->radio1->url))
        {
            if (trx2->radio1->lastTx)
            {
                trx2->radio1->lastTx = false;
                m_PttPressed2 = false;
                if (localSidetoneLoopbackOn & (trx1->radio1->lastTx == false))
                    localSidetoneLoopbackOn = false;
                if (onLocalSidetoneLoopbackChanged != localSidetoneLoopbackOn)
                {
                    onLocalSidetoneLoopbackChanged = localSidetoneLoopbackOn;
                    QString message = QString("{\"menuID\":\"onLocalSidetoneLoopbackChanged\", \"localSidetoneLoopbackOn\":%1, \"softPhoneID\":%2}")
                            .arg(localSidetoneLoopbackOn).arg(m_softPhoneID);
                    emit sendTextMessage(message);
                }
            }
        }

        if ((radioNodeID == trx1->radio1->radioNodeID) & (trxid == trx1->radio1->url) & (trx1->radio1->call_id == call_id))
        {
            if (!trx1->radio1->trxFailed)
            {
                trx1->radio1->callLastState = connState;
                trx1->radio1->trxStatus = trxState;
            }
            else
            {
                trx1->radio1->trxStatus = "";
                trx1->radio1->callLastState = trx1->radio1->ByeReason;
            }
        }
        else if ((radioNodeID == trx1->radio2->radioNodeID) & (trxid == trx1->radio2->url) & (trx1->radio2->call_id == call_id))
        {
            if (!trx1->radio2->trxFailed)
            {
                trx1->radio2->callLastState = connState;
                trx1->radio2->trxStatus = trxState;
            }
            else
            {
                trx1->radio2->trxStatus = "";
                trx1->radio2->callLastState = trx1->radio2->ByeReason;
            }            
        }

        else if ((radioNodeID == trx2->radio1->radioNodeID) & (trxid == trx2->radio1->url) & (trx2->radio1->call_id == call_id))
        {
            if (!trx2->radio1->trxFailed)
            {
                trx2->radio1->callLastState = connState;
                trx2->radio1->trxStatus = trxState;
            }
            else
            {
                trx2->radio1->trxStatus = "";
                trx2->radio1->callLastState = trx2->radio1->ByeReason;
            }
        }
        else if ((radioNodeID == trx2->radio2->radioNodeID) & (trxid == trx2->radio2->url) & (trx2->radio2->call_id == call_id))
        {
            if (!trx2->radio2->trxFailed)
            {
                trx2->radio2->callLastState = connState;
                trx2->radio2->trxStatus = trxState;
            }
            else
            {
                trx2->radio2->trxStatus = "";
                trx2->radio2->callLastState = trx2->radio2->ByeReason;
            }
        }
        if (((trxid == trx1->radio1->url)||(trxid == trx1->radio2->url)) && ((trx1->radio1->call_id == call_id)||(trx1->radio2->call_id == call_id)) && ((radioNodeID == trx1->radio1->radioNodeID) || (radioNodeID == trx1->radio2->radioNodeID)))
        {
            if (trx1->trxmode == TRXMODE_SEPARATE)
            {
                if ((trx1->radio1->trxStatus == PTTON)&&(trx1->radio2->trxStatus == RXON)){
                    trx1->trxStatus = PTTRX;
                }else if ((trx1->radio1->trxStatus == PTTON)&&(trx1->radio2->trxStatus != RXON)){
                    trx1->trxStatus = PTTON;
                }else if ((trx1->radio1->trxStatus != PTTON)&&(trx1->radio2->trxStatus == RXON)){
                    trx1->trxStatus = RXON;
                }else if ((trx1->radio1->trxStatus != PTTON)&&(trx1->radio2->trxStatus != RXON)){
                    trx1->trxStatus = trxState;
                }
            }
            else
            {
//                if ((trx1->radio1->trxStatus == RXON) || (trx1->radio2->trxStatus == RXON))
                trx1->trxStatus = trxState;

            }
        }
        if (((trxid == trx2->radio1->url)||(trxid == trx2->radio2->url)) && ((trx2->radio1->call_id == call_id)||(trx2->radio2->call_id == call_id)) && ((radioNodeID == trx2->radio1->radioNodeID) || (radioNodeID == trx2->radio2->radioNodeID)))
        {
            if (trx2->trxmode == TRXMODE_SEPARATE){
                if ((trx2->radio1->trxStatus == PTTON)&&(trx2->radio2->trxStatus == RXON)){
                    trx2->trxStatus = PTTRX;
                }else if ((trx2->radio1->trxStatus == PTTON)&&(trx2->radio2->trxStatus != RXON)){
                    trx2->trxStatus = PTTON;
                }else if ((trx2->radio1->trxStatus != PTTON)&&(trx2->radio2->trxStatus == RXON)){
                    trx2->trxStatus = RXON;
                }else if ((trx2->radio1->trxStatus != PTTON)&&(trx2->radio2->trxStatus != RXON)){
                    trx2->trxStatus = trxState;
                }
            }
            else
            {
                if ((trx2->radio1->trxStatus == RXON) || (trx2->radio2->trxStatus == RXON))
                    trx2->trxStatus = RXON;
            }
        }
//        qDebug() << "trx1->radio2->url" << trx1->radio2->url << trx1->radio2->trxStatus << "trx1->trxStatus" << trx1->trxStatus;

        updateTRxStatus();

        if (((trx1->trxStatus == STANDBY)||(trx1->trxStatus == ""))&&((trx2->trxStatus == STANDBY)||(trx2->trxStatus == "")))
        {
//            setTXLED(LEDOFF);
//            setRXLED(LEDOFF);
//            if (networking->get_gst_launch_process()){
//                if ((m_c_atis_fault == false) & (testModeEnable == 0))
//                {
//                    if((sqlOldPinOnStatus & connToRadio) == false)
//                    {
//                        //system("audioCapture.sh kill");
//                        //qDebug() << "audioCapture.sh kill";
//                    }
//                }
//                else
//                {

//                }

//            }
            qDebug() << "trx1->trxStatus" << trx1->trxStatus << "sqlOnStatus" << sqlOnStatus << "pttStatus" << pttStatus << "trx1->radio2->trxStatus" << trx1->radio2->trxStatus;
            sqlOnStatus = false;
            pttStatus = false;
            if (pttSQLOutputPin->getGpioVal() == 1)
            {
                if (localSidetoneLoopbackOn == false)
                {
                    pttSQLOutputPin->resetGpio();
                }
                else {
                    localSidetoneLoopbackOn = false;
                    if (onLocalSidetoneLoopbackChanged != localSidetoneLoopbackOn)
                    {
                        onLocalSidetoneLoopbackChanged = localSidetoneLoopbackOn;
                        QString message = QString("{\"menuID\":\"onLocalSidetoneLoopbackChanged\", \"localSidetoneLoopbackOn\":%1, \"softPhoneID\":%2}")
                                .arg(localSidetoneLoopbackOn).arg(m_softPhoneID);
                        emit sendTextMessage(message);
                    }
                    pttSQLOutputPin->resetGpio();
                }
            }
            recorder_released(0);
//            system("systemctl stop aloop.service");
        }
        else
        {
            if (!connToRadio)
            {
//                qDebug() << "if (!connToRadio)" << trxid << connState << trxState << "pttStatus" << pttStatus;
                if ((trxState == RXON) || (trxState == PTTRX) || (trxState == PTTON))
                {
//                    if (pttSQLOutputPin->getGpioVal() == 0){
//                        if (networking->get_gst_launch_process() == false)
//                        {
//                            if ((m_c_atis_fault == false) & (testModeEnable == 0))
//                            {
//                                //system("audioCapture.sh default");
//                                //qDebug() << "audioCapture.sh default";
//                            }
//                            else
//                            {
//                                //system("audioCapture.sh player");
//                                //qDebug() << "audioCapture.sh player";
//                            }
//                        }
//                    }
//                    setRXLED(LEDON);
                    if ((trx1->mainRx) & ((trx1->trxStatus == RXON) || (trx1->trxStatus == PTTRX)))
                    {
                        pttSQLOutputPin->setGpio();
                        if((pttStatus) & (localSidetone > 0))
                        {
                            localSidetoneLoopbackOn = true;
                            qDebug() << "pttStatusChange" << pttStatus << "localSidetoneLoopbackOn" << localSidetoneLoopbackOn;
                            if (onLocalSidetoneLoopbackChanged != localSidetoneLoopbackOn)
                            {
                                onLocalSidetoneLoopbackChanged = localSidetoneLoopbackOn;
                                QString message = QString("{\"menuID\":\"onLocalSidetoneLoopbackChanged\", \"localSidetoneLoopbackOn\":%1, \"softPhoneID\":%2}")
                                        .arg(localSidetoneLoopbackOn).arg(m_softPhoneID);
                                emit sendTextMessage(message);
                            }
                        }
                    }
//                    if ((trx1->mainTx) & ((trx1->trxStatus == PTTON) || (trx1->trxStatus == PTTRX)))
//                    {
//                        if((pttStatus) & (localSidetone > 0))
//                        {
//                            localSidetoneLoopbackOn = true;
//                            pttSQLOutputPin->setGpio();
//                            qDebug() << "pttStatusChange" << pttStatus << "localSidetoneLoopbackOn" << localSidetoneLoopbackOn;
//                            if (onLocalSidetoneLoopbackChanged != localSidetoneLoopbackOn)
//                            {
//                                onLocalSidetoneLoopbackChanged = localSidetoneLoopbackOn;
//                                QString message = QString("{\"menuID\":\"onLocalSidetoneLoopbackChanged\", \"localSidetoneLoopbackOn\":%1, \"softPhoneID\":%2}")
//                                        .arg(localSidetoneLoopbackOn).arg(m_softPhoneID);
//                                emit sendTextMessage(message);
//                            }
//                        }
//                    }

                    if ((trx2->mainRx) & ((trx2->trxStatus == RXON) || (trx2->trxStatus == PTTRX)))
                    {
                        pttSQLOutputPin->setGpio();
                        if((pttStatus) & (localSidetone > 0))
                        {
                            localSidetoneLoopbackOn = true;
                            qDebug() << "pttStatusChange" << pttStatus << "localSidetoneLoopbackOn" << localSidetoneLoopbackOn;
                        }
                        if (onLocalSidetoneLoopbackChanged != localSidetoneLoopbackOn)
                        {
                            onLocalSidetoneLoopbackChanged = localSidetoneLoopbackOn;
                            QString message = QString("{\"menuID\":\"onLocalSidetoneLoopbackChanged\", \"localSidetoneLoopbackOn\":%1, \"softPhoneID\":%2}")
                                    .arg(localSidetoneLoopbackOn).arg(m_softPhoneID);
                            emit sendTextMessage(message);
                        }
                    }
//                    if ((trx2->mainTx) & ((trx2->trxStatus == PTTON) || (trx2->trxStatus == PTTRX)))
//                    {
//                        if((pttStatus) & (localSidetone > 0))
//                        {
//                            pttSQLOutputPin->setGpio();
//                            localSidetoneLoopbackOn = true;
//                            qDebug() << "pttStatusChange" << pttStatus << "localSidetoneLoopbackOn" << localSidetoneLoopbackOn;
//                        }
//                        if (onLocalSidetoneLoopbackChanged != localSidetoneLoopbackOn)
//                        {
//                            onLocalSidetoneLoopbackChanged = localSidetoneLoopbackOn;
//                            QString message = QString("{\"menuID\":\"onLocalSidetoneLoopbackChanged\", \"localSidetoneLoopbackOn\":%1, \"softPhoneID\":%2}")
//                                    .arg(localSidetoneLoopbackOn).arg(m_softPhoneID);
//                            emit sendTextMessage(message);
//                        }
//                    }
                    if ((trx1->mainRx == false) & (trx2->mainRx == false) & (localSidetoneLoopbackOn == false))
                    {
                        pttSQLOutputPin->resetGpio();
                    }
                    if (((trx1->trxStatus != RXON) & (trx1->trxStatus != PTTRX)) & ((trx2->trxStatus != RXON) & (trx2->trxStatus != PTTRX)) & (localSidetoneLoopbackOn == false))
                    {
                        pttSQLOutputPin->resetGpio();
                    }
//                    if (pttSQLOutputPin->getGpioVal() == 1){
//                        if (networking->get_gst_launch_process() == false)
//                        {
//                            if ((m_c_atis_fault == false) & (testModeEnable == 0))
//                            {
//                                //system("audioCapture.sh default");
//                                //qDebug() << "audioCapture.sh default";
//                            }
//                            else
//                            {
//                                //system("audioCapture.sh player");
//                                //qDebug() << "audioCapture.sh player";
//                            }
//                        }
//                    }
                }
                else
                {
                    if ((trx1->mainRx) & ((trx1->trxStatus == RXON) || (trx1->trxStatus == PTTRX)))
                    {
                        pttSQLOutputPin->setGpio();
                    }
                    else if ((trx2->mainRx) & ((trx2->trxStatus == RXON) || (trx2->trxStatus == PTTRX)))
                    {
                        pttSQLOutputPin->setGpio();
                    }
                    else
                    {
                        qDebug() << "trx1" << trx1->trxStatus << "trx2" << trx2->trxStatus << trx1->mainRx << trx2->mainRx;
                        if ((localSidetoneLoopbackOn == false) || (forceMuteSqlOn == true))
                            pttSQLOutputPin->resetGpio();
//                        if ((trx1->mainRx) & (trx1->trxStatus != RXON)))
//                        {
//                            pttSQLOutputPin->resetGpio();
//                        }
//                        if ((trx2->mainRx) & (trx2->trxStatus != RXON)){
//                            pttSQLOutputPin->resetGpio();
//                        }
                    }
                }
                if (trxState == PTTON)
                {
//                    setTXLED(LEDON);
                }
                else
                {

                }
            }
            else
            {
                if (trxState == PTTON)
                {
//                    if (pttSQLOutputPin->getGpioVal() == 0){
//                        if (networking->get_gst_launch_process() == false)
//                        {
//                            //system("audioCapture.sh default");
//                            qDebug() << "PTTON audioCapture.sh default";
//                        }
//                    }
//                    setRXLED(LEDON);
                    if ((trx1->mainTx) & (trx1->trxStatus == PTTON))
                        pttSQLOutputPin->setGpio();
                    if ((trx2->mainTx) & (trx2->trxStatus == PTTON))
                        pttSQLOutputPin->setGpio();
                    if ((!trx1->mainTx) & (!trx2->mainTx))
                        pttSQLOutputPin->setGpio();

//                    if (pttSQLOutputPin->getGpioVal() == 1){
//                        if (networking->get_gst_launch_process() == false)
//                        {
//                            //system("audioCapture.sh default");
//                            //qDebug() << "audioCapture.sh default";
//                        }
//                    }
//                    system("systemctl stop aloop.service");
                    recorder_pressed(1,PTTON);
                }
                else
                {
                    if (((trx1->mainTx) || (trx2->mainTx)) & ((trx1->trxStatus == PTTON) || (trx2->trxStatus == PTTON)))
                    {
                        pttSQLOutputPin->setGpio();
                    }
                    else {
                        if (localSidetoneLoopbackOn == false)
                            pttSQLOutputPin->resetGpio();
//                        if ((trx1->mainTx) & (trx1->trxStatus != PTTON))
//                            pttSQLOutputPin->resetGpio();
//                        if ((trx2->mainTx) & (trx2->trxStatus != PTTON))
//                            pttSQLOutputPin->resetGpio();
                    }
                }
                if (trxState == RXON)
                {
//                    system("systemctl restart aloop.service");
                    recorder_pressed(1,RXON);
//                    setTXLED(LEDON);
//                    if (pttInput == 1)
//                    {
//                        if (networking->get_gst_launch_process() == false)
//                        {
//                            //system("audioCapture.sh default");
//                            qDebug() << "RXON audioCapture.sh default";
//                        }
//                    }
                }
            }
        }

        if ((trx1->radio1->callState)||(trx2->radio1->callState)||(trx1->radio2->callState)||(trx2->radio2->callState))
        {

        }
        else
        {

        }
    }
    else if (inviteMode == CLIENT)
    {
        qDebug() << "trxState" << trxState << "pttOnStatus" << pttOnStatus << "sqlOnStatus" << sqlOnStatus;
        if (trxState != "")
            trxStatus = trxState;
        if (trxState == PTTON)
        {
            pttOnStatus = true;
            pttSQLOutputPin->setGpio();
            pptTest_CallIn_Pressed(ptt_level);

            if (sqlOnStatus == false)
            {
                sqlTest_released();
            }
            else if ((sqlOnStatus == true) & ((m_TxRx1stPriority == RX1ST) || (m_TxRx1stPriority == FULLDUPLEX)))
            {
                sqlTest_pressed(7);
            }
        }
        else if (trxStatus == RXON)
        {
            if (m_TxRx1stPriority == RX1ST)
            {
                pttOnStatus = false;
                pttSQLOutputPin->resetGpio();
                pptTest_CallIn_Released(ptt_level);
                ptt_level = 0;
            }
            else if (m_TxRx1stPriority == TX1ST)
            {

            }
            if (pttOnStatus == false)
            {
                pptTest_CallIn_Released(ptt_level);
                pttSQLOutputPin->resetGpio();
                ptt_level = 0;
            }
        }
        if ((pttOnStatus) || (sqlOnStatus))
        {
            if (pttOnStatus & sqlOnStatus)
                trxStatus = PTTRX;
        }
        if (trxStatus == STANDBY)
        {
            if (pttOnStatus == false)
            {
                pptTest_CallIn_Released(ptt_level);
                pttSQLOutputPin->resetGpio();
                ptt_level = 0;
            }

            if (sqlOnStatus == false)
                sqlTest_released();
//            system("systemctl stop aloop.service");
        }
        else if ((trxStatus == PTTON)||(trxStatus == RXON))
        {
            if (trxState == RXON)
            {
//                system("systemctl restart aloop.service");
            }
            if (trxState == PTTON)
            {
//                system("systemctl stop aloop.service");
            }
        }
        else if (trxStatus == PTTRX)
        {
//            system("systemctl stop aloop.service");
        }
    }
    else if (inviteMode == CLIENT_RPT)
    {
        if (trxState != ""){
            trxStatus = trxState;

            if (trxState == PTTON)
            {
                qDebug() << "updateHomeDisplay" << trxid << connState << trxState;

//                system("systemctl stop aloop.service");
                pttSQLOutputPin->setGpio();
                pptTest_CallIn_Pressed(ptt_level);
            }
            else if (trxStatus == STANDBY)
            {
                qDebug() << "updateHomeDisplay if ((trxStatus == STANDBY)||(trxStatus == RXON))" << trxid << connState << trxState;
//                system("systemctl stop aloop.service");
                pttSQLOutputPin->resetGpio();
                pptTest_CallIn_Released(ptt_level);
                ptt_level = 0;
            }
            else if (trxStatus == RXON)
            {
                qDebug() << "updateHomeDisplay if ((trxStatus == STANDBY)||(trxStatus == RXON))" << trxid << connState << trxState;
//                system("systemctl restart aloop.service");
                pttSQLOutputPin->resetGpio();
                pptTest_CallIn_Released(ptt_level);
                ptt_level = 0;
            }
            else if(trxStatus == RPTON)
            {
                qDebug() << "updateHomeDisplay if(trxStatus == RPTON)" << trxid << connState << trxState;
//                system("systemctl stop aloop.service");
                sqlOnStatus = true;
                pttSQLOutputPin->setGpio();
            }
            if (trxStatus == STANDBY)
            {
                if (sqlOnStatus || pttStatus){
                    repeat_released(0);
                    recorder_released(0);
                }
                sqlOnStatus = false;
                pttStatus = false;
            }
            else if ((trxStatus == PTTON)||(trxStatus == RXON))
            {
                if (trxState == RXON){

                }
                if (trxState == PTTON){

                }
            }
        }
    }

//    emit onPttPressed(pttSQLOutputPin->getGpioVal() == 1);
/*
    if (currentmenuIndex == HOMEDISPALY)
    {
        if (inviteMode == SERVER){
            if (((trxid == trx1->radio1->url)||(trxid == trx1->radio2->url)) && ((trx1->radio1->call_id == call_id)||(trx1->radio2->call_id == call_id)) && ((radioNodeID == trx1->radio1->radioNodeID) || (radioNodeID == trx1->radio2->radioNodeID)))
            {
                Line1 = TRX1 + trx1->mainStandby + trx1->callLastState + trx1->trxStatus;
            }
            //else if ((trxid == trx2->radio1->url)||(trxid == trx2->radio2->url))
            else if (((trxid == trx2->radio1->url)||(trxid == trx2->radio2->url)) && ((trx2->radio1->call_id == call_id)||(trx2->radio2->call_id == call_id)) && ((radioNodeID == trx2->radio1->radioNodeID) || (radioNodeID == trx2->radio2->radioNodeID)))
            {
                Line2 = TRX2 + trx2->mainStandby + trx2->callLastState + trx2->trxStatus;
            }
        }
        else if (inviteMode == CLIENT)
        {
//            int i = 0;
//            callInNum = 0;
//            for (i=0; i<trx_incall.length();i++)
//            {
//                if (trx_incall.at(i)->callState) callInNum++;
//            }
            callInNum = getActiveCallCount();
            Line1 = QString("Node Connected:    %1").arg(callInNum);
            Line2 = QString("Tx-Rx Status  :   %1").arg(trxStatus);
        }
        else if (inviteMode == CLIENT_RPT)
        {
//            int i = 0;
//            callInNum = 0;
//            for (i=0; i<trx_incall.length();i++)
//            {
//                if (trx_incall.at(i)->callState) callInNum++;
//            }
            callInNum = getActiveCallCount();
            Line1 = QString("Node Connected:    %1").arg(callInNum);
            Line2 = QString("Tx-Rx Status  :   %1").arg(trxStatus);
        }
    }
    else if(currentmenuIndex == NODE1_RECONNECTING){
        if (currentNodeShow == 1)
            Line2 = trx1->radio1->callLastState;
        else
            Line2 = trx1->radio2->callLastState;
    }
    else if(currentmenuIndex == NODE2_RECONNECTING){
        if (currentNodeShow == 1)
            Line2 = trx2->radio1->callLastState;
        else
            Line2 = trx2->radio2->callLastState;
    }

//    qDebug() << "end of updateHomeDisplay" << trxid << connState << trxState;
*/
}
QString RoIP_ED137::getRadioFrequency(QString ipaddress, int snmpProfile, uint16_t port)
{
    unsigned long rxval64;
    QString address = ipaddress + QString(":%1").arg(port);
    if (snmpProfile == 1)
        rxval64 = SNMPStack->getsnmp_int_new(address.toStdString().c_str(),getFrequency,m_softPhoneID);
    else if (snmpProfile == 2)
        rxval64 = SNMPStack->getsnmp_int_new(address.toStdString().c_str(),getFrequency2,m_softPhoneID);
    else
        rxval64 = 0;
    QString rxfrequency = QString::number(rxval64);
    return rxfrequency;
}
int RoIP_ED137::getRFPower(QString ipaddress, int snmpProfile, uint16_t port)
{
    unsigned long rxval64;
    QString address = ipaddress + QString(":%1").arg(port);
    if (snmpProfile == 1)
        rxval64 = SNMPStack->getsnmp_int_new(address.toStdString().c_str(),getRFPowerdBm,m_softPhoneID);
    else if (snmpProfile == 2)
        rxval64 = SNMPStack->getsnmp_int_new(address.toStdString().c_str(),getRFPowerdBm2,m_softPhoneID);
    else
        return 0;
    if (rxval64 == 0) return 0;
    float rfpower = float(pow(10,((rxval64-30.0)/10.0)));
    return int(rfpower);
}
float RoIP_ED137::getRadioVSWR(QString ipaddress, int snmpProfile, uint16_t port)
{
    if (snmpProfile == 0) return 0.0;
    QString address = ipaddress + QString(":%1").arg(port);
    QString rxval64;
    if (snmpProfile == 1)
        rxval64 = SNMPStack->getsnmp_string_new(address.toStdString().c_str(),getVSWR,m_softPhoneID);
    else if (snmpProfile == 2)
        rxval64 = SNMPStack->getsnmp_string_new(address.toStdString().c_str(),getVSWR2,m_softPhoneID);
    else
        rxval64 = "0.0";
    float vswr = rxval64.trimmed().toFloat();
    if (vswr < 1) return 0.0;
    return vswr;
}
QString RoIP_ED137::getRadioSqlchThrhCarrier(QString ipaddress, int snmpProfile, uint16_t port)
{
    if (snmpProfile == 0) return "0";
    QString address = ipaddress + QString(":%1").arg(port);
    int rxval64;
    if (snmpProfile == 1)
        rxval64 = SNMPStack->getsnmp_int_new(address.toStdString().c_str(),SqlchThrhCarrier,m_softPhoneID);
    else if (snmpProfile == 2)
        rxval64 = SNMPStack->getsnmp_int_new(address.toStdString().c_str(),SqlchThrhCarrier2,m_softPhoneID);
    else
        rxval64 = 0;
    QString SqlchThrh = QString::number(rxval64);
    return SqlchThrh;
}

QString RoIP_ED137::getRadioStatus(QString ipaddress, int snmpProfile, uint16_t port)
{
    if (snmpProfile == 0) return "";
    QString val;
    QString address = ipaddress + QString(":%1").arg(port);
    if (snmpProfile == 1)
        val = SNMPStack->getsnmp_string_new(address.toStdString().c_str(),getRadiogrOpStat,m_softPhoneID);
    else if (snmpProfile == 2)
        val = SNMPStack->getsnmp_string_new(address.toStdString().c_str(),getRadiogrOpStat2,m_softPhoneID);
    else
        val = "";

    if (val.contains("2,1,0"))
        return "GO, OK";
    else if (val.contains("2,1,1"))
        return "GO, WARNING";
    else if (val.contains("2,0,2"))
        return "NOGO, ERROR";
    else if (val.contains("No Such Object"))
        return "";
    return "";
}

int RoIP_ED137::getSnmpProfile(QString ipaddress, uint16_t port)
{
    QString val ="";
    QString address = ipaddress + QString(":%1").arg(port);
    val = snmpStack->getsnmp_string_new(address.toStdString().c_str(),getRadioType,m_softPhoneID);
//    qDebug() << address << val;
    if (val != "") return 1;
//    if (!val.contains("No Such Object")) return 1;
    val = snmpStack->getsnmp_string_new(address.toStdString().c_str(),getRadioType2,m_softPhoneID);
//    qDebug() << address << val;
    if (val != "") return 2;
//    if (!val.contains("No Such Object")) return 2;
    return 0;
}
void RoIP_ED137::updateVUMeter(){
    int inputLevel = int(m_input_level*20.0);
    int outputLevel = int(m_output_level*20.0);
    Line1 = "    Output Level    ";
    Line2 = "";
//    for (int i=0;i<inputLevel;i++) {
//        Line1 += "|";
//    }
    for (int i=0;i<outputLevel;i++) {
        Line2 += "|";
    }

}
void RoIP_ED137::updateDisplay()
{
    displayCount++;

    if (inviteMode == SERVER)
    {
//        socketClient->broadcastMessageNodeState(1,trx1->radio1->callLastState,trx1->radio1->connDuration,trx1->trxStatus,trx1->radio1->radiostatus,QString::number(trx1->radio1->vswr),trx1->radio2->connDuration,trx1->radio2->callLastState,trx1->radio2->radiostatus, m_softPhoneID);
//        socketClient->broadcastMessageNodeState(2,trx2->radio1->callLastState,trx2->radio1->connDuration,trx2->trxStatus,trx2->radio1->radiostatus,QString::number(trx2->radio1->vswr),trx2->radio2->connDuration,trx2->radio2->callLastState,trx2->radio2->radiostatus, m_softPhoneID);
//        if ((trx1->mainStandby == MAIN)&(trx2->mainStandby != MAIN))
//            socketClient->broadcastSystemMessage("Node 1", m_softPhoneID);
//        else if ((trx1->mainStandby != MAIN)&(trx2->mainStandby == MAIN))
//            socketClient->broadcastSystemMessage("Node 2", m_softPhoneID);
//        else if ((trx1->mainStandby == MAIN)&(trx2->mainStandby == MAIN))
//            socketClient->broadcastSystemMessage("Node 1 and Node 2", m_softPhoneID);
//        else
//            socketClient->broadcastSystemMessage("None", m_softPhoneID);
        sendChannelMessage();
    }
    else if ((displayCount%5 == 0)&(inviteMode == CLIENT))
    {
        getActiveCallCount();
        QString message = QString("{\"menuID\":\"connStatus\", \"connNum\":%1, \"TxRx\":\"%2\", \"pttURI\":\"%3\", \"softPhoneID\":%4}")
                .arg(callInNum).arg(trxStatus).arg(lastPttOn).arg(m_softPhoneID);
        if ((connStatusMessage != message))
        {
            socketClient->sendTextMessage(message);
            connStatusMessage = message;
//            qDebug() << "message" << message;
        }

        for (int i = 0; i<trx_incall.length(); i++)
        {
            message = QString("{\"menuID\":\"ConnDuration\", \"uri\":\"%1\", \"duration\":\"%2\", \"softPhoneID\":%3, \"pttOn\":%4, \"sqlOn\":%5, \"callState\":%6}")
                    .arg(trx_incall.at(i)->url).arg(trx_incall.at(i)->connDuration).arg(m_softPhoneID).arg(trx_incall.at(i)->pttLevel).arg(trx_incall.at(i)->sqlTestPressed).arg(trx_incall.at(i)->callState);
            if (trx_incall.at(i)->ConnDurationMessage != message)
            {
                socketClient->sendTextMessage(message);
                trx_incall.at(i)->ConnDurationMessage = message;
            }
        }

    }
}

void RoIP_ED137::getAudioInputLevelFault()
{
    myDatabase->getLastEventCheckAudio(warnAudioLevelTime,warnPercentFault,warnPTTMinute);
    checkInputTimer->stop();
    checkInputTimer->start(1000*10);
}
void RoIP_ED137::display_show(){
    updateDisplay();
    checkMainStandby();
    if (Line1 != Line1Buf){
        Line1Buf = Line1;
        // display->printLine(Line1,LCD_LINE_1,ALIGNLEFT);
    }
    if (Line2 != Line2Buf){
        Line2Buf = Line2;
        // display->printLine(Line2,LCD_LINE_2,ALIGNLEFT);
    }
    if (Line3 != Line3Buf){
        Line3Buf = Line3;
        // display->printLine(Line3,LCD_LINE_3,ALIGNLEFT);
    }
    if (Line4 != Line4Buf){
        Line4Buf = Line4;
        // display->printLine(Line4,LCD_LINE_4,ALIGNLEFT);
    }

}
int RoIP_ED137::findNextNodeConnected(int currentnode){
    int nextNode = -1;
    if (currentnode < trx_incall.length()-1){
        for(int i=currentnode+1; i<trx_incall.length();i++){
            if(trx_incall.at(i)->callState){
                nextNode = i;
                break;
            }
        }
    }

    qDebug() << "findNextNodeConnected" << "currentnode" << currentnode << "nextNode" << nextNode;
    if (nextNode >= 0)
        return nextNode;
    return currentnode;
}
int RoIP_ED137::findPreviousNodeConnected(int currentnode){
    int nextNode = -1;
    if (currentnode < trx_incall.length()-1){
        for(int i=currentnode-1; i>=0;i--){
            if(trx_incall.at(i)->callState){
                nextNode = i;
                break;
            }
        }
    }
    qDebug() << "findPreviousNodeConnected" << "currentnode" << currentnode << "nextNode" << nextNode;
    if (nextNode >= 0)
        return nextNode;
    return currentnode;
}
void RoIP_ED137::setNode1Type()
{
    currentmenuIndex = NODE1_NODETYPE;
    Line1 = "Node 1: " + trx1->channelName;
    if (trx1->trxmode == TRXMODE_SEPARATE){
        if (currentNodeShow == 1){
            Line2 = "Node Tx: " + trx1->radio1->callName;
        }
        else{
            Line2 = "Node Tx: " + trx1->radio2->callName;
        }
    }
    else if(trx1->trxmode == TRXMODE_RX){
        Line2 = "Node Rx: " + trx1->radio1->callName;
    }
    else if(trx1->trxmode == TRXMODE_TX){
        Line2 = "Node Tx: " + trx1->radio1->callName;
    }
    else if(trx1->trxmode == TRXMODE_TRX){
        Line2 = "Node TxRx: " + trx1->radio1->callName;
    }
}

void RoIP_ED137::setNode1Line1(){
    if (trx1->trxmode == TRXMODE_SEPARATE){
        if (currentNodeShow == 1){
            Line1 = "Node1 Tx: " + trx1->radio1->callName;
        }
        else{
            Line1 = "Node1 Rx: " + trx1->radio2->callName;
        }
    }
    else if(trx1->trxmode == TRXMODE_RX){
        Line2 = "Node1 Rx: " + trx1->radio1->callName;
    }
    else if(trx1->trxmode == TRXMODE_TX){
        Line2 = "Node1 Tx: " + trx1->radio1->callName;
    }
    else if(trx1->trxmode == TRXMODE_TRX){
        Line2 = "Node1 TxRx: " + trx1->radio1->callName;
    }
}
void RoIP_ED137::setNode2Type()
{
    currentmenuIndex = NODE1_NODETYPE;
    Line1 = "Node 2: " + trx2->channelName;
    if (trx2->trxmode == TRXMODE_SEPARATE){
        if (currentNodeShow == 1){
            Line2 = "Node Tx: " + trx2->radio1->callName;
        }
        else{
            Line2 = "Node Tx: " + trx2->radio2->callName;
        }
    }
    else if(trx2->trxmode == TRXMODE_RX){
        Line2 = "Node Rx: " + trx2->radio1->callName;
    }
    else if(trx2->trxmode == TRXMODE_TX){
        Line2 = "Node Tx: " + trx2->radio1->callName;
    }
    else if(trx2->trxmode == TRXMODE_TRX){
        Line2 = "Node TxRx: " + trx2->radio1->callName;
    }
}

void RoIP_ED137::setNode2Line1(){
    if (trx2->trxmode == TRXMODE_SEPARATE){
        if (currentNodeShow == 1){
            Line1 = "Node2 Tx: " + trx2->radio1->callName;
        }
        else{
            Line1 = "Node2 Rx: " + trx2->radio2->callName;
        }
    }
    else if(trx2->trxmode == TRXMODE_RX){
        Line2 = "Node2 Rx: " + trx2->radio1->callName;
    }
    else if(trx2->trxmode == TRXMODE_TX){
        Line2 = "Node2 Tx: " + trx2->radio1->callName;
    }
    else if(trx2->trxmode == TRXMODE_TRX){
        Line2 = "Node2 TxRx: " + trx2->radio1->callName;
    }
}

RoIP_ED137::~RoIP_ED137()
{
    /*fftw_destroy_plan(planFft);
    fftw_free(in);
    fftw_free(out);*/

    delete [] d_realFftData;
    delete [] d_iirFftData;
}


QString RoIP_ED137::getUptime(){
    string getval_str = "/proc/uptime";
    QString val;
    int total_seconds;
    float sec;
    QString uptimeVal;
    string value;
    ifstream getuptime(getval_str.c_str());// open value file for gpio
    if (getuptime.fail()){
        cout << " OPERATION FAILED: Unable to get value of uptime" << " ."<< endl;
        return "";
    }
    getuptime >> value;
    val = QString::fromStdString(value);
    getuptime.close();
    sec = val.toFloat();
    total_seconds = int(sec);
    int MINUTE  = 60;
    int HOUR    = MINUTE * 60;
    int DAY     = HOUR * 24;

    int days    = int( total_seconds / DAY );
    int hours   = int( ( total_seconds % DAY ) / HOUR );
    int minutes = int( ( total_seconds % HOUR ) / MINUTE );
    int seconds = int( total_seconds % MINUTE );

    uptimeVal = "";
    if (days > 0)
        uptimeVal += QString::number(days) + "d";
    if ((uptimeVal != "") || (hours > 0))
        uptimeVal += QString::number(hours).rightJustified(2, '0')  + ":";
    if ((uptimeVal != "" )||(minutes > 0))
        uptimeVal += QString::number(minutes).rightJustified(2, '0')  + ":";
    uptimeVal += QString::number(seconds).rightJustified(2, '0');

    return uptimeVal;
}

RoIP_ED137 *RoIP_ED137::instance()
{
    return theInstance_;
}
void RoIP_ED137::detectR2SPacketAndReconn()
{
    qint64 secDiff;
    read_r2s_count++;
//    if (read_r2s_count >= 2)
    {
        read_r2s_count = 0;
        qint64 currentTimeR2S = QDateTime::currentMSecsSinceEpoch();
        if (inviteMode == SERVER){
            if ((trx1->radio1->callState)&(trx1->radio1->call_id!=PJSUA_INVALID_ID))
            {
                trx1->radio1->lastR2SPacket = get_R2SStatus(trx1->radio1->call_id);
                secDiff = currentTimeR2S - trx1->radio1->lastR2SPacket ;
                if (secDiff > (trx1->radio1->r2sPeriod*3))
                {
                    if (trx1->radio1->r2sCount == 5){
                        qDebug() << "trx1 radio1 secDiff" << secDiff;
                        pj_str_t reason = pj_str((char*)"WG-67 ;cause=2001; text=\"missing R2S KeepAlive\"");
                        trx_call_hangup(trx1->radio1->call_id,500,&reason);
                    }
                    trx1->radio1->r2sCount++;
                }
                else{
                    trx1->radio1->r2sCount = 0;                    
                }
                trx1->radio1->autoConnectCount = 0;

                if ((trx1->mainTxRx == false) & (trx1->alwaysConnect == false))
                {
                    if ((trx2->mainRx) & (trx2->mainTx))
                        trx_call_hangup(trx1->radio1->call_id,500,NULL);
                    else if ((trx2->trxmode == TRXMODE_RX) & (trx2->mainRx)) {
                        trx_call_hangup(trx1->radio1->call_id,500,NULL);
                    }
                    else if ((trx2->trxmode == TRXMODE_TX) & (trx2->mainTx)) {
                        trx_call_hangup(trx1->radio1->call_id,500,NULL);
                    }
                    else if ((trx2->trxmode == TRXMODE_SEPARATE) & (trx2->mainTx) & (trx1->radio1->trxmode == TRXMODE_TX)) {
                        trx_call_hangup(trx1->radio1->call_id,500,NULL);
                    }
                }
            }
            else
            {
                trx1->radio1->autoConnectCount++;
                if (trx1->radio1->autoConnectCount > 30)
                {
                    if((trx1->alwaysConnect)||(trx1->mainTxRx)||(trx2->mainTxRx))
                    {
                        if (trx2->mainTxRx)
                        {
                            if (((trx2->trxmode != TRXMODE_SEPARATE) & (trx2->radio1->callState == false))
                                    || ((trx2->trxmode == TRXMODE_SEPARATE) & (trx2->radio1->callState == false) & (trx1->trxmode != TRXMODE_RX)))
                            {
                                trx1->pingEnable=true;
                                updateUri();
                                trxCallStartStop(trx1->radio1->call_id,trx1->radio1->url);
                                trx1->radio1->autoConnectCount = 0;
                            }
                            else if ((trx1->alwaysConnect)||(trx1->mainTxRx)) {
                                trx1->pingEnable=true;
                                updateUri();
                                trxCallStartStop(trx1->radio1->call_id,trx1->radio1->url);
                                trx1->radio1->autoConnectCount = 0;
                            }
                        }
                        else
                        {
                            trx1->pingEnable=true;
                            updateUri();
                            trxCallStartStop(trx1->radio1->call_id,trx1->radio1->url);
                            trx1->radio1->autoConnectCount = 0;
                        }
                    }
                    else
                    {
                        trx1->pingEnable=false;
                    }
                }
            }

            if ((trx1->radio2->callState)&(trx1->radio2->call_id!=PJSUA_INVALID_ID))
            {
                trx1->radio2->lastR2SPacket = get_R2SStatus(trx1->radio2->call_id);
                secDiff = currentTimeR2S - trx1->radio2->lastR2SPacket ;
                if (secDiff > (trx1->radio2->r2sPeriod*3))
                {
                    if (trx1->radio2->r2sCount == 5){
                        qDebug() << "trx1 radio2 secDiff" << secDiff;
                        pj_str_t reason = pj_str((char*)"WG-67 ;cause=2001; text=\"missing R2S KeepAlive\"");
                        trx_call_hangup(trx1->radio2->call_id,500,&reason);
                    }
                    trx1->radio2->r2sCount++;
                }
                else{
                    trx1->radio2->r2sCount = 0;
                }
                trx1->radio2->autoConnectCount = 0;

                if ((trx1->mainTxRx == false) & (trx1->alwaysConnect == false))
                {
                    if ((trx2->mainRx) & (trx2->mainTx))
                        trx_call_hangup(trx1->radio2->call_id,500,NULL);
                    else if ((trx2->trxmode == TRXMODE_RX) & (trx2->mainRx)) {
                        trx_call_hangup(trx1->radio2->call_id,500,NULL);
                    }
                    else if ((trx2->trxmode == TRXMODE_TX) & (trx2->mainTx)) {
                        trx_call_hangup(trx1->radio2->call_id,500,NULL);
                    }
                    else if ((trx2->trxmode == TRXMODE_SEPARATE) & (trx2->mainRx)) {
                        trx_call_hangup(trx1->radio2->call_id,500,NULL);
                    }
                }
            }
            else
            {
                trx1->radio2->autoConnectCount++;
                if (trx1->radio2->autoConnectCount > 30){
                    if((trx1->alwaysConnect)||(trx1->mainTxRx)||((trx2->mainTxRx)&(trx2->trxmode != TRXMODE_TX))&(trx2->mainRx == false))
                    {
                        trx1->pingEnable=true;
                        updateUri();
                        trxCallStartStop(trx1->radio2->call_id,trx1->radio2->url);
                        trx1->radio2->autoConnectCount = 0;
                    }
                    else
                    {
                        trx1->pingEnable=false;
                    }
                }
            }

            if ((trx2->radio1->callState)&(trx2->radio1->call_id!=PJSUA_INVALID_ID))
            {
                trx2->radio1->lastR2SPacket = get_R2SStatus(trx2->radio1->call_id);
                secDiff = currentTimeR2S - trx2->radio1->lastR2SPacket ;
                if (secDiff > (trx2->radio1->r2sPeriod*3))
                {
                    if (trx2->radio1->r2sCount == 5){
                        qDebug() << "trx2 secDiff" << secDiff;
                        pj_str_t reason = pj_str((char*)"WG-67 ;cause=2001; text=\"missing R2S KeepAlive\"");
                        trx_call_hangup(trx2->radio1->call_id,500,&reason);
                    }
                    trx2->radio1->r2sCount++;
                }
                else{
                    trx2->radio1->r2sCount = 0;
                }
                trx2->radio1->autoConnectCount = 0;

                if ((trx2->mainTxRx == false) & (trx2->alwaysConnect == false))
                {
                    if ((trx1->mainRx) & (trx1->mainTx))
                        trx_call_hangup(trx2->radio1->call_id,500,NULL);
                    else if ((trx1->trxmode == TRXMODE_RX) & (trx1->mainRx)) {
                        trx_call_hangup(trx2->radio1->call_id,500,NULL);
                    }
                    else if ((trx1->trxmode == TRXMODE_TX) & (trx1->mainTx)) {
                        trx_call_hangup(trx2->radio1->call_id,500,NULL);
                    }
                    else if ((trx1->trxmode == TRXMODE_SEPARATE) & (trx1->mainTx) & (trx2->radio1->trxmode == TRXMODE_TX)) {
                        trx_call_hangup(trx2->radio1->call_id,500,NULL);
                    }
                }
            }
            else
            {
                trx2->radio1->autoConnectCount++;
                if (trx2->radio1->autoConnectCount > 30){
                    if((trx2->alwaysConnect)||(trx2->mainTxRx)||(trx1->mainTxRx))
                    {
                        if (trx1->mainTxRx)
                        {
                            if (((trx1->trxmode != TRXMODE_SEPARATE) & (trx1->radio1->callState == false))
                                    || ((trx1->trxmode == TRXMODE_SEPARATE) & (trx1->radio1->callState == false) & (trx2->trxmode != TRXMODE_RX)))
                            {
                                trx2->pingEnable=true;
                                updateUri();
                                trxCallStartStop(trx2->radio1->call_id,trx2->radio1->url);
                                trx2->radio1->autoConnectCount = 0;
                            }
                            else if ((trx2->alwaysConnect)||(trx2->mainTxRx)) {
                                trx2->pingEnable=true;
                                updateUri();
                                trxCallStartStop(trx2->radio1->call_id,trx2->radio1->url);
                                trx2->radio1->autoConnectCount = 0;
                            }
                        }
                        else
                        {
                            trx2->pingEnable=true;
                            updateUri();
                            trxCallStartStop(trx2->radio1->call_id,trx2->radio1->url);
                            trx2->radio1->autoConnectCount = 0;
                        }
                    }
                }
            }



            if ((trx2->radio2->callState)&(trx2->radio2->call_id!=PJSUA_INVALID_ID))
            {
                trx2->radio2->lastR2SPacket = get_R2SStatus(trx2->radio2->call_id);
                secDiff = currentTimeR2S - trx2->radio2->lastR2SPacket ;
                if (secDiff > (trx2->radio2->r2sPeriod*3))
                {
                    if (trx2->radio2->r2sCount == 5){
                        qDebug() << "trx2 secDiff" << secDiff;
                        pj_str_t reason = pj_str((char*)"WG-67 ;cause=2001; text=\"missing R2S KeepAlive\"");
                        trx_call_hangup(trx2->radio2->call_id,500,&reason);
                    }
                    trx2->radio2->r2sCount++;
                }
                else
                {
                    trx2->radio2->r2sCount = 0;
                }
                trx2->radio2->autoConnectCount = 0;

                if ((trx2->mainTxRx == false) & (trx2->alwaysConnect == false))
                {
                    if ((trx1->mainRx) & (trx1->mainTx))
                        trx_call_hangup(trx2->radio2->call_id,500,NULL);
                    else if ((trx1->trxmode == TRXMODE_RX) & (trx1->mainRx)) {
                        trx_call_hangup(trx2->radio2->call_id,500,NULL);
                    }
                    else if ((trx1->trxmode == TRXMODE_TX) & (trx1->mainTx)) {
                        trx_call_hangup(trx2->radio2->call_id,500,NULL);
                    }
                    else if ((trx1->trxmode == TRXMODE_SEPARATE) & (trx1->mainRx)) {
                        trx_call_hangup(trx2->radio2->call_id,500,NULL);
                    }
                }
            }
            else
            {
                trx2->radio2->autoConnectCount++;
                if (trx2->radio2->autoConnectCount > 30){
                    if((trx2->alwaysConnect)||(trx2->mainTxRx)||((trx1->mainTxRx)&(trx1->trxmode != TRXMODE_TX))&(trx1->mainRx == false))
                    {
                        trx2->pingEnable=true;
                        updateUri();
                        trxCallStartStop(trx2->radio2->call_id,trx2->radio2->url);
                        trx2->radio2->autoConnectCount = 0;
                    }
                    else
                    {
                        trx2->pingEnable=false;
                    }
                }
            }
        }
        if ((inviteMode != SERVER) || (recorderConnected))
        {
            for(int i = 0; i < trx_incall.length(); i++)
            {
                if (trx_incall.at(i)->callState)
                {
                    trx_incall.at(i)->lastR2SPacket = get_R2SStatus(trx_incall.at(i)->call_id);
                    secDiff = currentTimeR2S - trx_incall.at(i)->lastR2SPacket;
                    if (secDiff > (trx_incall.at(i)->r2sPeriod*3))
                    {
                        if(trx_incall.at(i)->r2sCount == 5){
                            pj_str_t reason = pj_str((char*)"WG-67 ;cause=2001; text=\"missing R2S KeepAlive\"");
                            trx_call_hangup(trx_incall.at(i)->call_id, 500,&reason);
                            qDebug() << "trx_call_hangup" << "trx_incall.at" << i << "secDiff" << secDiff;
                        }
                        trx_incall.at(i)->r2sCount++;
                    }
                    else
                    {
                        trx_incall.at(i)->r2sCount = 0;
//                        trx_incall.at(i)->TestR2SCount++;
//                        if (trx_incall.at(i)->TestR2SCount == 100)
//                        {
//                            pj_str_t reason = pj_str((char*)"WG-67 ;cause=2001; text=\"missing R2S KeepAlive\"");
//                            trx_call_hangup(trx_incall.at(i)->call_id,500,&reason);
//                            exit(1);
//                        }
                    }
                    trx_incall.at(i)->autoConnectCount = 0;                    
//                    sendRtpType232(trx_incall.at(i)->call_id);
                }
            }
        }
    }

    checkEvents();
}

void RoIP_ED137::onPttStatusChange(bool pttstate){
    if (inviteMode == SERVER)
    {
        if (pttstate){
            pptTest_pressed();
        }
        else
        {
            pptTest_released();
        }
    }
    else if (inviteMode == CLIENT)
    {

    }
    else if (inviteMode == CLIENT_RPT)
    {

    }
}


void RoIP_ED137::onSqlOnChange(bool qslStatus)
{
    sqlOnStatusbackup = qslStatus;
    sqlOnStatus = qslStatus;

    if (inviteMode == CLIENT)
    {

        sqlPinLevel = qslStatus;
        if (m_TxRx1stPriority == TX1ST)
        {
            if (trxStatus == PTTON)
            {
                updateHomeDisplay("","",PTTON,0,0);
                sqlTest_released();
                sqlOnStatus = false;
                return;
            }
            else
            {
                sqlTest_released();
                recorder_released(0);
                updateHomeDisplay("","",STANDBY,0,0);
            }
        }
        else if ((m_TxRx1stPriority == RX1ST) || (m_TxRx1stPriority == FULLDUPLEX))
        {
            if (sqlOnStatus)
            {
                if (pttOnStatus)
                {
                    sqlTest_pressed(7);
                }
                else
                {
                    sqlTest_pressed(1);
                }
                recorder_pressed(1,RXON);
                updateHomeDisplay("","",RXON,0,0);
            }
            else
            {
                sqlTest_released();
                recorder_released(0);
                updateHomeDisplay("","",STANDBY,0,0);
            }
        }
    }
    else if (inviteMode == CLIENT_RPT)
    {
        sqlPinLevel = qslStatus;
        if (trxStatus == PTTON)
        {
            updateHomeDisplay("","",PTTON,0,0);
            recorder_pressed(1,RXON);
            sqlTest_released();
            sqlOnStatus = false;
            return;
        }
        if (qslStatus)
        {
            if (trxStatus == RPTON)
            {

            }
            else
            {
//                sqlTest_pressed();
                updateHomeDisplay("","",RXON,0,0);
            }
        }
        else
        {
            sqlTest_released();
            recorder_released(0);
            updateHomeDisplay("","",STANDBY,0,0);
        }
    }
    else if(inviteMode == SERVER)
    {
        if (qslStatus)
        {
            updateHomeDisplay("","",RXON,0,0);
            pptTest_pressed();
            recorder_pressed(1,RXON);
        }
        else
        {
            pptTest_released();
            recorder_released(0);
        }
    }
}

void RoIP_ED137::scanKeyPin()
{
    qDebug() << "start scanKeyPin thread";
    while (m_ScanKeyThreadRunning){
        QThread::msleep(20);
        boost::unique_lock<boost::mutex> scoped_lock(scankey_mutex);
        int ptt_or_sql_Value = 0;// = pttInputPin->getGpioVal() || externalGpio->getGpioValue();
        if (inviteMode == SERVER)
        {
            sqlActiveHigh = false;
        }
        if ((sqlActiveHigh == false) & (connToRadio))
        {
            ptt_or_sql_Value = (pttSQLInputPin->getGpioVal() == false);
        }
        else if (sqlActiveHigh)// & connToRadio)
        {
            ptt_or_sql_Value = (pttSQLInputPin->getGpioVal() == true);
        }
        else
        {
            ptt_or_sql_Value = (pttSQLInputPin->getGpioVal() == false);
        }

        ptt_or_sql_Value = ptt_or_sql_Value || pttTestMode;
        pttInput = ptt_or_sql_Value; //var pttInput for get input level
//        qDebug() << "ptt_or_sql_Value" << pttInput;
        if (inviteMode == SERVER)
        {
            if ((pttInput)&(pttDelayForTxOn))
            {
                pttInput_count++;
                if (pttInput_count > 1000) pttInput_count = 1000;
                pttDelay_count = 0;
                if (forceMuteSqlOn){
                    pttInput_count = 1;
                    forceMuteSqlOn = false;
                    emit onUpdateDisplay();
                }
            }
            else if ((pttInput)&(pttDelayForTxOn == false)) // BlockTx
            {
                pttInput_count++;
                if (pttInput_count > 1000) pttInput_count = 1000;
                if (forceMuteSqlOn)
                {
                    pttInput = false;
                }
                else
                {
                    pttDelay_count = 0;
                }
            }
//            else
//                pttInput_count=0;

            if ((trx1->mainTx == false) & (trx2->mainTx == false) & (pttDelayForTxOn)){
                pttInput_count = 0;
                ptt_or_sql_Value = false;
            }
        }


//        qDebug() << "pttStatus" << pttStatus << pttSQLInputPin0->getGpioVal() << pttSQLInputPin1->getGpioVal() << pttSQLInputPin2->getGpioVal();
        if (ptt_or_sql_Value >= 0)
        {
            if (!connToRadio)
            {
                if ((pttDelay_count > 0) & (pttDelay_set > 0) & pttStatus & forceMuteSqlOn)
                {
                    pttDelay_count++;
                }
                if ((ptt_or_sql_Value != pttStatus)||(ptt_or_sql_Value != pttStatusTmp)||((pttDelay_count >= pttDelay_set) & forceMuteSqlOn))
                {
                    if (ptt_or_sql_Value == false)
                    {
                        if (pttDelay_set > 0)
                        {
                            pttDelay_count++;
                        }
                        else
                            pttDelay_count=0;

                        if (((pttDelay_count >= pttDelay_set) || (pttDelay_count == 0)) & (pttDelayForTxOn == true) & (pttDelayForTxOn == false))
                        {
                            pttStatus = ptt_or_sql_Value;
                            pttStatusTmp  = ptt_or_sql_Value;
                            if(forceMuteSqlOn & (pttDelayForTxOn == true)){
                                pttInput_count = 0;
                                forceMuteSqlOn = false;
                                emit onUpdateDisplay();
                            }
                        }
                        else if (((pttDelay_count >= pttDelay_set) || (pttDelay_count == 0)) & (pttDelayForTxOn == false) & (pttGroupActive == false))
                        {
                            pttStatus = ptt_or_sql_Value;
                            pttStatusTmp  = ptt_or_sql_Value;
                            if(forceMuteSqlOn){
                                pttInput_count = 0;
                                forceMuteSqlOn = false;
                                emit onUpdateDisplay();
                            }
                        }
                        else
                        {
                            forceMuteSqlOn = true;
                            qDebug() << "pttStatusChange" << pttStatus << "pttDelay_count" << pttDelay_count << "forceMuteSqlOn" << forceMuteSqlOn;
                        }
                    }
                    else {
                        pttStatus = ptt_or_sql_Value;
                        pttStatusTmp  = ptt_or_sql_Value;
                        if (((pttDelay_count >= pttDelay_set) || (pttDelay_count == 0)) & (pttDelayForTxOn == false))
                        {
                            if(forceMuteSqlOn){
                                pttInput_count = 1;
                                forceMuteSqlOn = false;
                                emit onUpdateDisplay();
                            }
                        }
                        qDebug() << "pttStatusChange" << pttStatus << "pttDelay_count" << pttDelay_count << "forceMuteSqlOn" << forceMuteSqlOn;

                    }
                    if (inviteMode == SERVER)
                    {
                        if((pttStatus) & (localSidetone > 0))
                        {
//                            localSidetoneLoopbackOn = true;
//                            pttSQLOutputPin->setGpio();
                            qDebug() << "pttStatusChange" << pttStatus << "localSidetoneLoopbackOn" << localSidetoneLoopbackOn;
                        }
                        else
                        {
//                            if (localSidetoneLoopbackOn)
//                            {
//                                localSidetoneLoopbackOn = false;
//                                if  (sqlOnStatus == false)
//                                    pttSQLOutputPin->resetGpio();
//                            }
                        }
                    }
//                    if ((ptt_or_sql_Value != pttStatus)||(ptt_or_sql_Value != pttStatusTmp) || (pttDelay_count >= pttDelay_set))
                    {
                        emit onUpdateDisplay();
                        if(forceMuteSqlOn & (pttDelayForTxOn == false) & pttStatus)
                            emit pttStatusChange(false);
                        else
                            emit pttStatusChange(pttStatus);
                    }
                }

                if ((ptt_or_sql_Value == false) & ((forceMuteSqlOn == true)||(pttInput_count != 0)))
                {
                    if ((m_PttPressed1 == false) & (m_PttPressed2 == false) & pttDelayForTxOn)
                        pttInput_count = 0;
                    if ((forceMuteSqlOn == true) & (m_PttPressed1 == false) & (m_PttPressed2 == false) & (pttGroupActive == false))
                    {
                        if(pttDelayForTxOn == true){
                            forceMuteSqlOn = false;
                            emit onUpdateDisplay();
                        }
                    }
                }
            }
//            else if (connToRadio)
//            {
//                if (sqlAlwayOn)
//                {
//                    ptt_or_sql_Value = true;
//                }
//                if (ptt_or_sql_Value != sqlOldPinOnStatus){
//                    sqlOldPinOnStatus = ptt_or_sql_Value;
//                    qDebug() << "sqlStatusChange" << sqlOldPinOnStatus;
//                    emit sqlStatusChange(sqlOldPinOnStatus);
//                }
//            }
            else if (connToRadio)
            {
                if (m_TxRx1stPriority == TX1ST)
                {
                    if ((sqlAlwayOn||trxStatus == RPTON) & (trxStatus != PTTON))
                    {
                        ptt_or_sql_Value = true;
                    }

                    else if ((sqlAlwayOn) & (trxStatus == PTTON))
                    {
                        ptt_or_sql_Value = false;
                    }
                    else if ((trxStatus == PTTON) & (ptt_or_sql_Value == true))
                    {
                        ptt_or_sql_Value = false;
                    }
                }
                else if ((m_TxRx1stPriority == RX1ST) || (m_TxRx1stPriority == FULLDUPLEX))
                {
                    if (sqlAlwayOn||trxStatus == RPTON)
                    {
                        ptt_or_sql_Value = true;
                    }
                }



                if (ptt_or_sql_Value != sqlOldPinOnStatus)
                {
                    if (ptt_or_sql_Value == false)
                    {
                        sqlInputDelay++;
                        if (sqlInputDelay < 10)
                            ptt_or_sql_Value = true;
                    }
                    if (ptt_or_sql_Value != sqlOldPinOnStatus){
                        sqlOldPinOnStatus = ptt_or_sql_Value;
                        qDebug() << "sqlStatusChange" << sqlOldPinOnStatus;
                        emit sqlStatusChange(sqlOldPinOnStatus);
                    }
                }
                if (sqlAlwayOn & (sqlOnStatus == false))
                {
                    sqlOldPinOnStatus = ptt_or_sql_Value;
                    qDebug() << "sqlStatusChange" << sqlOldPinOnStatus;
                    emit sqlStatusChange(sqlOldPinOnStatus);
                }
            }            
//            if (inviteMode == SERVER)
//            {
//                if ((ptt_or_sql_Value != pttStatus)||(ptt_or_sql_Value != pttStatusTmp)){
//                    pttStatus = ptt_or_sql_Value;
//                    pttStatusTmp  = ptt_or_sql_Value;
//                    emit pttStatusChange(pttStatus);
//                }
//            }
//            else if (inviteMode == CLIENT)
//            {
//                if (ptt_or_sql_Value != sqlOnStatus){
//                    sqlOnStatus = ptt_or_sql_Value;
//                    emit sqlStatusChange(sqlOnStatus);
//                }
//            }
        }
    }
}

void RoIP_ED137::setupRadio(){

}

void RoIP_ED137::trxCallStartStop(pjsua_call_id call_id, QString trxID){
//    qDebug() << "trxCallStartStop" << trxID;
    pjsua_msg_data msg_data;
    if(call_id != PJSUA_INVALID_ID){
        pjsua_msg_data_init(&msg_data);

        // Heade 1
        pj_str_t hname = pj_str((char*)"Priority");
        pj_str_t hvalue = pj_str((char*)"normal");
        pjsip_generic_string_hdr my_hdr3;
        pjsip_generic_string_hdr_init2(&my_hdr3, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr3);

        // Header 2
        hname = pj_str((char*)"Subject");
        hvalue = pj_str((char*)"radio");
        pjsip_generic_string_hdr my_hdr1;
        pjsip_generic_string_hdr_init2(&my_hdr1, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr1);

        // Header 3
        hname = pj_str((char*)"WG67-Version");
        hvalue = pj_str((char*)"radio.01;radio.02");
        pjsip_generic_string_hdr my_hdr2;
        pjsip_generic_string_hdr_init2(&my_hdr2, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr2);


        // Header 4
        hname = pj_str((char*)"user-agent");
        hvalue = pj_str((char*)"PAS Radio");
        pjsip_generic_string_hdr my_hdr4;
        pjsip_generic_string_hdr_init2(&my_hdr4, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr4);
    }
    if (inviteMode == SERVER)
    {
        QString calluri = trxID;
        char sip_urichr[calluri.length()+1];
        strcpy( sip_urichr, calluri.toStdString().c_str());

        if ((call_id == trx1->radio1->call_id)&(trxID == trx1->radio1->url)&(trx1->radio1->pingOk))
        {
            if ((!trx1->radio1->callState)&&(trx1->radio1->enable)&&(!trx1->radio1->trxFailed)&(trx1->radio1->call_id == PJSUA_INVALID_ID))
            {
                trx1->radio1->ByeReason = "";
                makeCallIndex = trx1->radio1->trxmode;
                trx1->radio1->call_id = makeCall_Radio(sip_urichr);
                if(trx1->radio1->call_id == PJSUA_INVALID_ID)
                {
                    trx1->radio1->callState = false;
                    trx1->radio1->connDuration = "0";
                    trx1->radio1->m_PttPressed = false;
                    if (trx1->radio1->pptTestPressed){
                        pttStatus = false;
                    }
                    stopRing();
                    trx1->radio1->callLastState = DISCONNECTED;                    
                    updateHomeDisplay(trx1->radio1->url,trx1->callLastState,"",trx1->radio1->call_id,trx1->radio1->radioNodeID);
                }
            }
            else if (trx1->radio1->call_id != PJSUA_INVALID_ID)
            {
                stopRing();
                trx1->radio1->callLastState = DISCONNECTED;
                pj_status_t status = pjsua_call_hangup(trx1->radio1->call_id, PJSIP_SC_BUSY_HERE, NULL ,  &msg_data);
                if (status == PJ_SUCCESS)
                {

                }
                else
                {
                    qDebug() << "PJ_FAILED" << status;
                    //pjHangupAll();
                    trx1->radio1->call_id = PJSUA_INVALID_ID;
                }
             }
            else if ((trx1->radio1->call_id == PJSUA_INVALID_ID) & (trx1->radio1->callState))
            {
                trx1->radio1->callState = false;
                trx1->radio1->callLastState = DISCONNECTED;
            }
        }
        else if ((call_id == trx1->radio2->call_id)&(trxID == trx1->radio2->url)&(trx1->radio2->pingOk))
        {
            if ((!trx1->radio2->callState)&&(trx1->radio2->enable)&&(!trx1->radio2->trxFailed)&(trx1->radio2->call_id == PJSUA_INVALID_ID))
            {
                trx1->radio2->ByeReason = "";
                makeCallIndex = trx1->radio2->trxmode;
                trx1->radio2->call_id = makeCall_Radio(sip_urichr);
                if(trx1->radio2->call_id == PJSUA_INVALID_ID)
                {
                    trx1->radio2->callState = false;
                    trx1->radio2->connDuration = "0";
                    trx1->radio2->m_PttPressed = false;
                    if (trx1->radio2->pptTestPressed){
                        pttStatus = false;
                    }
                    stopRing();
                    trx1->callLastState = DISCONNECTED;
                    updateHomeDisplay(trx1->radio2->url,trx1->callLastState,"",trx1->radio2->call_id,trx1->radio2->radioNodeID);
                }
            }

            else if(trx1->radio2->call_id != PJSUA_INVALID_ID)
            {
                stopRing();
                trx1->radio2->callLastState = DISCONNECTED;
                pj_status_t status = pjsua_call_hangup(trx1->radio2->call_id, PJSIP_SC_BUSY_HERE, NULL ,  &msg_data);
                if (status == PJ_SUCCESS) {

                }
                else
                {
                    qDebug() << "PJ_FAILED" << status;
//                    pjHangupAll();
                    trx1->radio2->call_id = PJSUA_INVALID_ID;
                }
            }
            else if ((trx1->radio2->call_id == PJSUA_INVALID_ID) & (trx1->radio2->callState))
            {
                trx1->radio2->callState = false;
                trx1->radio2->callLastState = DISCONNECTED;
            }
        }
        else if ((call_id == trx2->radio1->call_id)&(trxID == trx2->radio1->url)&(trx2->radio1->pingOk))
        {
            //qDebug() << "makeCall_Radio" << sip_urichr << trx2->radio1->callState << trx2->radio1->enable << trx2->radio1->trxFailed << trx2->radio1->call_id;
            if ((!trx2->radio1->callState)&&(trx2->radio1->enable)&&(!trx2->radio1->trxFailed)&(trx2->radio1->call_id == PJSUA_INVALID_ID))
            {
                trx2->radio1->ByeReason = "";
                makeCallIndex = trx2->radio1->trxmode;
                trx2->radio1->call_id = makeCall_Radio(sip_urichr);
                if(trx2->radio1->call_id == PJSUA_INVALID_ID)
                {
                    trx2->radio1->callState = false;
                    trx2->radio1->connDuration = "0";
                    trx2->radio1->m_PttPressed = false;
                    stopRing();
                    trx2->radio1->callLastState = DISCONNECTED;
                    updateHomeDisplay(trx2->radio1->url,trx2->callLastState,"",trx2->radio1->call_id,trx2->radio1->radioNodeID);
                    if (trx2->radio1->pptTestPressed){
                        pttStatus = false;
                    }
                }
            }
            else if(trx2->radio1->call_id != PJSUA_INVALID_ID)
            {
                stopRing();
                trx2->radio1->callLastState = DISCONNECTED;
                pj_status_t status = pjsua_call_hangup(trx2->radio1->call_id, PJSIP_SC_BUSY_HERE, NULL ,  &msg_data);
                if (status == PJ_SUCCESS) {

                }
                else
                {
                    qDebug() << "PJ_FAILED" << status;
//                    pjHangupAll();
                    trx2->radio1->call_id = PJSUA_INVALID_ID;
                }
            }
            else if ((trx2->radio1->call_id == PJSUA_INVALID_ID) & (trx2->radio1->callState))
            {
                trx2->radio1->callState = false;
                trx2->radio1->callLastState = DISCONNECTED;
            }
        }
        else if ((call_id == trx2->radio2->call_id)&(trxID == trx2->radio2->url)&(trx2->radio2->pingOk))
        {
            if ((!trx2->radio2->callState)&&(trx2->radio2->enable)&&(!trx2->radio2->trxFailed)&(trx2->radio2->call_id == PJSUA_INVALID_ID))
            {
                trx2->radio2->ByeReason = "";
                makeCallIndex = trx2->radio2->trxmode;
                trx2->radio2->call_id = makeCall_Radio(sip_urichr);
                if(trx2->radio2->call_id == PJSUA_INVALID_ID)
                {
                    trx2->radio2->callState = false;
                    trx2->radio2->connDuration = "0";
                    trx2->radio2->m_PttPressed = false;
                    stopRing();
                    trx2->radio2->callLastState = DISCONNECTED;
                    updateHomeDisplay(trx2->radio2->url,trx2->callLastState,"",trx2->radio2->call_id,trx2->radio2->radioNodeID);
                    if (trx2->radio2->pptTestPressed){
                        pttStatus = false;
                    }
                }
            }
            else if(trx2->radio2->call_id != PJSUA_INVALID_ID)
            {
                stopRing();
                trx2->radio2->callLastState = DISCONNECTED;
                pj_status_t status = pjsua_call_hangup(trx2->radio2->call_id, PJSIP_SC_BUSY_HERE, NULL ,  &msg_data);
                if (status == PJ_SUCCESS)
                {

                }
                else
                {
                    qDebug() << "PJ_FAILED" << status;
//                    pjHangupAll();
                    trx2->radio2->call_id = PJSUA_INVALID_ID;
                }
            }
            else if ((trx2->radio2->call_id == PJSUA_INVALID_ID) & (trx2->radio2->callState))
            {
                trx2->radio2->callState = false;
                trx2->radio2->callLastState = DISCONNECTED;
            }
        }
    }
    else if (inviteMode == CLIENT)
    {
        for (int i = 0;i < trx_incall.length();i++)
        {
            if (call_id == trx_incall.at(i)->call_id)
            {
                if ((!trx_incall.at(i)->callState)&&(!trx_incall.at(i)->trxFailed))
                {
                    if(trx_incall.at(i)->call_id == PJSUA_INVALID_ID)
                    {
                        trx_incall.at(i)->callState = false;
                        stopRing();
                        trx_incall.at(i)->callLastState = DISCONNECTED;
                        updateHomeDisplay(trx_incall.at(i)->callIndexName,trx_incall.at(i)->callLastState,"",trx_incall.at(i)->call_id,0);
                    }
                }
                else if ((trx_incall.at(i)->callState))
                {
                    stopRing();
                    if(trx_incall.at(i)->call_id != PJSUA_INVALID_ID)
                    {
                        trx_incall.at(i)->callLastState = DISCONNECTED;
                        pj_status_t status = pjsua_call_hangup(trx_incall.at(i)->call_id, PJSIP_SC_BUSY_HERE, NULL ,  &msg_data);
                        if (status == PJ_SUCCESS)
                        {
                            qDebug() << "pjsua_call_hangup(trx_incall.at(i)->call_id, code, reason ,  &msg_data): PJ_SUCCESS" ;
                            trx_incall.at(i)->callLastState = DISCONNECTED;
                            trx_incall.at(i)->call_id = PJSUA_INVALID_ID;
                            trx_incall.at(i)->callState = false;
                            trx_incall.at(i)->connDuration = "0";
                            trx_incall.at(i)->sec_connDuration = 0;
//                            trx_incall.at(i)->m_PttPressed = false;
                            updateHomeDisplay(trx_incall.at(i)->callIndexName,DISCONNECTED,"",trx_incall.at(i)->call_id,0);
                            if (trx_incall.at(i)->pptTestPressed){
                                pttStatus = false;
                            }
                            if (trx_incall.at(i)->callName == recorderAlloweURI)
                            {
                                recorderConnected = false;
                            }
                        }
                        else
                        {
                            qDebug() << "pjsua_call_hangup(trx_incall.at(i)->call_id, code, reason ,  &msg_data): PJ_FAILED" ;
                        }
                    }
                 }
            }
        }
//        callInNum = 0;
//        for (int i=0; i<trx_incall.length(); i++)
//        {
//            if ((trx_incall.at(i)->call_id  != PJSUA_INVALID_ID) & (trx_incall.at(i)->callState == true))
//                callInNum++;
//        }
        callInNum = getActiveCallCount();
    }
    else if (inviteMode == CLIENT_RPT)
    {
        for (int i = 0;i < trx_incall.length();i++)
        {
            if (call_id == trx_incall.at(i)->call_id)
            {
                if ((!trx_incall.at(i)->callState)&&(!trx_incall.at(i)->trxFailed))
                {
                    if(trx_incall.at(i)->call_id == PJSUA_INVALID_ID)
                    {
                        trx_incall.at(i)->callState = false;
                        stopRing();
                        trx_incall.at(i)->callLastState = DISCONNECTED;
                        updateHomeDisplay(trx_incall.at(i)->callIndexName,trx_incall.at(i)->callLastState,"",trx_incall.at(i)->call_id,0);
                    }
                }
                else if ((trx_incall.at(i)->callState))
                {
                    stopRing();
                    if(trx_incall.at(i)->call_id != PJSUA_INVALID_ID)
                    {
                        trx_incall.at(i)->callLastState = DISCONNECTED;
                        pj_status_t status = pjsua_call_hangup(trx_incall.at(i)->call_id, PJSIP_SC_BUSY_HERE, NULL ,  &msg_data);
                        if (status == PJ_SUCCESS) {
                            qDebug() << "pjsua_call_hangup(trx_incall.at(i)->call_id, code, reason ,  &msg_data): PJ_SUCCESS" ;
                            trx_incall.at(i)->callLastState = DISCONNECTED;
                            trx_incall.at(i)->call_id = PJSUA_INVALID_ID;
                            trx_incall.at(i)->callState = false;
                            trx_incall.at(i)->connDuration = "0";
                            trx_incall.at(i)->sec_connDuration = 0;
//                            trx_incall.at(i)->m_PttPressed = false;
//                            trx_incall.at(i)->sqlTestPressed = false;
//                            trx_incall.at(i)->pptTestPressed = false;
                            updateHomeDisplay(trx_incall.at(i)->callIndexName,DISCONNECTED,"",trx_incall.at(i)->call_id,0);
                            if (trx_incall.at(i)->pptTestPressed){
                                pttStatus = false;
                            }
                            if (trx_incall.at(i)->callName == recorderAlloweURI)
                            {
                                recorderConnected = false;
                            }
                        }
                        else
                        {
                            qDebug() << "pjsua_call_hangup(trx_incall.at(i)->call_id, code, reason ,  &msg_data): PJ_FAILED" ;
                        }
                    }
                 }
            }
        }
//        callInNum = 0;
//        for (int i=0; i<trx_incall.length(); i++)
//        {
//            if ((trx_incall.at(i)->call_id  != PJSUA_INVALID_ID) & (trx_incall.at(i)->callState == true))
//                callInNum++;
//        }
        callInNum = getActiveCallCount();
    }
}

void RoIP_ED137::trx_incall_hangup(pjsua_call_id call_id, unsigned code, pj_str_t *reason, int i)
{
    if(call_id != PJSUA_INVALID_ID){
        pjsua_msg_data msg_data;
        pjsua_msg_data_init(&msg_data);

        // Heade 1
        pj_str_t hname = pj_str((char*)"Priority");
        pj_str_t hvalue = pj_str((char*)"normal");
        pjsip_generic_string_hdr my_hdr3;
        pjsip_generic_string_hdr_init2(&my_hdr3, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr3);

        // Header 2
        hname = pj_str((char*)"Subject");
        hvalue = pj_str((char*)"radio");
        pjsip_generic_string_hdr my_hdr1;
        pjsip_generic_string_hdr_init2(&my_hdr1, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr1);

        // Header 3
        hname = pj_str((char*)"WG67-Version");
        hvalue = pj_str((char*)"radio.01;radio.02");
        pjsip_generic_string_hdr my_hdr2;
        pjsip_generic_string_hdr_init2(&my_hdr2, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr2);

        hname = pj_str((char*)"Reason");
        pjsip_generic_string_hdr my_hdr4;
        pjsip_generic_string_hdr_init2(&my_hdr4, &hname, reason);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr4);

        // Header 4
        hname = pj_str((char*)"user-agent");
        hvalue = pj_str((char*)"PAS Radio");
        pjsip_generic_string_hdr my_hdr5;
        pjsip_generic_string_hdr_init2(&my_hdr5, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr5);

        if (call_id == trx_incall.at(i)->call_id){
            qDebug() << "pjsua_call_hangup" << trx_incall.at(i)->url << trx_incall.at(i)->call_id << call_id;
            trx_incall.at(i)->trxFailed = false;
            if ((trx_incall.at(i)->callState))
            {
                stopRing();
                if(trx_incall.at(i)->call_id != PJSUA_INVALID_ID)
                {
                    pj_status_t status = pjsua_call_hangup(trx_incall.at(i)->call_id, code, reason ,  &msg_data);
                    if (status == PJ_SUCCESS) {
                        qDebug() << "pjsua_call_hangup(trx_incall.at(i)->call_id, code, reason ,  &msg_data): PJ_SUCCESS" ;
                        trx_incall.at(i)->callLastState = DISCONNECTED;
                        trx_incall.at(i)->call_id = PJSUA_INVALID_ID;
                        trx_incall.at(i)->callState = false;
                        trx_incall.at(i)->connDuration = "0";
                        trx_incall.at(i)->sec_connDuration = 0;
//                        trx_incall.at(i)->m_PttPressed = false;
                        updateHomeDisplay(trx_incall.at(i)->callIndexName,DISCONNECTED,"",trx_incall.at(i)->call_id,0);
                        if (trx_incall.at(i)->pptTestPressed)
                        {
                            pttStatus = false;
                        }
                    }
                    else
                    {
                        qDebug() << "pjsua_call_hangup(trx_incall.at(i)->call_id, code, reason ,  &msg_data): PJ_FAILED" ;
                    }
                }
            }
        }
     }
}
void RoIP_ED137::trx_call_hangup(pjsua_call_id call_id, unsigned code, pj_str_t *reason)
{
    if(call_id != PJSUA_INVALID_ID){
        pjsua_msg_data msg_data;
        pjsua_msg_data_init(&msg_data);

        // Heade 1
        pj_str_t hname = pj_str((char*)"Priority");
        pj_str_t hvalue = pj_str((char*)"normal");
        pjsip_generic_string_hdr my_hdr3;
        pjsip_generic_string_hdr_init2(&my_hdr3, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr3);

        // Header 2
        hname = pj_str((char*)"Subject");
        hvalue = pj_str((char*)"radio");
        pjsip_generic_string_hdr my_hdr1;
        pjsip_generic_string_hdr_init2(&my_hdr1, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr1);

        // Header 3
        hname = pj_str((char*)"WG67-Version");
        hvalue = pj_str((char*)"radio.01;radio.02");
        pjsip_generic_string_hdr my_hdr2;
        pjsip_generic_string_hdr_init2(&my_hdr2, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr2);

        hname = pj_str((char*)"Reason");
        pjsip_generic_string_hdr my_hdr4;
        pjsip_generic_string_hdr_init2(&my_hdr4, &hname, reason);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr4);

        // Header 4
        hname = pj_str((char*)"user-agent");
        hvalue = pj_str((char*)"PAS Radio");
        pjsip_generic_string_hdr my_hdr5;
        pjsip_generic_string_hdr_init2(&my_hdr5, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr5);

        if (call_id == trx1->radio1->call_id){
            trx1->radio1->trxFailed = false;
            if ((trx1->radio1->callState))
            {
                stopRing();
                pj_status_t status = pjsua_call_hangup(call_id, code, reason ,  &msg_data);
                if (status == PJ_SUCCESS) {
                    trx1->radio1->callState = false;
                    trx1->radio1->connDuration = "0";
                    trx1->radio1->m_PttPressed = false;
                    stopRing();
                    trx1->radio1->callLastState = DISCONNECTED;
                    updateHomeDisplay(trx1->radio1->url,DISCONNECTED,"",call_id,trx1->radio1->radioNodeID);
                    if (trx1->radio1->pptTestPressed){
                        pttStatus = false;
                    }
                    qDebug() << trx1->radio1->url  << "pjsua_call_hangup: PJ_SUCCESS: Success" ;
                }
                else
                {
                    qDebug() << "pjsua_call_hangup PJ_SUCCESS: False" ;
                }
            }
        }
        else if (call_id == trx1->radio2->call_id){
            trx1->radio2->trxFailed = false;
            if ((trx1->radio2->callState))
            {
                stopRing();
                pj_status_t status = pjsua_call_hangup(call_id, code, reason ,  &msg_data);
                if (status == PJ_SUCCESS) {
                    trx1->radio2->callState = false;
                    trx1->radio2->connDuration = "0";
                    trx1->radio2->m_PttPressed = false;
                    stopRing();
                    trx1->radio2->callLastState = DISCONNECTED;
                    updateHomeDisplay(trx1->radio2->url,DISCONNECTED,"",call_id,trx1->radio2->radioNodeID);
                    if (trx1->radio2->pptTestPressed){
                        pttStatus = false;
                    }
                    qDebug() << trx1->radio2->url  << "pjsua_call_hangup: PJ_SUCCESS: Success" ;
                }
                else
                {
                    qDebug() << "pjsua_call_hangup PJ_SUCCESS: False" ;
                }
            }
        }
        else if (call_id == trx2->radio1->call_id){
            trx2->radio1->trxFailed = false;
            if ((trx2->radio1->callState))
            {
                stopRing();
                pj_status_t status = pjsua_call_hangup(call_id, code, reason ,  &msg_data);
                if (status == PJ_SUCCESS) {
                    trx2->radio1->callState = false;
                    trx2->radio1->connDuration = "0";
                    trx2->radio1->m_PttPressed = false;
                    stopRing();
                    trx2->radio1->callLastState = DISCONNECTED;
                    updateHomeDisplay(trx2->radio1->url,DISCONNECTED,"",call_id,trx2->radio1->radioNodeID);
                    if (trx2->radio1->pptTestPressed){
                        pttStatus = false;
                    }
                    qDebug() << trx2->radio1->url << "pjsua_call_hangup: PJ_SUCCESS: Success" ;
                }
                else
                {
                    qDebug() << "pjsua_call_hangup PJ_SUCCESS: False" ;
                }
            }
        }
        else if (call_id == trx2->radio2->call_id)
        {
            trx2->radio2->trxFailed = false;
            if ((trx2->radio2->callState))
            {
                stopRing();
                pj_status_t status = pjsua_call_hangup(call_id, code, reason ,  &msg_data);
                if (status == PJ_SUCCESS) {
                    trx2->radio2->callState = false;
                    trx2->radio2->connDuration = "0";
                    trx2->radio2->m_PttPressed = false;
                    stopRing();
                    trx2->radio2->callLastState = DISCONNECTED;
                    updateHomeDisplay(trx2->radio2->url,DISCONNECTED,"",call_id,trx2->radio2->radioNodeID);
                    if (trx2->radio2->pptTestPressed){
                        pttStatus = false;
                    }
                    qDebug() << trx2->radio2->url  << "pjsua_call_hangup: PJ_SUCCESS: Success" ;
                }
                else
                {
                    qDebug() << "pjsua_call_hangup PJ_SUCCESS: False" ;
                }
            }
        }
        else
        {
            for (int i = 0;i < trx_incall.length();i++){
                if (call_id == trx_incall.at(i)->call_id){
                    qDebug() << "pjsua_call_hangup" << trx_incall.at(i)->url << trx_incall.at(i)->call_id << call_id;
                    trx_incall.at(i)->trxFailed = false;
                    if ((trx_incall.at(i)->callState))
                    {
                        stopRing();
                        if(trx_incall.at(i)->call_id != PJSUA_INVALID_ID)
                        {
                            trx_incall.at(i)->callLastState = DISCONNECTED;
                            pj_status_t status = pjsua_call_hangup(trx_incall.at(i)->call_id, code, reason ,  &msg_data);
                            if (status == PJ_SUCCESS)
                            {
                                qDebug() << "pjsua_call_hangup(trx_incall.at(i)->call_id, code, reason ,  &msg_data): PJ_SUCCESS" ;
                                trx_incall.at(i)->callState = false;
                                trx_incall.at(i)->connDuration = "0";
                                trx_incall.at(i)->sec_connDuration = 0;
//                                trx_incall.at(i)->m_PttPressed = false;
                                stopRing();
                                trx_incall.at(i)->callLastState = DISCONNECTED;
                                updateHomeDisplay(trx_incall.at(i)->url,DISCONNECTED,"",call_id,0);
                                if (trx_incall.at(i)->pptTestPressed)
                                {
                                    pttStatus = false;
                                }
                                if (trx_incall.at(i)->sqlTestPressed)
                                {
                                    sqlOnStatus = false;
                                }
                                if (trx_incall.at(i)->callName == recorderAlloweURI)
                                {
                                    recorderConnected = false;
                                }
                            }
                            else
                            {
                                qDebug() << "pjsua_call_hangup(trx_incall.at(i)->call_id, code, reason ,  &msg_data): PJ_FAILED" ;
                            }
                        }
                    }
                }
            }
        }
    }
}



void RoIP_ED137::StartSip()
{
    pj_status_t status;

    //pjsua_acc_id acc_id;

    /* Create pjsua first! */
    status = pjsua_create();
    if (status != PJ_SUCCESS) error_exit("Error in pjsua_create()", status);

    /* Init pjsua */
    {
    pjsua_config cfg;
    pjsua_logging_config log_cfg;
    pjsua_media_config media_cfg;
    pjsua_transport_id tp_id;

    pjsua_config_default(&cfg);
    cfg.max_calls = PJSUA_MAX_CALLS;    
    cfg.cb.on_call_media_state = &::on_call_media_state;
    cfg.cb.on_call_state = &::on_call_state;
    cfg.cb.on_reg_state = &::on_reg_state;
    cfg.cb.on_create_media_transport= &::on_create_media_transport;
    cfg.cb.on_incoming_call = &::on_incoming_call;
//    cfg.cb.on_incoming_subscribe = &::on_incoming_subscribe;
    //cfg.cb.on_stream_created = &::on_stream_created;
    //cfg.cb.on_stream_destroyed = &::on_stream_destroyed;

    pjsua_logging_config_default(&log_cfg);
    log_cfg.console_level = PJSUA_LOG_LEVEL;

    pjsua_media_config_default(&media_cfg);

    media_cfg.no_vad = 1;
    media_cfg.enable_ice = 0;
    media_cfg.snd_auto_close_time = 1;
    media_cfg.clock_rate = clock_rate;
    media_cfg.snd_clock_rate = clock_rate;

    media_cfg.ec_tail_len = 0; // Disable echo cancellation
    media_cfg.channel_count = CHANNEL_COUNT;
    media_cfg.quality = 5;




    printf("Media_cfg:\n");
    printf("%d %d %d %d %d %d\n", media_cfg.no_vad,media_cfg.enable_ice,media_cfg.snd_auto_close_time,media_cfg.clock_rate,media_cfg.snd_clock_rate,media_cfg.ec_tail_len);

    qDebug () << "/* Add UDP transport. */";
    {
    pjsua_transport_config cfg;

    pjsua_transport_config_default(&cfg);

    QString SipIp = getIpAddress();
    char cSipIp[SipIp.length()+1];
    strcpy( cSipIp, SipIp.toStdString().c_str());
    cfg.port = sip_port;
    if (SipIp != "cannothandled"){
        cfg.public_addr = pj_str((char *)cSipIp);
        cfg.bound_addr = pj_str((char *)cSipIp);
    }

    status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, &tp_id);
    if (status != PJ_SUCCESS) error_exit("Error creating transport", status);
    }

   /* pj_status_t status = pjsua_acc_add_local(tp_id, true, &acc_id);
    if (status != PJ_SUCCESS)
    {
        printf("pjsua_acc_add_local Failed\n");
    }*/


    RegisterDefault();

    status = pjsua_init(&cfg, &log_cfg, &media_cfg);
    //exit(0);
    if (status != PJ_SUCCESS) error_exit("Error in pjsua_init()", status);
    }

    /* Initialization is done, now start pjsua */
    status = pjsua_start();
    if (status != PJ_SUCCESS) error_exit("Error starting pjsua", status);
    else if (status == PJ_SUCCESS)
    {
        init_ringTone();
    }
}
QString RoIP_ED137::getIpAddress()
{
    QString ipaddress = "";
    defaultEthernet = "bond0";
    networking->getIPAddress(defaultEthernet);
    ipaddress = networking->netWorkCardAddr;
    qDebug() << "defaultEthernet" << defaultEthernet << ipaddress;
    if (ipaddress != "")
        return ipaddress;
    else
    {
        if ((defaultEthernet == "eth0") & (eth0.dhcpmethod.contains("off")) & (eth0.ipaddress != ""))
            return eth0.ipaddress;
        else if ((defaultEthernet == "eth1") & (eth1.dhcpmethod.contains("off")) & (eth1.ipaddress != ""))
            return eth1.ipaddress;
    }
    return "cannothandled";
}
void RoIP_ED137::RegisterDefault()
{
    pj_status_t status;

//    std::string SipIp = Utility::getIpAddress();
//    QString defaultAddress = getIpAddress();

    std::string SipIp = getIpAddress().toStdString();

    char sipuri[50];
    char sipuser[50];
    char defaultAddress[15];
    strcpy( sipuser, SipUser.c_str());
    strcpy( defaultAddress, SipIp.c_str());
    QString sip_usernamestr = QString::fromStdString(SipUser);
    char sipUser[sip_usernamestr.length()+1];
    strcpy( sipUser, sip_usernamestr.toStdString().c_str());
    //getlogin_r(sipuser, sizeof(sipuser));

    pj_ansi_snprintf(sipuri, sizeof(sipuri), "<sip:%s@%s>",SipUser.c_str(),SipIp.c_str());
    qDebug() << ".................................................RegisterDefault sipuri" << sipuri << defaultAddress;

    pjsua_acc_config cfg;    
    pjsua_acc_config_default(&cfg);
    cfg.lock_codec = 0;
    cfg.allow_sdp_nat_rewrite = 1;
    cfg.id = pj_str(sipuri);
    cfg.rtp_cfg.port = rtp_cfg_port;
    cfg.rtp_cfg.public_addr = pj_str(defaultAddress);
//    cfg.reg_uri = pj_str(sipuri);
    cfg.cred_info[0].username = pj_str((char *)sipUser);
    status = pjsua_acc_add(&cfg, PJ_TRUE, &acc_id);
    if (status != PJ_SUCCESS)
    {
        error_exit("Error adding account", status);
        qDebug() << "Error adding account";
    }
    status = pjsua_acc_set_default(acc_id);
    if(status == PJ_SUCCESS)
    {
        sprintf(log_buffer, "Number of Accounts: %d Current Account: %d is default.\n",pjsua_acc_get_count(),acc_id);
        qDebug() << log_buffer;
        //AppentTextBrowser(log_buffer);
    }

}

void RoIP_ED137::UpdateDefault()
{
    pj_status_t status;

    if(pjsua_acc_is_valid(acc_id))
    {
        status = pjsua_acc_set_default(acc_id);
        if(status == PJ_SUCCESS)
        {
            sprintf(log_buffer, "Number of Accounts: %d Current Account: %d is default.\n",pjsua_acc_get_count(),acc_id);
            //AppentTextBrowser(log_buffer);
        }
    }
}

void RoIP_ED137::RegisterUser()
{

}
void RoIP_ED137::UnRegisterUser()
{

}

bool RoIP_ED137::removeDefault()
{
    bool result = false;
    pj_status_t status;

    if(pjsua_acc_is_valid(acc_id))
    {
        if(trx1->radio1->call_id != PJSUA_INVALID_ID)
        {
             pjsua_call_hangup(trx1->radio1->call_id, PJSIP_SC_BUSY_HERE, NULL, NULL);
        }
        if(trx1->radio2->call_id != PJSUA_INVALID_ID)
        {
             pjsua_call_hangup(trx1->radio2->call_id, PJSIP_SC_BUSY_HERE, NULL, NULL);
        }
        if(trx2->radio1->call_id != PJSUA_INVALID_ID)
        {
             pjsua_call_hangup(trx2->radio1->call_id, PJSIP_SC_BUSY_HERE, NULL, NULL);
        }
        if(trx2->radio2->call_id != PJSUA_INVALID_ID)
        {
             pjsua_call_hangup(trx2->radio2->call_id, PJSIP_SC_BUSY_HERE, NULL, NULL);
        }
        for(int i = 0; i < trx_incall.length(); i++){
            if(trx_incall.at(i)->call_id != PJSUA_INVALID_ID)
            {
                 pjsua_call_hangup(trx_incall.at(i)->call_id, PJSIP_SC_BUSY_HERE, NULL, NULL);
            }
        }

        status = pjsua_acc_set_registration(acc_id, PJ_FALSE);
        if (status == PJ_SUCCESS)
        {
            status = pjsua_acc_del(acc_id);
            if (status != PJ_SUCCESS)
            {
               pjsua_perror(THIS_FILE, "Error removing default account", status);
            }
            else
            {
                acc_id = PJSUA_INVALID_ID;
                qDebug() << "removing default account";
                result = true;
            }
        }
    }
    return result;
}

bool RoIP_ED137::removeUser()
{
    bool result = false;
    pj_status_t status;

    if(pjsua_acc_is_valid(acc_id))
    {
        if(current_call != PJSUA_INVALID_ID)
        {
             pjsua_call_hangup(current_call, PJSIP_SC_BUSY_HERE, NULL, NULL);
        }

        status = pjsua_acc_set_registration(acc_id, PJ_FALSE);
        if (status == PJ_SUCCESS)
        {
            status = pjsua_acc_del(acc_id);
            if (status != PJ_SUCCESS)
            {
               pjsua_perror(THIS_FILE, "Error removing asterisk account", status);
            }
            else
            {
                acc_id = PJSUA_INVALID_ID;
                result = true;
            }
        }
    }
    return result;
}

void RoIP_ED137::searchAudioDevice()
{
  void **hints;
  const char *ifaces[] = {"pcm", 0};
  int index = 0;
  void **str;
  char *name;
  char *desc;
  int devIdx = 0;
  size_t tPos;

  snd_config_update();

  while (ifaces[index]) {

    printf("Querying interface %s \n", ifaces[index]);
    if (snd_device_name_hint(-1, ifaces[index], &hints) < 0)
    {
      printf("Querying devices failed for %s.\n", ifaces[index]);
      index++;
      continue;
    }
    str = hints;
    while (*str)
    {
      name = snd_device_name_get_hint(*str, "NAME");
      desc = snd_device_name_get_hint(*str, "DESC");

      string tNameStr = "";
      if (name != NULL)
          tNameStr = string(desc);

      printf("\n");

      // search for "default:", if negative result then go on searching for next device
      if ((tNameStr != "") && ((tPos = tNameStr.find("USB")) != string::npos))
      {
          printf("Deafult Sound Card : %d : %s\n%s\n\n",devIdx, name,desc);
          //snd_device_name_free_hint(hints);

         // return;
      }

      free(name);
      free(desc);
      devIdx++;
      str++;
    }
    index++;
    snd_device_name_free_hint(hints);
  }
  return;
}

/*
   +-----------+ stereo +-----------------+ 2x mono +-----------+
   | AUDIO DEV |<------>| SPLITCOMB   left|<------->|#0  BRIDGE |
   +-----------+        |            right|<------->|#1         |
                        +-----------------+         +-----------+
 */

bool RoIP_ED137::initSlaveSoundCard()
{
    pj_status_t status;

        int samples_per_frame = clock_rate * 20 / 1000;
        int bits_per_sample = 16;
        int channel_count = 6;
        int snd_dev_id = 6;

        pool = pjsua_pool_create("pjsua-app", 1000, 1000);

        // Disable existing sound device
        conf_port = pjsua_set_no_snd_dev();

        // Create stereo-mono splitter/combiner
        status = pjmedia_splitcomb_create(pool,
                          clock_rate, // clock rate ,
                          channel_count,
                          channel_count * samples_per_frame,
                          bits_per_sample,
                          0,    // options ,
                          &sc);
        pj_assert(status == PJ_SUCCESS);

        // Connect channel0 to conference port slot0
        status = pjmedia_splitcomb_set_channel(
                                sc,0, // ch0 ,
                                0, //options,
                                conf_port);
        pj_assert(status == PJ_SUCCESS);

        ///////////////////////////////////////////////////////

        // Create reverse channel for channel1
        status = pjmedia_splitcomb_create_rev_channel(pool,
                              sc,
                              1, // ch1 ,
                              0 , // options ,
                              &sc_ch1);
        pj_assert(status == PJ_SUCCESS);

        status = pjsua_conf_add_port(pool, sc_ch1,
                     &sc_ch1_slot);
        pj_assert(status == PJ_SUCCESS);


        // Create reverse channel for channel2
        status = pjmedia_splitcomb_create_rev_channel(pool,
                              sc,
                              2, // ch2 ,
                              0 , // options ,
                              &sc_ch2);
        pj_assert(status == PJ_SUCCESS);

        status = pjsua_conf_add_port(pool, sc_ch2,
                     &sc_ch2_slot);
        pj_assert(status == PJ_SUCCESS);

        // Create reverse channel for channel3
        status = pjmedia_splitcomb_create_rev_channel(pool,
                              sc,
                              3, // ch3 ,
                              0 , // options ,
                              &sc_ch3);
        pj_assert(status == PJ_SUCCESS);

        status = pjsua_conf_add_port(pool, sc_ch3,
                     &sc_ch3_slot);
        pj_assert(status == PJ_SUCCESS);

        // Create reverse channel for channel4
        status = pjmedia_splitcomb_create_rev_channel(pool,
                              sc,
                              4, // ch4 ,
                              0 , // options ,
                              &sc_ch4);
        pj_assert(status == PJ_SUCCESS);

        status = pjsua_conf_add_port(pool, sc_ch4,
                     &sc_ch4_slot);
        pj_assert(status == PJ_SUCCESS);

        // Create reverse channel for channel5
        status = pjmedia_splitcomb_create_rev_channel(pool,
                              sc,
                              5, // ch5 ,
                              0 , // options ,
                              &sc_ch5);
        pj_assert(status == PJ_SUCCESS);

        status = pjsua_conf_add_port(pool, sc_ch5,
                     &sc_ch5_slot);
        pj_assert(status == PJ_SUCCESS);


        ////////////////////////////////////////////////////////

       /* status = pjmedia_snd_port_create(pool, dev_id,dev_id,
                         clock_rate,
                         channel_count,
                         channel_count * samples_per_frame,
                         bits_per_sample,
                         0, &sndPlayback);
        pj_assert(status == PJ_SUCCESS);

        status = pjmedia_snd_port_connect(sndPlayback, sc);
        pj_assert(status == PJ_SUCCESS);*/

        // Create sound playback device
        status = pjmedia_snd_port_create_player(pool, snd_dev_id,
                         clock_rate,
                         channel_count,
                         channel_count * samples_per_frame,
                         bits_per_sample,
                         0, &sndPlayback);
        pj_assert(status == PJ_SUCCESS);

        status = pjmedia_snd_port_connect(sndPlayback, sc);
        pj_assert(status == PJ_SUCCESS);

    return true;
}

void RoIP_ED137::listAudioDevInfo()
{
    printf("\nRoIP_ED137::setAudioDevInfo() begins\n");
    unsigned devCount = pjmedia_aud_dev_count();

    for (unsigned i = 0; i < devCount; i++)
    {
        pjmedia_aud_dev_info info;
        pj_status_t status = pjmedia_aud_dev_get_info(i, &info);

        if (status == PJ_SUCCESS)
        {
           m_AudioDevInfoVector.push_back(info);
           sprintf(log_buffer, "Card Num: %d - Card Name: %s %dHz",i,info.name,info.default_samples_per_sec);
           //AppentTextBrowser(log_buffer);
           printf("%s\n",log_buffer);
        }
    }
    printf("\n");
}

void RoIP_ED137::setAudioDevInfo()
{
    char *name;
    size_t tPos;
    bool defaultCaptureFound = false;
    bool defaultPlayoutFound = false;

    unsigned iCaptureFound;
    unsigned iPlayoutFound;

    {
        unsigned devCount = pjmedia_aud_dev_count();
        for (iCaptureFound = 0; iCaptureFound < devCount; iCaptureFound++)
        {
            pjmedia_aud_dev_info info;
            pj_status_t status = pjmedia_aud_dev_get_info(iCaptureFound, &info);

            if (status == PJ_SUCCESS)
            {
               m_AudioDevInfoVector.push_back(info);
               string tNameStr = "";
               name = info.name;
               if (name != NULL)
                   tNameStr = string(name);
               if (inviteMode == CLIENT_RPT){
                   if (((tPos = tNameStr.find(CARD_CAPTURE_RPT)) != string::npos))
                   {
                       defaultCaptureFound = true;
                       break;
                   }
               }
               else
               {
                   if ((inviteMode == CLIENT) & (recorderSuported))
                   {
                       if (((tPos = tNameStr.find(CARD_CAPTURE_NORMAL_RECON)) != string::npos))
                       {
                           defaultCaptureFound = true;
                           break;
                       }
                   }
                   else if (((inviteMode == CLIENT) & (recorderSuported == false)) || (inviteMode == SERVER)) {
                       if (((tPos = tNameStr.find(CARD_CAPTURE_NORMAL_NOREC.toStdString())) != string::npos))
                       {
                           defaultCaptureFound = true;
                           break;
                       }
                   }

               }
            }
        }

        for (iPlayoutFound = 0; iPlayoutFound < devCount; iPlayoutFound++)
        {
            pjmedia_aud_dev_info info;
            pj_status_t status = pjmedia_aud_dev_get_info(iPlayoutFound, &info);

            if (status == PJ_SUCCESS)
            {
               m_AudioDevInfoVector.push_back(info);
               string tNameStr = "";
               name = info.name;
               if (name != NULL)
                   tNameStr = string(name);
               if (((tPos = tNameStr.find(CARD_PLAYOUT.toStdString())) != string::npos))
               {
                   defaultPlayoutFound = true;
                   break;
               }
            }
        }
    }

    qDebug() << "RoIP_ED137::setAudioDevInfo() begins" << "inviteMode" << inviteMode << "defaultCaptureFound" << defaultCaptureFound << CARD_CAPTURE_NORMAL_NOREC << "defaultPlayoutFound" << defaultPlayoutFound << CARD_PLAYOUT;


    if (defaultCaptureFound & defaultPlayoutFound)
    {
          initSoundCard(iPlayoutFound,iCaptureFound);
    }
    else
    {
        exit(0);
//        initSoundCard(0,0);
    }
}

void RoIP_ED137::setCodecPriority()
{

    pjsua_codec_info c[128];

    unsigned i, count = PJ_ARRAY_SIZE(c);
    pj_str_t codec_s;

    printf("List of audio codecs:\n");
    pjsua_enum_codecs(c, &count);
    for (i = 0; i < count; ++i)
    {
        pjsua_codec_set_priority(pj_cstr(&codec_s, c[i].codec_id.ptr), PJMEDIA_CODEC_PRIO_DISABLED);
    }


//    pjsua_codec_set_priority(pj_cstr(&codec_s, "R2S/8000/1"), PJMEDIA_CODEC_PRIO_HIGHEST);
    pjsua_codec_set_priority(pj_cstr(&codec_s, "PCMA/8000/1"), PJMEDIA_CODEC_PRIO_HIGHEST);
    pjsua_codec_set_priority(pj_cstr(&codec_s, "PCMU/8000/1"), PJMEDIA_CODEC_PRIO_NEXT_HIGHER);
    pjsua_codec_set_priority(pj_cstr(&codec_s, "G729/8000/1"), PJMEDIA_CODEC_PRIO_NEXT_HIGHER);


    pjsua_enum_codecs(c, &count);
    for (i = 0; i < count; ++i)
    {
        printf("%d\t%.*s\n", c[i].priority, (int) c[i].codec_id.slen, c[i].codec_id.ptr);
    }

}

void RoIP_ED137::initSoundCard(int playoutCardNumber, int captureCardNumber)
{
    pj_status_t status = pjsua_set_snd_dev(captureCardNumber, playoutCardNumber);
    if (status != PJ_SUCCESS)
    {
        printf("Error setting sound card device cardNumber: %d\n",playoutCardNumber);
        exit(0);
    }
    else
    {
       //connectPort(0,m_NullSlot);

        std::string cardName;
        getSoundCardName(captureCardNumber,cardName);
        printf("Card %s Set As Master SUCCESS.\n", cardName.c_str());
        sprintf(log_buffer, "Card: %s Set As Master",cardName.c_str());

        getSoundCardName(playoutCardNumber,cardName);
        printf("Card %s Set As Master SUCCESS.\n", cardName.c_str());
        sprintf(log_buffer, "Card: %s Set As Master",cardName.c_str());
        //AppentTextBrowser(log_buffer);
    }
}

bool RoIP_ED137::checkSipUrl(std::string extNumber)
{
    bool valid = false;
    size_t pos1 = extNumber.find_first_of(":");
    size_t pos2 = extNumber.find_first_of("@", pos1);

    if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1 +1)
    {
        valid = true;
    }

    return valid;
}

int RoIP_ED137::makeSipCall(std::string const& extNumber)
{
    registerThread();
    isSipCall = true;

    if (!checkSipUrl(extNumber.c_str()))
    {
        printf("Invalid Url: %s\n", extNumber.c_str());
        return -1;
    }

    pjsua_call_id callId = PJSUA_INVALID_ID;

    enableEd137 = PJ_FALSE;

    pj_status_t status;

    pjsua_acc_id accId = pjsua_acc_get_default();

    std::string strUrl = extNumber.c_str();
    pj_str_t url;
    pj_cstr(&url, strUrl.c_str());

    printf("Making Sip Call Ext: %s\n", strUrl.c_str());

    status = pjsua_call_make_call(accId, &url, NULL, NULL, NULL, &callId);

    if (status != PJ_SUCCESS)
    {
        stopRing();
        printf("Error on Making Call Ext: %s\n", extNumber.c_str());
        return PJSUA_INVALID_ID;
    }

    return callId;
}

int RoIP_ED137::makeCall(std::string const& extNumber)
{
    registerThread();
    isSipCall = false;
    printf("Making Asterisk Call Ext: %s\n", extNumber.c_str());

    pjsua_call_id callId = PJSUA_INVALID_ID;

    enableEd137 = PJ_FALSE;

    pj_status_t status;

    pjsua_acc_id accId = pjsua_acc_get_default();

    std::string strUrl = getSipUrl(extNumber.c_str());
    pj_str_t url;
    pj_cstr(&url, strUrl.c_str());

    status = pjsua_call_make_call(accId, &url, NULL, NULL, NULL, &callId);

    if (status != PJ_SUCCESS)
    {
        stopRing();
        printf("Error on Making Call Ext: %s\n", extNumber.c_str());
        return PJSUA_INVALID_ID;
    }


    return callId;
}

int RoIP_ED137::makeCall_Radio(std::string const& extNumber)
{
    qDebug() << "makeCall_Radio" << QString::fromStdString(extNumber);
    registerThread();

    pjsua_call_id callId = PJSUA_INVALID_ID;

    enableEd137 = PJ_TRUE;

    pjsua_msg_data msg_data;
    pjsua_msg_data_init(&msg_data);

    // Heade 1
    pj_str_t hname = pj_str((char*)"Priority");
    pj_str_t hvalue = pj_str((char*)"normal");
    pjsip_generic_string_hdr my_hdr3;
    pjsip_generic_string_hdr_init2(&my_hdr3, &hname, &hvalue);
    pj_list_push_back(&msg_data.hdr_list, &my_hdr3);

    // Header 2
    hname = pj_str((char*)"Subject");
    hvalue = pj_str((char*)"radio");
    pjsip_generic_string_hdr my_hdr1;
    pjsip_generic_string_hdr_init2(&my_hdr1, &hname, &hvalue);
    pj_list_push_back(&msg_data.hdr_list, &my_hdr1);

    // Header 3
    hname = pj_str((char*)"WG67-Version");
    hvalue = pj_str((char*)"radio.01;radio.02");
    pjsip_generic_string_hdr my_hdr2;
    pjsip_generic_string_hdr_init2(&my_hdr2, &hname, &hvalue);
    pj_list_push_back(&msg_data.hdr_list, &my_hdr2);


    pj_status_t status;

    pjsua_acc_id accId = pjsua_acc_get_default();

    std::string strUrl = extNumber.c_str();
    pj_str_t url;
    pj_cstr(&url, strUrl.c_str());



    status = pjsua_call_make_call(accId, &url, NULL, NULL, &msg_data, &callId);

    if (status != PJ_SUCCESS)
    {
        printf("Error on Making Radio Call Ext: %s\n", extNumber.c_str());
        return PJSUA_INVALID_ID;
    }

    return callId;
}


/////////////callbacks///////////////////

static void on_reg_state(pjsua_acc_id acc_id)
{
    RoIP_ED137::instance()->on_reg_state(acc_id);
}

void RoIP_ED137::on_reg_state(pjsua_acc_id acc_id)
{
    qDebug() << "on_reg_state";
    pjsua_acc_info ci;

    pj_status_t status = pjsua_acc_get_info(acc_id, &ci);
    if (status != PJ_SUCCESS)
    {
     return;
    }

    if (ci.expires > 0)
    {
        m_Register = true;
    }
    else if(ci.expires < 0)
    {
        m_Register = false;
    }

//    emit stateRegChanged(m_Register);
}
void RoIP_ED137::onRegStateChanged(bool regState)
{


}

void RoIP_ED137::updateActions(QString extnumber, QString action, pjsua_call_id call_id)
{
    std::string extNumber = extnumber.toStdString();
    if (action.contains("Confirmed"))
    {
        sqlOldPinOnStatus =false;
    }

    //if ((extNumber.find(trx1->radio1->callName.toStdString()) != std::string::npos)&(extNumber.find(trx1->radio1->ipAddress.toStdString()) != std::string::npos))
    if ((extnumber == trx1->radio1->url) & (call_id == trx1->radio1->call_id))
    {
        qDebug() << QString::fromStdString(extNumber) << "................................................  " << action << "call ID:" << trx1->radio1->call_id;
        trx1->radio1->CallextNumber = extNumber;
        if (action.contains("Confirmed"))
        {
            if (trx1->radio1->call_id != PJSUA_INVALID_ID){
                trx1->radio1->callState = true;
                trx1->radio1->trxStatus = "";
                trx1->radio1->pptTestPressed = false;
                trx1->radio1->getSNMPCount = 0;
                trx1->radio1->vswr = 0;
                trx1->radio1->frequency = "0";
                trx1->radio1->r2sCount = 0;
                updateHomeDisplay(trx1->radio1->url,CONNECTED,STANDBY,trx1->radio1->call_id,trx1->radio1->radioNodeID);
            }
        }
        else if (action.contains("Connecting"))
        {
            updateHomeDisplay(trx1->radio1->url,CONNECTING,"",trx1->radio1->call_id,trx1->radio1->radioNodeID);
        }
        else if (action.contains("Disconnected"))
        {
            trx1->radio1->callState = false;
            trx1->radio1->connDuration = "0";
            trx1->radio1->m_PttPressed = false;
            updateHomeDisplay(trx1->radio1->url,DISCONNECTED,"",trx1->radio1->call_id,trx1->radio1->radioNodeID);
            if (trx1->radio1->pptTestPressed)
            {
                pttStatus = false;
            }
        }
        else if (action.contains("Calling"))
        {
            updateHomeDisplay(trx1->radio1->url,CALLING,"",trx1->radio1->call_id,trx1->radio1->radioNodeID);
        }
    }
//    else if ((extNumber.find(trx1->radio2->callName.toStdString()) != std::string::npos)&(extNumber.find(trx1->radio2->ipAddress.toStdString()) != std::string::npos))
    else if ((extnumber == trx1->radio2->url) & (call_id == trx1->radio2->call_id))
    {
        qDebug() << QString::fromStdString(extNumber) << "................................................  " << action;
        trx1->radio2->CallextNumber = extNumber;
        if (action.contains("Confirmed")){
            if (trx1->radio2->call_id != PJSUA_INVALID_ID){
                trx1->radio2->trxStatus = "";
                trx1->radio2->callState = true;
                trx1->radio2->pptTestPressed = false;
                trx1->radio2->getSNMPCount = 0;
                trx1->radio2->vswr = 0;
                trx1->radio2->frequency = "0";
                trx1->radio2->r2sCount = 0;
                updateHomeDisplay(trx1->radio2->url,CONNECTED,STANDBY,trx1->radio2->call_id,trx1->radio2->radioNodeID);
            }
        }
        else if (action.contains("Connecting")){
            updateHomeDisplay(trx1->radio2->url,CONNECTING,"",trx1->radio2->call_id,trx1->radio2->radioNodeID);
        }
        else if (action.contains("Disconnected")){
            trx1->radio2->callState = false;
            trx1->radio2->connDuration = "0";
            trx1->radio2->m_PttPressed = false;
            updateHomeDisplay(trx1->radio2->url,DISCONNECTED,"",trx1->radio2->call_id,trx1->radio2->radioNodeID);
            if (trx1->radio2->pptTestPressed){
                pttStatus = false;
            }
        }
        else if (action.contains("Calling")){
            updateHomeDisplay(trx1->radio2->url,CALLING,"",trx1->radio2->call_id,trx1->radio2->radioNodeID);
        }
    }
//    else if ((extNumber.find(trx2->radio1->callName.toStdString()) != std::string::npos)&(extNumber.find(trx2->radio1->ipAddress.toStdString()) != std::string::npos))
    else if ((extnumber == trx2->radio1->url) & (call_id == trx2->radio1->call_id))
    {
        qDebug() << QString::fromStdString(extNumber) << "................................................  " << action;
        trx2->radio1->CallextNumber = extNumber;
        if (action.contains("Confirmed")){
            if (trx2->radio1->call_id != PJSUA_INVALID_ID){
                trx2->radio1->callState = true;
                trx2->radio1->trxStatus = "";
                trx2->radio1->pptTestPressed = false;
                trx2->radio1->getSNMPCount = 0;
                trx2->radio1->vswr = 0;
                trx2->radio1->frequency = "0";
                trx2->radio1->r2sCount = 0;
                updateHomeDisplay(trx2->radio1->url,CONNECTED,STANDBY,trx2->radio1->call_id,trx2->radio1->radioNodeID);
            }
        }
        else if (action.contains("Connecting"))
        {
            updateHomeDisplay(trx2->radio1->url,CONNECTING,"",trx2->radio1->call_id,trx2->radio1->radioNodeID);
        }
        else if (action.contains("Disconnected")){
            trx2->radio1->callState = false;
            trx2->radio1->connDuration = "0";
            trx2->radio1->m_PttPressed = false;
            updateHomeDisplay(trx2->radio1->url,DISCONNECTED,"",trx2->radio1->call_id,trx2->radio1->radioNodeID);
            if (trx2->radio1->pptTestPressed){
                pttStatus = false;
            }
        }
        else if (action.contains("Calling"))
        {
            updateHomeDisplay(trx2->radio1->url,CALLING,"",trx2->radio1->call_id,trx2->radio1->radioNodeID);
        }
    }
//    else if ((extNumber.find(trx2->radio2->callName.toStdString()) != std::string::npos)&(extNumber.find(trx2->radio2->ipAddress.toStdString()) != std::string::npos))
    else if ((extnumber == trx2->radio2->url) & (call_id == trx2->radio2->call_id))
    {
        qDebug() << QString::fromStdString(extNumber) << "................................................  " << action;
        trx2->radio2->CallextNumber = extNumber;
        if (action.contains("Confirmed")){
            if (trx2->radio2->call_id != PJSUA_INVALID_ID){
                trx2->radio2->callState = true;
                trx2->radio2->trxStatus = "";
                trx2->radio2->pptTestPressed = false;
                trx2->radio2->getSNMPCount = 0;
                trx2->radio2->vswr = 0;
                trx2->radio2->frequency = "0";
                trx2->radio2->r2sCount = 0;
                updateHomeDisplay(trx2->radio2->url,CONNECTED,STANDBY,trx2->radio2->call_id,trx2->radio2->radioNodeID);
            }
        }
        else if (action.contains("Connecting")){
            updateHomeDisplay(trx2->radio2->url,CONNECTING,"",trx2->radio2->call_id,trx2->radio2->radioNodeID);
        }
        else if (action.contains("Disconnected")){
            trx2->radio2->callState = false;
            trx2->radio2->connDuration = "0";
            trx2->radio2->m_PttPressed = false;
            updateHomeDisplay(trx2->radio2->url,DISCONNECTED,"",trx2->radio2->call_id,trx2->radio2->radioNodeID);
            if (trx2->radio2->pptTestPressed){
                pttStatus = false;
            }
        }
        else if (action.contains("Calling")){
            updateHomeDisplay(trx2->radio2->url,CALLING,"",trx2->radio2->call_id,trx2->radio2->radioNodeID);
        }
    }
    else
    {
        for (int i = 0;i < trx_incall.length();i++)
        {
            if (action.contains("Confirmed")){
                qDebug() << "updateActions extnumber" << extnumber << i << "................................................  " << trx_incall.at(i)->ext_uriName << "callID" << call_id;
                if (trx_incall.at(i)->ext_uriName == "")
                {
                    trx_incall.at(i)->ext_uriName = extnumber;
                    trx_incall.at(i)->call_id = call_id;
                    trx_incall.at(i)->callState = true;
                    trx_call_hangup(call_id,500,NULL);
                    break;

                }
            }
            if ((QString::fromStdString(extNumber) == trx_incall.at(i)->ext_uriName) & (trx_incall.at(i)->call_id == call_id))
            {
                qDebug() << QString::fromStdString(extNumber) << "Call Income................................................  " << action;
                trx_incall.at(i)->CallextNumber = extNumber;
                if (action.contains("Confirmed")){
                    if (trx_incall.at(i)->call_id != PJSUA_INVALID_ID){
                        trx_incall.at(i)->callState = true;
                        trx_incall.at(i)->r2sCount = 0;
                        trx_incall.at(i)->m_PttPressed = false;
                        trx_incall.at(i)->sqlTestPressed = false;
                        trx_incall.at(i)->pptTestPressed = false;
                        trx_incall.at(i)->connDuration = "0";
                        trx_incall.at(i)->sec_connDuration = 0;
                        //updateHomeDisplay(trx_incall.at(i)->callIndexName,CONNECTED,"",trx_incall.at(i)->call_id,0);
                    }

                }
                else if (action.contains("Connecting")){
                    updateHomeDisplay(trx_incall.at(i)->callIndexName,CONNECTING,"",trx_incall.at(i)->call_id,0);
                }
                else if (action.contains("Disconnected")){
                    trx_incall.at(i)->callState = false;
                    trx_incall.at(i)->connDuration = "0";
                    trx_incall.at(i)->sec_connDuration = 0;
//                    trx_incall.at(i)->m_PttPressed = false;

//                    if (trx_incall.at(i)->pptTestPressed)
//                    {
//                        pttStatus = false;
//                    }

                    updateHomeDisplay(trx_incall.at(i)->callIndexName,DISCONNECTED,"",trx_incall.at(i)->call_id,0);
                    if (trx_incall.at(i)->callName == recorderAlloweURI)
                    {
                        recorderConnected = false;
                    }
                }
                else if (action.contains("Calling")){
                    updateHomeDisplay(trx_incall.at(i)->callIndexName,CALLING,"",trx_incall.at(i)->call_id,0);
                }
                break;
            }
        }
    }

}

void RoIP_ED137::printCallState(int state,std::string extNumber)
{
    switch (state)
    {
        case PJSIP_INV_STATE_EARLY:
            snprintf(log_buffer, sizeof(log_buffer),"Call state Early: %s",extNumber.c_str());
            //AppentTextBrowser(log_buffer);
            qDebug() << log_buffer;
            break;
        case PJSIP_INV_STATE_CALLING:
            snprintf(log_buffer, sizeof(log_buffer),"Call state Calling: %s",extNumber.c_str());
            //AppentTextBrowser(log_buffer);
            qDebug() << log_buffer;
            break;
        case PJSIP_INV_STATE_INCOMING:
            snprintf(log_buffer, sizeof(log_buffer),"Call state Incoming: %s",extNumber.c_str());
            qDebug() << log_buffer;
            //AppentTextBrowser(log_buffer);
            break;
        case PJSIP_INV_STATE_CONNECTING:
            snprintf(log_buffer, sizeof(log_buffer),"Call state Connecting: %s",extNumber.c_str());
            //AppentTextBrowser(log_buffer);
            qDebug() << log_buffer;
            break;
        case PJSIP_INV_STATE_CONFIRMED:
            snprintf(log_buffer, sizeof(log_buffer),"Call state Confirmed: %s",extNumber.c_str());
            //AppentTextBrowser(log_buffer);
            qDebug() << log_buffer;
            break;
        case PJSIP_INV_STATE_DISCONNECTED:
            snprintf(log_buffer, sizeof(log_buffer),"Call state Disconnected: %s",extNumber.c_str());
            //AppentTextBrowser(log_buffer);
            qDebug() << log_buffer;
            break;
    default:
        break;
    }
}

void RoIP_ED137::callStateChanged(pjsua_call_id call_id, pjsip_event *e)
{
    qDebug() << call_id << e->type << "..........................................................callStateChanged";
    QString myString;
    QStringList rxMsgList;
    pjsua_call_info call_info;
    pjsua_call_get_info(call_id, &call_info);
    //qDebug() << "RoIP_ED137::callStateChanged" << QString::fromUtf8(e->body.tsx_state.src.rdata->msg_info.msg_buf);
    std::string extNumber;
    //int clientPort = getConfPortNumber(call_id);

    switch (e->type){
    case PJSIP_EVENT_UNKNOWN:
        qDebug() << "/** Unidentified event. */";
        break;

    case PJSIP_EVENT_TIMER :
        qDebug() << "/** Timer event, normally only used internally in transaction. */";
        break;

    case PJSIP_EVENT_TX_MSG:
        qDebug() << "/** Message transmission event. */";
        break;

    case PJSIP_EVENT_RX_MSG:
        qDebug() << "/** Message received event. */";
        break;

    case PJSIP_EVENT_TRANSPORT_ERROR:
        qDebug() << "/** Transport error event. */";
        break;

    case PJSIP_EVENT_TSX_STATE:
        qDebug() << "/** Transaction state changed event. */";
        switch (e->body.tsx_state.type) {
        case PJSIP_EVENT_TX_MSG:
            qDebug() << "PJSIP_EVENT_TX_MSG";
            break;
        case PJSIP_EVENT_RX_MSG:
            qDebug() << "PJSIP_EVENT_RX_MSG";
            myString = QString::fromUtf8(e->body.tsx_state.src.rdata->msg_info.msg_buf);
            myString = myString.replace("\r","");
            rxMsgList = myString.split("\n");
            if (inviteMode == SERVER)
            {
                if (call_id == trx1->radio1->call_id)// ((myString.contains(trx1->radio1->callName))&(myString.contains(trx1->radio1->ipAddress)))
                {
                    if (myString.contains("200 OK"))
                    {
                        trx1->radio1->sdpAnswer = rxMsgList;
                        foreach(QString msgList, rxMsgList) {
                            if (msgList.contains("R2S-KeepAlivePeriod")){
                                if (msgList.split(":").size() >= 2)
                                    trx1->radio1->r2sPeriod = QString(msgList.split(":").at(1)).toInt();
                                qDebug() << "R2S-KeepAlivePeriod" << trx1->radio1->r2sPeriod;
                            }
                            else if(msgList.contains("m=audio")){
                                if (msgList.split(" ").size() >= 2)
                                    trx1->radio1->RTPPort = QString(msgList.split(" ").at(1));
                                qDebug() << "RTP Port: " << trx1->radio1->RTPPort;
                            }
                        }
                    }
                    else if (myString.contains("BYE")){
                        trx1->radio1->call_id = PJSUA_INVALID_ID;
                        trx1->radio1->callState = false;
    //                    trx1->radio1->trxFailed = true;
                        pttStatus = false;
                        trx1->radio1->ByeReason = "";
                        trx1->radio1->ByeAnswer = rxMsgList;
                        foreach(QString msgList, rxMsgList) {
                            if (msgList.contains("Reason:")){
                                if (msgList.split(":").size() >= 2){
                                    QString reason = (msgList.split(":")).at(1);
                                    QStringList reasonList = reason.split(";");
                                    foreach (QString text, reasonList) {
                                        if (text.contains("text")){
                                            text = QString(text.replace("\"","")).split("=").at(1);
                                            trx1->radio1->ByeReason = text;
                                            qDebug() << "BYE" << trx1->radio1->ByeReason;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (trx1->radio1->ByeReason != "") break;
                        }
                        if (trx1->radio1->ByeReason == "") trx1->radio1->ByeReason = "BYE";
                    }

                }
                else if (call_id == trx1->radio2->call_id)// ((myString.contains(trx1->radio2->callName))&(myString.contains(trx1->radio2->ipAddress)))
                {
                    if (myString.contains("200 OK"))
                    {
                        trx1->radio2->sdpAnswer = rxMsgList;
                        foreach(QString msgList, rxMsgList) {
                            if (msgList.contains("R2S-KeepAlivePeriod")){
                                if (msgList.split(":").size() >= 2)
                                    trx1->radio2->r2sPeriod = QString(msgList.split(":").at(1)).toInt();
                                qDebug() << "R2S-KeepAlivePeriod" << trx1->radio2->r2sPeriod;
                            }
                            else if(msgList.contains("m=audio")){
                                if (msgList.split(" ").size() >= 2)
                                    trx1->radio2->RTPPort = QString(msgList.split(" ").at(1));
                                qDebug() << "RTP Port: " << trx1->radio2->RTPPort;
                            }
                        }
                    }
                    else if (myString.contains("BYE")){
                        trx1->radio2->call_id = PJSUA_INVALID_ID;
                        trx1->radio2->callState = false;
    //                    trx1->radio2->trxFailed = true;
                        pttStatus = false;
                        trx1->radio2->ByeReason = "";
                        trx1->radio2->ByeAnswer = rxMsgList;
                        foreach(QString msgList, rxMsgList) {
                            if (msgList.contains("Reason:")){
                                if (msgList.split(":").size() >= 2){
                                    QString reason = (msgList.split(":")).at(1);
                                    QStringList reasonList = reason.split(";");
                                    foreach (QString text, reasonList) {
                                        if (text.contains("text")){
                                            text = QString(text.replace("\"","")).split("=").at(1);
                                            trx1->radio2->ByeReason = text;
                                            qDebug() << QString::fromStdString(extNumber) << trx1->radio2->ByeReason;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (trx1->radio2->ByeReason != "") break;
                        }
                        if (trx1->radio2->ByeReason == "") trx1->radio2->ByeReason = "BYE";
                    }

                }
                else if (call_id == trx2->radio1->call_id)// ((myString.contains(trx2->radio1->callName))&(myString.contains(trx2->radio1->ipAddress)))
                {
                    if (myString.contains("200 OK"))
                    {
                        trx2->radio1->sdpAnswer = rxMsgList;
                        foreach(QString msgList, rxMsgList) {
                            if (msgList.contains("R2S-KeepAlivePeriod")){
                                if (msgList.split(":").size() >= 2)
                                    trx2->radio1->r2sPeriod = QString(msgList.split(":").at(1)).toInt();
                                qDebug() << "R2S-KeepAlivePeriod" << trx2->radio1->r2sPeriod;
                            }
                            else if(msgList.contains("m=audio")){
                                if (msgList.split(" ").size() >= 2)
                                    trx2->radio1->RTPPort = QString(msgList.split(" ").at(1));
                                qDebug() << "RTP Port: " << trx2->radio1->RTPPort;
                            }
                        }
                    }
                    else if (myString.contains("BYE")){
                        trx2->radio1->call_id = PJSUA_INVALID_ID;
                        trx2->radio1->callState = false;
    //                    trx2->radio1->trxFailed = true;
                        pttStatus = false;
                        trx2->radio1->ByeReason = "";
                        trx2->radio1->ByeAnswer = rxMsgList;
                        foreach(QString msgList, rxMsgList) {
                            if (msgList.contains("Reason:")){
                                if (msgList.split(":").size() >= 2){
                                    QString reason = (msgList.split(":")).at(1);
                                    QStringList reasonList = reason.split(";");
                                    foreach (QString text, reasonList) {
                                        if (text.contains("text")){
                                            text = QString(text.replace("\"","")).split("=").at(1);
                                            trx2->radio1->ByeReason = text;
                                            qDebug() << QString::fromStdString(extNumber) << trx2->radio1->ByeReason;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (trx2->radio1->ByeReason != "") break;
                        }
                        if (trx2->radio1->ByeReason == "") trx2->radio1->ByeReason = "BYE";
                    }
                }
                else if (call_id == trx2->radio2->call_id)//((myString.contains(trx2->radio2->callName))&(myString.contains(trx2->radio2->ipAddress)))
                {
                    if (myString.contains("200 OK"))
                    {
                        trx2->radio2->sdpAnswer = rxMsgList;
                        foreach(QString msgList, rxMsgList) {
                            if (msgList.contains("R2S-KeepAlivePeriod")){
                                if (msgList.split(":").size() >= 2)
                                    trx2->radio2->r2sPeriod = QString(msgList.split(":").at(1)).toInt();
                                qDebug() << "R2S-KeepAlivePeriod" << trx2->radio2->r2sPeriod;
                            }
                            else if(msgList.contains("m=audio")){
                                if (msgList.split(" ").size() >= 2)
                                    trx2->radio2->RTPPort = QString(msgList.split(" ").at(1));
                                qDebug() << "RTP Port: " << trx2->radio2->RTPPort;
                            }
                        }
                    }
                    else if (myString.contains("BYE")){
                        trx2->radio2->call_id = PJSUA_INVALID_ID;
                        trx2->radio2->callState = false;
    //                    trx2->radio2->trxFailed = true;
                        pttStatus = false;
                        trx2->radio2->ByeReason = "";
                        trx2->radio2->ByeAnswer = rxMsgList;
                        foreach(QString msgList, rxMsgList) {
                            if (msgList.contains("Reason:")){
                                if (msgList.split(":").size() >= 2){
                                    QString reason = (msgList.split(":")).at(1);
                                    QStringList reasonList = reason.split(";");
                                    foreach (QString text, reasonList) {
                                        if (text.contains("text")){
                                            text = QString(text.replace("\"","")).split("=").at(1);
                                            trx2->radio2->ByeReason = text;
                                            qDebug() << QString::fromStdString(extNumber) << trx2->radio2->ByeReason;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (trx2->radio2->ByeReason != "") break;
                        }
                        if (trx2->radio2->ByeReason == "") trx2->radio2->ByeReason = "BYE";
                    }
                }
            }
            else
            {
                for (int i = 0;i < trx_incall.length();i++)
                {
                    if (call_id == trx_incall.at(i)->call_id)
                    {
                         if (myString.contains("200 OK"))
                         {
                             qDebug() << "PJSIP_EVENT_RX_MSG" << myString;
                             trx_incall.at(i)->sdpAnswer = rxMsgList;
                             foreach(QString msgList, rxMsgList) {
                                 if (msgList.contains("R2S-KeepAlivePeriod")){
                                     if (msgList.split(":").size() >= 2)
                                         trx_incall.at(i)->r2sPeriod = QString(msgList.split(":").at(1)).toInt();
                                     qDebug() << "R2S-KeepAlivePeriod" << trx_incall.at(i)->r2sPeriod;
                                 }
                                 else if(msgList.contains("m=audio")){
                                     if (msgList.split(" ").size() >= 2)
                                         trx_incall.at(i)->RTPPort = QString(msgList.split(":").at(1));
                                     qDebug() << "RTP Port: " << trx_incall.at(i)->RTPPort;
                                 }
                                 else if(msgList.contains("a=type")){
                                     if (msgList.split(":").size() >= 2)
                                         trx_incall.at(i)->type = QString(msgList.split(":").at(1));
                                     qDebug() << "Radio type: " << trx_incall.at(i)->type;
                                 }
                                 else if(msgList.contains("a=txrxmode"))
                                 {
                                     if (msgList.split(":").size() >= 2)
                                         trx_incall.at(i)->txrxmode = QString(msgList.split(":").at(1));
                                     qDebug() << "txrxmode: " << trx_incall.at(i)->txrxmode;
                                 }
                             }
                         }
                         else if (myString.contains("BYE"))
                         {
                             qDebug() << call_id << trx_incall.at(i)->ext_uriName << "..........................................................callStateChanged" << "BYE";
                             trx_incall.at(i)->call_id = PJSUA_INVALID_ID;
                             trx_incall.at(i)->callState = false;
                             trx_incall.at(i)->trxFailed = true;
                             pttStatus = false;
                             trx_incall.at(i)->ByeReason = "";
                             trx_incall.at(i)->ByeAnswer = rxMsgList;
                             foreach(QString msgList, rxMsgList)
                             {
                                 if (msgList.contains("Reason:"))
                                 {
                                     if (msgList.split(":").size() >= 2)
                                     {
                                         QString reason = (msgList.split(":")).at(1);
                                         QStringList reasonList = reason.split(";");
                                         foreach (QString text, reasonList) {
                                             if (text.contains("text")){
                                                 text = QString(text.replace("\"","")).split("=").at(1);
                                                 trx_incall.at(i)->ByeReason = text;
                                                 qDebug() << trx_incall.at(i)->ext_uriName << trx_incall.at(i)->ByeReason;
                                                 break;
                                             }
                                         }
                                     }
                                 }
                                 if (trx_incall.at(i)->ByeReason != "") break;
                             }
                             if (trx_incall.at(i)->ByeReason == "") trx_incall.at(i)->ByeReason = "BYE";
                         }
                    }
                }
            }
            break;
        case PJSIP_EVENT_TRANSPORT_ERROR:
            qDebug() << "PJSIP_EVENT_TRANSPORT_ERROR";
            break;
        case PJSIP_EVENT_USER:
            qDebug() << "PJSIP_EVENT_USER";
            break;
        case PJSIP_EVENT_TIMER:
            qDebug() << "PJSIP_EVENT_TIMER";
            break;
        default:
            qDebug() << "\n\n";
            break;
        }
        break;

    case PJSIP_EVENT_USER:
        qDebug() << "/** Indicates that the event was triggered by user action. */";
        break;

    default:
        qDebug() << "/** Unidentified event. */";
        break;

    }

    if(getExtFromCallID(call_id,extNumber))
    {
       printCallState(call_info.state,extNumber);
    }
    else
        return;

    switch (call_info.state)
    {
        case PJSIP_INV_STATE_EARLY:
        qDebug() << "PJSIP_INV_STATE_EARLY";
            if (e->type == PJSIP_EVENT_TSX_STATE)
            {
                pjsip_msg *msg;

                if (e->body.tsx_state.type == PJSIP_EVENT_RX_MSG)
                {
                    msg = e->body.tsx_state.src.rdata->msg_info.msg;
                }
                else {
                    msg = e->body.tsx_state.src.tdata->msg;
                }

                int code = msg->line.status.code;

                // Start ringback for 180 for UAC unless there's SDP in 180
                if (call_info.role == PJSIP_ROLE_UAC && code == 180  && msg->body == NULL && call_info.media_status == PJSUA_CALL_MEDIA_NONE)
                {
                    playRing();
                }
            }

            break;
        case PJSIP_INV_STATE_CALLING:
            updateActions(QString::fromStdString(extNumber),"Calling", call_id);
            break;
        case PJSIP_INV_STATE_INCOMING:
//            playRing();
            break;
        case PJSIP_INV_STATE_CONNECTING:
            updateActions(QString::fromStdString(extNumber),"Connecting", call_id);
            break;
        case PJSIP_INV_STATE_CONFIRMED:
            stopRing();
            updateActions(QString::fromStdString(extNumber),"Confirmed", call_id);

            pjsua_conf_port_info portInfo;
            getCallPortInfo(extNumber, portInfo);

            sampleRate = portInfo.clock_rate;

            if(sampleRate != PJSUA_INVALID_ID)
            {
                snprintf(log_buffer, sizeof(log_buffer),
                "\nCall Activated for Ext#: %s\nClock Rate: %d\nChannel Count: %d\nSamples Per Frame: %d\nBits Per Sample: %d\n",
                extNumber.c_str(),portInfo.clock_rate,portInfo.channel_count,portInfo.samples_per_frame,portInfo.bits_per_sample);
                //AppentTextBrowser(log_buffer);
            }

//            if (m_EnableRecording)
//            {
//                m_WavWriter->start(extNumber,sampleRate);
//                connectPort(clientPort,m_NullSlot);
//            }

            break;
        case PJSIP_INV_STATE_DISCONNECTED:
            stopRing();
            updateActions(QString::fromStdString(extNumber),"Disconnected", call_id);

//            if ((extNumber.find(trx1->radio1->callName.toStdString()) != std::string::npos)&(extNumber.find(trx1->radio1->ipAddress.toStdString()) != std::string::npos))
            if ((QString::fromStdString(extNumber) == trx1->radio1->ext_uriName)&(trx1->radio1->call_id == call_id))
            {
                trx1->radio1->call_id = PJSUA_INVALID_ID;
                if(m_AutoCall)
                {
                    QString calluri = trx1->radio1->url;
                    char sip_urichr[calluri.length()+1];
                    strcpy( sip_urichr, calluri.toStdString().c_str());

                    trx1->radio1->call_id = makeCall_Radio(sip_urichr);
                }
            }
//            else if ((extNumber.find(trx1->radio2->callName.toStdString()) != std::string::npos)&(extNumber.find(trx1->radio2->ipAddress.toStdString()) != std::string::npos))
            else if ((QString::fromStdString(extNumber) == trx1->radio2->ext_uriName)&(trx1->radio2->call_id == call_id))
            {
                trx1->radio2->call_id = PJSUA_INVALID_ID;
                if(m_AutoCall)
                {
                    QString calluri = trx1->radio2->url;
                    char sip_urichr[calluri.length()+1];
                    strcpy( sip_urichr, calluri.toStdString().c_str());

                    trx1->radio2->call_id = makeCall_Radio(sip_urichr);
                }
            }
//            else if ((extNumber.find(trx2->radio1->callName.toStdString()) != std::string::npos)&(extNumber.find(trx2->radio1->ipAddress.toStdString()) != std::string::npos))
            else if ((QString::fromStdString(extNumber) == trx2->radio1->ext_uriName)&(trx2->radio1->call_id == call_id))
            {
                trx2->radio1->call_id = PJSUA_INVALID_ID;
                if(m_AutoCall)
                {
                    QString calluri = trx2->radio1->url;
                    char sip_urichr[calluri.length()+1];
                    strcpy( sip_urichr, calluri.toStdString().c_str());

                    trx2->radio1->call_id = makeCall_Radio(sip_urichr);
                }
            }
//            else if ((extNumber.find(trx2->radio2->callName.toStdString()) != std::string::npos)&(extNumber.find(trx2->radio2->ipAddress.toStdString()) != std::string::npos))
            else if ((QString::fromStdString(extNumber) == trx2->radio2->ext_uriName)&(trx2->radio2->call_id == call_id))
            {
                trx2->radio2->call_id = PJSUA_INVALID_ID;
                if(m_AutoCall)
                {
                    QString calluri = trx2->radio2->url;
                    char sip_urichr[calluri.length()+1];
                    strcpy( sip_urichr, calluri.toStdString().c_str());

                    trx2->radio2->call_id = makeCall_Radio(sip_urichr);
                }
            }
            else{
                for (int i = 0;i < trx_incall.length();i++){
                    if ((QString::fromStdString(extNumber) == trx_incall.at(i)->ext_uriName)&(trx_incall.at(i)->call_id == call_id))
                    {
                         trx_incall.at(i)->call_id = PJSUA_INVALID_ID;
                    }
                }
            }


            {
                std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(call_id) ;
                if( iter != transport_map.end())
                {
                    transport_map.erase( iter );
                }
            }

            break;
    default:
        break;
    }

}

static void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
    RoIP_ED137::instance()->on_call_state(call_id, e);
}

void RoIP_ED137::on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
    callStateChanged(call_id,e);
}

////////////////////////////////////////
static void on_incoming_subscribe(pjsua_acc_id acc_id,pjsua_srv_pres *srv_pres,pjsua_buddy_id buddy_id,const pj_str_t *from,pjsip_rx_data *rdata,pjsip_status_code *code,pj_str_t *reason,pjsua_msg_data *msg_data)
{
    qDebug() << "...............................................................on_incoming_subscribe";
    RoIP_ED137::instance()->on_incoming_subscribe(acc_id,srv_pres,buddy_id, from,rdata,code,reason,msg_data);
}
void RoIP_ED137::on_incoming_subscribe(pjsua_acc_id acc_id,pjsua_srv_pres *srv_pres,pjsua_buddy_id buddy_id,const pj_str_t *from,pjsip_rx_data *rdata,pjsip_status_code *code,pj_str_t *reason,pjsua_msg_data *msg_data)
{
    qDebug() << "...............................................................on_incoming_subscribe";
}
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
                             pjsip_rx_data *rdata)
{
    qDebug() << "...............................................................on_incoming_call";
    RoIP_ED137::instance()->on_incoming_call(acc_id, call_id, rdata);
}

////* Callback called by the library upon receiving incoming call */
void RoIP_ED137::on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
                             pjsip_rx_data *rdata)
{    
    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);
    pjsua_call_info call_info;
    pjsua_call_get_info(call_id, &call_info);
    bool urlexit = false;
    bool urlok = false;
    // Response Ringing = 200
    QString ext_uriName;
    QString org_ext_uriName;
    QString local_uriName;
    QString ipAddress;
    std::string extNumber;
    std::string localNumber;
    QString txrxmode;
    QString radiotype;
    urlList *newuri;
    newuri = new urlList;
    QString myString = QString::fromUtf8(rdata->msg_info.msg_buf);
    myString = myString.replace("\r","");
    QStringList rxMsgList = myString.split("\n");

    foreach(QString msgList, rxMsgList)
    {
        if(msgList.contains("a=type"))
        {
            if (msgList.split(":").size() >= 2)
                radiotype = QString(msgList.split(":").at(1));
            qDebug() << msgList << "Radio type: " << radiotype;
        }
        else if(msgList.contains("a=txrxmode"))
        {
            if (msgList.split(":").size() >= 2)
                txrxmode = QString(msgList.split(":").at(1));
            qDebug() << msgList << "txrxmode: " << txrxmode;
        }
    }

    setRadioTxrxModeAndRadioType(call_id,txrxmode,radiotype);

    snprintf(log_buffer, sizeof(log_buffer), " %s",call_info.remote_info.ptr);
    qDebug() << "Incoming Call:" << log_buffer << "callID" << call_id;



    if(getExtFromCallID(call_id,extNumber))
    {       

        int i = 0;
        ext_uriName = QString::fromStdString(extNumber);
        newuri->uri = ext_uriName;
        newuri->txrxmode = txrxmode;
        newuri->radiotype = radiotype;
        foreach(urlList *uri, urllist)
        {
            if ((uri->uri == ext_uriName) & (urllist.at(i)->txrxmode == txrxmode) & (urllist.at(i)->radiotype == radiotype))
            {
                urlexit = true;
                urlok = true;
                break;
            }
            else if (uri->uri == ext_uriName)
            {
                urlexit = true;
                urllist.at(i)->uri = ext_uriName;
                urllist.at(i)->txrxmode = txrxmode;
                urllist.at(i)->radiotype = radiotype;
                break;
            }
            i++;
        }
        if (urlexit == false)
        {
            urllist.append(newuri);
        }

       org_ext_uriName = ext_uriName;
       while(ext_uriName.mid(0,4) != "<sip")
       {

            ext_uriName = ext_uriName.mid(1,(ext_uriName.length()));
            if (ext_uriName.length() <= 4) break;
       }
       if (ext_uriName.mid(0,4) == "<sip"){
           if (ext_uriName.split("@").size() >=2 ){
               ext_uriName = ext_uriName.split("@").at(0);
               ipAddress  = org_ext_uriName.split("@").at(1);
               if (ipAddress.contains(">")){
                   while(ipAddress.mid(ipAddress.length()-1,1) != ">"){
                        ipAddress.chop(1);
                        if (ipAddress.length() < 7){
                            ipAddress = "";
                            break;
                        }
                   }
               }
               ext_uriName = ext_uriName.replace("<sip:","");
               qDebug() << "ext_uriName" << ext_uriName << "ipAddress" << ipAddress;
               printCallState(call_info.state,extNumber);
           }
       }
    }
    if(getLocalNumberFromCallID(call_id,localNumber))
    {
       printCallState(call_info.state,localNumber);
       local_uriName = QString::fromStdString(localNumber);
       qDebug() << "local_uriName" << local_uriName;
       local_uriName = local_uriName.split("@").at(0);
       local_uriName = local_uriName.replace("<sip:","");
    }
    if(call_id!= PJSUA_INVALID_ID){
        bool callanswer = false;
        pjsua_msg_data msg_data;
        pjsua_msg_data_init(&msg_data);

//        // Heade 0
//        pj_str_t hname = pj_str((char*)"Allow");
//        pj_str_t hvalue = pj_str((char*)"INVITE, ACK, BYE, CANCEL, SUBSCRIBE");
//        pjsip_generic_string_hdr my_hdr;
//        pjsip_generic_string_hdr_init2(&my_hdr, &hname, &hvalue);
//        pj_list_push_back(&msg_data.hdr_list, &my_hdr);

//        hname = pj_str((char*)"Supported");
//        hvalue = pj_str((char*)"");
//        pjsip_generic_string_hdr my_hdr0;
//        pjsip_generic_string_hdr_init2(&my_hdr0, &hname, &hvalue);
//        pj_list_push_back(&msg_data.hdr_list, &my_hdr0);

        // Heade 1
        pj_str_t hname = pj_str((char*)"Priority");
        pj_str_t hvalue = pj_str((char*)"normal");
        pjsip_generic_string_hdr my_hdr3;
        pjsip_generic_string_hdr_init2(&my_hdr3, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr3);

        // Header 2
        hname = pj_str((char*)"Subject");
        hvalue = pj_str((char*)"radio");
        pjsip_generic_string_hdr my_hdr1;
        pjsip_generic_string_hdr_init2(&my_hdr1, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr1);

        // Header 3
        hname = pj_str((char*)"WG67-Version");
        hvalue = pj_str((char*)"radio.01");
        pjsip_generic_string_hdr my_hdr2;
        pjsip_generic_string_hdr_init2(&my_hdr2, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr2);


        // Header 4
        hname = pj_str((char*)"user-agent");
        hvalue = pj_str((char*)"PAS Radio");
        pjsip_generic_string_hdr my_hdr4;
        pjsip_generic_string_hdr_init2(&my_hdr4, &hname, &hvalue);
        pj_list_push_back(&msg_data.hdr_list, &my_hdr4);



//        qDebug() << "msg_data.content_type.ptr" << QString(msg_data.content_type.ptr);

        if ((ext_uriName == recorderAlloweURI) & (recorderSuported))
        {
            int i = 0;
            for (i=0; i<trx_incall.length(); i++){
                if (trx_incall.at(i)->callState == false) break;
            }
            if (i < trx_incall.length())
            {
                callanswer = true;
                qDebug() << "callanswer" << callanswer;
                trx_incall.at(i)->on_call_audio_state_ini = false;
                trx_incall.at(i)->call_id = call_id;
//                trx_incall.at(i)->callState = true;
                trx_incall.at(i)->txrxmode = txrxmode;
                trx_incall.at(i)->type = radiotype;

//                callInNum++;
                callInNum = getActiveCallCount();
                recorderConnected = true;
                trx_incall.at(i)->ext_uriName = org_ext_uriName;
                trx_incall.at(i)->callName = ext_uriName;
                trx_incall.at(i)->ipAddress = ipAddress.replace(">","");
                trx_incall.at(i)->url = trx_incall.at(i)->callName+"@"+trx_incall.at(i)->ipAddress;

                if (local_uriName == QString::fromStdString(SipUser))
                {
                    pj_status_t status =  pjsua_call_answer(call_id, 200, NULL, &msg_data);
                    qDebug() << "pjsua_call_answer 200 Ok msg_body" << QString(msg_data.msg_body.ptr);
                    if (status != PJ_SUCCESS)
                    {
                        printf("Error in status ringing\n");
                    }
                    else
                    {
                        qDebug() << "answed Recorder Incoming Call:" << recorderAlloweURI;
                    }
                }
                else
                {
                    pj_status_t status =  pjsua_call_answer(call_id, 404, NULL, &msg_data);
                    if (status != PJ_SUCCESS)
                    {
                        printf("Error in status ringing\n");
                    }
                }
            }
        }

        else if ((inviteMode == CLIENT)&(callInNum < numConnection))
        {
            foreach (QString allowUri, uriNameAllowList)
            {
                if ((ext_uriName == allowUri) & (urlok == true))
                {
                    int i = 0;
                    for (i=0; i<trx_incall.length(); i++)
                    {
                        if (trx_incall.at(i)->callState == false) break;
                    }
                    if (i < trx_incall.length())
                    {
                        trx_incall.at(i)->on_call_audio_state_ini = false;
                        callanswer = true;
//                        trx_incall.at(i)->callState = true;
//                        callInNum++;
                        callInNum = getActiveCallCount();
                        trx_incall.at(i)->call_id = call_id;
                        trx_incall.at(i)->ext_uriName = org_ext_uriName;
                        trx_incall.at(i)->callName = ext_uriName;
                        trx_incall.at(i)->ipAddress = ipAddress.replace(">","");
                        trx_incall.at(i)->url = trx_incall.at(i)->callName+"@"+trx_incall.at(i)->ipAddress;
                        trx_incall.at(i)->txrxmode = txrxmode;
                        trx_incall.at(i)->type = radiotype;
                        if (local_uriName == QString::fromStdString(SipUser))
                        {
                            qDebug() << "callanswer" << callanswer;
                            pj_status_t status =  pjsua_call_answer(call_id, 200, NULL, &msg_data);
                            qDebug() << "pjsua_call_answer 200 Ok msg_body" << QString(msg_data.msg_body.ptr);
                            if (status != PJ_SUCCESS)
                            {
                                printf("Error in status ringing\n");
                            }
                        }
                        else
                        {
                            pj_status_t status =  pjsua_call_answer(call_id, 404, NULL, &msg_data);
                            if (status != PJ_SUCCESS)
                            {
                                printf("Error in status ringing\n");
                            }
                        }
                        break;
                    }
                }
            }
        }
        else if ((inviteMode == CLIENT_RPT)&(callInNum < numConnection))
        {
            foreach (QString allowUri, uriNameAllowList)
            {
                if ((ext_uriName == allowUri) & (urlok == true))
                {
                    int i = 0;
                    for (i=0; i<trx_incall.length(); i++){
                        if (trx_incall.at(i)->callState == false) break;
                    }
                    if (i < trx_incall.length())
                    {
                        callanswer = true;
                        trx_incall.at(i)->call_id = call_id;
//                        trx_incall.at(i)->callState = true;
//                        callInNum++;
                        callInNum = getActiveCallCount();
                        trx_incall.at(i)->ext_uriName = org_ext_uriName;
                        trx_incall.at(i)->callName = ext_uriName;
                        trx_incall.at(i)->ipAddress = ipAddress.replace(">","");
                        trx_incall.at(i)->url = trx_incall.at(i)->callName+"@"+trx_incall.at(i)->ipAddress;
                        trx_incall.at(i)->txrxmode = txrxmode;
                        trx_incall.at(i)->type = radiotype;
                        if (local_uriName == QString::fromStdString(SipUser))
                        {
                            pj_status_t status =  pjsua_call_answer(call_id, 200, NULL, &msg_data);
                            qDebug() << "pjsua_call_answer 200 Ok";
                            if (status != PJ_SUCCESS)
                            {
                                printf("Error in status ringing\n");
                            }
                        }
                        else {
                            pj_status_t status =  pjsua_call_answer(call_id, 404, NULL, &msg_data);
                            if (status != PJ_SUCCESS)
                            {
                                printf("Error in status ringing\n");
                            }
                        }
                        break;
                    }
                }
            }
        }

        if(!callanswer)
        {
            pj_status_t status =  pjsua_call_answer(call_id, 603, NULL, &msg_data);
            if (status != PJ_SUCCESS)
            {
                printf("Error in status ringing\n");
            }
        }
    }


    //AppentTextBrowser(log_buffer);

}

//////////////////////////////////////////

static void on_call_media_state(pjsua_call_id call_id)
{
    RoIP_ED137::instance()->on_call_media_state(call_id);
}

/* Callback called by the library when call's media state has changed */
void RoIP_ED137::on_call_media_state(pjsua_call_id call_id)
{    
    pjsua_call_info call_info;
//    qDebug() << "on_call_media_state"<< "call_info.media_cnt" << call_info.media_cnt << "call_id" << call_id;
    unsigned mi;
    pj_bool_t has_error = PJ_FALSE;

    pjsua_call_get_info(call_id, &call_info);

    for (mi=0; mi<call_info.media_cnt; ++mi)
    {

    switch (call_info.media[mi].type) {
    case PJMEDIA_TYPE_AUDIO:
       on_call_audio_state(&call_info, mi, &has_error, call_id);

       break;
    default:
       break;
    }
    }

    if (has_error) {
    pj_str_t reason = pj_str((char*)("Media failed"));
    pjsua_call_hangup(call_id, 500, &reason, NULL);
    }
}

void RoIP_ED137::on_call_audio_state(pjsua_call_info *ci, unsigned mi,pj_bool_t *has_error,pjsua_call_id call_id)
{
//    qDebug() << "on_call_audio_state";
//    if (on_call_audio_state_ini == true) return;
//    on_call_audio_state_ini = true;
    if ((inviteMode == CLIENT) || (inviteMode == CLIENT_RPT))
    {
        for (int i = 0;i < trx_incall.length(); i++)
        {
            if (trx_incall.at(i)->call_id == call_id)
            {
                if (trx_incall.at(i)->on_call_audio_state_ini == true) return;
                trx_incall.at(i)->on_call_audio_state_ini = true;
            }
        }
    }
    PJ_UNUSED_ARG(has_error);

    if (ci->media[mi].status == PJSUA_CALL_MEDIA_ACTIVE ||
    ci->media[mi].status == PJSUA_CALL_MEDIA_REMOTE_HOLD)
    {
        pjsua_conf_port_id call_conf_slot;
        call_conf_slot = ci->media[mi].stream.aud.conf_slot;

       // pjsua_conf_connect(call_conf_slot, 0);
       // pjsua_conf_connect(0,call_conf_slot);


        std::string strRemoteInfo;
        strRemoteInfo.append(ci->remote_info.ptr, ci->remote_info.slen);
        qDebug() << "strRemoteInfo................." << QString::fromStdString(strRemoteInfo);
//        if (strRemoteInfo.find("S4") != std::string::npos)
        {
             pjsua_conf_connect(call_conf_slot, sc_ch1_slot);
             pjsua_conf_connect(sc_ch1_slot,call_conf_slot);

             pjsua_conf_connect(call_conf_slot, sc_ch2_slot);
             pjsua_conf_connect(sc_ch2_slot,call_conf_slot);

             pjsua_conf_connect(call_conf_slot, sc_ch3_slot);
             pjsua_conf_connect(sc_ch3_slot,call_conf_slot);

             pjsua_conf_connect(call_conf_slot, sc_ch4_slot);
             pjsua_conf_connect(sc_ch4_slot,call_conf_slot);

             pjsua_conf_connect(call_conf_slot, sc_ch5_slot);
             pjsua_conf_connect(sc_ch5_slot,call_conf_slot);
        }
//        else
//        {
//             pjsua_conf_connect(call_conf_slot, 0);
//             pjsua_conf_connect(0,call_conf_slot);
//        }
        listConfs();
    }
}

pjmedia_transport* on_create_media_transport(pjsua_call_id call_id, unsigned media_idx,pjmedia_transport *base_tp,unsigned flags)
{
    qDebug() << "on_create_media_transport";
    pjmedia_transport *adapter;
    adapter = RoIP_ED137::instance()->on_create_media_transport(call_id,media_idx,base_tp,flags);
    return adapter;
}

pjmedia_transport* RoIP_ED137::on_create_media_transport(pjsua_call_id call_id, unsigned media_idx,
        pjmedia_transport *base_tp, unsigned flags)
{

    registerThread();

    pjmedia_transport *adapter;
    pj_status_t status;
    PJ_UNUSED_ARG(media_idx);
        /* Create the adapter */

    if (makeCallIndex == TRXMODE_TRX){
        qDebug() << "makeCallIndex..............................................trx1" << makeCallIndex;
        status = pjmedia_custom_tp_adapter_create(pjsua_get_pjmedia_endpt(),
                                           NULL,
                                           base_tp,
                                           (flags & PJSUA_MED_TP_CLOSE_MEMBER),
                                           enableEd137,
                                           PJ_FALSE,
                                           "Radio-TxRx",
                                           call_id,
                                           &adapter,
                                           "trx",
                                           TRXMODE_TRX,
                                           keepAlivePeroid,
                                           connToRadio,
                                                  pttWithPayload);

    }
    else if (makeCallIndex == TRXMODE_TX){
        qDebug() << "makeCallIndex..............................................trx2" << makeCallIndex;
        status = pjmedia_custom_tp_adapter_create(pjsua_get_pjmedia_endpt(),
                                           NULL,
                                           base_tp,
                                           (flags & PJSUA_MED_TP_CLOSE_MEMBER),
                                           enableEd137,
                                           PJ_FALSE,
                                           "Radio-TxRx",
                                           call_id,
                                           &adapter,
                                           "tx",
                                           TRXMODE_TX,
                                           keepAlivePeroid,
                                           connToRadio,
                                                  pttWithPayload);
    }
    else if (makeCallIndex == TRXMODE_RX){
        qDebug() << "makeCallIndex..............................................trx3" << makeCallIndex;
        status = pjmedia_custom_tp_adapter_create(pjsua_get_pjmedia_endpt(),
                                           NULL,
                                           base_tp,
                                           (flags & PJSUA_MED_TP_CLOSE_MEMBER),
                                           enableEd137,
                                           PJ_FALSE,
                                           "Radio-Rxonly",
                                           call_id,
                                           &adapter,
                                           "rx",
                                           "Rx",
                                           keepAlivePeroid,
                                           connToRadio,
                                                  pttWithPayload);
    }
    else
    {
        QString txrxmode = TRXMODE_TRX;
        QString type    = "Radio-Idle";
        std::string extNumber;
        int i = 0;
        enableEd137 = PJ_TRUE;
        if(getExtFromCallID(call_id,extNumber))
        {
            QString ext_uriName = QString::fromStdString(extNumber);
            foreach(urlList *uri, urllist)
            {
                if (uri->uri == ext_uriName)
                {
                    urllist.at(i)->uri = ext_uriName;
                    txrxmode = urllist.at(i)->txrxmode;
                    type = urllist.at(i)->radiotype;
                    break;
                }
                i++;
            }
         }
//        for (int i = 0;i < trx_incall.length(); i++)
//        {
//            if (trx_incall.at(i)->call_id == call_id)
//            {
//                    txrxmode = trx_incall.at(i)->txrxmode;
//                    type = trx_incall.at(i)->type;
//            }
//        }
        qDebug() << "makeCallIndex..............................................incoming" << txrxmode << type;
        status = pjmedia_custom_tp_adapter_create(pjsua_get_pjmedia_endpt(),
                                           NULL,
                                           base_tp,
                                           (flags & PJSUA_MED_TP_CLOSE_MEMBER),
                                           enableEd137,
                                           PJ_TRUE,
                                           type.toStdString().c_str(),
                                           call_id,
                                           &adapter,
                                           "callImcome",
                                           txrxmode.toStdString().c_str(),
                                           keepAlivePeroid,
                                           connToRadio,
                                                  pttWithPayload);
    }

    makeCallIndex = "";
    if (status != PJ_SUCCESS) {
        PJ_PERROR(1,(NULL, status, "Error creating adapter\n"));
        return NULL;
    }

    if(adapter!=NULL)
    {
        transport_map.insert(std::make_pair(call_id ,adapter));
    }

    enableEd137 = PJ_FALSE;
    return adapter;
}

void on_stream_created(pjsua_call_id call_id, pjmedia_stream *strm, unsigned stream_idx,pjmedia_port **p_port)
{
   RoIP_ED137::instance()->on_stream_created(call_id,strm,stream_idx,p_port);
}

void RoIP_ED137::on_stream_created(pjsua_call_id call_id, pjmedia_stream *strm, unsigned stream_idx,pjmedia_port **p_port)
{
    qDebug() << "on_stream_created";
    (void) call_id;
    (void) strm;
    (void) stream_idx;
    (void) p_port;
}

void on_stream_destroyed(pjsua_call_id call_id, pjmedia_stream *strm, unsigned stream_idx)
{
   RoIP_ED137::instance()->on_stream_destroyed(call_id,strm,stream_idx);
}

void RoIP_ED137::on_stream_destroyed(pjsua_call_id call_id, pjmedia_stream *strm, unsigned stream_idx)
{
    (void) call_id;
    (void) strm;
    (void) stream_idx;
}


/////////////////////////////////////////

/////////PTT MANAGER CALLBACKS//////////////


QString RoIP_ED137::getTimeDuratio(pjsua_call_id callId){
    pjsua_call_info ci;
    pj_status_t status;
    QString durationTimeVal;
    if (callId == trx1->radio1->call_id){
        if (!trx1->radio1->callState){
            trx1->radio1->connDuration = "0";
            trx1->radio1->sec_connDuration = 0;
            return "0";
        }
    }else if (callId == trx2->radio1->call_id){
        if (!trx2->radio1->callState){
            trx2->radio1->connDuration = "0";
            trx2->radio1->sec_connDuration = 0;
            return "0";
        }
    }else if (callId == trx1->radio2->call_id){
        if (!trx1->radio2->callState){
            trx1->radio2->connDuration = "0";
            trx1->radio2->sec_connDuration = 0;
            return "0";
        }
    }else if (callId == trx2->radio2->call_id){
        if (!trx2->radio2->callState){
            trx2->radio2->connDuration = "0";
            trx2->radio2->sec_connDuration = 0;
            return "0";
        }
    }
    else {
        for (int i = 0;i < trx_incall.length(); i++){
            if (trx_incall.at(i)->call_id == callId){
                if (!trx_incall.at(i)->callState){
                    trx_incall.at(i)->connDuration = "0";
                    trx_incall.at(i)->sec_connDuration = 0;
                    return "0";
                }
            }
        }
    }

    status = pjsua_call_get_info(callId, &ci);
    if (status != PJ_SUCCESS)
    {
        return "0";
    }

    int total_seconds = ci.connect_duration.sec;
    int MINUTE  = 60;
    int HOUR    = MINUTE * 60;
    int DAY     = HOUR * 24;

    int days    = int( total_seconds / DAY );
    int hours   = int( ( total_seconds % DAY ) / HOUR );
    int minutes = int( ( total_seconds % HOUR ) / MINUTE );
    int seconds = int( total_seconds % MINUTE );

    durationTimeVal = "";
    if (days > 0)
        durationTimeVal += QString::number(days) + "d";
    if ((durationTimeVal != "") || (hours > 0))
        durationTimeVal += QString::number(hours).rightJustified(2, '0')  + ":";
    if ((durationTimeVal != "" )||(minutes > 0))
        durationTimeVal += QString::number(minutes).rightJustified(2, '0')  + ":";
    durationTimeVal += QString::number(seconds).rightJustified(2, '0');

    if (callId == trx1->radio1->call_id){
        trx1->radio1->connDuration = durationTimeVal;
        trx1->radio1->sec_connDuration = total_seconds;
    }
    else if (callId == trx2->radio1->call_id){
        trx2->radio1->connDuration = durationTimeVal;
        trx2->radio1->sec_connDuration = total_seconds;
    }
    else if (callId == trx1->radio2->call_id){
        trx1->radio2->connDuration = durationTimeVal;
        trx1->radio2->sec_connDuration = total_seconds;
    }
    else if (callId == trx2->radio2->call_id){
        trx2->radio2->connDuration = durationTimeVal;
        trx2->radio2->sec_connDuration = total_seconds;
    }
    else {
        for (int i = 0;i < trx_incall.length(); i++){
            if (callId == trx_incall.at(i)->call_id){
                trx_incall.at(i)->connDuration = durationTimeVal;
                trx_incall.at(i)->sec_connDuration = total_seconds;
            }
        }
    }

    return durationTimeVal;
}

bool RoIP_ED137::setSlotVolume(int callId, bool increase, bool current)
{    
        pjsua_call_info ci;
        pj_status_t status = pjsua_call_get_info(callId, &ci);

        if (status != PJ_SUCCESS)
        {
            return false;
        }

        if(!current)
        {
            if(increase)
            {
                SLOT_VOLUME  += 0.1f;
                if (SLOT_VOLUME >= MAX_SLOT_VOLUME)
                    SLOT_VOLUME = MAX_SLOT_VOLUME;
            }
            else
            {
                SLOT_VOLUME -= 0.1f;
                if (SLOT_VOLUME <= MIN_SLOT_VOLUME)
                    SLOT_VOLUME =  MIN_SLOT_VOLUME;
            }

        }

        QString vollevel = QString::number(SLOT_VOLUME);

        if (ci.conf_slot != PJSUA_INVALID_ID)
        {
            status = pjsua_conf_adjust_rx_level(ci.conf_slot, SLOT_VOLUME);

            if (status != PJ_SUCCESS)
            {
                return false;
            }
        }
        else {
            return false;
        }

//    qDebug() << "setSlotVolume" << callId << current << SLOT_VOLUME;
    return true;
}

void* RoIP_ED137::ThreadFunc( void* pTr )
{
    RoIP_ED137* pThis = static_cast<RoIP_ED137*>(pTr);
}

void RoIP_ED137::getSnmp()
{
    qDebug() << "start getSNMP thread" << connToRadio << "m_IsSNMPThreadRunning" << m_IsSNMPThreadRunning;
    SNMPStack =  new SnmpStack;
    while (m_IsSNMPThreadRunning)
    {
        if(!connToRadio)
        {
            ::usleep(250000);
            boost::unique_lock<boost::mutex> scoped_lock(io_mutex);
            if (trx1->radio1->pingOk == false) {
                trx1->radio1->getSNMPCount = 0;
                trx1->radio1->frequency = "";
                trx1->radio1->radiostatus = "";
                trx1->radio1->rfPower = 0;
                trx1->radio1->SqlchThrhCarrier = "0";
            }
            if(trx1->radio1->getSNMPCount > 40)
            {
                trx1->radio1->vswr = getRadioVSWR(trx1->radio1->ipAddress,trx1->radio1->snmpProfile,  trx1->radio1->snmpPort);
                trx1->radio1->getSNMPCount = 0;
            }
            else if(trx1->radio1->getSNMPCount == 30){
                QString frequency = getRadioFrequency(trx1->radio1->ipAddress,trx1->radio1->snmpProfile,  trx1->radio1->snmpPort);
                trx1->radio1->frequency = frequency;
            }
            else if(trx1->radio1->getSNMPCount == 20){
                QString snmpRadiostatus = getRadioStatus(trx1->radio1->ipAddress,trx1->radio1->snmpProfile,  trx1->radio1->snmpPort);
                if (trx1->radio1->radiostatus != snmpRadiostatus){
                    trx1->radio1->radiostatus  = snmpRadiostatus;
                    qDebug() << "trx1->radio1->radiostatus" << trx1->radio1->radiostatus;
                }
                if (trx1->radio1->radiostatus.contains("ERROR"))
                {
                    trx1->radio1->call_hangup = true;
                    trx1->radio1->ByeReason = trx1->radio1->radiostatus;
                    trx1->radio1->trxFailed = true;
                    pttStatus = false;
                }
                else
                {
                    if(trx1->radio1->trxFailed){
                        trx1->radio1->trxFailed = false;
                        trx1->radio1->autoConnectCount = 0;
                    }
                }

            }
            else if(trx1->radio1->getSNMPCount == 15){
                if(trx1->trxmode != TRXMODE_RX)
                    trx1->radio1->rfPower = getRFPower(trx1->radio1->ipAddress,trx1->radio1->snmpProfile,  trx1->radio1->snmpPort);
//                qDebug() << "trx1->radio1->rfPower" << trx1->radio1->rfPower;
            }
            else if(trx1->radio1->getSNMPCount == 10){
                trx1->radio1->SqlchThrhCarrier = getRadioSqlchThrhCarrier(trx1->radio1->ipAddress,trx1->radio1->snmpProfile,  trx1->radio1->snmpPort);
            }
            else if(trx1->radio1->getSNMPCount == 1){
                for(int i = 0; i < 5; i++)
                {
                    trx1->radio1->snmpProfile = getSnmpProfile(trx1->radio1->ipAddress,trx1->radio1->snmpPort);
                    if (trx1->radio1->snmpProfile > 0) break;
                    else {
                        trx1->radio1->snmpPort++;
                        if (trx1->radio1->snmpPort > 164) trx1->radio1->snmpPort = 160;
                    }

                }
            }
            trx1->radio1->getSNMPCount++;

            if (trx1->radio2->pingOk == false) {
                trx1->radio2->getSNMPCount = 0;
                trx1->radio2->frequency = "";
                trx1->radio2->radiostatus = "";
                trx1->radio2->rfPower = 0;
                trx1->radio2->SqlchThrhCarrier = "0";
            }
            if(trx1->radio2->getSNMPCount > 40)
            {
                trx1->radio2->vswr = getRadioVSWR(trx1->radio2->ipAddress,trx1->radio2->snmpProfile,  trx1->radio2->snmpPort);
                trx1->radio2->getSNMPCount = 0;
            }
            else if(trx1->radio2->getSNMPCount == 30){
                QString frequency = getRadioFrequency(trx1->radio2->ipAddress,trx1->radio2->snmpProfile,  trx1->radio2->snmpPort);
                trx1->radio2->frequency = frequency;
//                if (trx1->trxmode == TRXMODE_SEPARATE)
//                    emit newfrequency(1,trx1->radio1->frequency,trx1->radio2->SqlchThrhCarrier,trx1->radio2->frequency,trx1->radio1->rfPower);
//                else if(trx1->trxmode != TRXMODE_TX)
//                    emit newfrequency(1,trx1->radio1->frequency,trx1->radio1->SqlchThrhCarrier,trx1->radio2->frequency,trx1->radio1->rfPower);
//                else
//                    emit newfrequency(1,trx1->radio1->frequency,trx1->radio1->SqlchThrhCarrier,trx1->radio2->frequency,trx1->radio1->rfPower);
            }
            else if(trx1->radio2->getSNMPCount == 20){
                QString snmpRadiostatus = getRadioStatus(trx1->radio2->ipAddress,trx1->radio2->snmpProfile,  trx1->radio2->snmpPort);
                if (trx1->radio2->radiostatus != snmpRadiostatus){
                    trx1->radio2->radiostatus  = snmpRadiostatus;
                    qDebug() << "trx1->radio2->radiostatus" << trx1->radio2->radiostatus;
                }
    //            if ((trx1->radio2->radiostatus.contains("WARNING"))||(trx1->radio2->radiostatus.contains("ERROR")))
                if (trx1->radio2->radiostatus.contains("ERROR"))
                {
                    trx1->radio2->call_hangup = true;
    //                trx_call_hangup(trx1->radio2->call_id,500,NULL);
                    trx1->radio2->ByeReason = trx1->radio2->radiostatus;
                    trx1->radio2->trxFailed = true;
                    pttStatus = false;
                }
                else{
                    if(trx1->radio2->trxFailed){
                        trx1->radio2->trxFailed = false;
                        trx1->radio2->autoConnectCount = 0;
                    }
                }
            }
            else if(trx1->radio2->getSNMPCount == 10){
                trx1->radio2->SqlchThrhCarrier = getRadioSqlchThrhCarrier(trx1->radio2->ipAddress,trx1->radio2->snmpProfile,  trx1->radio2->snmpPort);
            }
            else if(trx1->radio2->getSNMPCount == 1){
                for(int i = 0; i < 5; i++)
                {
                    trx1->radio2->snmpProfile = getSnmpProfile(trx1->radio2->ipAddress,trx1->radio2->snmpPort);
                    if (trx1->radio2->snmpProfile > 0) break;
                    else {
                        trx1->radio2->snmpPort++;
                        if (trx1->radio2->snmpPort > 164) trx1->radio2->snmpPort = 160;
                    }

                }
            }

            trx1->radio2->getSNMPCount++;
            if (trx1->trxmode != TRXMODE_SEPARATE) trx1->radio2->getSNMPCount = 0;


            if (trx2->radio1->pingOk == false) {
                trx2->radio1->getSNMPCount = 0;
                trx2->radio1->frequency = "";
                trx2->radio1->radiostatus = "";
                trx2->radio1->rfPower = 0;
                trx2->radio1->SqlchThrhCarrier = "0";
            }
            if(trx2->radio1->getSNMPCount > 40)
            {
                trx2->radio1->vswr = getRadioVSWR(trx2->radio1->ipAddress,trx2->radio1->snmpProfile,  trx2->radio1->snmpPort);
                trx2->radio1->getSNMPCount = 0;
            }
            else if(trx2->radio1->getSNMPCount == 30){
                QString frequency = getRadioFrequency(trx2->radio1->ipAddress,trx2->radio1->snmpProfile,  trx2->radio1->snmpPort);
                trx2->radio1->frequency = frequency;
//                if (trx2->trxmode == TRXMODE_SEPARATE)
//                    emit newfrequency(2,trx2->radio1->frequency,trx2->radio2->SqlchThrhCarrier,trx2->radio2->frequency,trx2->radio1->rfPower);
//                else if(trx2->trxmode != TRXMODE_TX)
//                    emit newfrequency(2,trx2->radio1->frequency,trx2->radio1->SqlchThrhCarrier,trx2->radio2->frequency,trx2->radio1->rfPower);
//                else
//                    emit newfrequency(2,trx2->radio1->frequency,trx2->radio1->SqlchThrhCarrier,trx2->radio2->frequency,trx2->radio1->rfPower);

            }
            else if(trx2->radio1->getSNMPCount == 20){
                QString snmpRadiostatus = getRadioStatus(trx2->radio1->ipAddress,trx2->radio1->snmpProfile,  trx2->radio1->snmpPort);
                if (trx2->radio1->radiostatus != snmpRadiostatus){
                    trx2->radio1->radiostatus  = snmpRadiostatus;
                    qDebug() << "trx2->radio1->radiostatus" << trx2->radio1->radiostatus;
                }
    //            if ((trx2->radio1->radiostatus.contains("WARNING"))||(trx2->radio1->radiostatus.contains("ERROR")))
                if (trx2->radio1->radiostatus.contains("ERROR"))
                {
                    trx2->radio1->call_hangup = true;
    //                trx_call_hangup(trx2->radio1->call_id,500,NULL);
                    trx2->radio1->ByeReason = trx2->radio1->radiostatus;
                    trx2->radio1->trxFailed = true;
                    pttStatus = false;
                }
                else
                {
                    if(trx2->radio1->trxFailed){
                        trx2->radio1->trxFailed = false;
                        trx2->radio1->autoConnectCount = 0;
                    }
                }
            }
            else if(trx2->radio1->getSNMPCount == 15){
                if(trx2->trxmode != TRXMODE_RX)
                    trx2->radio1->rfPower = getRFPower(trx2->radio1->ipAddress,trx2->radio1->snmpProfile,  trx2->radio1->snmpPort);
            }
            else if(trx2->radio1->getSNMPCount == 10){
                trx2->radio1->SqlchThrhCarrier = getRadioSqlchThrhCarrier(trx2->radio1->ipAddress,trx2->radio1->snmpProfile,  trx2->radio1->snmpPort);
            }
            else if(trx2->radio1->getSNMPCount == 1){
                for(int i = 0; i < 5; i++)
                {
                    trx2->radio1->snmpProfile = getSnmpProfile(trx2->radio1->ipAddress,trx2->radio1->snmpPort);
                    if (trx2->radio1->snmpProfile > 0) break;
                    else {
                        trx2->radio1->snmpPort++;
                        if (trx2->radio1->snmpPort > 164) trx2->radio1->snmpPort = 160;
                    }

                }
            }
            trx2->radio1->getSNMPCount++;


            if (trx2->radio2->pingOk == false) {
                trx2->radio2->getSNMPCount = 0;
                trx2->radio2->frequency = "";
                trx2->radio2->radiostatus = "";
                trx2->radio2->rfPower = 0;
                trx2->radio2->SqlchThrhCarrier = "0";
            }
            if(trx2->radio2->getSNMPCount > 40)
            {
                trx2->radio2->vswr = getRadioVSWR(trx2->radio2->ipAddress,trx2->radio2->snmpProfile, trx2->radio2->snmpPort);
                trx2->radio2->getSNMPCount = 0;
            }
            else if(trx2->radio2->getSNMPCount == 30){
                QString frequency = getRadioFrequency(trx2->radio2->ipAddress,trx2->radio2->snmpProfile, trx2->radio2->snmpPort);
                trx2->radio2->frequency = frequency;
//                if (trx2->trxmode == TRXMODE_SEPARATE)
//                    emit newfrequency(2,trx2->radio1->frequency,trx2->radio2->SqlchThrhCarrier,trx2->radio2->frequency,trx2->radio1->rfPower);
//                else if(trx2->trxmode != TRXMODE_TX)
//                    emit newfrequency(2,trx2->radio1->frequency,trx2->radio1->SqlchThrhCarrier,trx2->radio2->frequency,trx2->radio1->rfPower);
//                else
//                    emit newfrequency(2,trx2->radio1->frequency,trx2->radio1->SqlchThrhCarrier,trx2->radio2->frequency,trx2->radio1->rfPower);
            }
            else if(trx2->radio2->getSNMPCount == 20){
                QString snmpRadiostatus = getRadioStatus(trx2->radio2->ipAddress,trx2->radio2->snmpProfile, trx2->radio2->snmpPort);
                if (trx2->radio2->radiostatus != snmpRadiostatus){
                    trx2->radio2->radiostatus  = snmpRadiostatus;
                    qDebug() << "trx2->radio2->radiostatus" << trx2->radio2->radiostatus;
                }
    //            if ((trx2->radio2->radiostatus.contains("WARNING"))||(trx2->radio2->radiostatus.contains("ERROR")))
                if (trx2->radio2->radiostatus.contains("ERROR"))
                {
                    trx2->radio2->call_hangup = true;
    //                trx_call_hangup(trx2->radio2->call_id,500,NULL);
                    trx2->radio2->ByeReason = trx2->radio2->radiostatus;
                    trx2->radio2->trxFailed = true;
                    pttStatus = false;
                }
                else
                {
                    if(trx2->radio2->trxFailed){
                        trx2->radio2->trxFailed = false;
                        trx2->radio2->autoConnectCount = 0;
                    }
                }
            }
            else if(trx2->radio2->getSNMPCount == 10){
                trx2->radio2->SqlchThrhCarrier = getRadioSqlchThrhCarrier(trx2->radio2->ipAddress,trx2->radio2->snmpProfile, trx2->radio2->snmpPort);
            }
            else if(trx2->radio2->getSNMPCount == 1){
                for(int i = 0; i < 5; i++)
                {
                    trx2->radio2->snmpProfile = getSnmpProfile(trx2->radio2->ipAddress,trx2->radio2->snmpPort);
                    if (trx2->radio2->snmpProfile > 0) break;
                    else {
                        trx2->radio2->snmpPort++;
                        if (trx2->radio2->snmpPort > 164) trx2->radio2->snmpPort = 160;
                    }
                }
            }

            trx2->radio2->getSNMPCount++;
            if (trx2->trxmode != TRXMODE_SEPARATE) trx2->radio2->getSNMPCount = 0;


            trx1->radio1->pingCount++;
            if (trx1->radio1->pingCount > 30){
                if ((trx1->radio1->callState == false) & (trx1->radio1->enable) & (trx1->pingEnable))
                    trx1->radio1->pingOk = validateIpAddress(trx1->radio1->ipAddress.toStdString().c_str());
                trx1->radio1->pingCount = 0;
            }
            trx1->radio2->pingCount++;
            if (trx1->radio2->pingCount > 30){
                if ((trx1->radio2->callState == false) & (trx1->radio2->enable) & (trx1->pingEnable))
                    trx1->radio2->pingOk = validateIpAddress(trx1->radio2->ipAddress.toStdString().c_str());
                trx1->radio2->pingCount = 0;
            }
            trx2->radio1->pingCount++;
            if (trx2->radio1->pingCount > 30){
                if ((trx2->radio1->callState == false) & (trx2->radio1->enable) & (trx2->pingEnable))
                    trx2->radio1->pingOk = validateIpAddress(trx2->radio1->ipAddress.toStdString().c_str());
                trx2->radio1->pingCount = 0;
            }
            trx2->radio2->pingCount++;
            if (trx2->radio2->pingCount > 30){
                if ((trx2->radio2->callState == false) & (trx2->radio2->enable) & (trx1->pingEnable))
                    trx2->radio2->pingOk = validateIpAddress(trx2->radio2->ipAddress.toStdString().c_str());
                trx2->radio2->pingCount = 0;
            }
        }
        else
        {
            ::usleep(250000);
            trx1->radio1->pingCount++;
            if (trx1->radio1->pingCount > 30){
                if ((trx1->radio1->callState == false) & (trx1->radio1->enable))
                    trx1->radio1->pingOk = validateIpAddress(trx1->radio1->ipAddress.toStdString().c_str());
                trx1->radio1->pingCount = 0;
            }
            trx1->radio2->pingCount++;
            if (trx1->radio2->pingCount > 30){
                if ((trx1->radio2->callState == false) & (trx1->radio2->enable))
                    trx1->radio2->pingOk = validateIpAddress(trx1->radio2->ipAddress.toStdString().c_str());
                trx1->radio2->pingCount = 0;
            }
            trx2->radio1->pingCount++;
            if (trx2->radio1->pingCount > 30){
                if ((trx2->radio1->callState == false) & (trx2->radio1->enable))
                    trx2->radio1->pingOk = validateIpAddress(trx2->radio1->ipAddress.toStdString().c_str());
                trx2->radio1->pingCount = 0;
            }
            trx2->radio2->pingCount++;
            if (trx2->radio2->pingCount > 30){
                if ((trx2->radio2->callState == false) & (trx2->radio2->enable))
                    trx2->radio2->pingOk = validateIpAddress(trx2->radio2->ipAddress.toStdString().c_str());
                trx2->radio2->pingCount = 0;
            }
        }
    }

    {
        if ((trx1->radio1->callState == false) & (trx1->radio1->enable))
            trx1->radio1->pingOk = validateIpAddress(trx1->radio1->ipAddress.toStdString().c_str());
        trx1->radio1->pingCount = 0;
    }

    {
        if ((trx1->radio2->callState == false) & (trx1->radio2->enable))
            trx1->radio2->pingOk = validateIpAddress(trx1->radio2->ipAddress.toStdString().c_str());
        trx1->radio2->pingCount = 0;
    }

    {
        if ((trx2->radio1->callState == false) & (trx2->radio1->enable))
            trx2->radio1->pingOk = validateIpAddress(trx2->radio1->ipAddress.toStdString().c_str());
        trx2->radio1->pingCount = 0;
    }

    {
        if ((trx2->radio2->callState == false) & (trx2->radio2->enable))
            trx2->radio2->pingOk = validateIpAddress(trx2->radio2->ipAddress.toStdString().c_str());
        trx2->radio2->pingCount = 0;
    }
}
void RoIP_ED137::scan_call_err()
{
    if (trx1->radio1->call_hangup){
        trx1->radio1->call_hangup = false;
        trx_call_hangup(trx1->radio1->call_id,500,NULL);
        trx1->radio1->trxFailed = true;
    }
    if (trx1->radio2->call_hangup){
        trx1->radio2->call_hangup = false;
        trx_call_hangup(trx1->radio2->call_id,500,NULL);
        trx1->radio2->trxFailed = true;
    }
    if (trx2->radio1->call_hangup){
        trx2->radio1->call_hangup = false;
        trx_call_hangup(trx2->radio1->call_id,500,NULL);
        trx2->radio1->trxFailed = true;
    }
    if (trx2->radio2->call_hangup){
        trx2->radio2->call_hangup = false;
        trx_call_hangup(trx2->radio2->call_id,500,NULL);
        trx2->radio2->trxFailed = true;
    }
}
void RoIP_ED137::checkEvents()
{
    scan_call_err();
    if (inviteMode == SERVER)
    {
        if ((trx1->radio1->call_id != PJSUA_INVALID_ID)&&(trx1->radio1->callState))
        {
    //        int bssrx = 0;
            int sqlon = 0;
            int pttstatus = 0;

            keeplogAudioLevel(trx1->radio1);
            if (trx1->trxmode != TRXMODE_TX){

            }

            if (trx1->radio1->trxmode != TRXMODE_TX)
            {
                sqlon = (int)get_IPRadioSquelch(trx1->radio1->call_id);
                trx1->radio1->SQLOn = sqlon;

                if (pttGroupActive == true)// Disable Rx When another iGate PTT Status is on;
                {
                    if ((groupMute == MUTEALL)||(forceMuteSqlOn))
                        sqlon = false;
                    trx1->radio1->SqlGroupDelayCount = 0;
                    pttGroupActiveTmp = pttGroupActive;
                    qDebug() << "pttGroupActive" << pttGroupActive << "sqlon" << sqlon << "forceMuteSqlOn" << forceMuteSqlOn;
                }
                else if (forceMuteSqlOn == true)
                {
                    sqlon = false;
                }
                else
                {
                    if (pttGroupActive != pttGroupActiveTmp)
                    {
                        trx1->radio1->SqlGroupDelayCount++;
                        if ((groupMute == MUTEALL)||(forceMuteSqlOn))
                            sqlon = false;
                        qDebug() << "SqlGroupDelayCount" << trx1->radio1->SqlGroupDelayCount << "sqlon" << sqlon;
                        if (trx1->radio1->SqlGroupDelayCount == SqlGroupDelay)
                        {
                            pttGroupActiveTmp = pttGroupActive;
                        }
                    }
                }

                trx1->radio1->rssi=(int)get_IPRadioBss(trx1->radio1->call_id);
                // if (sqlon != trx1->radio1->lastRx) {
                //     if (sqlon == 0){
                //         trx1->radio1->lastRxmsec++;
                //         if (trx1->radio1->lastRxmsec < 1)
                //         {
                //             sqlon = 1;
                //         }
                //     }
                // }
                // else
                // {
                //     trx1->radio1->lastRxmsec = 0;
                // }

            }

            if (trx1->radio1->trxmode != TRXMODE_RX){
                pttstatus = get_IPRadioPttStatus(trx1->radio1->call_id);
                if (pttstatus != m_PttPressed1)
                {
                    if ((localSidetoneLoopbackOn) & (pttstatus == false))
                    {
                        localSidetoneLoopbackOn = false;
                        if (onLocalSidetoneLoopbackChanged != localSidetoneLoopbackOn)
                        {
                            onLocalSidetoneLoopbackChanged = localSidetoneLoopbackOn;
                            QString message = QString("{\"menuID\":\"onLocalSidetoneLoopbackChanged\", \"localSidetoneLoopbackOn\":%1, \"softPhoneID\":%2}")
                                    .arg(localSidetoneLoopbackOn).arg(m_softPhoneID);
                            emit sendTextMessage(message);
                        }
                    }
                }
            }



            if((pttstatus != m_PttPressed1) || (trx1->radio1->lastRx != sqlon))
            {
                qDebug() << "trx1->radio1->trxmode"  << pttstatus << m_PttPressed1 << trx1->radio1->lastRx << "sqlon" << sqlon;
                if ((pttstatus > 0) & (sqlon > 0))
                {
                    updateHomeDisplay(trx1->radio1->url,CONNECTED,PTTRX,trx1->radio1->call_id,trx1->radio1->radioNodeID);                    
                }
                else if ((pttstatus > 0) & (sqlon == 0))
                {
                    updateHomeDisplay(trx1->radio1->url,CONNECTED,PTTON,trx1->radio1->call_id,trx1->radio1->radioNodeID);
                }
                else if ((pttstatus == 0) & (sqlon > 0))
                {
                    updateHomeDisplay(trx1->radio1->url,CONNECTED,RXON,trx1->radio1->call_id,trx1->radio1->radioNodeID);
                }
                else
                {
                    updateHomeDisplay(trx1->radio1->url,CONNECTED,STANDBY,trx1->radio1->call_id,trx1->radio1->radioNodeID);
                }
                trx1->radio1->lastRx = sqlon;
                trx1->radio1->lastTx = pttstatus;
                m_PttPressed1 = pttstatus;

                if (trx1->radio1->lastRx == false)
                    setvolume(trx1->radio1->call_id,MUTE);
            }

            getTimeDuratio(trx1->radio1->call_id);

        }


        if ((trx1->radio2->call_id != PJSUA_INVALID_ID)&&(trx1->radio2->callState))
        {
    //        int bssrx = 0;
            keeplogAudioLevel(trx1->radio2);
            int sqlon = 0;
            if (trx1->trxmode != TRXMODE_TX){

            }

            if (trx1->radio2->trxmode != TRXMODE_TX){
                sqlon = (int)get_IPRadioSquelch(trx1->radio2->call_id);
                trx1->radio2->SQLOn = sqlon;

                if (pttGroupActive == true)// Disable Rx When another iGate PTT Status is on;
                {
                    if ((groupMute == MUTEALL)||(forceMuteSqlOn))
                        sqlon = false;
                    trx1->radio2->SqlGroupDelayCount = 0;
                    pttGroupActiveTmp = pttGroupActive;
                     qDebug() << "pttGroupActive" << pttGroupActive << "sqlon" << sqlon << "forceMuteSqlOn" << forceMuteSqlOn;
                }
                else if (forceMuteSqlOn == true)
                {
                    sqlon = false;
                }
                else
                {
                    if (pttGroupActive != pttGroupActiveTmp)
                    {
                        trx1->radio2->SqlGroupDelayCount++;
                        if ((groupMute == MUTEALL)||(forceMuteSqlOn))
                            sqlon = false;
                        //qDebug() << "SqlGroupDelayCount" << trx1->radio2->SqlGroupDelayCount << "sqlon" << sqlon;
                        if (trx1->radio2->SqlGroupDelayCount == SqlGroupDelay)
                        {
                            pttGroupActiveTmp = pttGroupActive;
                        }
                    }
                }

                trx1->radio2->rssi=(int)get_IPRadioBss(trx1->radio2->call_id);
                // if (sqlon != trx1->radio2->lastRx) {
                //     if (sqlon == 0){
                //         trx1->radio2->lastRxmsec++;
                //         if (trx1->radio2->lastRxmsec < 1){
                //             sqlon = 1;
                //         }
                //     }
                // }
                // else
                // {
                //     trx1->radio2->lastRxmsec = 0;
                // }

            }

            if(trx1->radio2->lastRx != sqlon)
            {
                qDebug() << "trx1->radio2->trxmode " << m_PttPressed1 << trx1->radio2->lastRx << sqlon;
                if (sqlon > 0)
                {
                    updateHomeDisplay(trx1->radio2->url,CONNECTED,RXON,trx1->radio2->call_id,trx1->radio2->radioNodeID);
                }
                else
                {
                    updateHomeDisplay(trx1->radio2->url,CONNECTED,STANDBY,trx1->radio2->call_id,trx1->radio2->radioNodeID);
                }
                trx1->radio2->lastRx = sqlon;
                if (trx1->radio2->lastRx == false)
                    setvolume(trx1->radio2->call_id,MUTE);
            }
            getTimeDuratio(trx1->radio2->call_id);
        }




        if ((trx2->radio1->call_id != PJSUA_INVALID_ID)&&(trx2->radio1->callState))
        {
    //        int bssrx = 0;
            keeplogAudioLevel(trx2->radio1);
            int sqlon = 0;
            int pttstatus = 0;

            if (trx2->trxmode != TRXMODE_TX){

            }

            if (trx2->radio1->trxmode != TRXMODE_TX)
            {
                sqlon = (int)get_IPRadioSquelch(trx2->radio1->call_id);
                trx2->radio1->SQLOn = sqlon;

                if (pttGroupActive == true)// Disable Rx When another iGate PTT Status is on;
                {
                    if ((groupMute == MUTEALL)||(forceMuteSqlOn))
                        sqlon = false;
                    trx2->radio1->SqlGroupDelayCount = 0;
                    pttGroupActiveTmp = pttGroupActive;
                }
                else if (forceMuteSqlOn == true)
                {
                    sqlon = false;
                }
                else
                {
                    if (pttGroupActive != pttGroupActiveTmp)
                    {
                        trx2->radio1->SqlGroupDelayCount++;
                        if ((groupMute == MUTEALL)||(forceMuteSqlOn))
                            sqlon = false;
                        //qDebug() << "SqlGroupDelayCount" << trx2->radio1->SqlGroupDelayCount << "sqlon" << sqlon;
                        if (trx2->radio1->SqlGroupDelayCount == SqlGroupDelay)
                        {
                            pttGroupActiveTmp = pttGroupActive;
                        }
                    }
                }

                trx2->radio1->rssi=(int)get_IPRadioBss(trx2->radio1->call_id);
                if (sqlon != trx2->radio1->lastRx) {
                    if (sqlon == 0){
                        trx2->radio1->lastRxmsec++;
                        if (trx2->radio1->lastRxmsec < 1)
                        {
                            sqlon = 1;
                        }
                    }
                }
                else
                {
                    trx2->radio1->lastRxmsec = 0;
                }
            }

            if (trx2->radio1->trxmode != TRXMODE_RX)
                pttstatus = get_IPRadioPttStatus(trx2->radio1->call_id);


            if (pttstatus != m_PttPressed2)
            {
                if ((localSidetoneLoopbackOn) & (pttstatus == false))
                {
                    localSidetoneLoopbackOn = false;
                    if (onLocalSidetoneLoopbackChanged != localSidetoneLoopbackOn)
                    {
                        onLocalSidetoneLoopbackChanged = localSidetoneLoopbackOn;
                        QString message = QString("{\"menuID\":\"onLocalSidetoneLoopbackChanged\", \"localSidetoneLoopbackOn\":%1, \"softPhoneID\":%2}")
                                .arg(localSidetoneLoopbackOn).arg(m_softPhoneID);
                        emit sendTextMessage(message);
                    }
                }
            }

            if((pttstatus != m_PttPressed2) || (trx2->radio1->lastRx != sqlon))
            {
                qDebug() << "trx2->radio1->trxmode "  << pttstatus << m_PttPressed2 << trx2->radio1->lastRx << sqlon;
                if ((pttstatus > 0) & (sqlon > 0))
                {
                    updateHomeDisplay(trx2->radio1->url,CONNECTED,PTTRX,trx2->radio1->call_id,trx2->radio1->radioNodeID);
                }
                else if ((pttstatus > 0) & (sqlon == 0))
                {
                    updateHomeDisplay(trx2->radio1->url,CONNECTED,PTTON,trx2->radio1->call_id,trx2->radio1->radioNodeID);
                }
                else if ((pttstatus == 0) & (sqlon > 0))
                {
                    updateHomeDisplay(trx2->radio1->url,CONNECTED,RXON,trx2->radio1->call_id,trx2->radio1->radioNodeID);
                }
                else
                {
                    updateHomeDisplay(trx2->radio1->url,CONNECTED,STANDBY,trx2->radio1->call_id,trx2->radio1->radioNodeID);
                }
                trx2->radio1->lastRx = sqlon;
                m_PttPressed2 = pttstatus;

                if (trx2->radio1->lastRx == false)
                    setvolume(trx2->radio1->call_id,MUTE);
            }

            getTimeDuratio(trx2->radio1->call_id);

        }


        if ((trx2->radio2->call_id != PJSUA_INVALID_ID)&&(trx2->radio2->callState))
        {
    //        int bssrx = 0;
            keeplogAudioLevel(trx2->radio2);
            int sqlon = 0;
            if (trx2->trxmode != TRXMODE_TX)
            {

            }

            if (trx2->radio2->trxmode != TRXMODE_TX){
                sqlon = (int)get_IPRadioSquelch(trx2->radio2->call_id);
                trx2->radio2->SQLOn = sqlon;

                if (pttGroupActive == true)// Disable Rx When another iGate PTT Status is on;
                {
                    if ((groupMute == MUTEALL)||(forceMuteSqlOn))
                        sqlon = false;
                    trx2->radio2->SqlGroupDelayCount = 0;
                    pttGroupActiveTmp = pttGroupActive;
                }
                else if (forceMuteSqlOn == true)
                {
                    sqlon = false;
                }
                else
                {
                    if (pttGroupActive != pttGroupActiveTmp)
                    {
                        trx2->radio2->SqlGroupDelayCount++;
                        if ((groupMute == MUTEALL)||(forceMuteSqlOn))
                            sqlon = false;
                        //qDebug() << "SqlGroupDelayCount" << trx2->radio2->SqlGroupDelayCount << "sqlon" << sqlon;
                        if (trx2->radio2->SqlGroupDelayCount == SqlGroupDelay)
                        {
                            pttGroupActiveTmp = pttGroupActive;
                        }
                    }
                }

                trx2->radio2->rssi=(int)get_IPRadioBss(trx2->radio2->call_id);
                if (sqlon != trx2->radio2->lastRx)
                {
                    if (sqlon == 0){
                        trx2->radio2->lastRxmsec++;
                        if (trx2->radio2->lastRxmsec < 1){
                            sqlon = 1;
                        }
                    }
                }
                else
                {
                    trx2->radio2->lastRxmsec = 0;
                }
            }

            if(trx2->radio2->lastRx != sqlon)
            {
                qDebug() << "trx2->radio2->trxmode " << m_PttPressed2 << trx2->radio2->lastRx << sqlon;
                if (sqlon > 0)
                {
                    updateHomeDisplay(trx2->radio2->url,CONNECTED,RXON,trx2->radio2->call_id,trx2->radio2->radioNodeID);
                }
                else
                {
                    updateHomeDisplay(trx2->radio2->url,CONNECTED,STANDBY,trx2->radio2->call_id,trx2->radio2->radioNodeID);
                }
                trx2->radio2->lastRx = sqlon;
                if (trx2->radio2->lastRx == false)
                    setvolume(trx2->radio2->call_id,MUTE);
            }
            getTimeDuratio(trx2->radio2->call_id);
        }

        if (rxBestSignalEnable == true)
        {
            if ((trx1->radio1->callState == false) || (trx1->radio1->lastRx == 0))
            {                
                if (trx1->radio1->audioSQLOn == true)
                {
                    sqlStatusCount = 0;
                    sqlStatusOn = false;
                }
                trx1->radio1->audioSQLOn = false;
                trx1->radio1->rssi = -1;
            }
            if ((trx1->radio2->callState == false) || (trx1->radio2->lastRx == 0))
            {
                if (trx1->radio2->audioSQLOn == true)
                {
                    sqlStatusCount = 0;
                    sqlStatusOn = false;
                }
                trx1->radio2->audioSQLOn = false;
                trx1->radio2->rssi = -1;
            }
            if ((trx2->radio1->callState == false) || (trx2->radio1->lastRx == 0))
            {
                if (trx2->radio1->audioSQLOn == true)
                {
                    sqlStatusCount = 0;
                    sqlStatusOn = false;
                }
                trx2->radio1->audioSQLOn = false;
                trx2->radio1->rssi = -1;
            }
            if ((trx2->radio2->callState == false) || (trx2->radio2->lastRx == 0))
            {
                if (trx2->radio2->audioSQLOn == true)
                {
                    sqlStatusCount = 0;
                    sqlStatusOn = false;
                }
                trx2->radio2->audioSQLOn = false;
                trx2->radio2->rssi = -1;
            }
            if ((trx1->radio1->lastRx > 0) || (trx1->radio2->lastRx > 0) || (trx2->radio1->lastRx > 0) || (trx2->radio2->lastRx > 0))
            {
                sqlStatusCount++;
                if ((sqlStatusCount >= 5) & (sqlStatusOn == false))
                {
                    if (trx1->radio1->lastRx > 0)
                        setvolume(trx1->radio1->call_id,MUTE);
                    if (trx1->radio2->lastRx > 0)
                        setvolume(trx1->radio2->call_id,MUTE);
                    if (trx2->radio1->lastRx > 0)
                        setvolume(trx2->radio1->call_id,MUTE);
                    if (trx2->radio2->lastRx > 0)
                        setvolume(trx2->radio2->call_id,MUTE);
                    sqlStatusOn = true;

                    qDebug() << "sqlStatusOn" << sqlStatusOn << "trx1->radio1" << trx1->radio1->lastRx << "BSS" << trx1->radio1->rssi
                             << "trx1->radio2" << trx1->radio2->lastRx  << "BSS" << trx1->radio2->rssi
                             << "trx2->radio1" << trx2->radio1->lastRx << "BSS" << trx2->radio1->rssi
                             << "trx2->radio2" << trx2->radio2->lastRx << "BSS" << trx2->radio2->rssi ;

                    if     ((trx1->radio1->rssi >= trx1->radio2->rssi) &
                            (trx1->radio1->rssi >= trx2->radio1->rssi) &
                            (trx1->radio1->rssi >= trx2->radio2->rssi) &
                            (trx1->radio1->lastRx))
                    {
                        trx1->radio1->audioSQLOn = true;
                        trx1->radio2->audioSQLOn = false;
                        trx2->radio1->audioSQLOn = false;
                        trx2->radio2->audioSQLOn = false;
                        if(trx1->radio1->callState)
                            setvolume(trx1->radio1->call_id,UNMUTE);
                        trx1->mainRx = true;
                        trx2->mainRx = false;
                        updateHomeDisplay(trx1->radio1->url,CONNECTED,RXON,trx1->radio1->call_id,trx1->radio1->radioNodeID);
                    }

                    else if ((trx1->radio2->rssi >= trx1->radio1->rssi) &
                             (trx1->radio2->rssi >= trx2->radio1->rssi) &
                             (trx1->radio2->rssi >= trx2->radio2->rssi) &
                             (trx1->radio2->lastRx))
                    {
                        trx1->radio1->audioSQLOn = false;
                        trx1->radio2->audioSQLOn = true;
                        trx2->radio1->audioSQLOn = false;
                        trx2->radio2->audioSQLOn = false;
                        if(trx1->radio2->callState)
                            setvolume(trx1->radio2->call_id,UNMUTE);
                        trx1->mainRx = true;
                        trx2->mainRx = false;
                        updateHomeDisplay(trx1->radio2->url,CONNECTED,RXON,trx1->radio2->call_id,trx1->radio2->radioNodeID);
                    }

                    else if ((trx2->radio1->rssi >= trx1->radio1->rssi) &
                             (trx2->radio1->rssi >= trx1->radio2->rssi) &
                             (trx2->radio1->rssi >= trx2->radio2->rssi) &
                             (trx2->radio1->lastRx))
                    {
                        trx1->radio1->audioSQLOn = false;
                        trx1->radio2->audioSQLOn = false;
                        trx2->radio1->audioSQLOn = true;
                        trx2->radio2->audioSQLOn = false;
                        if(trx2->radio1->callState)
                            setvolume(trx2->radio1->call_id,UNMUTE);
                        trx1->mainRx = false;
                        trx2->mainRx = true;
                        updateHomeDisplay(trx2->radio1->url,CONNECTED,RXON,trx2->radio1->call_id,trx2->radio1->radioNodeID);
                    }

                    else if ((trx2->radio2->rssi >= trx1->radio1->rssi) &
                             (trx2->radio2->rssi >= trx1->radio2->rssi) &
                             (trx2->radio2->rssi >= trx2->radio1->rssi) &
                             (trx2->radio2->lastRx))
                    {
                        trx1->radio1->audioSQLOn = false;
                        trx1->radio2->audioSQLOn = false;
                        trx2->radio1->audioSQLOn = false;
                        trx2->radio2->audioSQLOn = true;
                        if(trx2->radio2->callState)
                            setvolume(trx2->radio2->call_id,UNMUTE);
                        trx1->mainRx = false;
                        trx2->mainRx = true;
                        updateHomeDisplay(trx2->radio2->url,CONNECTED,RXON,trx2->radio2->call_id,trx2->radio2->radioNodeID);
                    }
                }
            }
            else
            {
                sqlStatusCount = 0;
                sqlStatusOn = false;
                trx1->radio1->audioSQLOn = false;
                trx1->radio2->audioSQLOn = false;
                trx2->radio1->audioSQLOn = false;
                trx2->radio2->audioSQLOn = false;
            }
        }

    }
    else if (inviteMode == CLIENT)
    {
        int checkPTT = 0;
        for (int i = 0;i < trx_incall.length(); i++)
        {
            if (trx_incall.at(i)->callName != recorderAlloweURI)
            {
                if ((trx_incall.at(i)->call_id != PJSUA_INVALID_ID)&&(trx_incall.at(i)->callState))
                {
                    int pttstatus = 0;
                    if (trx_incall.at(i)->trxmode != TRXMODE_RX)
                    {
//                        if (!((m_TxRx1stPriority == RX1ST) & (trxStatus == RXON)))
                        pttstatus=get_IPRadioPttStatus(trx_incall.at(i)->call_id);

                        if (pttstatus != trx_incall.at(i)->lastTx) {
                            if (pttstatus == 0)
                            {
                                trx_incall.at(i)->lastTxmsec++;
                                if (trx_incall.at(i)->lastTxmsec < 6)
                                {
                                    pttstatus = 1;
                                }
                            }
                        }
                        else
                        {
                            trx_incall.at(i)->lastTxmsec = 0;
                        }

                        trx_incall.at(i)->lastTx = pttstatus;

                        trx_incall.at(i)->pttLevel = pttstatus;
                        if (pttstatus){
                            if (pttstatus > ptt_level)
                            {
                                ptt_level = pttstatus;
                                SLOT_VOLUME = 2.0f;
                                setSlotVolume(trx_incall.at(i)->call_id,false,true);
                                for (int j = 0;j < trx_incall.length(); j++){
                                    if (trx_incall.at(j)->m_PttPressed){
                                        SLOT_VOLUME = 0.0f;
                                        if (trx_incall.at(j)->call_id != trx_incall.at(i)->call_id)
                                        {
                                            setSlotVolume(trx_incall.at(j)->call_id,false,true);
                                            qDebug() << trx_incall.at(i)->callName << "Mute" << "ptt_level" << ptt_level;

                                        }
                                    }
                                }
                                qDebug() << trx_incall.at(i)->callName << "Unmute" << "ptt_level" << ptt_level;
                                lastPttOn = trx_incall.at(i)->url;
                                //system("audioCapture.sh default");
                            }
                            else if (pttstatus == ptt_level)
                            {


                            }
                            else
                            {

                            }
                        }

                    }

                    if((pttstatus > 0)&(!trx_incall.at(i)->m_PttPressed))
                    {
                        trx_incall.at(i)->m_PttPressed = true;
                        updateHomeDisplay(trx_incall.at(i)->callIndexName,CONNECTED,PTTON,trx_incall.at(i)->call_id,0);
                        recorder_pressed(pttstatus,PTTON);
                    }

                    else if((pttstatus == 0)&(trx_incall.at(i)->m_PttPressed))
                    {
                         trx_incall.at(i)->m_PttPressed = false;

                         for (int j = 0;j < trx_incall.length(); j++)
                         {
                             if(trx_incall.at(j)->m_PttPressed)
                                  checkPTT++;
                         }
                         if (checkPTT == 0)
                         {
                             qDebug() << "checkPTT" << checkPTT;
                             pttOnStatus = false;
                             if (sqlOnStatus == false)
                                 updateHomeDisplay(trx_incall.at(i)->callIndexName,CONNECTED,STANDBY,trx_incall.at(i)->call_id,0);
                             else
                                 updateHomeDisplay(trx_incall.at(i)->callIndexName,CONNECTED,RXON,trx_incall.at(i)->call_id,0);
                             recorder_released(pttstatus);
                         }
                         SLOT_VOLUME = 0.0f;
                         setSlotVolume(trx_incall.at(i)->call_id,false,true);
                         qDebug() << trx_incall.at(i)->callName << "Mute" << "ptt_level" << ptt_level << pttstatus;
                         //system("audioCapture.sh kill");
                         ptt_level = pttstatus;
                    }
                    getTimeDuratio(trx_incall.at(i)->call_id);
                }
            }
            else if (trx_incall.at(i)->callName == recorderAlloweURI)
            {

            }
        }
    }
    else if (inviteMode == CLIENT_RPT)
    {
        int checkPTT = 0;
        for (int i = 0;i < trx_incall.length(); i++){
            if (trx_incall.at(i)->callName != recorderAlloweURI)
            {
                if ((trx_incall.at(i)->call_id != PJSUA_INVALID_ID)&&(trx_incall.at(i)->callState))
                {
                    getTimeDuratio(trx_incall.at(i)->call_id);
                    int pttstatus = 0;
                    if (trx_incall.at(i)->trxmode != TRXMODE_RX)
                    {
                        pttstatus=get_IPRadioPttStatus(trx_incall.at(i)->call_id);
                        trx_incall.at(i)->pttLevel = pttstatus;
                        if (pttstatus){
                            if (pttstatus > ptt_level){
                                ptt_level = pttstatus;
                                SLOT_VOLUME = 2.0f;
                                setSlotVolume(trx_incall.at(i)->call_id,false,true);
                                for (int j = 0;j < trx_incall.length(); j++){
                                    if (trx_incall.at(j)->m_PttPressed){
                                        SLOT_VOLUME = 0.0f;
                                        if (trx_incall.at(j)->call_id != trx_incall.at(i)->call_id)
                                        {
                                            setSlotVolume(trx_incall.at(j)->call_id,false,true);
                                            qDebug() << trx_incall.at(i)->callName << "Mute" << "ptt_level" << ptt_level;

                                        }
                                    }
                                }
                                qDebug() << trx_incall.at(i)->callName << "Unmute" << "ptt_level" << ptt_level;
                                lastPttOn = trx_incall.at(i)->url;
                                //system("audioCapture.sh default");

                            }else if (pttstatus == ptt_level){


                            }else{

                            }
                        }
                    }

                    if((pttstatus > 0)&(!trx_incall.at(i)->m_PttPressed)&(!trx_incall.at(i)->sqlTestPressed))
                    {
                        trx_incall.at(i)->m_PttPressed = true;
                        updateHomeDisplay(trx_incall.at(i)->callIndexName,CONNECTED,RPTON,trx_incall.at(i)->call_id,0);
                        repeat_pressed(trx_incall.at(i)->call_id,pttstatus);
                        recorder_pressed(pttstatus,PTTON);
                        delayCount = 0;
                    }
                    else if((trx_incall.at(i)->sqlTestPressed == false) && (trx_incall.at(i)->sec_connDuration > 1) && ((trxStatus == RPTON) || (trxStatus == RXON)) && (trx_incall.at(i)->m_PttPressed == false))
                    {
                        trx_incall.at(i)->sqlTestPressed = true;
                        sqlTest_pressed(trx_incall.at(i)->call_id,1);
                    }

                    else if((pttstatus == 0)&(trx_incall.at(i)->m_PttPressed))
                    {
                        delayCount++;
                        if (delayCount < 20) return;
                         trx_incall.at(i)->m_PttPressed = false;

                         for (int j = 0;j < trx_incall.length(); j++){
                             if(trx_incall.at(j)->m_PttPressed)
                                  checkPTT++;
                         }
                         if (checkPTT == 0){
                             qDebug() << "checkPTT" << checkPTT << delayCount;
                             updateHomeDisplay(trx_incall.at(i)->callIndexName,CONNECTED,STANDBY,trx_incall.at(i)->call_id,0);
                             repeat_released(pttstatus);
                             recorder_released(pttstatus);
                         }
                         SLOT_VOLUME = 0.0f;
                         setSlotVolume(trx_incall.at(i)->call_id,false,true);
                         qDebug() << trx_incall.at(i)->callName << "Mute" << "ptt_level" << ptt_level << pttstatus;
                         ptt_level = pttstatus;
                         delayCount = 0;
                    }
                    else if((pttstatus > 0)&(!trx_incall.at(i)->m_PttPressed))
                    {
                        SLOT_VOLUME = 0.0f;
                        setSlotVolume(trx_incall.at(i)->call_id,false,true);
                        qDebug() << trx_incall.at(i)->callName << "Mute" << "ptt_level" << ptt_level << pttstatus;
                    }
                }
                else if((trx_incall.at(i)->call_id == PJSUA_INVALID_ID)&&(trx_incall.at(i)->callState == false))
                {
                    if ((trx_incall.at(i)->m_PttPressed) || (trx_incall.at(i)->sqlTestPressed))
                    {
                        trx_incall.at(i)->sqlTestPressed = false;
                        qDebug() << trx_incall.at(i)->callIndexName << DISCONNECTED << trx_incall.at(i)->m_PttPressed << trx_incall.at(i)->sqlTestPressed;
                        if ((trx_incall.at(i)->m_PttPressed))
                        {
                            for (int j = 0;j < trx_incall.length(); j++)
                            {
                                if(trx_incall.at(j)->m_PttPressed)
                                     checkPTT++;
                            }
                            if (checkPTT == 1){
                                qDebug() << "checkPTT" << checkPTT << delayCount;
                                updateHomeDisplay(trx_incall.at(i)->callIndexName,CONNECTED,STANDBY,trx_incall.at(i)->call_id,0);
                                repeat_released(0);
                                recorder_released(0);
                            }
                            trx_incall.at(i)->m_PttPressed = false;
                        }
                        if (trx_incall.at(i)->callName == recorderAlloweURI)
                        {
                            recorderConnected = false;
                        }
                    }
                }
            }
        }
    }
}

///////Spectrum/////////

void RoIP_ED137::initSpectrumGraph()
{
    d_realFftData = (float*) malloc(DEFAULT_FFT_SIZE * sizeof(float));
    d_iirFftData  = (float*) malloc(DEFAULT_FFT_SIZE * sizeof(float));

    for (int i = 0; i < DEFAULT_FFT_SIZE; i++)
        d_iirFftData[i] = -70.0f;  // dBFS

    for (int i = 0; i < DEFAULT_FFT_SIZE; i++)
        d_realFftData[i] = -70.0f;


    d_fftAvg = 1.0 - 1.0e-2 * ((float)75);
}



//////////button events//////////


void RoIP_ED137::initLabels()
{

}



void RoIP_ED137::changeUplinkOrder(const char* Input, char* Output, size_t sizeOfCoded, int g726UplinkBitrate) {


    if (g726UplinkBitrate == 1) //16kbps (2 bit)
    {
        for(unsigned int k = 0; k < sizeOfCoded ; k++)  // TO-DO
        {
            unsigned char temp = (Input[k] << 6) | (Input[k] >> 6) | ((Input[k] & 0x30) >> 2) | ((Input[k] & 0x0C) << 2);
            Output[k] = temp;
        }
    }
    else if (g726UplinkBitrate == 2) //24kbps (3 bit)
    {
        #pragma pack(push)
        #pragma pack(1)
            struct my_samples {
                unsigned char S3:2;
                unsigned char S2:3;
                unsigned char S1:3;
                unsigned char S6:1;
                unsigned char S5:3;
                unsigned char S4:3;
                unsigned char S3_:1;
                unsigned char S8:3;
                unsigned char S7:3;
                unsigned char S6_:2;
            };
            typedef struct my_samples my_samples_t;

            union data {
                unsigned char tempBuff[3];
                unsigned int v;
            };
        #pragma pack(pop)

        union data d;
        static my_samples_t samples;

        d.v = 0;
        int offset = 0;

        int numberOfBytes = sizeOfCoded;
        while(offset < (numberOfBytes))
        {
            memcpy (d.tempBuff, &Input[offset] , 3);

            unsigned int  V = d.v;

            samples.S1 = V & 0x00000007;
            samples.S2 = (V >> 3) & 0x00000007;
            samples.S3 = (V >> 7) & 0x00000003;
            samples.S3_ = (V >> 6) & 0x00000001;
            samples.S4 = (V >> 9) & 0x00000007;
            samples.S5 = (V >> 12) & 0x00000007;
            samples.S6 = (V >> 17) & 0x00000001;
            samples.S6_ = (V >> 15) & 0x00000003;
            samples.S7 = (V >> 18) & 0x00000007;
            samples.S8 = (V >> 21) & 0x00000007;

            memcpy (&Output[offset], &samples , 3);
            offset +=3;
        }
    }
    else if (g726UplinkBitrate == 3) //32kbps (4 bit)
    {
        for(unsigned int k = 0; k < sizeOfCoded ; k++)  // TO-DO
        {
            unsigned char temp = (Input[k]>>4) | (Input[k]<<4);
            Output[k] = temp;
        }
    }
    else if (g726UplinkBitrate == 4) //40kbps (5 bit)
    {
        #pragma pack(push)
        #pragma pack(1)
        struct my_samples {
        unsigned char S2:3;
        unsigned char S1:5;
        unsigned char S4:1;
        unsigned char S3:5;
        unsigned char S2_:2;
        unsigned char S5:4;
        unsigned char S4_:4;
        unsigned char S7:2;
        unsigned char S6:5;
        unsigned char S5_:1;
        unsigned char S8:5;
        unsigned char S7_:3;
        };
        typedef struct my_samples my_samples_t;

        unsigned char tempBuff[5];
        #pragma pack(pop)

        static my_samples_t samples;

        int offset = 0;

        int numberOfBytes = sizeOfCoded;
        while(offset < (numberOfBytes))
        {
            memcpy (tempBuff, &Input[offset] , 5);

            samples.S1 = tempBuff[0] & 0x1F;
            samples.S2 = ((tempBuff[1] << 1) | (tempBuff[0] >> 7)) & 0x07;
            samples.S2_ = (tempBuff[0] >> 5) & 0x04;
            samples.S3 = (tempBuff[1] >> 2) & 0x1F;
            samples.S4 = (tempBuff[2] >> 3) & 0x01;
            samples.S4_ = ((tempBuff[2] << 1) | (tempBuff[1] >> 7)) & 0x0F;
            samples.S5 = ((tempBuff[3] << 3) | (tempBuff[2] >> 5)) & 0x0F;
            samples.S5_ = (tempBuff[2] >> 4) & 0x01;
            samples.S6 = (tempBuff[3] >> 1) & 0x1F;
            samples.S7 = (tempBuff[4] >> 1) & 0x03;
            samples.S7_ = ((tempBuff[4] << 2) | (tempBuff[3] >> 6)) & 0x07;
            samples.S8 = tempBuff[4] >> 3;

            memcpy (&Output[offset], &samples, 5);
            offset +=5;
        }
    }
}
void RoIP_ED137::setOutgoingRTP(tp_adapter* adapter)
{
    (void) adapter;

    unsigned int pktlen    = adapter->send_payload_bufSize;
    const char* payloadbuf = (char*)adapter->tmp_payload_buf;
    const char* pktbuf     = (char*)adapter->send_pkt_buff;
    int payloadlen         = adapter->send_payload_bufSize;
    char sendPacket[payloadlen];
    if (inviteMode == SERVER)
    {
        uint8_t audioLevel = 0;
        int audioLevelSum = 0;
        for (int i = 0; i < payloadlen; i++)
        {
            audioLevelSum += payloadbuf[i];
        }
        audioLevel = audioLevelSum/payloadlen;
//        qDebug() << "setOutgoingRTP" << audioLevel << audioLevelSum << payloadlen;
        if (trx1->radio1->call_id == adapter->callID)
        {
            trx1->radio1->OutgoingRTP = audioLevel;
        }
        else if (trx1->radio2->call_id == adapter->callID)
        {
            trx1->radio2->OutgoingRTP = audioLevel;
        }
        else if (trx2->radio1->call_id == adapter->callID)
        {
            trx2->radio1->OutgoingRTP = audioLevel;
        }
        else if (trx2->radio2->call_id == adapter->callID)
        {
            trx2->radio2->OutgoingRTP = audioLevel;
        }
    }
}
void RoIP_ED137::setIncomingED137Value(uint32_t ed137_value,pjsua_acc_id acc_id)
{
    checkEvents();
    qDebug() << "on Rtp Audio Changed callID" << acc_id << ed137_value;
}
void RoIP_ED137::setIncomingRTP(tp_adapter* adapter)
{
    //    qDebug() << "setIncomingRTP";
        (void) adapter;

    //    if(isSipCall)
    //        return;

        unsigned int pktlen    = adapter->payload_bufSize;
        const char* payloadbuf = (char*)adapter->payload_buff;
        const char* pktbuf     = (char*)adapter->pkt_buff;
        int payloadlen         = adapter->payload_bufSize;
        char sendPacket[payloadlen];

    if (inviteMode == SERVER)
    {
        int i = 0;
        uint8_t audioLevel = 0;
        int audioLevelSum = 0;

        if (payloadlen == 164) i = 4;
        else if (payloadlen == 24) i = 4;

        for (i = 0; i < payloadlen; i++)
        {
            audioLevelSum += payloadbuf[i];
        }
        audioLevel = audioLevelSum/payloadlen;

        if (trx1->radio1->call_id == adapter->callID)
        {
            trx1->radio1->IncomingRTP = audioLevel;
        }
        else if (trx1->radio2->call_id == adapter->callID)
        {
            trx1->radio2->IncomingRTP = audioLevel;
        }
        else if (trx2->radio1->call_id == adapter->callID)
        {
            trx2->radio1->IncomingRTP = audioLevel;
        }
        else if (trx2->radio2->call_id == adapter->callID)
        {
            trx2->radio2->IncomingRTP = audioLevel;
        }
    }
}
void RoIP_ED137::sqlTest_pressed(int call_id, int level)
{
    setRadioSqlOnbyCallID(true,call_id,level);
}
void RoIP_ED137::sqlTest_pressed(int sqlLevel)
{
    if (inviteMode == CLIENT)
    {
        //system("audioCapture.sh default");
        for (int i = 0;i < trx_incall.length();i++)
        {
            if (trx_incall.at(i)->callName != recorderAlloweURI)
            {
                if (trx_incall.at(i)->callState)
                {
                    trx_incall.at(i)->sqlTestPressed = true;
                    if ((m_radioAutoInactive == true) & (m_radioMainStandby == STANDBYRADIO) & (trx_incall.at(i)->mainRadioReceiverUsed == true))
                        trx_incall.at(i)->sqlTestPressed = false;
                    else if ((m_radioAutoInactive == true) & (m_radioMainStandby == MAINRADIO) & (trx_incall.at(i)->mainRadioReceiverUsed == false))
                        trx_incall.at(i)->sqlTestPressed = false;
                    setRadioSqlOnbyCallID(trx_incall.at(i)->sqlTestPressed,trx_incall.at(i)->call_id,sqlLevel,rssi_raw);
                }
            }
        }
    }
    else if (inviteMode == CLIENT_RPT)
    {
        for (int i = 0;i < trx_incall.length();i++)
        {
            if (trx_incall.at(i)->callName != recorderAlloweURI)
            {
                if (trx_incall.at(i)->callState){
                    trx_incall.at(i)->sqlTestPressed = true;
                    setRadioSqlOnbyCallID(trx_incall.at(i)->sqlTestPressed,trx_incall.at(i)->call_id,sqlLevel,rssi_raw);
                }
            }
        }
    }
    else if (inviteMode == SERVER)
    {
        if ((trx2->mainRx)&&(trx1->mainRx))
        {
            if (trx1->radio1->callState){
                trx1->radio1->pptTestPressed = true;
                setRadioSqlOnbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,sqlLevel);
            }
            if (trx2->radio1->callState){
                trx2->radio1->pptTestPressed = true;
                setRadioSqlOnbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,sqlLevel);
            }
        }
        else if ((trx1->mainRx)&&(!trx2->mainRx))
        {
            if (trx1->radio1->callState){
                trx1->radio1->pptTestPressed = true;
                setRadioSqlOnbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,sqlLevel);
            }
            if (trx2->radio1->callState){
                trx2->radio1->pptTestPressed = false;
                setRadioSqlOnbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,sqlLevel);
            }
        }
        else if ((!trx1->mainRx)&&(trx2->mainRx))
        {
            if (trx1->radio1->callState){
                trx1->radio1->pptTestPressed = false;
                setRadioSqlOnbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,sqlLevel);
            }
            if (trx2->radio1->callState){
                trx2->radio1->pptTestPressed = true;
                setRadioSqlOnbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,sqlLevel);
            }
        }
        else if(txScheduler != SCH_PTT_DISABLE)
        {
            if (trx1->radio1->callState){
                trx1->radio1->pptTestPressed = true;
                setRadioSqlOnbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,sqlLevel);
                trx2->radio1->pptTestPressed = false;
                setRadioSqlOnbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,sqlLevel);
            } else if (trx2->radio1->callState){
                trx1->radio1->pptTestPressed = false;
                setRadioSqlOnbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,sqlLevel);
                trx2->radio1->pptTestPressed = true;
                setRadioSqlOnbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,sqlLevel);
            }
        }
        else
        {
            trx1->radio1->pptTestPressed = false;
            trx2->radio1->pptTestPressed = false;
            if (trx1->radio1->callState){
                setRadioSqlOnbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,sqlLevel);
            }
            if (trx2->radio1->callState){
                setRadioSqlOnbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,sqlLevel);
            }
        }
    }
}

void RoIP_ED137::repeat_pressed(int ignorID,int pttLevel)
{
    if (inviteMode == CLIENT_RPT)
    {
        //system("audioCapture.sh loop");
        for (int i = 0;i < trx_incall.length();i++)
        {
            if (trx_incall.at(i)->callName != recorderAlloweURI)
            {
                if ((trx_incall.at(i)->callState) & (trx_incall.at(i)->call_id != ignorID))
                {
                    trx_incall.at(i)->sqlTestPressed = true;
                    setRadioSqlOnbyCallID(trx_incall.at(i)->sqlTestPressed,trx_incall.at(i)->call_id,pttLevel,rssi_raw);
                }
                else if ((trx_incall.at(i)->callState)&(trx_incall.at(i)->call_id == ignorID))
                {
                    trx_incall.at(i)->m_PttPressed = true;
                    setRadioPttbyCallID(trx_incall.at(i)->m_PttPressed,trx_incall.at(i)->call_id,pttLevel,0);
                }
            }
        }
    }
}
void RoIP_ED137::repeat_released(int pttLevel)
{
    if (inviteMode == CLIENT_RPT)
    {
        for (int i = 0;i < trx_incall.length();i++)
        {
            if (trx_incall.at(i)->callName != recorderAlloweURI)
            {
                if (trx_incall.at(i)->callState)
                {
                    trx_incall.at(i)->sqlTestPressed = false;
                    trx_incall.at(i)->pptTestPressed = false;
                    setRadioSqlOnbyCallID(trx_incall.at(i)->sqlTestPressed,trx_incall.at(i)->call_id,pttLevel,rssi_raw);
                    setRadioPttbyCallID(trx_incall.at(i)->pptTestPressed,trx_incall.at(i)->call_id,pttLevel,0);
                }
            }
        }
    }
}

void RoIP_ED137::recorder_released(int pttLevel)
{
    for (int i = 0;i < trx_incall.length();i++)
    {
        if (trx_incall.at(i)->callName == recorderAlloweURI)
        {
            if (trx_incall.at(i)->callState)
            {
                qDebug() << "recorder_released";
                trx_incall.at(i)->sqlTestPressed = false;
                trx_incall.at(i)->pptTestPressed = false;
                setRadioSqlOnbyCallID(trx_incall.at(i)->sqlTestPressed,trx_incall.at(i)->call_id,pttLevel,rssi_raw);
                setRadioPttbyCallID(trx_incall.at(i)->pptTestPressed,trx_incall.at(i)->call_id,pttLevel,1);
            }
        }
    }
}

void RoIP_ED137::recorder_pressed(int pttLevel, QString txon)
{
    for (int i = 0;i < trx_incall.length();i++)
    {
        if (trx_incall.at(i)->callName == recorderAlloweURI)
        {
            if (trx_incall.at(i)->callState)
            {
                setAdaptercallRecorder(trx_incall.at(i)->call_id,1);
                qDebug() << "recorder_pressed" << txon;
                if (txon == PTTON)
                {
                    trx_incall.at(i)->pptTestPressed = true;
                    setRadioPttbyCallID(trx_incall.at(i)->pptTestPressed,trx_incall.at(i)->call_id,pttLevel,1);
                }
                else
                {
                    trx_incall.at(i)->sqlTestPressed = true;
                    setRadioSqlOnbyCallID(trx_incall.at(i)->sqlTestPressed,trx_incall.at(i)->call_id,pttLevel,rssi_raw);
                }
            }
        }
    }
}

void RoIP_ED137::sqlTest_released()
{
    if (inviteMode == CLIENT)
    {
        //system("audioCapture.sh kill");
        for (int i = 0;i < trx_incall.length();i++)
        {
            if (trx_incall.at(i)->callName != recorderAlloweURI)
            {
                if (trx_incall.at(i)->callState)
                {
                    trx_incall.at(i)->sqlTestPressed = false;
                    setRadioSqlOnbyCallID(trx_incall.at(i)->sqlTestPressed,trx_incall.at(i)->call_id,1,rssi_raw);
                }
            }
        }
    }
    else if (inviteMode == CLIENT_RPT)
    {
        //system("audioCapture.sh kill");
        for (int i = 0;i < trx_incall.length();i++)
        {
            if (trx_incall.at(i)->callName != recorderAlloweURI)
            {
                if (trx_incall.at(i)->callState){
                    trx_incall.at(i)->sqlTestPressed = false;
                    setRadioSqlOnbyCallID(trx_incall.at(i)->sqlTestPressed,trx_incall.at(i)->call_id,1,rssi_raw);
                }
            }
        }
    }
    else if (inviteMode == SERVER)
    {
        trx1->radio1->pptTestPressed = false;
        trx2->radio1->pptTestPressed = false;
        if (trx1->radio1->callState)
        {
            setRadioSqlOnbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,1);
        }
        if (trx2->radio1->callState)
        {
            setRadioSqlOnbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,1);
        }
    }
}

void RoIP_ED137::pptTest_CallIn_Pressed(int pttLevel)
{
    for(int i=0;i<trx_incall.length();i++)
    {
        if (trx_incall.at(i)->callName != recorderAlloweURI)
        {
            if (trx_incall.at(i)->callState)
            {
                trx_incall.at(i)->pptTestPressed = true;
                setRadioPttbyCallID(trx_incall.at(i)->pptTestPressed,trx_incall.at(i)->call_id,pttLevel,0);
            }
        }
    }
}

void RoIP_ED137::pptTest_CallIn_Released(int pttLevel)
{
    if (inviteMode == CLIENT)
    {
        for(int i=0;i<trx_incall.length();i++)
        {
            if (trx_incall.at(i)->callName != recorderAlloweURI)
            {
                if (trx_incall.at(i)->callState){
                    trx_incall.at(i)->pptTestPressed = false;
                    setRadioPttbyCallID(trx_incall.at(i)->pptTestPressed,trx_incall.at(i)->call_id,pttLevel,0);
                }
            }
        }
    }
    else if (inviteMode == CLIENT_RPT)
    {
        for(int i=0;i<trx_incall.length();i++)
        {
            if (trx_incall.at(i)->callName != recorderAlloweURI)
            {
                if (trx_incall.at(i)->callState)
                {
                    trx_incall.at(i)->sqlTestPressed = false;
                    trx_incall.at(i)->pptTestPressed = false;
    //                setRadioSqlOnbyCallID(trx_incall.at(i)->sqlTestPressed,trx_incall.at(i)->call_id,pttLevel);
                    setRadioPttbyCallID(trx_incall.at(i)->pptTestPressed,trx_incall.at(i)->call_id,pttLevel,0);
                }
            }
        }
    }
}

void RoIP_ED137::setvolumeSiteTone(pjsua_call_id call_id)
{
    if ((call_id == trx1->radio1->call_id)||(call_id == trx1->radio2->call_id))
        SLOT_VOLUME = sidetone;
    else if ((call_id == trx2->radio1->call_id)||(call_id == trx2->radio2->call_id))
        SLOT_VOLUME = sidetone;
    else
        SLOT_VOLUME = 0.5f;
    setSlotVolume(call_id,false,true);
}

void RoIP_ED137::pptTest_pressed()
{
    if (!connToRadio)
    {        
        qDebug() << "mainTx" << trx1->mainTx << trx2->mainTx;
        if ((trx2->mainTx)&&(trx1->mainTx))
        {
            if (trx1->radio1->callState)
            {
                trx1->radio1->pptTestPressed = true;
                setRadioPttbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,1,0);
            }
            if (trx2->radio1->callState){
                trx2->radio1->pptTestPressed = true;
                setRadioPttbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,1,0);
            }
        }
        else if ((trx1->mainTx)&&(!trx2->mainTx))
        {
            if (trx1->radio1->callState){
                trx1->radio1->pptTestPressed = true;
                setRadioPttbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,1,0);
            }
            if (trx2->radio1->callState){
                trx2->radio1->pptTestPressed = false;
                setRadioPttbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,1,0);
            }
        }
        else if ((!trx1->mainTx)&&(trx2->mainTx))
        {
            if (trx1->radio1->callState){
                trx1->radio1->pptTestPressed = false;
                setRadioPttbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,1,0);
            }
            if (trx2->radio1->callState){
                trx2->radio1->pptTestPressed = true;
                setRadioPttbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,1,0);
            }
        }
        else
        {
            trx1->radio1->pptTestPressed = false;
            trx2->radio1->pptTestPressed = false;
            if (trx1->radio1->callState){
                setRadioPttbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,1,0);
            }
            if (trx2->radio1->callState){
                setRadioPttbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,1,0);
            }
        }

        if (trx1->radio1->pptTestPressed)
            createPTTEventDataLogger(trx1->radio1,"pptTest_pressed");
        if (trx2->radio1->pptTestPressed)
            createPTTEventDataLogger(trx2->radio1,"pptTest_pressed");
    }
    else if (connToRadio)
    {
        if ((trx2->mainTx)&&(trx1->mainTx)){
            if (trx1->radio1->callState){
                trx1->radio1->sqlTestPressed = true;
                setRadioSqlOnbyCallID(trx1->radio1->sqlTestPressed,trx1->radio1->call_id,1);
            }
            if (trx2->radio1->callState){
                trx2->radio1->sqlTestPressed = true;
                setRadioSqlOnbyCallID(trx2->radio1->sqlTestPressed,trx2->radio1->call_id,1);
            }
        }
        else if ((trx1->mainTx)&&(!trx2->mainTx)){
            if (trx1->radio1->callState){
                trx1->radio1->sqlTestPressed = true;
                setRadioSqlOnbyCallID(trx1->radio1->sqlTestPressed,trx1->radio1->call_id,1);
            }
            if (trx2->radio1->callState){
                trx2->radio1->sqlTestPressed = false;
                setRadioSqlOnbyCallID(trx2->radio1->sqlTestPressed,trx2->radio1->call_id,1);
            }
        }
        else if ((!trx1->mainTx)&&(trx2->mainTx)){
            if (trx1->radio1->callState){
                trx1->radio1->sqlTestPressed = false;
                setRadioSqlOnbyCallID(trx1->radio1->sqlTestPressed,trx1->radio1->call_id,1);
            }
            if (trx2->radio1->callState){
                trx2->radio1->sqlTestPressed = true;
                setRadioSqlOnbyCallID(trx2->radio1->sqlTestPressed,trx2->radio1->call_id,1);
            }
        }
        else if(txScheduler != SCH_PTT_DISABLE){
            if (trx1->radio1->callState){
                trx1->radio1->sqlTestPressed = true;
                setRadioSqlOnbyCallID(trx1->radio1->sqlTestPressed,trx1->radio1->call_id,1);
                trx2->radio1->sqlTestPressed = false;
                setRadioSqlOnbyCallID(trx2->radio1->sqlTestPressed,trx2->radio1->call_id,1);
            } else if (trx2->radio1->callState){
                trx1->radio1->sqlTestPressed = false;
                setRadioSqlOnbyCallID(trx1->radio1->sqlTestPressed,trx1->radio1->call_id,1);
                trx2->radio1->sqlTestPressed = true;
                setRadioSqlOnbyCallID(trx2->radio1->sqlTestPressed,trx2->radio1->call_id,1);
            }
        }
        else{
            trx1->radio1->sqlTestPressed = false;
            trx2->radio1->sqlTestPressed = false;
            if (trx1->radio1->callState){
                setRadioSqlOnbyCallID(trx1->radio1->sqlTestPressed,trx1->radio1->call_id,1);
            }
            if (trx2->radio1->callState){
                setRadioSqlOnbyCallID(trx2->radio1->sqlTestPressed,trx2->radio1->call_id,1);
            }
        }
        if (trx1->radio1->pptTestPressed)
            createPTTEventDataLogger(trx1->radio1,"SQLTest_pressed");
        if (trx2->radio1->pptTestPressed)
            createPTTEventDataLogger(trx2->radio1,"SQLTest_pressed");
    }

}

void RoIP_ED137::pptTest_released()
{
    //system("audioCapture.sh kill");
    qDebug() << "pptTest_released";
    if (!connToRadio)
    {
        if (trx1->radio1->pptTestPressed)
            createPTTEventDataLogger(trx1->radio1,"pptTest_released");
        if (trx2->radio1->pptTestPressed)
            createPTTEventDataLogger(trx2->radio1,"pptTest_released");

        trx1->radio1->pptTestPressed = false;
        trx2->radio1->pptTestPressed = false;
        if (trx1->radio1->callState){
            setRadioPttbyCallID(trx1->radio1->pptTestPressed,trx1->radio1->call_id,1,0);
        }
        if (trx2->radio1->callState){
            setRadioPttbyCallID(trx2->radio1->pptTestPressed,trx2->radio1->call_id,1,0);
        }
    }
    else // 4Wire Connect to Radio
    {
        trx1->radio1->sqlTestPressed = false;
        trx2->radio1->sqlTestPressed = false;
        if (trx1->radio1->callState){
            setRadioSqlOnbyCallID(trx1->radio1->sqlTestPressed,trx1->radio1->call_id,1);
        }
        if (trx2->radio1->callState){
            setRadioSqlOnbyCallID(trx2->radio1->sqlTestPressed,trx2->radio1->call_id,1);
        }

        if (trx1->radio1->pptTestPressed)
            createPTTEventDataLogger(trx1->radio1,"sqlTest_released");
        if (trx2->radio1->pptTestPressed)
            createPTTEventDataLogger(trx2->radio1,"sqlTest_released");
    }
}

void RoIP_ED137::onConnected()
{
    QString message = QString("{"
                      "\"menuID\"               :\"startSoftPhone\", "
                      "\"softPhoneID\"          :%1 "
                      "}"
                      ).arg(m_softPhoneID);
    emit cppCommand(message);
}

void RoIP_ED137::onTextMessageReceived(QString message)
{
    qDebug() << "Message received:" << message;
    commandProcess(message);
}


void RoIP_ED137::onTextMessageToSend(QString message)
{
    qDebug() << "Message to send:" << message;
    socketClient->sendTextMessage(message);
}



void RoIP_ED137::sendChannelMessage()
{
    QString  message = "";

    if (inviteMode == SERVER)
    {
        message = QString("{"
                          "\"menuID\"                          :\"channelMessage\", "
                          "\"softPhoneID\"                     :%1, "
                          "\"pttInput_count\"                  :%2, "
                          "\"forceMuteSqlOn\"                  :%3 "
                          "}").arg(m_softPhoneID).arg(pttInput_count).arg(forceMuteSqlOn);

        if (iGateGroupMNGClientMessage != message)
        {
            if((pttInput_count <= 1) || (forceMuteSqlOn))
                iGateGroupMNGClient->sendTextMessage(message);
            iGateGroupMNGClientMessage = message;
//            qDebug() << "iGateGroupMNGClientMessage" << message;
        }
    }
//    qDebug() << "sendChannelMessage trx1->trxStatus" << trx1->trxStatus;
    int numConn = getActiveCallCount();
    QString radioType = "";
    int frequency = 0;
    int numTxRx = trx1->radio1->enable +trx1->radio2->enable +trx2->radio1->enable +trx2->radio2->enable;
    bool mainTxConnStatus = trx1->radio1->callState;
    bool mainRxConnStatus = trx1->radio2->callState;
    bool standbyTxConnStatus = trx2->radio1->callState;
    bool standbyRxConnStatus = trx2->radio2->callState;
    bool mainTxPTTOn = ((trx1->trxStatus == PTTON) || (trx1->trxStatus == PTTRX));
    bool mainRxSQLOn = ((trx1->trxStatus == RXON) || (trx1->trxStatus == PTTRX));
    bool standbyTxPTTOn = ((trx2->trxStatus == PTTON) || (trx2->trxStatus == PTTRX));
    bool standbyRxSQLOn = ((trx2->trxStatus == RXON) || (trx2->trxStatus == PTTRX));
    bool mainRxRadioVolumeActive = trx1->radio2->sqlTestPressed;
    bool standbyRxRadioVolumeActive = trx2->radio2->sqlTestPressed;
    int mainRadioBSSRxLevel = trx1->radio2->rssi;
    int standbyRadioBSSRxLevel = trx2->radio2->rssi;
    int mainRadioTxRFPower = trx1->radio1->rfPower;
    int standbyRadioTxRFPower = trx2->radio1->rfPower;
    QString mainSqlchThrhCarrier = trx1->radio2->SqlchThrhCarrier;
    QString standbySqlchThrhCarrier = trx2->radio2->SqlchThrhCarrier;
    QString mainTxConnDuration = trx1->radio1->connDuration;
    QString mainRxConnDuration = trx1->radio2->connDuration;
    QString standbyTxConnDuration = trx2->radio1->connDuration;
    QString standbyRxConnDuration = trx2->radio2->connDuration;
    QString mainTxFrequency = trx1->radio1->frequency;
    QString mainRxFrequency = trx1->radio2->frequency;
    QString standbyTxFrequency = trx2->radio1->frequency;
    QString standbyRxFrequency = trx2->radio2->frequency;
    QString mainTxRadiostatus = trx1->radio1->radiostatus;
    QString mainRxRadiostatus = trx1->radio2->radiostatus;
    QString standbyTxRadiostatus = trx2->radio1->radiostatus;
    QString standbyRxRadiostatus = trx2->radio2->radiostatus;
    bool main01_is_mainTx = trx1->mainTx;
    bool main01_is_mainRx = trx1->mainRx;
    bool main02_is_mainRx = trx1->mainRx;
    bool standby01_is_mainTx = trx2->mainTx;
    bool standby01_is_mainRx = trx2->mainRx;
    bool standby02_is_mainRx = trx2->mainRx;
    bool standByEnable = (trx1->enable & trx2->enable);
    QString mainRadio01URI = trx1->radio1->url;
    QString mainRadio02URI = trx1->radio2->url;
    QString standbyRadio01URI = trx2->radio1->url;
    QString standbyRadio02URI = trx2->radio2->url;
    QString mainRadio01IPAddr = trx1->radio1->ipAddress;
    QString mainRadio02IPAddr = trx1->radio2->ipAddress;
    QString standbyRadio01IPAddr = trx2->radio1->ipAddress;
    QString standbyRadio02IPAddr = trx2->radio2->ipAddress;
    QString mainRadioTRxType = trx1->trxmode;
    QString standbyRadioTRxType = trx2->trxmode;
    int softPhoneID = m_softPhoneID;

    if (localSidetoneLoopbackOn)
    {
        mainTxPTTOn = true;        
    }
//    qDebug() << "sendChannelMessage localSidetoneLoopbackOn" << localSidetoneLoopbackOn;

    message = QString("{"
                               "\"menuID\"                          :\"channelMessage\", "
                               "\"numConn\"                         :  %1, "
                               "\"radioType\"                       :\"%2\", "
                               "\"frequency\"                       :  %3, "
                               "\"numTxRx\"                         :  %4, "
                               "\"mainTxConnStatus\"                :  %5, "
                               "\"mainRxConnStatus\"                :  %6, "
                               "\"standbyTxConnStatus\"             :  %7, "
                               "\"standbyRxConnStatus\"             :  %8, "
                               "\"mainTxPTTOn\"                     :  %9, "
                               "\"mainRxSQLOn\"                     :  %10, "
                               "\"standbyTxPTTOn\"                  :  %11, "
                               "\"standbyRxSQLOn\"                  :  %12, "
                               "\"mainRxRadioVolumeActive\"         :  %13, "
                               "\"standbyRxRadioVolumeActive\"      :  %14, "
                               "\"mainRadioBSSRxLevel\"             :  %15, "
                               "\"standbyRadioBSSRxLevel\"          :  %16, "
                               "\"mainRadioTxRFPower\"              :\"%17\", "
                               "\"standbyRadioTxRFPower\"           :\"%18\", "
                               "\"mainSqlchThrhCarrier\"            :  %19, "
                               "\"standbySqlchThrhCarrier\"         :  %20, "
                               "\"mainTxConnDuration\"              :\"%21\", "
                               "\"mainRxConnDuration\"              :\"%22\", "
                               "\"standbyTxConnDuration\"           :\"%23\", "
                               "\"standbyRxConnDuration\"           :\"%24\", "
                               "\"mainTxFrequency\"                 :\"%25\", "
                               "\"mainRxFrequency\"                 :\"%26\", "
                               "\"standbyTxFrequency\"              :\"%27\", "
                               "\"standbyRxFrequency\"              :\"%28\", "
                               "\"mainTxRadiostatus\"               :\"%29\", "
                               "\"mainRxRadiostatus\"               :\"%30\", "
                               "\"standbyTxRadiostatus\"            :\"%31\", "
                               "\"standbyRxRadiostatus\"            :\"%32\", "
                               "\"main01_is_mainTx\"                :  %33, "
                               "\"main01_is_mainRx\"                :  %34, "
                               "\"main02_is_mainRx\"                :  %35, "
                               "\"standby01_is_mainTx\"             :  %36, "
                               "\"standby01_is_mainRx\"             :  %37, "
                               "\"standby02_is_mainRx\"             :  %38, "
                               "\"standByEnable\"                   :  %39, "
                               "\"mainRadio01URI\"                  :\"%40\", "
                               "\"mainRadio02URI\"                  :\"%41\", "
                               "\"standbyRadio01URI\"               :\"%42\", "
                               "\"standbyRadio02URI\"               :\"%43\", "
                               "\"mainRadio01IPAddr\"               :\"%44\", "
                               "\"mainRadio02IPAddr\"               :\"%45\", "
                               "\"standbyRadio01IPAddr\"            :\"%46\", "
                               "\"standbyRadio02IPAddr\"            :\"%47\", "
                               "\"softPhoneID\"                     :  %48,   "
                               "\"mainRadioTRxType\"                :\"%49\", "
                               "\"standbyRadioTRxType\"             :\"%50\", "
                               "\"pttInput_count\"                  :%51,  "
                               "\"forceMuteSqlOn\"                    :%52 "
                               "}"
                               ).arg(numConn)
                                .arg(radioType)
                                .arg(frequency)
                                .arg(numTxRx)
                                .arg(mainTxConnStatus)
                                .arg(mainRxConnStatus)
                                .arg(standbyTxConnStatus)
                                .arg(standbyRxConnStatus)
                                .arg(mainTxPTTOn)
                                .arg(mainRxSQLOn)
                                .arg(standbyTxPTTOn)
                                .arg(standbyRxSQLOn)
                                .arg(mainRxRadioVolumeActive)
                                .arg(standbyRxRadioVolumeActive)
                                .arg(mainRadioBSSRxLevel)
                                .arg(standbyRadioBSSRxLevel)
                                .arg(mainRadioTxRFPower)
                                .arg(standbyRadioTxRFPower)
                                .arg(mainSqlchThrhCarrier)
                                .arg(standbySqlchThrhCarrier)
                                .arg(mainTxConnDuration)
                                .arg(mainRxConnDuration)
                                .arg(standbyTxConnDuration)
                                .arg(standbyRxConnDuration)
                                .arg(mainTxFrequency)
                                .arg(mainRxFrequency)
                                .arg(standbyTxFrequency)
                                .arg(standbyRxFrequency)
                                .arg(mainTxRadiostatus)
                                .arg(mainRxRadiostatus)
                                .arg(standbyTxRadiostatus)
                                .arg(standbyRxRadiostatus)
                                .arg(main01_is_mainTx)
                                .arg(main01_is_mainRx)
                                .arg(main02_is_mainRx)
                                .arg(standby01_is_mainTx)
                                .arg(standby01_is_mainRx)
                                .arg(standby02_is_mainRx)
                                .arg(standByEnable)
                                .arg(mainRadio01URI)
                                .arg(mainRadio02URI)
                                .arg(standbyRadio01URI)
                                .arg(standbyRadio02URI)
                                .arg(mainRadio01IPAddr)
                                .arg(mainRadio02IPAddr)
                                .arg(standbyRadio01IPAddr)
                                .arg(standbyRadio02IPAddr)
                                .arg(softPhoneID)
                                .arg(mainRadioTRxType)
                                .arg(standbyRadioTRxType)
                                .arg(pttInput_count)
                                .arg(forceMuteSqlOn)
            ;
    if (dataChannelMessage != message)
    {
        dataChannelMessage = message;
        socketClient->sendTextMessage(message);
        if (pttInput_count == 1)
            qDebug() << "channelMessage" << message;
    }
}
void RoIP_ED137::commandProcess(QString message)
{
    QJsonDocument d = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject command = d.object();
    QString getCommand =  QJsonValue(command["menuID"]).toString();
//    qDebug() << "c++:" << "getCommand" << getCommand << message;
    int softPhoneID             = QJsonValue(command["softPhoneID"]).toInt();
    if (m_softPhoneID == softPhoneID){
//            qDebug() << "c++:" << "getCommand" << getCommand << message;
        if (getCommand == ("controlerData"))
        {
            string _SipUser             = QString(QJsonValue(command["sipUser"]).toString()).toStdString();
            int _clock_rate             = QJsonValue(command["clock_rate"]).toInt();
            quint16 _sip_port           = QJsonValue(command["sipPort"]).toInt();
            quint16 _keepAlivePeroid    = QJsonValue(command["keepAlivePeroid"]).toInt();
            quint8 _txScheduler         = QJsonValue(command["txScheduler"]).toInt();
            quint8 _numConnection       = QJsonValue(command["numConnection"]).toInt();
            quint8 _inviteMode          = QJsonValue(command["inviteMode"]).toInt();
            float _sidetone             = QJsonValue(command["sidetone"]).toDouble();
            float _localSidetone             = QJsonValue(command["localSidetone"]).toDouble();
            bool _connToRadio           = QJsonValue(command["WireConnectMode"]).toInt();
            bool _sqlAlwayOn            = QJsonValue(command["sqlAlwayOn"]).toInt();
            QString _defaultEthernet    = QJsonValue(command["defaultEthernet"]).toString();
            bool _sqlActiveHigh         = QJsonValue(command["sqlActiveHigh"]).toInt();
            int _rtp_cfg_port           = QJsonValue(command["rtpStartPort"]).toInt();
            int channelID               = QJsonValue(command["channelID"]).toInt();
            bool _radioAutoInactive       = QJsonValue(command["radioAutoInactive"]).toInt();
            bool _mainRadioTransmitterUsed              = QJsonValue(command["mainRadioTransmitterUsed"]).toInt();
            bool _mainRadioReceiverUsed              = QJsonValue(command["mainRadioReceiverUsed"]).toInt();
            int _radioMainStandby              = QJsonValue(command["radioMainStandby"]).toInt();
            QString defaultIPAddr       = QJsonValue(command["defaultIPAddr"]).toString();
            bool _rxBestSignalEnable    = QJsonValue(command["rxBestSignalEnable"]).toInt();
            bool _groupMute    = QJsonValue(command["groupMute"]).toInt();
            int _pttDelay    = QJsonValue(command["pttDelay"]).toInt();

            qDebug() << "controlerData" << message;
            initSoftPhone(_SipUser, _clock_rate, _sip_port, _rtp_cfg_port, _keepAlivePeroid, _txScheduler, _numConnection, _inviteMode, _sidetone, _connToRadio, _sqlAlwayOn, _defaultEthernet, _sqlActiveHigh, _mainRadioTransmitterUsed, _mainRadioReceiverUsed, _radioAutoInactive, _radioMainStandby,_rxBestSignalEnable,_localSidetone,_groupMute,_pttDelay);
        }
        else if(getCommand == ("uriAllowList"))
        {
            QStringList uriAllowList;
            uriAllowList << QJsonValue(command["allowUri1"]).toString();
            uriAllowList << QJsonValue(command["allowUri2"]).toString();
            uriAllowList << QJsonValue(command["allowUri3"]).toString();
            uriAllowList << QJsonValue(command["allowUri4"]).toString();
            uriAllowList << QJsonValue(command["allowUri5"]).toString();
            uriAllowList << QJsonValue(command["allowUri6"]).toString();
            uriAllowList << QJsonValue(command["allowUri7"]).toString();
            uriAllowList << QJsonValue(command["allowUri8"]).toString();
            initUriNameAllowList(uriAllowList);
        }
        else if(getCommand == ("initRadio"))
        {
            quint8 trxMainSTB = QJsonValue(command["trxMainSTB"]).toInt(); //1 Main, 2 Standby
            QString channelName = QJsonValue(command["channelName"]).toString();
            QString trxmode = QJsonValue(command["trxmode"]).toString();
            bool enable = QJsonValue(command["enable"]).toInt();
            bool alwaysConnect = QJsonValue(command["alwaysConnect"]).toInt();
            QString radio1callName = QJsonValue(command["radio1callName"]).toString();
            QString radio1ipAddress = QJsonValue(command["radio1ipAddress"]).toString();
            QString radio1portAddr = QJsonValue(command["radio1portAddr"]).toString();
            QString radio2callName = QJsonValue(command["radio2callName"]).toString();
            QString radio2ipAddress = QJsonValue(command["radio2ipAddress"]).toString();
            QString radio2portAddr = QJsonValue(command["radio2portAddr"]).toString();


            initRadio( trxMainSTB,  channelName,  trxmode,  enable, alwaysConnect,  radio1callName,  radio1ipAddress, radio1portAddr,  radio2callName,  radio2ipAddress,  radio2portAddr);

        }
        else if(getCommand == ("startSoftphone"))
        {
            startSoftPhone();
        }

        else if (getCommand == ("updateDeviceName"))
        {
            QString deviceName = QJsonValue(command["deviceName"]).toString();
            updateDeviceName(deviceName);
        }
        else if (getCommand == ("updateHostCfg"))
        {
            int keepAlivePeroid = QJsonValue(command["keepAlivePeroid"]).toInt();
            int sipPort = QJsonValue(command["sipPort"]).toInt();
            QString sipUser =  QJsonValue(command["name"]).toString();
            QString defaultEthernet = QJsonValue(command["defaultEthernet"]).toString();
            qDebug() << "updateHostCfg" << sipUser << sipPort << keepAlivePeroid << defaultEthernet;
            updateHostConfig(sipUser, sipPort, keepAlivePeroid, defaultEthernet);
        }
        else if (getCommand == ("updateInputgain"))
        {

        }
        else if (getCommand == ("updateOutputgain"))
        {

        }
        else if (getCommand == ("disconnect"))
        {
            QString value = QJsonValue(command["uri"]).toString();
            value = value.replace(" ","");
            disconnectUri(value);
        }
        else if (getCommand == ("updateSidetone"))
        {
            sidetone = QJsonValue(command["sidetone"]).toDouble();
        }
        else if (getCommand == ("updateLocalSidetone"))
        {
            localSidetone = QJsonValue(command["localSidetone"]).toDouble();
        }
        else if (getCommand == ("updateGroupMute"))
        {
            groupMute = QJsonValue(command["groupMute"]).toInt();
        }
        else if (getCommand == ("updatePttDelay"))
        {
            pttDelay_set = QJsonValue(command["pttDelay"]).toInt();
        }
        else if (getCommand == ("updateURILits"))
        {
            QStringList urlList = QJsonValue(command["urlList"]).toString().split(",");
            updateURILits(urlList);

            int numconn = QJsonValue(command["numconn"]).toInt();
            updateNumconn(numconn);

        }
        else if (getCommand == ("updateSQLDefeat"))
        {
            uint8_t softPhoneID = QJsonValue(command["softPhoneID"]).toInt();
            updateSQLDefeat(QJsonValue(command["sqlDefeat"]).toString().contains("true"));
        }
        else if (getCommand == ("updateSQLActiveHigh"))
        {
            uint8_t softPhoneID = QJsonValue(command["softPhoneID"]).toInt();
            updateSQLActiveHigh(QJsonValue(command["sqlActive"]).toString().contains("true"));
        }
        else if (getCommand == ("updateControler"))
        {
            QString sipUser = QJsonValue(command["sipUser"]).toString();
            int sipPort = QJsonValue(command["sipPort"]).toInt();
            int keepAlive = QJsonValue(command["keepAlivePeroid"]).toInt();
            int softPhoneId = QJsonValue(command["softPhoneID"]).toInt();
            defaultEthernet = QJsonValue(command["defaultEthernet"]).toString();
            qDebug() << "updateControler" << "defaultEthernet" << defaultEthernet;
            if (softPhoneId == m_softPhoneID)
            {
                updateHostConfig(sipUser,sipPort,keepAlive);

                QString message = QString("{"
                                  "\"menuID\"               :\"controlerData\", "
                                  "\"sipUser\"              :\"%1\", "
                                  "\"sipPort\"              :%2, "
                                  "\"keepAlivePeroid\"      :%3 "
                                  "}"
                                  ).arg(QString::fromStdString(SipUser)).arg(sip_port).arg(keepAlivePeroid);
                emit cppCommand(message);
            }
        }
        else if (getCommand == ("updateControler2"))
        {
            string _SipUser             = QString(QJsonValue(command["sipUser"]).toString()).toStdString();
            int _clock_rate             = QJsonValue(command["clock_rate"]).toInt();
            quint16 _sip_port           = QJsonValue(command["sipPort"]).toInt();
            quint16 _keepAlivePeroid    = QJsonValue(command["keepAlivePeroid"]).toInt();
            quint8 _txScheduler         = QJsonValue(command["txScheduler"]).toInt();
            quint8 _numConnection       = QJsonValue(command["numConnection"]).toInt();
            quint8 _inviteMode          = QJsonValue(command["inviteMode"]).toInt();
            float _sidetone             = QJsonValue(command["sidetone"]).toDouble();
            float _localSidetone             = QJsonValue(command["localSidetone"]).toDouble();
            bool _connToRadio           = QJsonValue(command["WireConnectMode"]).toInt();
            bool _sqlAlwayOn            = QJsonValue(command["sqlAlwayOn"]).toInt();
            QString _defaultEthernet    = QJsonValue(command["defaultEthernet"]).toString();
            bool _sqlActiveHigh         = QJsonValue(command["sqlActiveHigh"]).toInt();
            int _rtp_cfg_port           = QJsonValue(command["rtpStartPort"]).toInt();
            int channelID               = QJsonValue(command["channelID"]).toInt();
            bool _radioAutoInactive       = QJsonValue(command["radioAutoInactive"]).toInt();
            bool _mainRadioTransmitterUsed              = QJsonValue(command["mainRadioTransmitterUsed"]).toInt();
            bool _mainRadioReceiverUsed              = QJsonValue(command["mainRadioReceiverUsed"]).toInt();
            int _radioMainStandby              = QJsonValue(command["radioMainStandby"]).toInt();
            QString defaultIPAddr       = QJsonValue(command["defaultIPAddr"]).toString();
            bool _rxBestSignalEnable    = QJsonValue(command["rxBestSignalEnable"]).toInt();
            bool _groupMute    = QJsonValue(command["groupMute"]).toInt();
            int _pttDelay    = QJsonValue(command["pttDelay"]).toInt();
            int softPhoneId = QJsonValue(command["softPhoneID"]).toInt();
            if (softPhoneId == m_softPhoneID)
            {
                qDebug() << "updateControler2" << message;
                initSoftPhone(_SipUser, _clock_rate, _sip_port, _rtp_cfg_port, _keepAlivePeroid, _txScheduler, _numConnection, _inviteMode, _sidetone, _connToRadio, _sqlAlwayOn, _defaultEthernet, _sqlActiveHigh, _mainRadioTransmitterUsed, _mainRadioReceiverUsed, _radioAutoInactive,_radioMainStandby,_rxBestSignalEnable,_localSidetone,_groupMute,_pttDelay);
                updateHostConfig(QString(QJsonValue(command["sipUser"]).toString()),_sip_port,_keepAlivePeroid);

                QString message = QString("{"
                                  "\"menuID\"               :\"controlerData\", "
                                  "\"sipUser\"              :\"%1\", "
                                  "\"sipPort\"              :%2, "
                                  "\"keepAlivePeroid\"      :%3 "
                                  "}"
                                  ).arg(QString::fromStdString(SipUser)).arg(sip_port).arg(keepAlivePeroid);
                emit cppCommand(message);
            }
        }
        else if (getCommand == ("restartSoftphone"))
        {
            int softPhoneId = QJsonValue(command["softPhoneID"]).toInt();
            if (softPhoneId != m_softPhoneID) return;
            pjHangupAll();
            removeDefault();
            RegisterDefault();
            updateUri();
            qDebug() << "restartSoftphone";
            exit(0);
        }
        else if (getCommand == ("updateRadioAutoInactive"))
        {
            int softPhoneId = QJsonValue(command["softPhoneID"]).toInt();
            if (softPhoneId != m_softPhoneID) return;
            m_radioAutoInactive = (QJsonValue(command["radioAutoInactive"]).toInt() == 1) || QString(QJsonValue(command["radioAutoInactive"]).toString()).contains("true");

//            int i=0;
//            if (m_radioAutoInactive == true)
//            {
//                for (i=0; i<trx_incall.length(); i++)
//                {
//                    if (trx_incall.at(i)->callState)
//                    {
//                        i++;
//                    }
//                }

//                if (i > 0)
//                {
//                    mainRadioTransmitterUsed = true;
//                    mainRadioReceiverUsed = true;
//                }
//                else
//                {
//                    mainRadioTransmitterUsed = false;
//                    mainRadioReceiverUsed = false;
//                }
//            }

//            for (i=0; i<trx_incall.length(); i++)
//            {
//                if (trx_incall.at(i)->callState)
//                {
//                    setSlaveEnable(trx_incall.at(i)->call_id,(!mainRadioReceiverUsed), (!mainRadioTransmitterUsed));
//                }
//            }
            QString message = QString("{"
                                      "\"menuID\"                       :\"RadioActive\", "
                                      "\"softPhoneID\"                  :%1, "
                                      "\"radioAutoInactive\"            :%2, "
                                      "\"radioMainStandby\"             :%3 "
                                      "}"
                                      ).arg(m_softPhoneID).arg(m_radioAutoInactive).arg(m_radioMainStandby);
            emit cppCommand(message);

        }
        else if (getCommand == ("updateMainRadioReceiverUsed"))
        {
//            int softPhoneId = QJsonValue(command["softPhoneID"]).toInt();
//            if (softPhoneId != m_softPhoneID) return;
//            mainRadioReceiverUsed = (QJsonValue(command["mainRadioReceiverUsed"]).toInt() == 1) || QString(QJsonValue(command["mainRadioReceiverUsed"]).toString()).contains("true");
//            QString message = QString("{"
//                                      "\"menuID\"                       :\"RadioActive\", "
//                                      "\"softPhoneID\"                  :%1, "
//                                      "\"radioAutoInactive\"            :%2, "
//                                      "\"mainRadioReceiverUsed\"        :%3, "
//                                      "\"mainRadioTransmitterUsed\"     :%4, "
//                                      "\"radioMainStandby\"             :%5 "
//                                      "}"
//                                      ).arg(m_softPhoneID).arg(m_radioAutoInactive).arg(mainRadioReceiverUsed).arg(mainRadioTransmitterUsed).arg(m_radioMainStandby);
//            emit cppCommand(message);
//            int i=0;
//            for (i=0; i<trx_incall.length(); i++)
//            {
//                if (trx_incall.at(i)->callState)
//                {
//                    setSlaveEnable(trx_incall.at(i)->call_id,(!mainRadioReceiverUsed), (!mainRadioTransmitterUsed));
//                }
//            }
        }

        else if (getCommand == ("updateMainRadioTransmitterUsed"))
        {
//            int softPhoneId = QJsonValue(command["softPhoneID"]).toInt();
//            if (softPhoneId != m_softPhoneID) return;
//            mainRadioTransmitterUsed = (QJsonValue(command["mainRadioTransmitterUsed"]).toInt() == 1) || QString(QJsonValue(command["mainRadioTransmitterUsed"]).toString()).contains("true");
//            QString message = QString("{"
//                                      "\"menuID\"                       :\"RadioActive\", "
//                                      "\"softPhoneID\"                  :%1, "
//                                      "\"radioAutoInactive\"            :%2, "
//                                      "\"mainRadioReceiverUsed\"        :%3, "
//                                      "\"mainRadioTransmitterUsed\"     :%4, "
//                                      "\"radioMainStandby\"             :%5 "
//                                      "}"
//                                      ).arg(m_softPhoneID).arg(m_radioAutoInactive).arg(mainRadioReceiverUsed).arg(mainRadioTransmitterUsed).arg(m_radioMainStandby);
//            emit cppCommand(message);
//            int i=0;
//            for (i=0; i<trx_incall.length(); i++)
//            {
//                if (trx_incall.at(i)->callState)
//                {
//                    setSlaveEnable(trx_incall.at(i)->call_id,(!mainRadioReceiverUsed), (!mainRadioTransmitterUsed));
//                }
//            }
        }
        else if (getCommand == ("updateRadioMainStandby"))
        {
            int softPhoneId = QJsonValue(command["softPhoneID"]).toInt();
            if (softPhoneId != m_softPhoneID) return;
            m_radioMainStandby = QJsonValue(command["radioMainStandby"]).toInt();
            QString message = QString("{"
                                      "\"menuID\"                       :\"RadioActive\", "
                                      "\"softPhoneID\"                  :%1, "
                                      "\"radioAutoInactive\"            :%2, "
                                      "\"radioMainStandby\"             :%3 "
                                      "}"
                                      ).arg(m_softPhoneID).arg(m_radioAutoInactive).arg(m_radioMainStandby);
            emit cppCommand(message);
        }
        else if (getCommand == ("updateRxBestSignalEnable"))
        {
            int softPhoneId = QJsonValue(command["softPhoneID"]).toInt();
            if (softPhoneId != m_softPhoneID) return;
            rxBestSignalEnable = QJsonValue(command["value"]).toInt() == 1;
            QString message = QString("{"
                                      "\"menuID\"                       :\"rxBestSignalEnable\", "
                                      "\"softPhoneID\"                  :%1, "
                                      "\"value\"                        :%2"
                                      "}"
                                      ).arg(m_softPhoneID).arg(rxBestSignalEnable);
            emit cppCommand(message);
        }
        else if (getCommand == "pttGroupActive")
        {
            int softPhoneId = QJsonValue(command["softPhoneID"]).toInt();

            if ((pttDelay_count > 0) & (pttDelay_count < pttDelay_set) & (forceMuteSqlOn == true)) return;

            if ((softPhoneId != m_softPhoneID) || (inviteMode != SERVER) || (groupMute == MUTEIGNORE))
            {
                if (pttGroupActive)
                {
                    pttGroupActive = false;
                }
                return;
            }
            pttGroupActive = QJsonValue(command["pttGroupActive"]).toInt() == 1;
            forceMuteSqlOn = QJsonValue(command["forceMuteSqlOn"]).toInt() == 1;

            qDebug() << "pttGroupActive softPhoneID" << softPhoneId << m_softPhoneID << "pttGroupActive" << pttGroupActive << "forceMuteSqlOn" << forceMuteSqlOn;
            if(pttGroupActive)
            {
                if (forceMuteSqlOn)
                {
                    emit sqlStatusChange(false);
                    pptTest_released();
                    if (trx1->radio1->call_id != PJSUA_INVALID_ID)
                        updateHomeDisplay(trx1->radio1->url,CONNECTED,STANDBY,trx1->radio1->call_id,trx1->radio1->radioNodeID);
                    if (trx1->radio2->call_id != PJSUA_INVALID_ID)
                        updateHomeDisplay(trx1->radio2->url,CONNECTED,STANDBY,trx1->radio2->call_id,trx1->radio2->radioNodeID);
                    if (trx2->radio1->call_id != PJSUA_INVALID_ID)
                        updateHomeDisplay(trx2->radio1->url,CONNECTED,STANDBY,trx2->radio1->call_id,trx2->radio1->radioNodeID);
                    if (trx2->radio2->call_id != PJSUA_INVALID_ID)
                        updateHomeDisplay(trx2->radio2->url,CONNECTED,STANDBY,trx2->radio2->call_id,trx2->radio2->radioNodeID);

                    trx1->radio1->audioSQLOn = false;
                    trx1->radio2->audioSQLOn = false;
                    trx2->radio1->audioSQLOn = false;
                    trx2->radio2->audioSQLOn = false;

                    setvolume(trx1->radio1->call_id,MUTE);
                    setvolume(trx1->radio2->call_id,MUTE);
                    setvolume(trx2->radio1->call_id,MUTE);
                    setvolume(trx2->radio2->call_id,MUTE);
                }
                else if (groupMute == MUTEALL)
                {
                    emit sqlStatusChange(false);
                    pptTest_released();
                    if (trx1->radio1->call_id != PJSUA_INVALID_ID)
                        updateHomeDisplay(trx1->radio1->url,CONNECTED,STANDBY,trx1->radio1->call_id,trx1->radio1->radioNodeID);
                    if (trx1->radio2->call_id != PJSUA_INVALID_ID)
                        updateHomeDisplay(trx1->radio2->url,CONNECTED,STANDBY,trx1->radio2->call_id,trx1->radio2->radioNodeID);
                    if (trx2->radio1->call_id != PJSUA_INVALID_ID)
                        updateHomeDisplay(trx2->radio1->url,CONNECTED,STANDBY,trx2->radio1->call_id,trx2->radio1->radioNodeID);
                    if (trx2->radio2->call_id != PJSUA_INVALID_ID)
                        updateHomeDisplay(trx2->radio2->url,CONNECTED,STANDBY,trx2->radio2->call_id,trx2->radio2->radioNodeID);

                    trx1->radio1->audioSQLOn = false;
                    trx1->radio2->audioSQLOn = false;
                    trx2->radio1->audioSQLOn = false;
                    trx2->radio2->audioSQLOn = false;

                    setvolume(trx1->radio1->call_id,MUTE);
                    setvolume(trx1->radio2->call_id,MUTE);
                    setvolume(trx2->radio1->call_id,MUTE);
                    setvolume(trx2->radio2->call_id,MUTE);
                }
                else if(groupMute == MUTEAUDIO)
                {
                    setvolume(trx1->radio1->call_id,MUTE);
                    setvolume(trx1->radio2->call_id,MUTE);
                    setvolume(trx2->radio1->call_id,MUTE);
                    setvolume(trx2->radio2->call_id,MUTE);
                }
            }
            else
                emit sqlStatusChange(sqlOnStatusbackup);
        }
        else if (getCommand == "pttTestMode")
        {
            int softPhoneId = QJsonValue(command["softPhoneID"]).toInt();
            if (softPhoneId != m_softPhoneID) return;
            if ((QJsonValue(command["value"]).toInt() != 0) != pttTestMode)
                qDebug() << "c++:" << "pttTestMode" << "value" << pttTestMode;

            pttTestMode = QJsonValue(command["value"]).toInt() != 0;
        }


    }
    else if(getCommand == "MCP3208_0")
    {
        int adcID = QJsonValue(command["channel"]).toInt();
        double value = QJsonValue(command["value"]).toDouble();
        rssi_raw = QJsonValue(command["ADC_output_code"]).toInt();
    }
    else if(getCommand == "broadcastVUMeter")
    {
        switch (m_softPhoneID) {
        case 1:
            audioInLevel = QJsonValue(command["in1"]).toDouble();
            audioOutLevel = QJsonValue(command["out1"]).toDouble();
            audioInLevel_dB = QJsonValue(command["in1dB"]).toDouble();
            audioOutLevel_dB = QJsonValue(command["out1dB"]).toDouble();
            break;
        case 2:
            audioInLevel = QJsonValue(command["in2"]).toDouble();
            audioOutLevel = QJsonValue(command["out2"]).toDouble();
            audioInLevel_dB = QJsonValue(command["in2dB"]).toDouble();
            audioOutLevel_dB = QJsonValue(command["out2dB"]).toDouble();
            break;
        case 3:
            audioInLevel = QJsonValue(command["in3"]).toDouble();
            audioOutLevel = QJsonValue(command["out3"]).toDouble();
            audioInLevel_dB = QJsonValue(command["in3dB"]).toDouble();
            audioOutLevel_dB = QJsonValue(command["out3dB"]).toDouble();
            break;
        case 4:
            audioInLevel = QJsonValue(command["in4"]).toDouble();
            audioOutLevel = QJsonValue(command["out4"]).toDouble();
            audioInLevel_dB = QJsonValue(command["in4dB"]).toDouble();
            audioOutLevel_dB = QJsonValue(command["out4dB"]).toDouble();
            break;
        default:
            break;
        }
    }
}

#include "roip_ed137.h"

#define THIS_FILE "Functions.cpp"
//#include <QAudioDeviceInfo>
//#include <QAudioInput>
// display error and exit application
void RoIP_ED137::error_exit(const char *title, pj_status_t status)
{
    (void)title;
    (void)status;
        pjHangupAll();
        pjsua_acc_set_registration(acc_id, PJ_FALSE);
        pjsua_destroy();
        exit(1);
}
void RoIP_ED137::reconnectTrx(int trxID)
{
    qDebug() << "" << trxID;
    if (trxID == 1)
    {
        trx_call_hangup(trx1->radio1->call_id,500,NULL);
        trx_call_hangup(trx1->radio2->call_id,500,NULL);
        trx1->radio1->trxFailed = false;
        trx1->radio2->trxFailed = false;
        trx1->radio1->autoConnectCount = 0;
        trx1->radio2->autoConnectCount = 0;
    }
    else if (trxID == 2)
    {
        trx_call_hangup(trx2->radio1->call_id,500,NULL);
        trx2->radio1->trxFailed = false;
        trx2->radio1->autoConnectCount = 0;

        trx_call_hangup(trx2->radio2->call_id,500,NULL);
        trx2->radio2->trxFailed = false;
        trx2->radio2->autoConnectCount = 0;
    }
}

void RoIP_ED137::updateswitchInviteMode(int value){
    if ((value > 0) & (value <= SERVER_RPT))
    {
        QSettings *settings;
        const QString cfgfile = FILESETTING;
        qDebug() << "Loading configuration from:" << cfgfile;
        if(QDir::isAbsolutePath(cfgfile)){
            settings = new QSettings(cfgfile,QSettings::IniFormat);
            qDebug() << "Configuration file:" << settings->fileName();
            settings->setValue("SIP_SETTINGS/inviteMode",value);
        }
        else{
            qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
        }
        delete settings;
        bool conToRadio = false;
        if(value == SERVER){
            conToRadio = false;
        }
        else if(value == SERVER_RPT){
            conToRadio = true;
        }
        if ((value != inviteMode) || (conToRadio != connToRadio))
            inviteModeUpdate();
    }
}



void RoIP_ED137::updateTrxConfig(int trxID, QString CallName, QString Address, QString sipPort, QString trxMode, QString active, QString CallNameRx, QString AddressRx, QString sipPortRx, QString channelName, QString AlwaysConn){
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();

        if (trxID == 1){
            trx1->radio1->trxFailed = false;
            trx1->radio1->autoConnectCount = 0;
            trx1->radio2->autoConnectCount = 0;
            settings->setValue("TRX1_SETTING/CallName",CallName);
            settings->setValue("TRX1_SETTING/Address",Address);
            settings->setValue("TRX1_SETTING/sipPort",sipPort);
            settings->setValue("TRX1_SETTING/trxMode",trxMode);
            settings->setValue("TRX1_SETTING/enable",active);
            settings->setValue("TRX1_SETTING/AlwaysConn",AlwaysConn);
            settings->setValue("TRX1_SETTING/CallNameRx",CallNameRx);
            settings->setValue("TRX1_SETTING/AddressRx",AddressRx);
            settings->setValue("TRX1_SETTING/sipPortRx",sipPortRx);
            settings->setValue("TRX1_SETTING/channelName",channelName);
            trx_call_hangup(trx1->radio1->call_id,500,NULL);
            trx_call_hangup(trx1->radio2->call_id,500,NULL);
            while (trx1->radio1->callState == true){
                QThread::msleep(1000);
                qDebug() << "trx1 disconnecting......";
            }
            trx1->radio1->callName = CallName;
            trx1->radio1->ipAddress = Address;
            trx1->radio1->portAddr = sipPort;
            trx1->trxmode = trxMode;
            trx1->channelName = channelName;
            trx1->radio2->callName = CallNameRx;
            trx1->radio2->ipAddress = AddressRx;
            trx1->radio2->portAddr = sipPortRx;
            trx1->alwaysConnect = AlwaysConn.contains("true");
            trx1->enable = active.contains("true");
            if (!trx1->enable){
                trx1->radio1->enable = false;
                trx1->radio2->enable = false;
            }else{
                if(trx1->trxmode == TRXMODE_SEPARATE){
                    trx1->radio1->enable = true;
                    trx1->radio2->enable = true;
                    trx1->radio1->trxmode = TRXMODE_TX;
                    trx1->radio2->trxmode = TRXMODE_RX;
                }else{
                    trx1->radio1->enable = true;
                    trx1->radio2->enable = false;
                    trx1->radio1->trxmode = trx1->trxmode;
                }
            }
        }
        else if (trxID == 2){
            trx2->radio1->trxFailed = false;
            settings->setValue("TRX2_SETTING/CallName",CallName);
            settings->setValue("TRX2_SETTING/Address",Address);
            settings->setValue("TRX2_SETTING/sipPort",sipPort);
            settings->setValue("TRX2_SETTING/trxMode",trxMode);
            settings->setValue("TRX2_SETTING/enable",active);
            settings->setValue("TRX2_SETTING/AlwaysConn",AlwaysConn);
            settings->setValue("TRX2_SETTING/CallNameRx",CallNameRx);
            settings->setValue("TRX2_SETTING/AddressRx",AddressRx);
            settings->setValue("TRX2_SETTING/sipPortRx",sipPortRx);
            settings->setValue("TRX2_SETTING/channelName",channelName);
            trx_call_hangup(trx2->radio1->call_id,500,NULL);
            trx_call_hangup(trx2->radio2->call_id,500,NULL);
            trx2->radio1->autoConnectCount = 0;
            trx2->radio2->autoConnectCount = 0;
            while (trx2->radio1->callState == true){
                QThread::msleep(1000);
                qDebug() << "trx2 disconnecting......";
            }
            trx2->radio1->callName = CallName;
            trx2->radio1->ipAddress = Address;
            trx2->radio1->portAddr = sipPort;
            trx2->trxmode = trxMode;
            trx2->channelName = channelName;
            trx2->radio2->callName = CallNameRx;
            trx2->radio2->ipAddress = AddressRx;
            trx2->radio2->portAddr = sipPortRx;
            trx2->enable = active.contains("true");
            trx2->alwaysConnect = AlwaysConn.contains("true");
            if (!trx2->enable){
                trx2->radio1->enable = false;
                trx2->radio2->enable = false;
            }else{
                if(trx2->trxmode == TRXMODE_SEPARATE){
                    trx2->radio1->enable = true;
                    trx2->radio2->enable = true;
                    trx2->radio1->trxmode = TRXMODE_TX;
                    trx2->radio2->trxmode = TRXMODE_RX;
                }else{
                    trx2->radio1->enable = true;
                    trx2->radio2->enable = false;
                    trx2->radio1->trxmode = trx2->trxmode;
                }
            }
        }
    }
    else{
        qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
    }
    delete settings;
}
void RoIP_ED137::updateDeviceName(QString value)
{
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        settings->setValue("SIP_SETTINGS/deviceName",value);
        deviceName = value;
    }
}
void RoIP_ED137::updateNTPServer(QString value){
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        settings->setValue("NETWORK/ntpServer",value);
        settings->setValue("NETWORK/ntp","true");
        networking->setNTPServer(value);
        networking->resetNtp();
        ntpServer=value;
    }
    ntp = true;
}
void RoIP_ED137::setLocation(QString location){
    if (!location.contains("Select")){
        QString command = QString("ln -sf /usr/share/zoneinfo/%1  /etc/localtime").arg(location);
        system(command.toStdString().c_str());
        timeLocation = location;
    }
}
void RoIP_ED137::setcurrentDatetime(QString dateTime){
    QString date;
    QString time;
    if (dateTime.split(" ").size() >= 2){
        date = QString(dateTime.split(" ").at(0)).replace("/","-");
        time = QString(dateTime.split(" ").at(1))+":00";
        system("timedatectl set-ntp 0");
        QString command = QString("date --set '%1 %2'").arg(date).arg(time);
        system(command.toStdString().c_str());
    }

    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        settings->setValue("NETWORK/ntp","false");
    }
    ntp = false;
}

void RoIP_ED137::disconnectUri(QString uri)
{
    for (int i=0; i<trx_incall.length();i++){
        if ((trx_incall.at(i)->callState)&(trx_incall.at(i)->url == uri)){
            trx_incall_hangup(trx_incall.at(i)->call_id,NULL,NULL,i);
        }
    }
}
void RoIP_ED137::updateURILits(QStringList urlList){
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if (urlList.length() == 8){
        uriNameAllowList.clear();
        uriNameAllowList = urlList;
        if(QDir::isAbsolutePath(cfgfile)){
            settings = new QSettings(cfgfile,QSettings::IniFormat);
            qDebug() << "Configuration file:" << settings->fileName();
            settings->setValue("SERVER_LIST/USER1",uriNameAllowList.at(0));
            settings->setValue("SERVER_LIST/USER2",uriNameAllowList.at(1));
            settings->setValue("SERVER_LIST/USER3",uriNameAllowList.at(2));
            settings->setValue("SERVER_LIST/USER4",uriNameAllowList.at(3));
            settings->setValue("SERVER_LIST/USER5",uriNameAllowList.at(4));
            settings->setValue("SERVER_LIST/USER6",uriNameAllowList.at(5));
            settings->setValue("SERVER_LIST/USER7",uriNameAllowList.at(6));
            settings->setValue("SERVER_LIST/USER8",uriNameAllowList.at(7));
        }
        else{
            qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
        }
    }
    delete settings;
}
void RoIP_ED137::onNumClientChanged(int numclient)
{
    numClientConn = numclient;
    if (numclient == 0)
    {
        if (currentmenuIndex != SETAUDIO_VU_METER)
        {
            //AudioInput->Suspend(true);
            //AudioOutput->Suspend(true);
        }
    }
    else
    {
        //AudioInput->Suspend(false);
        //AudioOutput->Suspend(false);
    }
}

void RoIP_ED137::updateSQLActiveHigh(bool sqlActive)
{
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        settings->setValue("SIP_SETTINGS/sqlActiveHigh",sqlActive);
    }
    else{
        qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
    }
    delete settings;
    sqlActiveHigh = sqlActive;
//    sqlOldPinOnStatus =false;
}
void RoIP_ED137::updateSQLDefeat(bool sqlDefeat)
{
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        settings->setValue("SIP_SETTINGS/sqlAlwayOn",sqlDefeat);
    }
    else{
        qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
    }
    delete settings;
    sqlAlwayOn = sqlDefeat;
}
void RoIP_ED137::updateNumconn(int numconn){
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    numConnection = numconn;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        settings->setValue("SIP_SETTINGS/numConnection",numconn);
    }
    else{
        qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
    }
    delete settings;
}
void RoIP_ED137::updateGpioConfig(int gpioNum, int gpioVal)
{
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        if (gpioNum == 2)
            settings->setValue("GPIO/Out2Val",gpioVal);
        else if(gpioNum == 3)
            settings->setValue("GPIO/Out3Val",gpioVal);
        else if(gpioNum == 4)
            settings->setValue("GPIO/Out4Val",gpioVal);
    }
    else{
        qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
    }
    delete settings;
}

void RoIP_ED137::updateHostConfig(QString sipuser, int sipport, int keepaliveperoid, QString defaultEth)
{
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        settings->setValue("SIP_SETTINGS/SipUser",sipuser);
        settings->setValue("SIP_SETTINGS/sipPort",sipport);
        settings->setValue("SIP_SETTINGS/keepAlivePeroid",keepaliveperoid);
        settings->setValue("SIP_SETTINGS/defaultEthernet",defaultEth);
        if (SipUser != sipuser.toStdString() || (sip_port != sipport) || (keepAlivePeroid != keepaliveperoid) || (defaultEthernet != defaultEth))
        {
            pjHangupAll();
            SipUser = sipuser.toStdString();
            sip_port = sipport;
            keepAlivePeroid = keepaliveperoid;
            defaultEthernet = defaultEth;
            removeDefault();
            RegisterDefault();
        }
    }
    else{
        qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
    }
    delete settings;
}

void RoIP_ED137::updatetxBestsignalSelected(bool val)
{
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        settings->setValue("SIP_SETTINGS/rxBestSignalEnable",val);
        rxBestSignalEnable = val;
    }
    else
    {
        qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
    }
    delete settings;
}

void RoIP_ED137::updatetxScheduler(int schID)
{
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        settings->setValue("SIP_SETTINGS/txScheduler",schID);
        txScheduler = schID;
    }
    else{
        qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
    }
    delete settings;
}

bool RoIP_ED137::checkHashFileNotExit()
{
    QString fileName("/home/pi/.hashlet");
    if(QFileInfo::exists(fileName))
    {
        qDebug () << "hashlet file is exists" << endl;
        return false;
    }
    qDebug () << "hashlet file does not exists" << endl;
    return true;
}
void RoIP_ED137::myConfigurate(){
    QSettings *settings;
    const QString cfgfile = FILESETTING;
    qDebug() << "Loading configuration from:" << cfgfile;
    if(QDir::isAbsolutePath(cfgfile)){
        settings = new QSettings(cfgfile,QSettings::IniFormat);
        qDebug() << "Configuration file:" << settings->fileName();
        deviceName          = settings->value("SIP_SETTINGS/deviceName","iGate").toString();
        SipUser             = settings->value("SIP_SETTINGS/SipUser","iGate").toString().toStdString();
        sip_port            = settings->value("SIP_SETTINGS/sipPort",5060).toInt();
        keepAlivePeroid     = settings->value("SIP_SETTINGS/keepAlivePeroid",200).toInt();
        txScheduler         = settings->value("SIP_SETTINGS/txScheduler",2).toInt();
        inviteMode          = settings->value("SIP_SETTINGS/inviteMode",SERVER).toInt();
        numConnection       = settings->value("SIP_SETTINGS/numConnection",3).toInt();
        sidetone            = settings->value("SIP_SETTINGS/sidetone",0.1).toFloat();
        connToRadio         = settings->value("SIP_SETTINGS/connToRadio","false").toString().contains("true");
        sqlAlwayOn          = settings->value("SIP_SETTINGS/sqlAlwayOn","false").toString().contains("true");
        defaultEthernet     = settings->value("SIP_SETTINGS/defaultEthernet","eth1").toString();
        sqlActiveHigh       = settings->value("SIP_SETTINGS/sqlActiveHigh","false").toString().contains("true");




//        trx1->channelName           = settings->value("TRX1_SETTING/channelName","Node1").toString();
//        trx1->trxmode               = settings->value("TRX1_SETTING/trxMode",TRXMODE_TRX).toString();
//        trx1->enable                = settings->value("TRX1_SETTING/enable","false").toString().contains("true");
//        trx1->alwaysConnect         = settings->value("TRX1_SETTING/AlwaysConn","false").toString().contains("true");

//        trx1->radio1->callName      = settings->value("TRX1_SETTING/CallName","t6t_uhf1").toString();
//        trx1->radio1->ipAddress     = settings->value("TRX1_SETTING/Address","192.168.10.10").toString();
//        trx1->radio1->portAddr      = settings->value("TRX1_SETTING/sipPort","5060").toString();

//        trx1->radio2->callName      = settings->value("TRX1_SETTING/CallNameRx","t6r_uhf1").toString();
//        trx1->radio2->ipAddress     = settings->value("TRX1_SETTING/AddressRx","192.168.10.11").toString();
//        trx1->radio2->portAddr      = settings->value("TRX1_SETTING/sipPortRx","5060").toString();

//        trx2->channelName           = settings->value("TRX2_SETTING/channelName","Node2").toString();
//        trx2->trxmode               = settings->value("TRX2_SETTING/trxMode",TRXMODE_TRX).toString();
//        trx2->enable                = settings->value("TRX2_SETTING/enable","false").toString().contains("true");
//        trx2->alwaysConnect         = settings->value("TRX2_SETTING/AlwaysConn","false").toString().contains("true");

//        trx2->radio1->callName      = settings->value("TRX2_SETTING/CallName","T6").toString();
//        trx2->radio1->ipAddress     = settings->value("TRX2_SETTING/Address","192.168.10.11").toString();
//        trx2->radio1->portAddr      = settings->value("TRX2_SETTING/sipPort","5060").toString();

//        trx2->radio2->callName      = settings->value("TRX2_SETTING/CallNameRx","T6").toString();
//        trx2->radio2->ipAddress     = settings->value("TRX2_SETTING/AddressRx","192.168.10.11").toString();
//        trx2->radio2->portAddr      = settings->value("TRX2_SETTING/sipPortRx","5060").toString();

//        uriNameAllowList.clear();
//        uriNameAllowList      << settings->value("SERVER_LIST/USER1","iGate_1").toString();
//        uriNameAllowList      << settings->value("SERVER_LIST/USER2","iGate_2").toString();
//        uriNameAllowList      << settings->value("SERVER_LIST/USER3","iGate_3").toString();
//        uriNameAllowList      << settings->value("SERVER_LIST/USER4","iGate_4").toString();
//        uriNameAllowList      << settings->value("SERVER_LIST/USER5","iGate_5").toString();
//        uriNameAllowList      << settings->value("SERVER_LIST/USER6","iGate_6").toString();
//        uriNameAllowList      << settings->value("SERVER_LIST/USER7","iGate_7").toString();
//        uriNameAllowList      << settings->value("SERVER_LIST/USER8","iGate_8").toString();



//        eth0.dhcpmethod          = settings->value("NETWORK_ETH0/DHCP","on").toString();
//        eth0.ipaddress           = settings->value("NETWORK_ETH0/LocalAddress","192.168.10.9").toString();
//        eth0.subnet              = settings->value("NETWORK_ETH0/Netmask","24").toString();
//        eth0.gateway             = settings->value("NETWORK_ETH0/Gateway","").toString();
//        eth0.pridns              = settings->value("NETWORK_ETH0/DNS1","").toString();
//        eth0.secdns              = settings->value("NETWORK_ETH0/DNS2","").toString();

//        eth1.dhcpmethod          = settings->value("NETWORK_ETH1/DHCP","on").toString();
//        eth1.ipaddress           = settings->value("NETWORK_ETH1/LocalAddress","192.168.10.9").toString();
//        eth1.subnet              = settings->value("NETWORK_ETH1/Netmask","24").toString();
//        eth1.gateway             = settings->value("NETWORK_ETH1/Gateway","").toString();
//        eth1.pridns              = settings->value("NETWORK_ETH1/DNS1","").toString();
//        eth1.secdns              = settings->value("NETWORK_ETH1/DNS2","").toString();

//        lcdBacklight        = settings->value("SYSTEM/lcdBacklight",50).toInt();

//        clock_rate          = settings->value("AUDIO_SETTINGS/clockRate",48000).toInt();

//        m_EnableRecording   = settings->value("CATISCONF/EnableRecording","false").toString().contains("true");
//        warnAudioLevelTime  = settings->value("CATISCONF/warnAudioLevelTime",5).toInt();
//        warnPercentFault    = settings->value("CATISCONF/warnPercentFault",5).toInt();
//        warnPTTMinute       = settings->value("CATISCONF/warnPTTMinute",2).toInt();
//        backupAudio_min     = settings->value("CATISCONF/backupAudio_min",30).toInt();

//        recorderAlloweURI   = settings->value("SIP_SETTINGS/recorderAlloweURI","iRec").toString();
//        recorderSuported    = settings->value("SIP_SETTINGS/recorderSuported","false").toString().contains("true");
    }
    else{
        qDebug() << "Loading configuration from:" << cfgfile << " FILE NOT FOUND!";
    }
    qDebug() << "Loading configuration completed";
    delete settings;
}

/////////ring//////////

void RoIP_ED137::playRing()
{
    pjsua_conf_connect(in_ring_slot_, 0);
}
void RoIP_ED137::stopRing()
{
       pjsua_conf_disconnect(in_ring_slot_, 0);
       pjmedia_tonegen_rewind(in_ring_port_);
}
void RoIP_ED137::init_ringTone()
{
    pj_str_t name = pj_str(const_cast<char*>("inring"));
    pjmedia_tone_desc tone[3];
    pj_status_t status;
    pj_pool_t* pool_;
    pool_ = pjsua_pool_create("blabble", 4096, 4096);
    if (pool_ == NULL)
        throw std::runtime_error("Ran out of memory creating pool!");

    tone[0].freq1 = 440;
    tone[0].freq2 = 480;
    tone[0].on_msec = 2000;
    tone[0].off_msec = 1000;
    tone[1].freq1 = 440;
    tone[1].freq2 = 480;
    tone[1].on_msec = 2000;
    tone[1].off_msec = 4000;
    tone[2].freq1 = 440;
    tone[2].freq2 = 480;
    tone[2].on_msec = 2000;
    tone[2].off_msec = 3000;

    status = pjmedia_tonegen_create2(pool_, &name, clock_rate, 1, 160, 16, PJMEDIA_TONEGEN_LOOP, &in_ring_port_);
    if (status != PJ_SUCCESS)        {
        throw std::runtime_error("Failed inring pjmedia_tonegen_create2");
    }

    status = pjmedia_tonegen_play(in_ring_port_, 1, tone, PJMEDIA_TONEGEN_LOOP);
    if (status != PJ_SUCCESS)
    {
        throw std::runtime_error("Failed inring pjmedia_tonegen_play");
    }

    status = pjsua_conf_add_port(pool_, in_ring_port_, &in_ring_slot_);
    if (status != PJ_SUCCESS)
    {
        throw std::runtime_error("Failed inring pjsua_conf_add_port");
    }
}

////////////////////////

void RoIP_ED137::connectCallToSoundCard(int CallID)
{
    int   clientPort = getConfPortNumber(CallID);

    if(clientPort != -1)
    {
        //-- found sound card list
        std::vector<int> soundCardPortList;
        getSoundCardPorts(soundCardPortList);

        // is listener port a sound port?
        for (std::vector<int>::iterator itSoundPortList = soundCardPortList.begin(); itSoundPortList != soundCardPortList.end(); ++itSoundPortList)
        {
            unsigned soundCardPortID = *itSoundPortList;

            if((unsigned int) clientPort != (unsigned int) PJSUA_INVALID_ID && soundCardPortID != (unsigned int) PJSUA_INVALID_ID)
            {
                connectPort(clientPort,soundCardPortID);
                //Log_Debug(mp::mp_logger, "connectCallToSoundCard Port: %d CallID: %d", soundCardPortID,CallID);
            }
        }
    }
}

void RoIP_ED137::disConnectCallFromSoundCard(int CallID)
{

    int clientPort = getConfPortNumber(CallID);

    if(clientPort != -1)
    {
        //-- found sound card list
        std::vector<int> soundCardPortList;
        getSoundCardPorts(soundCardPortList);

        // is listener port a sound port?
        for (std::vector<int>::iterator itSoundPortList = soundCardPortList.begin(); itSoundPortList != soundCardPortList.end(); ++itSoundPortList)
        {
            unsigned soundCardPortID = *itSoundPortList;

            if((unsigned int) clientPort != (unsigned int) PJSUA_INVALID_ID && soundCardPortID != (unsigned int) PJSUA_INVALID_ID)
            {
                disconnectPort(clientPort,soundCardPortID);
                //Log_Debug(mp::mp_logger, "disConnectCallFromSoundCard Port: %d CallID: %d", soundCardPortID,CallID);
            }
        }
    }
}


int RoIP_ED137::getConfPortNumber(int callId)
{
    int portNumber = -1;

    pjsua_call_info ci;

    if (callId != PJSUA_INVALID_ID)
    {
        pj_status_t status = pjsua_call_get_info(callId, &ci);

        if (status == PJ_SUCCESS)
        {
            portNumber = ci.conf_slot;
        }
        else {
            //VOIP_PJ_LOG(m_logger, "Error on getting call info", status);
        }
    }

    return portNumber;
}

bool RoIP_ED137::getLocalNumberFromCallID(int CallID, std::string& localNumber) const
{
    bool valid = false;

    if(CallID != -1)
    {
        pjsua_call_info ci;
        pj_status_t status;

        status = pjsua_call_get_info(CallID, &ci);
        if (status != PJ_SUCCESS)
        {
            return false;
        }

        std::string strLocalInfo;
        strLocalInfo.append(ci.local_info.ptr, ci.local_info.slen);

        size_t posrx = strLocalInfo.find("<sip:tr");
        size_t postx = strLocalInfo.find("<sip:tx");

        if (posrx != std::string::npos || postx != std::string::npos)
        {
            localNumber = strLocalInfo;
            valid = true;
        }
        else
        {
            localNumber = strLocalInfo;
            valid = true;
        }
    }
    return valid;
}

bool RoIP_ED137::getExtFromCallID(int CallID, std::string& extNumber) const
{
    bool valid = false;

    if(CallID != -1)
    {
        pjsua_call_info ci;
        pj_status_t status;

        status = pjsua_call_get_info(CallID, &ci);

        if (status != PJ_SUCCESS)
        {
            return false;
        }

        std::string strLocalInfo;
        strLocalInfo.append(ci.remote_info.ptr, ci.remote_info.slen);

        size_t posrx = strLocalInfo.find("<sip:tr");
        size_t postx = strLocalInfo.find("<sip:tx");

        if (posrx != std::string::npos || postx != std::string::npos)
        {
            extNumber = strLocalInfo;
            valid = true;
        }
        else
        {
            extNumber = strLocalInfo;
            valid = true;
        }
    }
    return valid;
}

bool RoIP_ED137::connectPort(int srcPort, int dstPort)
{
    pj_status_t status = pjsua_conf_connect(srcPort, dstPort);

    if (status != PJ_SUCCESS)
    {
        return false;
    }

    return true;
}

bool RoIP_ED137::disconnectPort(int srcPort, int dstPort)
{
    pj_status_t status = pjsua_conf_disconnect(srcPort, dstPort);

    if (status != PJ_SUCCESS)
    {
        return false;
    }

    return true;
}

//////////////calllll////////////////


void RoIP_ED137::getCallPortInfo(std::string ext, pjsua_conf_port_info& returnInfo)
{
    unsigned i, count;
    pjsua_conf_port_id id[PJSUA_MAX_CALLS];

    count = PJ_ARRAY_SIZE(id);
    pjsua_enum_conf_ports(id, &count);

    for (i = 0; i < count; ++i)
    {
        pjsua_conf_port_info info;
        pjsua_conf_get_port_info(id[i], &info);

        std::string cardname(info.name.ptr,(int) info.name.slen);
        std::size_t found = ext.find(cardname);
        if (found != std::string::npos)
        {
             returnInfo = info;
        }
    }
}

void RoIP_ED137::listConfs()
{
    int indexOfBuffer = 0;
    char stringBuffer [8192];
    indexOfBuffer += sprintf(stringBuffer+indexOfBuffer, "\n");
    indexOfBuffer += sprintf(stringBuffer+indexOfBuffer, "listConfs:\n");
    indexOfBuffer += sprintf(stringBuffer+indexOfBuffer, "Active Call Count: %d Accounts: %d\n", getActiveCallCount(),pjsua_acc_get_count());
    indexOfBuffer += sprintf(stringBuffer+indexOfBuffer, "Conference ports:\n");

    unsigned i, count;
    pjsua_conf_port_id id[PJSUA_MAX_CALLS];

    count = PJ_ARRAY_SIZE(id);
    pjsua_enum_conf_ports(id, &count);

    for (i = 0; i < count; ++i)
    {
        char txlist[PJSUA_MAX_CALLS * 4 + 10];
        unsigned j;
        pjsua_conf_port_info info;

        pjsua_conf_get_port_info(id[i], &info);

        txlist[0] = '\0';
        for (j = 0; j < info.listener_cnt; ++j)
        {
            char s[10];
            pj_ansi_sprintf(s, "#%d ", info.listeners[j]);
            pj_ansi_strcat(txlist, s);
        }

        indexOfBuffer += sprintf(stringBuffer+indexOfBuffer, "Port #%02d[%2dKHz/%dms/%d] %20.*s  transmitting to: %s\n",
                info.slot_id, info.clock_rate / 1000,
                info.samples_per_frame * 1000 / info.channel_count
                        / info.clock_rate, info.channel_count,
                (int) info.name.slen, info.name.ptr, txlist);
    }

    stringBuffer[indexOfBuffer] = '\0';

    printf("%s", stringBuffer);
}

int RoIP_ED137::getActiveCallCount()
{
    if (inviteMode == CLIENT)
    {
        int i = 0;
        callInNum = 0;
        for (i=0; i<trx_incall.length();i++)
        {
            if (trx_incall.at(i)->callState) callInNum++;
        }
        return callInNum;
    }
    else if(inviteMode == SERVER)
    {
        return (trx1->radio1->callState + trx1->radio2->callState + trx2->radio1->callState + trx2->radio2->callState);
    }
    else {
        return pjsua_call_get_count();
    }
}

void RoIP_ED137::getSoundCardPorts(std::vector<int>& portList) const
{
    portList.clear();
    int index = 0;

    std::vector<pjmedia_aud_dev_info>::const_iterator it = m_AudioDevInfoVector.begin();

    for (; it != m_AudioDevInfoVector.end(); ++it)
    {
        portList.push_back(index);
        index++;
    }
}

std::string RoIP_ED137::getSipUrl(const char* ext)
{
    std::string const& sipServer = sip_domainstr.toStdString() ;
    std::string const& sipURL = std::string("<sip:") + ext + std::string("@") + sipServer + std::string(">");
    return sipURL;
}

void RoIP_ED137::initializePttManager(const std::string pttDeviceName)
{
//    m_PttManager.reset(PttManager::initialize(pttDeviceName));
//    printf("\n");

//    if (m_PttManager.get() != 0)
//    {
//        m_PttManager->addPttListener(this);
//        printf("PTT Device : %s initialized\n",pttDeviceName.c_str());
//        sprintf(log_buffer, "PTT Device : %s initialized",pttDeviceName.c_str());
//        AppentTextBrowser(log_buffer);
//    }
//    else {
//        printf("PTT Device : %s not Found\n",pttDeviceName.c_str());
//        sprintf(log_buffer, "PTT Device : %s not Found",pttDeviceName.c_str());
//        AppentTextBrowser(log_buffer);
//    }
}

bool RoIP_ED137::getExtFromRemoteInfo(pj_str_t const& remoteInfo, std::string& extNumber) const
{
    bool valid = false;

    std::string strLocalInfo;
    strLocalInfo.append(remoteInfo.ptr, remoteInfo.slen);

    size_t pos1 = strLocalInfo.find_first_of(":");
    size_t pos2 = strLocalInfo.find_first_of("@", pos1);
    if (pos1 != std::string::npos && pos2 != std::string::npos)
    {
        extNumber = strLocalInfo.substr(pos1 + 1, pos2 - pos1 - 1);
        valid = true;
    }

    return valid;
}


////////////IP RADIO///////

void RoIP_ED137::setRadioTxrxModeAndRadioType(int callId, QString txrxmode, QString type)
{
    if(callId == PJSUA_INVALID_ID)
    {
        return;
    }

    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;

    if( iter != transport_map.end())
    {
        pj_status_t status = setAdapterRadioModeAndType (iter->second, type.toStdString().c_str(),txrxmode.toStdString().c_str());
        if (status != PJ_SUCCESS) {
            printf("Error setRadioTxrxModeAndRadioType\n");
        }
    }
}
void RoIP_ED137::setRadioPttbyCallID(bool pttval,int callId,int priority, int userRec)
{
    if (userRec != 0)
        qDebug() << "setRadioPttbyCallID" << callId << pttval;
    if(callId == PJSUA_INVALID_ID)
    {
        return;
    }

    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;

    if( iter != transport_map.end())
    {
        pj_status_t status = setAdapterPtt (iter->second, pttval, priority, userRec);
        if (status != PJ_SUCCESS) {
            printf("Error setRadioPttbyCallID : setAdapterPtt\n");
        }
    }
}

void RoIP_ED137::setAdaptercallRecorder(int callId,int val)
{
    if(callId == PJSUA_INVALID_ID)
    {
        return;
    }

    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;

    if( iter != transport_map.end())
    {
        pj_status_t status = setcallRecorder(iter->second, val);
        if (status != PJ_SUCCESS) {
            printf("Error : setAdaptercallRecorder\n");
        }
    }
}
void RoIP_ED137::setSlaveEnable (pjsua_call_id callId,pj_bool_t rx, pj_bool_t tx)
{
    if(callId == PJSUA_INVALID_ID)
    {
        printf("Error setRadioSqlOnbyCallID : PJSUA_INVALID_ID\n");
        return;
    }

    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;

    if( iter != transport_map.end())
    {
        pj_status_t status = setTxRxSlaveEnable (iter->second, rx, tx);
        if (status != PJ_SUCCESS) {
            printf("Error setRadioPttbyCallID : setAdapterPtt\n");
        }
    }
}
void RoIP_ED137::setRadioSqlOnbyCallID(bool sqlval,int callId,int priority)
{
    if(callId == PJSUA_INVALID_ID)
    {
        printf("Error setRadioSqlOnbyCallID : PJSUA_INVALID_ID\n");
        return;
    }

    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;

    if( iter != transport_map.end())
    {
        pj_status_t status = setAdapterQslOn (iter->second, sqlval, priority, 15);
        if (status != PJ_SUCCESS) {
            printf("Error setRadioPttbyCallID : setAdapterPtt\n");
        }
    }
}
void RoIP_ED137::setRadioSqlOnbyCallID(bool sqlval,int callId,int priority, uint16_t rssi)
{
    if(callId == PJSUA_INVALID_ID)
    {
        printf("Error setRadioSqlOnbyCallID : PJSUA_INVALID_ID\n");
        return;
    }

    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;

    if( iter != transport_map.end())
    {
        pj_status_t status = setAdapterQslOn (iter->second, sqlval, priority, (rssi >> 6));
        if (status != PJ_SUCCESS) {
            printf("Error setRadioPttbyCallID : setAdapterPtt\n");
        }
    }
}

int RoIP_ED137::get_IPRadioBss(pjsua_call_id callId)
{
    pjsua_call_info ci;
    pj_status_t status = pjsua_call_get_info(callId, &ci);
    if (status == PJ_SUCCESS)
    {
        std::string extNumber;
        if (getExtFromRemoteInfo(ci.remote_info, extNumber))
        {
//            if (extNumber.find("rx") !=  std::string::npos || extNumber.find("Rx") !=  std::string::npos
//               || extNumber.find("rX") !=  std::string::npos || extNumber.find("RX") !=  std::string::npos)
//            {
                pj_uint32_t ed137_val = 0;
                std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;
                if( iter != transport_map.end())
                    ed137_val = get_ed137_value(iter->second);

                uint32_t i = ed137_val & 0x000000f8;//bss mask
                i = i >> 3;//shift 3 bits
                return i;
//            }
        }
    }
    return 0;
}

void RoIP_ED137::sendRtpType232(pjsua_call_id callId)
{
    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;
    if( iter != transport_map.end())
        sendR2SStatus(iter->second);
}

qint64 RoIP_ED137::get_R2SStatus(pjsua_call_id callId)
{
    qint64 r2sStatus = QDateTime::currentMSecsSinceEpoch()-3000;

    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;
    if( iter != transport_map.end())
        r2sStatus = getR2SStatus(iter->second);

    return r2sStatus;
}

int RoIP_ED137::get_IPRadioPttStatus(pjsua_call_id callId)
{
    pj_uint32_t ed137_val = 0;
    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;
    int i = 0;
    bool currentMainRadioTxUsed = false;
    bool currentMainRadioRxUsed = false;
    uint32_t ed137_val_old = 0;
    if (inviteMode == CLIENT)
    {

        for (i=0; i<trx_incall.length();i++)
        {
            if (trx_incall.at(i)->call_id == callId) {
                currentMainRadioTxUsed = trx_incall.at(i)->mainRadioTransceiverUsed;
                currentMainRadioRxUsed = trx_incall.at(i)->mainRadioReceiverUsed;
                ed137_val_old = trx_incall.at(i)->ed137_val_old;
                break;
            }

        }
    }
    else if (inviteMode == SERVER)
    {
        if (trx1->radio1->call_id == callId)
            ed137_val_old = trx1->radio1->ed137_val_old;
        else if (trx1->radio2->call_id == callId)
            ed137_val_old = trx1->radio2->ed137_val_old;
        else if (trx2->radio1->call_id == callId)
            ed137_val_old = trx2->radio1->ed137_val_old;
        else if (trx2->radio2->call_id == callId)
            ed137_val_old = trx2->radio2->ed137_val_old;
    }

    if( iter != transport_map.end())
        ed137_val = get_ed137_value(iter->second);

    if (inviteMode == CLIENT)
    {
        if (ed137_val != ed137_val_old)
        {
//            qDebug() << QString("0x%1").arg(ed137_val, 0, 16);
            if ((ed137_val & 0x00013100) == 0x00013100)
            {
                bool mainRadioTxUsed = !((ed137_val & 0x00000080) == 0x80);
                bool mainRadioRxUsed = !((ed137_val & 0x00000040) == 0x40);
                if (currentMainRadioTxUsed != mainRadioTxUsed)
                {
                    currentMainRadioTxUsed = mainRadioTxUsed;
                    qDebug() << "OnTxEnableChanged: Main Tx:" << currentMainRadioTxUsed;
                }
                if (currentMainRadioRxUsed != mainRadioRxUsed)
                {
                    currentMainRadioRxUsed = mainRadioRxUsed;
                    qDebug() << "OnRxEnableChanged: Main Rx:" << currentMainRadioRxUsed;
                }
                setSlaveEnable(callId,(!currentMainRadioRxUsed), (!currentMainRadioTxUsed));
                QString message = QString("{"
                                  "\"menuID\"                       :\"RadioActive\", "
                                  "\"softPhoneID\"                  :%1, "
                                  "\"radioAutoInactive\"            :%2, "
                                  "\"mainRadioReceiverUsed\"        :%3, "
                                  "\"mainRadioTransmitterUsed\"     :%4, "
                                  "\"radioMainStandby\"             :%5, "
                                  "\"callName\"                     :\"%6\" "
                                  "}"
                                  ).arg(m_softPhoneID).arg(m_radioAutoInactive).arg(currentMainRadioRxUsed).arg(currentMainRadioTxUsed).arg(m_radioMainStandby).arg(trx_incall.at(i)->callName);
                emit cppCommand(message);
            }
        }
        trx_incall.at(i)->ed137_val_old = ed137_val;
        trx_incall.at(i)->mainRadioTransceiverUsed = currentMainRadioTxUsed;
        trx_incall.at(i)->mainRadioReceiverUsed = currentMainRadioRxUsed;
    }
    if (inviteMode == SERVER)
    {

        if (ed137_val != ed137_val_old)
        {
//            qDebug() << QString("0x%1").arg(ed137_val, 0, 16);
            if (trx1->radio1->call_id == callId)
                trx1->radio1->ed137_val_old = ed137_val;
            else if (trx1->radio2->call_id == callId)
                trx1->radio2->ed137_val_old = ed137_val;
            else if (trx2->radio1->call_id == callId)
                trx2->radio1->ed137_val_old = ed137_val;
            else if (trx2->radio2->call_id == callId)
                trx2->radio2->ed137_val_old = ed137_val;
        }
    }

    uint32_t j = ed137_val & 0xe0000000;//pttid mask
    j = j >> 29;//shift 22 bits
    return j;
}

int RoIP_ED137::get_IPRadioPttId(pjsua_call_id callId)
{
    pj_uint32_t ed137_val=0;
    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;
    if( iter != transport_map.end())
        ed137_val = get_ed137_value(iter->second);

    uint32_t i = ed137_val & 0x0fc00000;//pttid mask
    i = i >> 22;//shift 22 bits
    return i;
}

int RoIP_ED137::get_IPRadioSquelch(pjsua_call_id callId)
{
    pj_uint32_t ed137_val=0;
    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;
    if( iter != transport_map.end())
        ed137_val = get_ed137_value(iter->second);

    uint32_t i=ed137_val & 0x10000000;//pttid mask
    i = i >> 28;//shift 22 bits
    return i;
}

bool RoIP_ED137::get_IPRadioStatus(pjsua_call_id callId)
{
    pj_uint32_t ed137_val=0;
    std::map<int,pjmedia_transport*>::iterator iter = transport_map.find(callId) ;
    if( iter != transport_map.end())
        ed137_val = get_ed137_value(iter->second);

    uint32_t i = ed137_val;

    if(i>0)
    {
         return true;
    }
    return false;
}

//////////////////////////

bool RoIP_ED137::createNullPort()
{

    if(m_NullSlot == PJ_INVALID_SOCKET)
    {
        pj_status_t status = PJ_SUCCESS;
        pj_pool_t *pool;

        pool = pjsua_pool_create("VoIP-pool", (4 * 1024 * 1024), (1 * 1024 * 1024));
        if (pool == NULL)
        {
            printf("Error allocating memory pool\n");
            return false;
        }

        status = pjmedia_null_port_create(pool, clock_rate, CHANNEL_COUNT, SAMPLES_PER_FRAME*2, 16, &m_NullPort);

        if (status != PJ_SUCCESS) {
            printf("Error Null Port");
            return false;
        }

        m_NullPort->put_frame = &RoIP_ED137::PutData;
        m_NullPort->get_frame = &RoIP_ED137::GetData;

        pj_cstr(&m_NullPort->info.name, "Null Port");

        status = pjsua_conf_add_port(pool, m_NullPort, &m_NullSlot);
        if (status != PJ_SUCCESS) {
            printf("Error Null Slot");
            return false;
        }

    }
    return true;
}

pj_status_t RoIP_ED137::GetData(struct pjmedia_port *this_port, pjmedia_frame *frame)
{
    PJ_UNUSED_ARG(this_port);
    PJ_UNUSED_ARG(frame);

    return PJ_SUCCESS;
}

pj_status_t RoIP_ED137::PutData(struct pjmedia_port *this_port, pjmedia_frame *frame)
{
    PJ_UNUSED_ARG(this_port);
    PJ_UNUSED_ARG(frame);

   return PJ_SUCCESS;
}

void RoIP_ED137::getSoundCardName(int number, std::string& cardName)
{
    int index = 0;
    std::vector<pjmedia_aud_dev_info>::const_iterator it = m_AudioDevInfoVector.begin();

    for (; it != m_AudioDevInfoVector.end(); ++it)
    {
        pjmedia_aud_dev_info const& info = *it;
        if(index == number)
        {
            cardName = std::string(info.name);
            break;
        }
        ++index;
    }
}


void RoIP_ED137::delete_oldrecords(std::string folderName)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp = opendir(folderName.c_str())) != NULL)
    {
        while ((dirp = readdir(dp)) != NULL) {
            std::string fileToDell = folderName + "/" + dirp->d_name ;
            if(fileToDell.substr(fileToDell.find_last_of(".") + 1) == "wav")
            {
               remove(fileToDell.c_str());
            }
        }
    }
}

void RoIP_ED137::registerThread()
{
    if (!pj_thread_is_registered())
    {
        pj_thread_desc desc;
        pj_thread_t* this_thread;

        pj_bzero(desc, sizeof(desc));

        pj_status_t status = pj_thread_register("CurrentThread", desc, &this_thread);

        if (status != PJ_SUCCESS)
        {
            printf("CurrentThread could not be registered. Status: %d\n", status);
            return;
        }else{
            printf("CurrentThread can be registered. Status: %d\n", status);
        }

        this_thread = pj_thread_this();
        if (this_thread == NULL)
        {
            printf("pj_thread_this() returns NULL!\n");
            return;
        }

        if (pj_thread_get_name(this_thread) == NULL)
        {
            printf("pj_thread_get_name() returns NULL!\n");
            return;
        }
    }
}


void RoIP_ED137::recordRtp(const char *pktbuf,const char *payloadbuf,unsigned int pktlen,unsigned int payloadlen)
{
//    if(!m_EnableRecording)
        return;
    //record data
//    m_WavWriter->writeRTPWav(pktbuf,payloadbuf,pktlen,payloadlen);

    if(m_SendPackets)
    {
        //print rtp data
        unsigned int i;
        int indexOfBuffer = 0;
        char stringBuffer [1000];
        indexOfBuffer += sprintf(stringBuffer + indexOfBuffer, "\n");
        u_int8_t value;
        indexOfBuffer += sprintf(stringBuffer + indexOfBuffer, "RTP Data Size = %d\n", (int) pktlen);
        for (i = 0; i < pktlen; i++)
        {
        if (i > 0)
        {
           indexOfBuffer += sprintf(stringBuffer + indexOfBuffer, ":");
        }
        value = pktbuf[i];
        indexOfBuffer += sprintf(stringBuffer + indexOfBuffer,"%02X", value);
        }

        indexOfBuffer += sprintf(stringBuffer + indexOfBuffer, "\n");
        stringBuffer[indexOfBuffer] = '\0';
        printf("%s", stringBuffer);
        qDebug() << stringBuffer;
        //AppentTextBrowser(stringBuffer);
    }
}


bool RoIP_ED137::isRadio(std::string extNumber)
{
    if(extNumber.substr(0, 7).compare("<sip:tx") == 0 || extNumber.substr(0, 2).compare("tx") == 0 ||
            extNumber.substr(0, 7).compare("<sip:tr") == 0 || extNumber.substr(0, 2).compare("rx") == 0)
    {
        return true;
    }
    return false;
}

bool RoIP_ED137::findSoundCard(const std::string &soundDevice)
{
    bool found = false;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    std::vector<std::string> devAttrVec;
    std::vector<std::string>::const_iterator devAttrIt;
    devAttrVec.push_back("idVendor");
    devAttrVec.push_back("idProduct");
    devAttrVec.push_back("manufacturer");
    devAttrVec.push_back("product");
    devAttrVec.push_back("serial");

    // Create the udev object
    udev = udev_new();
    if (!udev)
    {
        return false;
    }
    // Create a list of the devices in the 'hidraw' subsystem.
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "hidraw");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {

        const char *path;

        /// Get the filename of the /sys entry for the device
        //  and create a udev_device object (dev) representing it
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
        if (!dev)
        {
            return false;
        }

        for (devAttrIt = devAttrVec.begin(); devAttrIt != devAttrVec.end(); ++devAttrIt)
        {
            const char* deviceInfo = udev_device_get_sysattr_value(dev, devAttrIt->c_str());

            if (deviceInfo)
            {
                //found = (strstr(deviceInfo, soundDevice.c_str()) != 0);
                std::string cardInfoName(deviceInfo);
                if(soundDevice.find(cardInfoName) != std::string::npos)
                    found = true;
                else
                    found =false;

                if (found) {
                    break;
                }
            }
        }

        udev_device_unref(dev);

        if (found) {
            break;
        }
    }

    // Free the enumerator object
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return found;
}

// clean application exit
void RoIP_ED137::app_exit()
{
//        m_UdpCommunicator->Uinit();
//        m_TcpCommunicator->Uinit();
        pjHangupAll();
        removeDefault();
        removeUser();
        pjsua_destroy();
        exit(0);
}

void RoIP_ED137::setTx1toMainTx()
{
    if ((trx1->radio1->callState) & (trx1->trxmode != TRXMODE_RX))
    {
        trx1->mainTx = true;
        trx2->mainTx = false;
    }
    else if ((trx2->radio1->callState) & (trx2->trxmode != TRXMODE_RX))
    {
        trx1->mainTx = false;
        trx2->mainTx = true;
    }
    else
    {
        trx1->mainTx = false;
        trx2->mainTx = false;
    }

//    qDebug() << "setTx1toMainTx" << trx1->trxmode << trx1->mainTx;
}

void RoIP_ED137::setTx2toMainTx()
{
    if ((trx2->radio1->callState) & (trx2->trxmode != TRXMODE_RX))
    {
        trx1->mainTx = false;
        trx2->mainTx = true;
    }
    else if ((trx1->radio1->callState) & (trx1->trxmode != TRXMODE_RX))
    {
        trx1->mainTx = true;
        trx2->mainTx = false;
    }
    else
    {
        trx1->mainTx = false;
        trx2->mainTx = false;
    }
}

void RoIP_ED137::setAllTxtoMainTx()
{    
    if ((trx1->radio1->callState) & (trx1->trxmode != TRXMODE_RX))
    {
        trx1->mainTx = true;
    }
    else
    {
        trx1->mainTx = false;
    }

    if ((trx2->radio1->callState) & (trx2->trxmode != TRXMODE_RX))
    {
        trx2->mainTx = true;
    }
    else
    {
        trx2->mainTx = false;
    }
}

void RoIP_ED137::setTx1toMainRx()
{
    if(rxBestSignalEnable == true) return;
    if ((trx1->radio1->callState) & (trx1->trxmode != TRXMODE_TX) & (trx1->trxmode != TRXMODE_SEPARATE))
    {
        trx1->mainRx = true;
        trx2->mainRx = false;
    }
    else if ((trx1->radio2->callState) & (trx1->trxmode == TRXMODE_SEPARATE))
    {
        trx1->mainRx = true;
        trx2->mainRx = false;
    }
    else if ((trx2->radio1->callState) & (trx2->trxmode != TRXMODE_TX) & (trx2->trxmode != TRXMODE_SEPARATE))
    {
        trx1->mainRx = false;
        trx2->mainRx = true;
    }
    else if ((trx2->radio2->callState) & (trx2->trxmode == TRXMODE_SEPARATE))
    {
        trx1->mainRx = false;
        trx2->mainRx = true;
    }
    else
    {
        trx1->mainRx = false;
        trx2->mainRx = false;
    }
}

void RoIP_ED137::setTx2toMainRx()
{
    if(rxBestSignalEnable == true) return;
    if ((trx2->radio1->callState) & (trx2->trxmode != TRXMODE_TX) & (trx2->trxmode != TRXMODE_SEPARATE))
    {
        trx1->mainRx = false;
        trx2->mainRx = true;
    }
    else if ((trx2->radio2->callState) & (trx2->trxmode == TRXMODE_SEPARATE))
    {
        trx1->mainRx = false;
        trx2->mainRx = true;
    }
    else if ((trx1->radio1->callState) & (trx1->trxmode != TRXMODE_TX) & (trx1->trxmode != TRXMODE_SEPARATE))
    {
        trx1->mainRx = true;
        trx2->mainRx = false;
    }
    else if ((trx1->radio2->callState) & (trx1->trxmode == TRXMODE_SEPARATE))
    {
        trx1->mainRx = true;
        trx2->mainRx = false;
    }
    else
    {
        trx1->mainRx = false;
        trx2->mainRx = false;
    }
}

void RoIP_ED137::setAllTxtoMainRx()
{
    if(rxBestSignalEnable == true) return;
    if ((trx1->radio1->callState) & (trx1->trxmode != TRXMODE_TX) & (trx1->trxmode != TRXMODE_SEPARATE))
    {
        trx1->mainRx = true;
    }
    else if ((trx1->radio2->callState) & (trx1->trxmode == TRXMODE_SEPARATE))
    {
        trx1->mainRx = true;
    }
    else
    {
        trx1->mainRx = false;
    }

    if ((trx2->radio1->callState) & (trx2->trxmode != TRXMODE_TX) & (trx2->trxmode != TRXMODE_SEPARATE))
    {
        trx2->mainRx = true;
    }
    else if ((trx2->radio2->callState) & (trx2->trxmode == TRXMODE_SEPARATE))
    {
        trx2->mainRx = true;
    }
    else
    {
        trx2->mainRx = false;
    }
}

void RoIP_ED137::setTRxAMain()
{
    if (trx1->trxmode != TRXMODE_SEPARATE){
        if (trx1->radio1->callState)
        {
            trx1->mainStandby = MAIN;
            trx2->mainStandby = STB;
        }
    }
    else if (trx1->radio2->callState == false)
    {
        if (trx2->trxmode != TRXMODE_SEPARATE){
            if (trx2->radio1->callState){
                trx1->mainStandby = STB;
                trx2->mainStandby = MAIN;
            }
        }
        else if (trx2->radio2->callState){
            if (trx2->radio1->callState){
                trx1->mainStandby = STB;
                trx2->mainStandby = MAIN;
            }else{
                if (trx1->radio1->callState){
                    trx1->mainStandby = MAIN;
                    trx2->mainStandby = STB;
                }
            }
        }else{
            if (trx1->radio1->callState){
                trx1->mainStandby = MAIN;
                trx2->mainStandby = STB;
            }
        }
    }else{
        if (trx1->radio1->callState){
            trx1->mainStandby = MAIN;
            trx2->mainStandby = STB;
        }
    }
}
void RoIP_ED137::setTRxBMain(){
    if (trx2->trxmode != TRXMODE_SEPARATE){
        if (trx2->radio1->callState){
            trx2->mainStandby = MAIN;
            trx1->mainStandby = STB;
        }
    }else if (trx2->radio2->callState == false){
        if (trx1->trxmode != TRXMODE_SEPARATE){
            if (trx1->radio1->callState){
                trx1->mainStandby = MAIN;
                trx2->mainStandby = STB;
            }
        }
        else if (trx1->radio2->callState){
            if(trx1->radio1->callState){
                trx1->mainStandby = MAIN;
                trx2->mainStandby = STB;
            }else{
                if (trx2->radio1->callState){
                    trx1->mainStandby = STB;
                    trx2->mainStandby = MAIN;
                }
            }
        }else{
            if (trx2->radio1->callState){
                trx2->mainStandby = MAIN;
                trx1->mainStandby = STB;
            }
        }
    }else{
        if (trx2->radio1->callState){
            trx2->mainStandby = MAIN;
            trx1->mainStandby = STB;
        }
    }
}
void RoIP_ED137::setvolume(pjsua_call_id call_id, bool mute)
{
    if ((pttGroupActiveTmp == true)&(groupMute != MUTEIGNORE)){
        SLOT_VOLUME = 0.0f;
        if (call_id != -1)
            setSlotVolume(call_id,false,true);
        return;
    }

    if (mute)
    {
        SLOT_VOLUME = 0.0f;
        if (call_id != -1)
            setSlotVolume(call_id,false,true);
    }
    else {
        if (!connToRadio)
        {
            SLOT_VOLUME = 2.0f;
            if (((trx1->radio1->pptTestPressed)||(trx2->radio1->pptTestPressed)||(trx1->trxStatus == PTTON)||(trx2->trxStatus == PTTON)) & (pttInput)){
                setvolumeSiteTone(call_id);
            }
            else
            {
                if (((trx1->trxStatus == RXON)||(trx2->trxStatus == RXON)|| (((trx1->trxStatus == PTTRX)||(trx2->trxStatus == PTTRX)) & (pttInput == false))) & localSidetoneLoopbackOn == false)
                    setSlotVolume(call_id,false,true);
            }
        }
        else
        {
            SLOT_VOLUME = 2.0f;
            if ((trx1->radio1->pptTestPressed)||(trx2->radio1->pptTestPressed)||(trx1->trxStatus == PTTON)||(trx2->trxStatus == PTTON)){
                setSlotVolume(call_id,false,true);
            }
            else
            {
//                if ((trx1->trxStatus == RXON)||(trx2->trxStatus == RXON))
//                    setSlotVolume(call_id,false,true);
            }
        }
    }
}


void RoIP_ED137::checkMainStandby()
{
//    if (((trx1->radio1->callState)&(trx1->radio1->trxmode != TRXMODE_RX))&((trx2->radio1->callState)&(trx2->radio1->trxmode != TRXMODE_RX)))
    int day = QDate::currentDate().day();
    int hr = QTime::currentTime().hour();
    int minute = QTime::currentTime().minute();
    switch (txScheduler)
    {
        case SCH_ONLY_A:
            setTRxAMain();
            trx1->mainTxRx = true;
            trx2->mainTxRx = false;
            break;

        case SCH_ONLY_B:
            setTRxBMain();
            trx1->mainTxRx = false;
            trx2->mainTxRx = true;
            break;

        case SCH_ALL:
            trx1->mainTxRx = true;
            trx2->mainTxRx = true;
        break;
    case SCH_ODD_EVEN_DAY:

        if (!(day%2 == 0))
        {
            trx1->mainTxRx = true;
            trx2->mainTxRx = false;
        }
        else
        {
            trx1->mainTxRx = false;
            trx2->mainTxRx = true;
        }
        break;

    case SCH_ODD_EVEN_HOUR:

        if (hr%2 == 0)
        {
            trx1->mainTxRx = true;
            trx2->mainTxRx = false;
        }
        else
        {
            trx1->mainTxRx = false;
            trx2->mainTxRx = true;
        }
        break;

    case SCH_ODD_EVEN_HALF_HR:

        if (minute <= 30)
        {
            trx1->mainTxRx = true;
            trx2->mainTxRx = false;
        }
        else
        {
            trx1->mainTxRx = false;
            trx2->mainTxRx = true;
        }
        break;

    case SCH_AM_PM:

        if (hr < 12)
        {
            trx1->mainTxRx = true;
            trx2->mainTxRx = false;
        }
        else
        {
            trx1->mainTxRx = false;
            trx2->mainTxRx = true;
        }
        break;
    case SCH_PTT_DISABLE:
        trx1->mainTxRx = false;
        trx2->mainTxRx = false;
        break;
    }

    if (trx1->mainTxRx & trx2->mainTxRx)
    {
        setAllTxtoMainTx();
        setAllTxtoMainRx();
    }
    else if (trx1->mainTxRx & (!trx2->mainTxRx))
    {
        setTx1toMainTx();
        setTx1toMainRx();
    }
    else if ((!trx1->mainTxRx) & (trx2->mainTxRx))
    {
        setTx2toMainTx();
        setTx2toMainRx();
    }
    else
    {
        trx1->mainTxRx = true;
        trx2->mainTxRx = true;
        trx1->mainTx = false;
        trx2->mainTx = false;

        setTx1toMainRx();
    }

    if (trx1->radio1->callState)
        setSlaveEnable(trx1->radio1->call_id,trx2->mainRx,trx2->mainTx);
    if (trx1->radio2->callState)
        setSlaveEnable(trx1->radio2->call_id,trx2->mainRx,trx2->mainTx);
    if (trx2->radio1->callState)
        setSlaveEnable(trx2->radio1->call_id,trx2->mainRx,trx2->mainTx);
    if (trx2->radio2->callState)
        setSlaveEnable(trx2->radio2->call_id,trx2->mainRx,trx2->mainTx);

    if ((trx1->mainTx) || (trx1->mainRx))
    {
        trx1->mainStandby = MAIN;
    }
    else if ((trx1->radio1->callState) || (trx1->radio2->callState))
    {
        trx1->mainStandby = STB;
    }
    else
    {
        trx1->mainStandby = " : ";
    }


    if ((trx2->mainTx) || (trx2->mainRx))
    {
        trx2->mainStandby = MAIN;
    }
    else if ((trx2->radio1->callState) || (trx2->radio2->callState))
    {
        trx2->mainStandby = STB;
    }
    else
    {
        trx2->mainStandby = " : ";
    }


    if ((trx1->mainTx)&&(!trx2->mainTx))
    {
        if((trx1->radio1->pptTestPressed == false)&(pttStatus == true))
        {
            pttStatus = false;
        }
        else if((trx2->radio1->pptTestPressed == true)&(pttStatus == true))
        {
            pttStatus = false;
        }
    }
    else if ((!trx1->mainTx)&&(trx2->mainTx))
    {
        if((trx2->radio1->pptTestPressed == false)&(pttStatus == true))
        {
            pttStatus = false;
        }
        else if((trx1->radio1->pptTestPressed == true)&(pttStatus == true))
        {
            pttStatus = false;
        }
    }
    else if ((trx1->mainTx)&&(trx2->mainTx))
    {
        if(((trx1->radio1->pptTestPressed == false)||(trx2->radio1->pptTestPressed == false))&(pttStatus == true))
        {
            pttStatus = false;
        }
    }
    else if(txScheduler == SCH_PTT_DISABLE)
    {
        if(((trx1->radio1->pptTestPressed == true)||(trx2->radio1->pptTestPressed == true))&(pttStatus == true))
            pttStatus = false;
    }

//    qDebug() << "rxBestSignalEnable" << rxBestSignalEnable << "trx1->mainRx" << trx1->mainRx << "trx1->radio2->call_id" << trx1->radio2->call_id  << "pptTestPressed" << trx1->radio2->SQLOn;
    if(rxBestSignalEnable == false)
    {
        if ((trx1->mainRx)&(trx2->mainRx))
        {
            if ((trx1->radio1->callState) &(trx1->radio1->trxmode != TRXMODE_TX)){
                if (trx1->radio1->SQLOn)
                    setvolume(trx1->radio1->call_id,false);
            }else if ((trx1->radio1->callState) &(trx1->radio1->trxmode == TRXMODE_TX)){
                setvolume(trx1->radio1->call_id,true);
            }
            if ((trx2->radio1->callState) &(trx2->radio1->trxmode != TRXMODE_TX)){
                if (trx2->radio1->SQLOn)
                    setvolume(trx2->radio1->call_id,false);
            }else if ((trx2->radio1->callState) &(trx2->radio1->trxmode == TRXMODE_TX)){
                setvolume(trx2->radio1->call_id,true);
            }
            if ((trx1->radio2->callState)&(trx1->radio2->SQLOn)){
                setvolume(trx1->radio2->call_id,false);
            }
            if ((trx2->radio2->callState)&(trx2->radio2->SQLOn)){
                setvolume(trx2->radio2->call_id,false);
            }
        }
        else if ((trx1->mainRx)&(!trx2->mainRx))
        {
            if ((trx1->radio2->callState) &(trx1->trxmode == TRXMODE_SEPARATE)){
                if (trx1->radio2->SQLOn)
                    setvolume(trx1->radio2->call_id,false);
                if (trx2->radio1->callState){
                    setvolume(trx2->radio1->call_id,true);
                }
                if (trx2->radio2->callState){
                    setvolume(trx2->radio2->call_id,true);
                }
            }
            else if ((trx1->radio1->callState) &(trx1->radio1->trxmode != TRXMODE_TX)){
                if (trx1->radio1->SQLOn)
                    setvolume(trx1->radio1->call_id,false);
                if (trx2->radio1->callState){
                    setvolume(trx2->radio1->call_id,true);
                }
                if (trx2->radio2->callState){
                    setvolume(trx2->radio2->call_id,true);
                }
            }else if ((trx1->radio1->callState) &(trx1->radio1->trxmode == TRXMODE_TX)){
                setvolume(trx1->radio1->call_id,true);
                if (trx1->radio2->callState){
                    if (trx1->radio2->SQLOn)
                        setvolume(trx1->radio2->call_id,false);

                    if (trx2->radio1->callState){
                        setvolume(trx2->radio1->call_id,true);
                    }
                    if (trx2->radio2->callState){
                        setvolume(trx2->radio2->call_id,true);
                    }

                }
                else if (!trx1->radio2->callState)
                {
                    if ((trx2->radio1->callState) &(trx2->radio1->trxmode != TRXMODE_TX)){
                        if (trx2->radio1->SQLOn)
                            setvolume(trx2->radio1->call_id,false);
                    }else if ((trx2->radio1->callState) &(trx2->radio1->trxmode == TRXMODE_TX)){
                        setvolume(trx2->radio1->call_id,true);
                    }
                    if ((trx2->radio2->callState)&(trx2->radio2->SQLOn)){
                        setvolume(trx2->radio2->call_id,false);
                    }
                }
            }else{
                if ((trx2->radio1->callState) &(trx2->radio1->trxmode != TRXMODE_TX))
                {
                    if (trx2->radio1->SQLOn)
                        setvolume(trx2->radio1->call_id,false);
                }
                else if ((trx2->radio1->callState) &(trx2->radio1->trxmode == TRXMODE_TX))
                {
                    setvolume(trx2->radio1->call_id,true);
                }
                if ((trx2->radio2->callState)&(trx2->radio2->SQLOn)){
                    setvolume(trx2->radio2->call_id,false);
                }
            }
        }
        else if ((!trx1->mainRx)&(trx2->mainRx))
        {
            if ((trx2->radio2->callState) &(trx2->trxmode == TRXMODE_SEPARATE)){
                if (trx2->radio2->SQLOn)
                    setvolume(trx2->radio2->call_id,false);
                if (trx1->radio1->callState){
                    setvolume(trx1->radio1->call_id,true);
                }
                if (trx1->radio2->callState){
                    setvolume(trx1->radio2->call_id,true);
                }
            }
            else if ((trx2->radio1->callState) &(trx2->radio1->trxmode != TRXMODE_TX)){
                if (trx2->radio1->SQLOn)
                    setvolume(trx2->radio1->call_id,false);
                if (trx1->radio1->callState){
                    setvolume(trx1->radio1->call_id,true);
                }
                if (trx1->radio2->callState){
                    setvolume(trx1->radio2->call_id,true);
                }
            }else if ((trx2->radio1->callState) &(trx2->radio1->trxmode == TRXMODE_TX)){
                setvolume(trx2->radio1->call_id,true);

                if ((trx2->radio2->callState)){
                    if (trx2->radio2->SQLOn)
                        setvolume(trx2->radio2->call_id,false);
                    if (trx1->radio1->callState){
                        setvolume(trx1->radio1->call_id,true);
                    }
                    if (trx1->radio2->callState){
                        setvolume(trx1->radio2->call_id,true);
                    }
                }
                else if (!trx2->radio2->callState){
                    if ((trx1->radio1->callState) &(trx1->radio1->trxmode != TRXMODE_TX)){
                        if (trx1->radio1->SQLOn)
                            setvolume(trx1->radio1->call_id,false);
                    }else if ((trx1->radio1->callState) &(trx1->radio1->trxmode == TRXMODE_TX)){
                        setvolume(trx1->radio1->call_id,true);
                    }
                    if ((trx1->radio2->callState)&(trx1->radio2->SQLOn)){
                        setvolume(trx1->radio2->call_id,false);
                    }
                }
            }else{
                if ((trx1->radio1->callState) &(trx1->radio1->trxmode != TRXMODE_TX)){
                    if (trx1->radio1->SQLOn)
                        setvolume(trx1->radio1->call_id,false);
                }else if ((trx1->radio1->callState) &(trx1->radio1->trxmode == TRXMODE_TX)){
                    setvolume(trx1->radio1->call_id,true);
                }
                if ((trx1->radio2->callState)&(trx1->radio2->SQLOn)){
                    setvolume(trx1->radio2->call_id,false);
                }
            }
        }
        else if ((!trx1->mainRx)&(!trx2->mainRx))
        {
            if (trx1->radio1->callState){
                setvolume(trx1->radio1->call_id,true);
            }
            if (trx2->radio1->callState){
                setvolume(trx2->radio1->call_id,true);
            }
            if (trx1->radio2->callState){
                setvolume(trx1->radio2->call_id,true);
            }
            if (trx2->radio2->callState){
                setvolume(trx2->radio2->call_id,true);
            }
        }
        else
        {
            {
                if ((trx1->radio1->callState) &(trx1->radio1->trxmode != TRXMODE_TX)){
                    if (trx1->radio1->SQLOn)
                        setvolume(trx1->radio1->call_id,false);
                }else if ((trx1->radio1->callState) &(trx1->radio1->trxmode == TRXMODE_TX)){
                    setvolume(trx1->radio1->call_id,true);
                }
                if ((trx2->radio1->callState) &(trx2->radio1->trxmode != TRXMODE_TX)){
                    if (trx2->radio1->SQLOn)
                        setvolume(trx2->radio1->call_id,false);
                }else if ((trx2->radio1->callState) &(trx2->radio1->trxmode == TRXMODE_TX)){
                    setvolume(trx2->radio1->call_id,true);
                }
                if ((trx1->radio2->callState)&(trx1->radio2->SQLOn)){
                    setvolume(trx1->radio2->call_id,false);
                }
                if ((trx2->radio2->callState)&(trx2->radio2->SQLOn)){
                    setvolume(trx2->radio2->call_id,false);
                }
            }
        }
    }
    else {
        if(trx1->radio1->audioSQLOn)
        {
            if (trx1->radio1->callState)
                setvolume(trx1->radio1->call_id,UNMUTE);
            trx1->mainRx = true;
            trx2->mainRx = false;
        }
        else if (trx1->radio1->callState)
            setvolume(trx1->radio1->call_id,MUTE);
        if(trx1->radio2->audioSQLOn)
        {
            if (trx1->radio2->callState)
                setvolume(trx1->radio2->call_id,UNMUTE);
            trx1->mainRx = true;
            trx2->mainRx = false;
        }
        else if (trx1->radio2->callState)
            setvolume(trx1->radio2->call_id,MUTE);
        if (trx2->radio1->audioSQLOn){
            if (trx2->radio1->callState)
                setvolume(trx2->radio1->call_id,UNMUTE);
            trx1->mainRx = false;
            trx2->mainRx = true;
        }
        else if (trx2->radio1->callState)
            setvolume(trx2->radio1->call_id,MUTE);
        if (trx2->radio2->audioSQLOn){
            if (trx2->radio2->callState)
                setvolume(trx2->radio2->call_id,UNMUTE);
            trx1->mainRx = false;
            trx2->mainRx = true;
        }
        else if (trx2->radio2->callState)
            setvolume(trx2->radio2->call_id,MUTE);

    }

}

void RoIP_ED137::updateHostConfig(QString sipuser, int sipport, int keepaliveperoid)
{
    pjHangupAll();
    SipUser = sipuser.toStdString();
    sip_port = sipport;
    keepAlivePeroid = keepaliveperoid;
    removeDefault();
    RegisterDefault();

    updateUri();

    exit(1);
}

void RoIP_ED137::keeplogAudioLevel(trx *radio)
{
    if(radio->eventPttSQL_In_LoggingOn == true)
    {
//        qDebug() << "keeplogAudioLevel" <<  radio->url << "SUM" << radio->level_in_av << radio->level_in_count << radio->OutgoingRTPSum;
        radio->level_in_count += 1;
        radio->level_in = audioInLevel;
        radio->level_in_av += audioInLevel;
        radio->OutgoingRTPSum += radio->OutgoingRTP;
        if (audioInLevel > radio->level_in_max)
            radio->level_in_max = audioInLevel;
        if (audioInLevel < radio->level_in_min)
            radio->level_in_min = audioInLevel;

        if (radio->OutgoingRTP > radio->OutgoingRTPmax)
            radio->OutgoingRTPmax = radio->OutgoingRTP;
        if (radio->OutgoingRTP < radio->OutgoingRTPmin)
            radio->OutgoingRTPmin = radio->OutgoingRTP;
    }
}


void RoIP_ED137::createPTTEventDataLogger(trx *radio, QString strEvent)
{
    if(inviteMode == SERVER)
    {
        qDebug() << "createPTTEventDataLogger" << radio->url << strEvent;
        if (strEvent == "pptTest_pressed")
        {
            if(radio->eventPttSQL_In_LoggingOn == false)
            {
                radio->eventPttSQL_In_LoggingOn = true;

                radio->level_in_count = 0;
                radio->level_in = 10*log10(audioInLevel);
                radio->level_in_av = 0;
                radio->level_in_max = 0;
                radio->level_in_min = 255;

                radio->OutgoingRTPSum = 0;
                radio->OutgoingRTPmax = 0;
                radio->OutgoingRTPmin = 255;

                QString message = QString("{"
                                  "\"menuID\"                       :\"PTTEventDataLogger\", "
                                  "\"softPhoneID\"                  :%1, "
                                  "\"Ptt\"                          :\"%2\", "
                                  "\"level_in_av\"                  :%3, "
                                  "\"level_in_max\"                 :%4, "
                                  "\"level_in_min\"                 :%5, "
                                  "\"radioUrl \"                    :\"%6\","
                                  "\"OutgoingRTPAv\"                :%7, "
                                  "\"OutgoingRTPmax\"               :%8, "
                                  "\"OutgoingRTPmin\"               :%9 "
                                  "}"
                                  ).arg(m_softPhoneID).arg(strEvent).arg(radio->level_in).arg(radio->level_in).arg(radio->level_in).arg(radio->url).arg(radio->OutgoingRTP).arg(radio->OutgoingRTP).arg(radio->OutgoingRTP);
                emit sendTextMessage(message);
//                qDebug() << "createPTTEventDataLogger" << message;
            }
            else
            {
                radio->eventPttSQL_In_LoggingOn = true;
            }
        }
        else if (strEvent == "pptTest_released")
        {
            if(radio->eventPttSQL_In_LoggingOn == true)
            {
                radio->eventPttSQL_In_LoggingOn = false;

                radio->level_in_av = 10*log10(radio->level_in_av/radio->level_in_count);
                radio->level_in_max = 10*log10(radio->level_in_max);
                radio->level_in_min = 10*log10(radio->level_in_min);

                radio->OutgoingRTPav = radio->OutgoingRTPSum/radio->level_in_count;

                QString message = QString("{"
                                  "\"menuID\"                       :\"PTTEventDataLogger\", "
                                  "\"softPhoneID\"                  :%1, "
                                  "\"Ptt\"                          :\"%2\", "
                                  "\"level_in_av\"                  :%3, "
                                  "\"level_in_max\"                 :%4, "
                                  "\"level_in_min\"                 :%5, "
                                  "\"radioUrl \"                    :\"%6\","
                                  "\"OutgoingRTPAv\"                :%7, "
                                  "\"OutgoingRTPmax\"               :%8, "
                                  "\"OutgoingRTPmin\"               :%9 "
                                  "}"
                                  ).arg(m_softPhoneID).arg(strEvent).arg(radio->level_in_av).arg(radio->level_in_max).arg(radio->level_in_min).arg(radio->url).arg(radio->OutgoingRTPav).arg(radio->OutgoingRTPmax).arg(radio->OutgoingRTPmin);
                emit sendTextMessage(message);
//                qDebug() << radio->level_in_count << "createPTTEventDataLogger" << message;

                radio->level_in_count = 0;
                radio->level_in = 0;
                radio->level_in_av = 0;
                radio->level_in_max = 0;
                radio->level_in_min = 255;
            }
            else
            {
                radio->eventPttSQL_In_LoggingOn = false;
            }
        }
    }
}

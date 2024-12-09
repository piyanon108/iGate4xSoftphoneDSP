#ifndef ROIP_ED137_H
#define ROIP_ED137_H

#include <QObject>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTimer>
#include <QDateTime>
#include <QElapsedTimer>
#include <iostream>
#include <fstream>
#include <QSettings>
#include <pjsua-lib/pjsua.h>
#include <pjmedia_audiodev.h>
#include <Utility.h>

#include "TransportAdapter.h"
#include "WavWriter.h"
#include <dirent.h>
#include <string.h>

#include <alsa/asoundlib.h>
#include "gpioclass.h"
#include <iostream>
#include <QTimer>
#include "menuindex.h"
#include "networkmng.h"
#include "at88sc.h"
#include "database.h"
#include "SnmpStack.h"
#include "StaticVariables.h"
#include "audiometer.h"
#include "graphics.h"
#include "GetInputEvent.h"
#include "ChatClient.h"

#include <boost/atomic.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include <libudev.h>


// SSD1322 supports 480x128 but display is 256x64.
#define COLS_VIS_MIN 0x00 // Visible cols - start.
#define COLS_VIS_MAX 0x3f // Visible cols - end.
#define ROWS_VIS_MIN 0x00 // Visible rows - start.
#define ROWS_VIS_MAX 0x3f // Visible rows - end.


#define TXMODE_DISABLE false
#define TXMODE_ENABLE true
#define TXSTATUS_DEFAULT 0
#define TXSTATUS_HASPTT 1
#define TXSTATUS_PRESSPTT 2
#define TRANSMITTER 0
#define RECEIVER 1
#define TRANSCEIVER 2
#define TRXMODE_TRX "TRx"
#define TRXMODE_TX "Tx"
#define TRXMODE_RX "Rx"
#define TRXMODE_SEPARATE "Separate"
#define CONNECT_DISABLE false
#define CONNECT_ENABLE true
#define RXMODE_DISABLE false
#define RXMODE_ENABLE true
#define RXSTATUS_DEFAULT 0
#define RXSTATUS_HASSQ 1

#define CONNECTED       "Connected "
#define DISCONNECTED    "Disconnected"
#define CONNECTING      "Connecting"
#define RECONNECTING    "Reconnecting"
#define CALLING         "Calling"
#define CONNSTB         "Standby"

#define TRX1    "Node1 "
#define TRX2    "Node2 "
#define STB   "Inactive : "
#define MAIN  "Active : "
#define RECEIVE "Rx Only : "

#define EMER            "EMR"
#define RPTON           "RPT"
#define PTTON           "Tx"
#define RXON            "Rx"
#define PTTRX           "Tx&Rx"
#define STANDBY         "--"
#define WAIT            "***"

#define MUTEAUDIO   0
#define MUTEALL     1
#define MUTEIGNORE  2

#define MAINRADIO   0
#define STANDBYRADIO   1
#define BOTH   2

#define UNMUTE  false
#define MUTE    true

#define HOMEDISPALY 0

#define LEDOFF 1
#define LEDON 0

#define SERVER 1
#define CLIENT 2
#define CLIENT_RPT 3
#define SERVER_RPT 4

#define CLOCK_RATE 8000
#define CHANNEL_COUNT 1
#define SAMPLES_PER_FRAME (CLOCK_RATE/100)
#define PTIME		    20
#define PJSUA_LOG_LEVEL 0
#define DEFAULT_SAMPLE_RATE		8000
#define DEFAULT_FFT_SIZE        1024
#define CARD_CAPTURE_RPT        "LoopbackIn1"
//#define CARD_CAPTURE_NORMAL     "voipIn1"
//edit for Suport recorder
#define CARD_CAPTURE_NORMAL_RECON     "LoopbackIn1"


#define HWMODEL_JSNANO

#define KEYDOWN     0
#define KEYUP       1
#define KEYCENTER   2
#define KEYLEFT     3
#define KEYRIGHT    4

#define GPS_RESET_PIN   3
#define A_SHD_PIN   4
#define A_MUTE_PIN   5
#define CODEC1U15_RESET_PIN   6
#define CODEC2U16_RESET_PIN   7
//#define TXLED_PIN       "27"
//#define RXLED_PIN       "12"
//#define RX_PTT_SEL_PIN  "5"

//#define LCDBLPWM_DEV    "0"
//#define LCDBLPWM_NUM    "1"


#define LCDSPI_DEV      "/dev/spidev1.0"
#define OLED_RESET      "66" //GPIO03 GPIO3_PI.02 I = 8  GPIO = (8*8)+2 = 66
#define OLED_DC         "63" //GPIO05 GPIO3_PH.07 H = 7  GPIO = (7*8)+7 = 63

#define SCH_ONLY_A          0
#define SCH_ONLY_B          1
#define SCH_ALL             2
#define SCH_ODD_EVEN_DAY    3
#define SCH_ODD_EVEN_HOUR   4
#define SCH_ODD_EVEN_HALF_HR   5
#define SCH_AM_PM           6
#define SCH_PTT_DISABLE     7

#define FOURWIRE2USER       0
#define FOURWIRE2RADIO      1

#define TX1ST  0
#define RX1ST  1
#define FULLDUPLEX  2


#define SWVERSION "3.3 03042023"
#define HWVERSION "iGate-4w 2023"

#define GPIO00	"PZ.01"     //"479"		//GPIO33_PZ.01		tegra234-gpio = (23*8)+1 = 185
#define GPIO01	"PQ.05"     //"453"		//GPIO33_PQ.05		tegra234-gpio = (15*8)+5 = 125
#define GPIO02	"PP.06"     //"446"		//GPIO33_PP.06		tegra234-gpio = (14*8)+6 = 118
#define GPIO03	"PCC.00"    //"328"		//GPIO33_PCC.00		tegra234-gpio-aon = (02*8)+0 = 16
#define GPIO04	"PCC.01"    //"329"		//GPIO33_PCC.01		tegra234-gpio-aon = (02*8)+1 = 17
#define GPIO05 	"PCC.02"    //"330"		//GPIO33_PCC.02		tegra234-gpio-aon = (02*8)+2 = 18
#define GPIO06	"PCC.03"    //"331"		//GPIO33_PCC.03		tegra234-gpio-aon = (02*8)+3 = 19
#define GPIO07  "PG.06"     //"389"		//GPIO33_PG.06		tegra234-gpio = (06*8)+6 = 54
#define GPIO08  "PQ.02"     //"450"		//GPIO33_PQ.02		tegra234-gpio = (15*8)+2 = 122 // NV_THERM_FAN_TACH0
#define GPIO09	"PAC.06"    //"492"		//GPIO33_PAC.06		tegra234-gpio = (24*8)+6 = 198
#define GPIO10	"PEE.02"    //"341"		//GPIO33_PEE.02		tegra234-gpio-aon = (04*8)+2 = 34
#define GPIO11	"PQ.06"     //"454"		//GPIO33_PQ.06		tegra234-gpio = (15*8)+6 = 126
#define GPIO12	"PN.01"     //"433"		//GPIO33_PN.01		tegra234-gpio = (13*8)+1 = 105
#define GPIO13  "PH.00"     //"391"		//GPIO33_PH.00		tegra234-gpio = (07*8)+0 = 56
#define GPIO14	"PX.03"     //"465"		//GPIO33_PX.03		tegra234-gpio = (21*8)+3 = 171

#define GPIO1_OFFSET 284 //i2c-1 address 0x20
#define GPA1_0 GPIO1_OFFSET+0
#define GPA1_1 GPIO1_OFFSET+1
#define GPA1_2 GPIO1_OFFSET+2
#define GPA1_3 GPIO1_OFFSET+3
#define GPA1_4 GPIO1_OFFSET+4
#define GPA1_5 GPIO1_OFFSET+5
#define GPA1_6 GPIO1_OFFSET+6
#define GPA1_7 GPIO1_OFFSET+7

#define GPB1_0 GPIO1_OFFSET+8
#define GPB1_1 GPIO1_OFFSET+9
#define GPB1_2 GPIO1_OFFSET+10
#define GPB1_3 GPIO1_OFFSET+11
#define GPB1_4 GPIO1_OFFSET+12
#define GPB1_5 GPIO1_OFFSET+13
#define GPB1_6 GPIO1_OFFSET+14
#define GPB1_7 GPIO1_OFFSET+15

#define GPIO2_OFFSET 300 //i2c-1 address 0x21
#define GPA2_0 GPIO2_OFFSET+0
#define GPA2_1 GPIO2_OFFSET+1
#define GPA2_2 GPIO2_OFFSET+2
#define GPA2_3 GPIO2_OFFSET+3
#define GPA2_4 GPIO2_OFFSET+4
#define GPA2_5 GPIO2_OFFSET+5
#define GPA2_6 GPIO2_OFFSET+6
#define GPA2_7 GPIO2_OFFSET+7

#define GPB2_0 GPIO2_OFFSET+8
#define GPB2_1 GPIO2_OFFSET+9
#define GPB2_2 GPIO2_OFFSET+10
#define GPB2_3 GPIO2_OFFSET+11
#define GPB2_4 GPIO2_OFFSET+12
#define GPB2_5 GPIO2_OFFSET+13
#define GPB2_6 GPIO2_OFFSET+14
#define GPB2_7 GPIO2_OFFSET+15

#define PTT_SQL_INPUT1 GPA2_7
#define PTT_SQL_INPUT2 GPA2_6
#define PTT_SQL_INPUT3 GPA2_5
#define PTT_SQL_INPUT4 GPA2_4

#define PTT_SQL_OUTPUT1 GPA2_3
#define PTT_SQL_OUTPUT2 GPA2_2
#define PTT_SQL_OUTPUT3 GPA2_1
#define PTT_SQL_OUTPUT4 GPA2_0

#define PTT_OFF         0x00    //PTT OFF
#define PTT_Normal      0x01    //NormalPTT ON
#define PTT_Coupling    0x02    //CouplingPTT ON
#define PTT_Priority    0x03    //PriorityPTT ON
#define PTT_Emergency   0x04    //EmergencyPTT ON

static const unsigned int SLEEP_PERIOD = 50;
static const unsigned int SLEEP_COUNTER= 1000;

static const size_t MAX_SIP_ID_LENGTH = 50;
static const size_t MAX_SIP_REG_URI_LENGTH = 50;

static const float MIN_SLOT_VOLUME  = 0.0;
static const float MAX_SLOT_VOLUME  = 2.0;

//typedef void (*re_decoderCB_t)(int,bool);

class RoIP_ED137 : public QObject
{
    Q_OBJECT

public:
    explicit RoIP_ED137(int softphoneID, QObject *parent = Q_NULLPTR);
    virtual ~RoIP_ED137();
    static RoIP_ED137 *instance();


    void on_reg_state(pjsua_acc_id acc_id);
    void on_call_state(pjsua_call_id call_id, pjsip_event *e);
    void on_call_audio_state(pjsua_call_info *ci, unsigned mi, pj_bool_t *has_error, pjsua_call_id call_id);
    void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id, pjsip_rx_data *rdata);
    void on_incoming_subscribe(pjsua_acc_id acc_id,pjsua_srv_pres *srv_pres,pjsua_buddy_id buddy_id,const pj_str_t *from,pjsip_rx_data *rdata,pjsip_status_code *code,pj_str_t *reason,pjsua_msg_data *msg_data);
    void on_call_media_state(pjsua_call_id call_id);
    pjmedia_transport* on_create_media_transport(pjsua_call_id call_id, unsigned media_idx,pjmedia_transport *base_tp,unsigned flags);
    void on_stream_created(pjsua_call_id call_id, pjmedia_stream *strm, unsigned stream_idx,pjmedia_port **p_port);
    void on_stream_destroyed(pjsua_call_id call_id, pjmedia_stream *strm, unsigned stream_idx);

    void setIncomingRTP(tp_adapter* adapter);
    void setOutgoingRTP(tp_adapter* adapter);
    void setIncomingED137Value(uint32_t ed137_value,pjsua_acc_id acc_id);


    void registerThread();

private Q_SLOTS:
    void sendChannelMessage();
private:
    bool pttGroupActive = false;
    bool pttGroupActiveTmp = false;
    bool forceMuteSqlOn = false;
    bool pttDelayForTxOn = false;
    int SqlGroupDelay = 2;
    uint8_t groupMute = MUTEAUDIO;


    double audioInLevel = 0;
    double audioOutLevel = 0;
    double audioInLevel_dB = 0;
    double audioOutLevel_dB = 0;

    QString FILESETTING;
    int m_softPhoneID = 0;
    bool m_radioAutoInactive = 0;
    int m_radioMainStandby = 0;
    int m_channelID = 0;
    ChatClient *socketClient;
    ChatClient *iGateGroupMNGClient;
    QString iGateGroupMNGClientMessage = "";

    int rotarydata = -1;
    QTimer *detectR2SPacketAndReconnTimer;
    QTimer *displayTimer;

    SnmpStack *snmpStack;
    boost::atomic_bool m_IsSNMPThreadRunning;
    boost::shared_ptr<boost::thread> *m_SNMPThreadInstance;
    boost::mutex io_mutex;

    boost::atomic_bool m_ScanKeyThreadRunning;
    boost::shared_ptr<boost::thread> *m_ScanKeyThreadInstance;
    boost::mutex scankey_mutex;

    boost::atomic_bool m_GetInputThreadRunning;
    boost::shared_ptr<boost::thread> *m_GetInputThreadInstance;
    boost::mutex getInput_mutex;

    boost::atomic_bool m_GetOutputThreadRunning;
    boost::shared_ptr<boost::thread> *m_GetOutputThreadInstance;
    boost::mutex getOutput_mutex;

//    UDPCommunicator *m_UdpCommunicator;
//    TCPCommunicator *m_TcpCommunicator;
//    WavWriter       *m_WavWriter;
    int clock_rate;

//    int inputAudioGainIndex;
//    int outputAudioGainIndex;
//    int speakerGainIndex;
//    int dacVolumeCtrlLEDVal;
//    bool speakermute;

    static void* ThreadFunc( void* pTr );
    typedef void * (*THREADFUNCPTR)(void *);
    pthread_t idThread;

    bool on_call_audio_state_ini = false;
private:    
    bool sipStarted = false;
    void initSoftPhone(string _SipUser, int _clock_rate, quint16 _sip_port, quint16 _rtp_cfg_port, quint16 _keepAlivePeroid, quint8 _txScheduler, quint8 _numConnection, quint8 _inviteMode, float _sidetone, bool _connToRadio, bool _sqlAlwayOn, QString _defaultEthernet, bool _sqlActiveHigh, bool mainRadioTxUsed, bool mainRadioRxUsed, bool radioautoinactive, int radioMainStandby, bool _rxBestSignalEnable, float _localSidetone, uint8_t _groupMute, uint8_t _pttDelay);
    void initRadio(quint8 trxMainSTB, QString channelName, QString trxmode, bool enable, bool alwaysConnect, QString radio1callName, QString radio1ipAddress, QString radio1portAddr, QString radio2callName, QString radio2ipAddress, QString radio2portAddr);
    void initUriNameAllowList(QStringList uriAllowList);
    void startSoftPhone();
    void error_exit(const char *title, pj_status_t status);
    void initLabels();
    void myConfigurate();
    bool checkHashFileNotExit();
    void StartSip();
    void startSipByUser();

    void initSoundCard(int captureCardNumber, int playoutCardNumber);
    void setAudioDevInfo();
    void listAudioDevInfo();
    void searchAudioDevice();
    bool initSlaveSoundCard();
    void setCodecPriority();
    void initializePttManager(const std::string pttDeviceName);

    void playRing();
    void stopRing();
    void init_ringTone();

    bool createNullPort();
    static pj_status_t PutData(struct pjmedia_port *this_port, pjmedia_frame *frame);
    static pj_status_t GetData(struct pjmedia_port *this_port, pjmedia_frame *frame);

    int getConfPortNumber(int callId);
    bool getExtFromCallID(int CallID, std::string& extNumber) const;
    bool getLocalNumberFromCallID(int CallID, std::string& localNumber) const;
    bool checkSipUrl(std::string extNumber);
    bool getExtFromRemoteInfo(pj_str_t const& remoteInfo, std::string& extNumber) const;
    void getSoundCardName(int number, std::string& cardName);
    void getCallPortInfo(std::string ext, pjsua_conf_port_info& returnInfo);
    void listConfs();
    int getActiveCallCount();
    void getSoundCardPorts(std::vector<int>& portList) const;
    void connectCallToSoundCard(int CallID);
    void disConnectCallFromSoundCard(int CallID);
    bool connectPort(int srcPort, int dstPort);
    bool disconnectPort(int srcPort, int dstPort);

    void changeUplinkOrder(const char* Input, char* Output, size_t sizeOfCoded, int g726UplinkBitrate);

    int makeCall(std::string const& extNumber);
    int makeSipCall(std::string const& extNumber);
    int makeCall_Radio(std::string const& extNumber);

    std::string getSipUrl(const char* ext);
    void delete_oldrecords(std::string folderName);
    bool setSlotVolume(int callId, bool increase, bool current);

    //---------------------------------------------------------------------------
    // IPttListener implementations
    //---------------------------------------------------------------------------

    bool findSoundCard(std::string const& soundDevice);

    void checkEvents();
    void scan_call_err();


    /////IP RADIO////
    void setRadioTxrxModeAndRadioType(int callId, QString txrxmode, QString type);
    void setRadioPttbyCallID(bool pttval, int callId, int priority, int userRec);
    void setAdaptercallRecorder(int callId,int val);
    void setRadioSqlOnbyCallID(bool sqlval, int callId, int priority);
    void setRadioSqlOnbyCallID(bool sqlval, int callId, int priority, uint16_t rssi);
    void setSlaveEnable(pjsua_call_id callId,pj_bool_t rx, pj_bool_t tx);
    int get_IPRadioBss(pjsua_call_id callId);
    qint64 get_R2SStatus(pjsua_call_id callId);
    int get_IPRadioPttStatus(pjsua_call_id callId);
    int get_IPRadioPttId(pjsua_call_id callId);
    int get_IPRadioSquelch(pjsua_call_id callId);
    bool get_IPRadioStatus(pjsua_call_id callId);
    bool isRadio(std::string extNumber);
    QString getTimeDuratio(pjsua_call_id callId);
    ///sip////
    void RegisterUser();
    void UnRegisterUser();

    QString getIpAddress();
    void RegisterDefault();
    void UpdateDefault();
    bool removeDefault();

    bool removeUser();

    void app_exit();

    void recordRtp(const char *pktbuf,const char *payloadbuf,unsigned int pktlen,unsigned int payloadlen);
    void printCallState(int state,std::string extNumber);

    ////SPECTRUM///
    void initSpectrumGraph();
    //void FFT(int n, bool inverse, const double *gRe, const double *gIm, double *GRe, double *GIm);


    std::string getexepath()
    {
      std::string path ="\0";
      char cwd[PATH_MAX];
      if (getcwd(cwd, sizeof(cwd)) != NULL)
            return std::string(cwd);
      return path;
    }

private:

    pjmedia_port *ring_port_, *in_ring_port_;
    pjsua_conf_port_id ring_slot_, in_ring_slot_;
    pj_bool_t enableEd137 = PJ_FALSE;

    pjsua_call_id current_call = PJSUA_INVALID_ID;


    QString makeCallIndex = "";

    pjmedia_port* m_NullPort;
    int m_NullSlot;

    char log_buffer[250];
    char plot_buffer[250];

    int m_RegTimeout;
    int sip_port;
    int keepAlivePeroid;
    int rtp_cfg_port;

    QString deviceName;
    std::string SipServer;
    std::string SipUser;
    std::string SipPassword;
    std::string SipCall;
    std::string RxIp;
    std::string TxIp;
    std::string PttDevice;
    std::string localAddress;
    std::string localPort;
    std::string remoteAddress;
    std::string remotePort;
    QString sip_domainstr;
    QString defaultEthernet = "bond0";

    pjsua_acc_id acc_id;

    bool m_Register;
    bool m_AutoCall;
    bool m_SendPackets;
    bool m_PttPressed1;
    bool m_PttPressed2;
    bool m_PttPressed3;
    bool m_isTimerRunning;
    bool m_EnableRecording;
    qint8 m_TxRx1stPriority = FULLDUPLEX;
    bool udpActive;
    bool tcpActive;
    bool isSipCall;
    bool monitorAudioIn;

    std::map<int,pjmedia_transport*> transport_map;
    std::vector<pjmedia_aud_dev_info> m_AudioDevInfoVector;

//    uint32_t ed137_val_old;


    unsigned int sampleRate;
    unsigned int sampleCount;
    unsigned int fftSize;

    float SLOT_VOLUME;
    int delayCount = 0;

    float   *d_realFftData;
    float   *d_iirFftData;
    float signalInput[DEFAULT_FFT_SIZE];
    float d_fftAvg;      /*!< FFT averaging parameter set by user (not the true gain). */

signals:
    void spectValueChanged(int fftSize);
    void spectClear();
    void stateRegChanged(bool regState);
    void stateCallChanged(pjsua_call_id call_id, pjsip_event *e);
    void  pttStatusChange(bool pttStatus);
    void sqlStatusChange(bool sqlStatus);
    void keyPress(int keyNum, bool val);
    void newfrequency(int nodeID, QString frequency, QString sqlLevel,QString frequencyRx, int rfPower);
    void onAction(QString extNumber, QString action, pjsua_call_id call_id);
    void onPttPressed(bool value);
    void cppCommand(QString jsonMsg);
    void sendTextMessage(QString message);
    void onUpdateDisplay();
//    void onTxEnableChange(bool mainRadioTxUsed);
//    void onTxnableChange(bool mainRadioRxUsed);
private slots:
//    void mainRadioTransmitterUsedChange(bool mainRadioTxUsed);
//    void txnableChange(bool mainRadioRxUsed);
    void commandProcess(QString message);
    void onConnected();
    void onTextMessageReceived(QString message);
    void onTextMessageToSend(QString jsonMsg);

    void updateActions(QString extnumber, QString action, pjsua_call_id call_id);
    void getSnmp();
    void onRegStateChanged(bool regState);
    void callStateChanged(pjsua_call_id call_id, pjsip_event *e);
    void trxCallStartStop(pjsua_call_id call_id, QString trxID);
    void trx_incall_hangup(pjsua_call_id call_id, unsigned code, pj_str_t *reason, int i);
    void trx_call_hangup(pjsua_call_id call_id,unsigned code, pj_str_t *reason);
    void pptTest_CallIn_Pressed(int pttLevel);
    void pptTest_CallIn_Released(int pttLevel);
    void checkMainStandby();
    void setvolume(pjsua_call_id call_id, bool mute);
    void setvolumeSiteTone(pjsua_call_id call_id);
    //V3.0
    void setTx1toMainTx();
    void setTx2toMainTx();
    void setAllTxtoMainTx();
    void setTx1toMainRx();
    void setTx2toMainRx();
    void setAllTxtoMainRx();
    //
    void setTRxAMain();
    void setTRxBMain();
    void pptTest_pressed();
    void sqlTest_pressed(int sqlLevel);
    void sqlTest_pressed(int call_id, int level);
    void sqlTest_released();
    void repeat_pressed(int ignorID,int pttLevel);
    void repeat_released(int pttLevel);
    void recorder_pressed(int pttLevel, QString txon);
    void recorder_released(int pttLevel);
    void onPttStatusChange(bool pttstate);
    void onSqlOnChange(bool qslStatus);
    void pptTest_released();
    void onKeyPress(int keyNum, bool val);
    void scanKeyPin();
    void getAudioInputLevel();
    void getAudioOutputLevel();
    void detectR2SPacketAndReconn();
    void display_show();
    void getAudioInputLevelFault();
    void updateWebData(QString menuIndex, QWebSocket* webClient);
    void updateTrxConfig(int trxID, QString CallName, QString Address, QString sipPort, QString trxMode, QString active, QString CallNameRx, QString AddressRx, QString sipPortRx, QString channelName, QString AlwaysConn);
    void updateHostConfig(QString sipuser, int sipport, int keepaliveperoid, QString defaultEth);
    void updateGpioConfig(int gpioNum, int gpioVal);
    void reconnectTrx(int trxID);
    void toggleGpioOut(int gpioNum,int gpioVal);
    void updateswitchInviteMode(int value);
    void updatetxBestsignalSelected(bool val);
    void updatetxScheduler(int schID);
    void updateNTPServer(QString value);
    void updateDeviceName(QString devicename);
    void setcurrentDatetime(QString dateTime);
    void setLocation(QString location);
    void disconnectUri(QString uri);
    void updateURILits(QStringList urlList);
    void updateNumconn(int numconn);
    void pjHangupAll();
    void updateFirmware();
    void updateSQLDefeat(bool sqlDefeat);
    void updateSQLActiveHigh(bool sqlActive);
    void onNumClientChanged(int numclient);
    QStringList findFile();
    void updateUri();
    void updateOutputLevel(int level);
    void updateInputLevel(int level);
    void sendRtpType232(pjsua_call_id callId);

    void updateDisplay();

protected:

private:    
    static RoIP_ED137 *theInstance_;
    void inviteModeUpdate();
    void setupRadio();
    void setNode1Line1();
    void setNode1Type();
    void setNode2Line1();
    void setNode2Type();

    void updateHomeDisplay(QString trxid, QString connState, QString trxState, pjsua_call_id call_id, int radioNodeID);
    void updateTRxStatus();
    void updateVUMeter();

    QString dataChannelMessage = "";

    QString getRadioFrequency(QString ipaddress, int snmpProfile, uint16_t port);
    int getRFPower(QString ipaddress, int snmpProfile, uint16_t port);
    QString getRadioStatus(QString ipaddress, int snmpProfile, uint16_t port);
    int getSnmpProfile(QString ipaddress, uint16_t port);
    float getRadioVSWR(QString ipaddress, int snmpProfile, uint16_t port);
    QString getRadioSqlchThrhCarrier(QString ipaddress, int snmpProfile, uint16_t port);
    QString getUptime();


    void updateHostConfig(QString sipuser, int sipport, int keepaliveperoid);


    int findNextNodeConnected(int currentnode);
    int findPreviousNodeConnected(int currentnode);
    int currentNodeShow;
    int testModeEnable = 0;
    QString connStatusMessage;


    int sqlStatusCount = 0;
    bool sqlStatusOn = false;
    struct trx
    {
        bool txModeEnable = TXMODE_DISABLE;
        bool rxModeEnable = RXMODE_DISABLE;
        bool enable = false;
        quint8 txButtonStatus = TXSTATUS_DEFAULT;
        bool pptTestPressed = false;
        bool sqlTestPressed = false;
        bool rxButtonStatus = RXSTATUS_DEFAULT;
        bool callState = false;
        bool on_call_audio_state_ini = false;
        qint64 lastR2SPacket = QDateTime::currentMSecsSinceEpoch();
        qint64 currentR2SPacket = QDateTime::currentMSecsSinceEpoch();
        bool Busy = false;
        int r2sCount = 0;
        int TestR2SCount = 0;
        int autoConnectCount = 0;
        std::string CallextNumber = "";
        int radioNodeID = 0;

        QString ConnDurationMessage;
        QString url = "";
        QString trxmode = "";
        QString callName = "";
        QString ext_uriName = "";
        QString ipAddress = "";
        uint16_t snmpPort = 160;
        QString portAddr = "";
        QString callLastState = DISCONNECTED;
        QString mainStandby = " :";
//        QString trxState = "";
        QString connDuration = "0";
        int sec_connDuration = 0;
        QString trxStatus = STANDBY;
        int r2sPeriod = 200;
        QString RTPPort = "";
        QStringList sdpAnswer;
        QStringList ByeAnswer;
        QString ByeReason;
        bool trxFailed = false;
        pjsua_call_id call_id  = PJSUA_INVALID_ID;
        QString callIndexName;
        int pttLevel;
        bool m_PttPressed;
        int getSNMPCount = 0;
        int pingCount = 30;
        QString frequency = "0";
        int rssi = 0;
        float vswr = 0.0;
        QString SqlchThrhCarrier = "0";
        int rfPower = 0;
        QString radiostatus = "";
        int snmpProfile = 1;
        bool pingOk = false;
        bool call_hangup = false;
        bool active = true;
        qint64 lastRxmsec = 0;
        qint64 lastTxmsec = 0;
        int lastRx = 0;
        int lastTx = 0;
        QString txrxmode = TRXMODE_TRX;
        QString type = "Radio-Idle";
        bool mainRadioReceiverUsed = false;
        bool mainRadioTransceiverUsed = false;
        uint32_t ed137_val_old;
        bool audioSQLOn = false;
        bool SQLOn = false;
        int SqlGroupDelayCount = 0;

        double level_in = 0;
        double level_out = 0;
        int level_in_count = 0;
        double level_out_count = 0;
        double level_in_av = 0;
        double level_out_av = 0;
        double level_in_max = 0;
        double level_out_max = 0;
        double level_in_min = 0;
        double level_out_min = 0;
        double log_level_in = 0;
        double log_level_out  = 0;
        QDateTime starEventtPttSQLIn;
        QDateTime stopEventPttSQLIn;
        QDateTime starEventtPttSQLOut;
        QDateTime stopEventPttSQLOut;
        bool eventPttSQL_Out_Status = false;
        bool eventPttSQL_In_Status = false;
        bool eventPttSQL_In_LoggingOn = false;
        bool eventPttSQ_LOut_LoggingOn = false;

        uint8_t OutgoingRTP = 0;
        uint16_t OutgoingRTPSum = 0;
        uint8_t OutgoingRTPav = 0;
        uint8_t OutgoingRTPmax = 0;
        uint8_t OutgoingRTPmin = 255;

        uint8_t IncomingRTP = 0;
        uint8_t IncomingRTPav = 0;
        uint8_t IncomingRTPmax = 0;
        uint8_t IncomingRTPmin = 255;
    };
    struct urlList
    {
        QString uri;
        QString txrxmode;
        QString radiotype;
        QString ipaddress;
        QString uriName;
    };

    QList<urlList* > urllist;

    struct channel{
        trx *radio1;
        trx *radio2;
        bool enable = false;
        QString trxmode = TRXMODE_TRX;
        QString channelName;
        QString callLastState = "";
        QString mainStandby = " : ";
        QString trxStatus = STANDBY;
        bool alwaysConnect = false;
        bool pingEnable = true;
        bool mainTxRx = true;
        bool mainTx = true;
        bool mainRx = true;
    };

    float sidetone = 0.1f;
    float localSidetone = 0.1f;
    int txScheduler;
    Database *myDatabase;
    channel *trx1;
    channel *trx2;

    bool rxBestSignalEnable = true;
    QList<trx *> trx_incall;
    QStringList uriNameAllowList;

    SnmpStack *SNMPStack;
    GPIOClass *pttSQLInputPin;
    GPIOClass *pttSQLOutputPin;

    int callInNum = 0;
    int callInNumTmp = 0;
    QString lastPttOn;
    bool trxPttState;
    bool sqlPinLevel;
    int ptt_level = 0;
    int inviteMode;
    int numConnection;
    bool pttStatus = false;
    bool m_c_atis_fault = false;

    bool connToRadio = true;
    bool sqlAlwayOn = false;
    bool pttWithPayload = true;
    bool pttStatusTmp = false;
    bool sqlOnStatus = false;
    bool sqlOnStatusbackup = false;
    bool pttOnStatus = false;
    bool sqlActiveHigh = false;
    bool sqlOldPinOnStatus = false;
    int sqlInputDelay = 0;

    bool localSidetoneLoopbackOn = false;
    bool onLocalSidetoneLoopbackChanged = false;


    QTimer *scankeyTimer;
    QTimer *checkInputTimer;

    QString gpioIn;
    QString gpioOut;
    QString CARD_CAPTURE_NORMAL_NOREC;
    QString CARD_PLAYOUT;

    QString Line1Buf;
    QString Line2Buf;
    QString Line3Buf;
    QString Line4Buf;
    QString Line1;
    QString Line2;
    QString Line3;
    QString Line4;
    uint32_t currentmenuIndex = 0;
    int displayCount;
    int displayCount2;
    NetWorkMng *networking;
    uint8_t portInterface;
    int lcdBacklight;
    bool ntp;
    QString SwVersion;
    QString HwVersion;

    struct phyNetwork{
        QString dhcpmethod;
        QString ipaddress;
        QString subnet;
        QString gateway;
        QString pridns;
        QString secdns;
    };
    phyNetwork eth0;
    phyNetwork eth1;

    QString ntpServer;
    QString timeLocation;
    QString trxStatus;

    float m_input_level = 0;
    float m_output_level = 0;
    float m_max_input_level = 0;
    float m_avg_input_level = 0;
    bool pttInput = 0;
    int pttInput_count = 0;
    int pttDelay_count = 0;
    int pttDelay_set = 5;
    int read_r2s_count = 0;
    qint64 m_input_level_count = 0;
    int warnAudioLevelTime = 5;
    int warnPercentFault = 10;
    int warnPTTMinute = 2;
    int backupAudio_min = 30;

    float max_input_level = 0;
    float avg_input_level = 0;
    int numClientConn = 0;

    AudioMeter *captureLevel;
    AudioMeter *playoutLevel;

    QString recorderAlloweURI;
    bool recorderConnected = false;
    bool recorderSuported = false;

//    bool mainRadioTransmitterUsed = true;
//    bool mainRadioReceiverUsed = true;
    int  radioMainStandby = 0;

    uint16_t rssi_raw = 0;

    void createPTTEventDataLogger(trx *radio, QString strEvent);
    void  keeplogAudioLevel(trx *radio);

    bool pttTestMode = false;


};

#endif // ROIP_ED137_H

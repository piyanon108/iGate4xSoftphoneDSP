#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include "QWebSocketServer"
#include "QWebSocket"
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class ChatClient : public QObject
{
    Q_OBJECT
public:
    explicit ChatClient(quint16 port, QObject *parent = Q_NULLPTR);

    void broadcastMessageNodeState(uint8_t nodeID, QString conn, QString duration, QString trxStatus, QString radioStatus, QString vswr, QString durationRx, QString connRx, QString radioStatusRx,int softPhoneID);
    void broadcastSystemMessage(QString nodeSelected, int softPhoneID);

public slots:
   void sendTextMessage(QString message);
signals :
//    void updateRadio(QString radioName, QString trxMode, QString uri, QString ipAddress, int r2sPeriod, int sipPort, QString frequency, int radioID);
//    void removeRadio(int radioID);

//    void updateChannel(QString channelName, QString mainRadioType, int mainRadio01ID, int mainRadio02ID, bool standbyEnable, QString standbyRadioType, int standbyRadio01ID, int standbyRadio02ID, QString mainRadioName, QString standbyRadioName, int channelID);
//    void removeChannel(int channelID);

//    void updateChannelTxRxEnable(bool mainRadioTransmitterUsed, bool mainRadioReceiverUsed, int channelID);
//    void updateControler(QString sipUser, int keepAlivePeroid, int sipPort, int rtpStartPort, int channelID, int softPhoneID);

    void connected();
    void textMessageReceived(QString message);

private:
    QWebSocket *m_pWebSocketClient;
    QList<QWebSocket *> m_clients;
    QWebSocket *newSocket;

private slots:
    void onConnected();
    void onTextMessageReceived(QString message);
};

#endif // CHATCLIENT_H

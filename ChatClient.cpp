#include "ChatClient.h"

ChatClient::ChatClient(quint16 port, QObject *parent) :
    QObject(parent)
{
    QString url = QString("ws://127.0.0.1:%1").arg(port);
    m_pWebSocketClient = new QWebSocket;
    m_pWebSocketClient->open(QUrl(url));
    connect(m_pWebSocketClient,SIGNAL(connected()), this, SLOT(onConnected()));
//    connect(m_pWebSocketClient,SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));

}

void ChatClient::sendTextMessage(QString message)
{
    m_pWebSocketClient->sendTextMessage(message);    
}

void ChatClient::broadcastMessageNodeState(uint8_t nodeID, QString conn, QString duration, QString trxStatus, QString radioStatus, QString vswr, QString durationRx, QString connRx, QString radioStatusRx,int softPhoneID)
{
    QString message = QString("{\"menuID\":\"connState\", \"nodeID\":%1, \"connStatus\":\"%2\", \"connDuration\":\"%3\", \"trxStatus\":\"%4\", \"radioStatus\":\"%5\", \"vswr\":"
                              "\"%6\", \"connDurationRx\":\"%7\", \"connStatusRx\":\"%8\", \"radioStatusRx\":\"%9\", \"softPhoneID\":%10}")
            .arg(nodeID).arg(conn).arg(duration).arg(trxStatus).arg(radioStatus).arg(vswr).arg(durationRx).arg(connRx).arg(radioStatusRx).arg(softPhoneID);
    m_pWebSocketClient->sendTextMessage(message);
}
void ChatClient::broadcastSystemMessage(QString nodeSelected, int softPhoneID)
{
    QString message = QString("{\"menuID\":\"nodeSelected\", \"softPhoneID\":%1, \"nodeSelected\":\"%3\"}")
            .arg(softPhoneID).arg(nodeSelected);
    m_pWebSocketClient->sendTextMessage(message);
}
void ChatClient::onConnected()
{
    qDebug() << "WebSocket connected";
    connect(m_pWebSocketClient, SIGNAL(textMessageReceived(QString)),
            this, SLOT(onTextMessageReceived(QString)));
    emit connected();
}
void ChatClient::onTextMessageReceived(QString message)
{
    emit textMessageReceived(message);
}


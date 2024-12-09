#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QtSql>
#include <QString>
#include <QWebSocket>
class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QString dbName, QString user, QString password, QString host, QObject *parent = nullptr);
    bool database_createConnection();            
    bool passwordVerify(QString password);
    void genHashKey();
    void hashletPersonalize();
    bool checkHashletNotData();


    void insertNewAudioRec(QString filePath, QString radioEvent);
    void updateAudioRec(QString filePath, float avg_level, float max_level);
    bool getLastEventCheckAudio(int time, int percentFault, int lastPttMinute);
    QString getNewFile(int warnPercentFault);
    qint64 getStandbyDuration();
    void removeAudioFile(int lastMin);
    int currentFileID = 0;
signals:
    void audioFault(bool fault);
    void databaseError();
public slots:

private:
    QSqlDatabase db;
    bool verifyMac();
    QString getPassword();
    qint64 getTimeDuration(QString filePath);
    void getLastEvent();
    void startProject(QString filePath, QString radioEvent);

    QString getSerial();
    QStringList getMac();
    void updateHashTable(QString mac, QString challenge ,QString meta, QString serial, QString password);



private slots:
    void reloadDatabase();
};

#endif // DATABASE_H

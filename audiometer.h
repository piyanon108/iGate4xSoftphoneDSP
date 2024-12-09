#ifndef AUDIOMETER_H
#define AUDIOMETER_H

#include <QObject>

class AudioMeter : public QObject
{
    Q_OBJECT
public:
    explicit AudioMeter(QString cardName, QObject *parent = nullptr);
    void getAudioLevel();
    bool run = true;
signals:
    void onValueChanged(int val);
public slots:


private:
QString FIFOFILE = "/tmp/audiofifo";
};

#endif // AUDIOMETER_H

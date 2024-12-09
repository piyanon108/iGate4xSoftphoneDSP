#include "audiometer.h"
#include<stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <QDebug>
#include <QString>


#define BUFFER 32
AudioMeter::AudioMeter(QString cardName, QObject *parent) : QObject(parent)
{
     FIFOFILE = "/tmp/capturefifo"+cardName;
}
void AudioMeter::getAudioLevel()
{
    char in[BUFFER];

    mkfifo(FIFOFILE.toStdString().c_str(), 0666);

    int in_fd=open(FIFOFILE.toStdString().c_str(), O_RDONLY);

    if (in_fd==-1) {
        perror("open error");
        exit(-1);
    }

    while (read(in_fd, in, BUFFER)>0) {
        qDebug() << QString::fromLocal8Bit(in).toInt() << (int(float((QString::fromLocal8Bit(in).toInt()*100.0)/30000.0)));
        emit onValueChanged(int(float((QString::fromLocal8Bit(in).toInt()*100.0)/30000.0)));
    }
    close(in_fd);
}


#ifndef GETINPUTEVENT_H
#define GETINPUTEVENT_H
#include "QKeyEvent"
#include <QObject>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/input.h>
#include <fcntl.h>

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include <linux/fb.h>
#include <sys/mman.h>

class GetInputEvent : public QObject
{
    Q_OBJECT
public:
    explicit GetInputEvent(char *inputdev, int codedetect, QObject *parent = nullptr);
    void run();
    void print_event(struct input_event *ie);
    int codeDetect;
    char *inputevent;

signals:
    void eventCode(int evCode, int value);
public slots:
};

#endif // GETINPUTEVENT_H

#include "GetInputEvent.h"
#include "QDebug"
GetInputEvent::GetInputEvent(char *inputdev, int codedetect, QObject *parent) : QObject(parent)
{
    inputevent = inputdev;
    codeDetect = codedetect;
}

void GetInputEvent::run()
{
    int fd;
    struct input_event ie;
    if ((fd = open(inputevent, O_RDONLY)) == -1) {
            perror("opening device");
            exit(EXIT_FAILURE);
        }

        while (read(fd, &ie, sizeof(struct input_event)))
        {
            if (ie.type == codeDetect)
            {
                emit eventCode(ie.code,ie.value);
            }
//            print_event(&ie);
        }
        close(fd);
}

void GetInputEvent::print_event(struct input_event *ie)
{

    switch (ie->type) {
    case EV_SYN:
        fprintf(stderr, "time:%ld.%06ld\ttype:%d\tcode:%d\tvalue:%d\n",
            ie->time.tv_sec, ie->time.tv_usec, ie->type,
            ie->code, ie->value);
        break;
    case EV_REL:
        fprintf(stderr, "time:%ld.%06ld\ttype:%d\tcode:%d\tvalue:%d\n",
            ie->time.tv_sec, ie->time.tv_usec, ie->type,
            ie->code, ie->value);
        break;
    case EV_KEY:
        fprintf(stderr, "time:%ld.%06ld\ttype:%d\tcode:%d\tvalue:%d\n",
            ie->time.tv_sec, ie->time.tv_usec, ie->type,
            ie->code, ie->value);
        break;
    default:
        fprintf(stderr, "time:%ld.%06ld\ttype:%d\tcode:%d\tvalue:%d\n",
            ie->time.tv_sec, ie->time.tv_usec, ie->type,
            ie->code, ie->value);
        break;
    }
}

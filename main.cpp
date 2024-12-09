#include <QCoreApplication>
#include "roip_ed137.h"
int main(int argc, char *argv[])
{
    int softphoneID = 0;
    QCoreApplication a(argc, argv);
    for (int i = 0;i < argc; i++ ) {
        qDebug() << "QCoreApplication argv" << i << QString::fromUtf8(argv[i]);
        if (i == 1) softphoneID = QString::fromUtf8(argv[i]).toInt();
    }
    if (softphoneID == 0) softphoneID = 1;
    RoIP_ED137 mainWindows(softphoneID);
    return a.exec();
}

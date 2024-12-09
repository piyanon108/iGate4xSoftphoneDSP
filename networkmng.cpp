#include "networkmng.h"
#include "QDebug"
#include <QFile>
// #include <fstream>
#include <string>
// #include <iostream>
// #include <sstream>
using namespace std;
NetWorkMng::NetWorkMng(QObject *parent) : QObject(parent)
{
    getAddressProcess = new QProcess(this);
    gstLaunchProcess = new QProcess(this);
}
NetWorkMng::~NetWorkMng(){
    delete getAddressProcess;
}
bool NetWorkMng::get_gst_launch_process()
{
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    QString output;
    arguments << "-c" << "pgrep gst-launch-1.0";
    gstLaunchProcess->start(prog , arguments);
    gstLaunchProcess->waitForFinished(100);
    output = QString(gstLaunchProcess->readAll()).trimmed();
    arguments.clear();
    if (output != "") return  true;
    return  false;
}
void NetWorkMng::getIPAddress(QString netWorkCard){
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    arguments << "-c" << QString("ifconfig %1 | grep 'inet ' | awk '{print $2}' | sed 's/addr://'").arg(netWorkCard);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished(100);
    netWorkCardAddr = QString(getAddressProcess->readAll()).trimmed();
    arguments.clear();
}

QString NetWorkMng::getTimezone()
{
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    QString Timezone;
    arguments << "-c" << QString("ls -la /etc/localtime | grep '/usr/share/zoneinfo/' | awk '{print $11}'");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished(100);
    Timezone = QString(getAddressProcess->readAll()).trimmed();
    arguments.clear();
    qDebug() << "Timezone" << Timezone;
    return Timezone.replace("/usr/share/zoneinfo/","");
}

bool NetWorkMng::checkPhyEth(QString eth)
{
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    QString output;
    arguments << "-c" << QString("ifconfig %1 | grep '%1'").arg(eth);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished(100);
    output = QString(getAddressProcess->readAll()).trimmed();
    arguments.clear();
    return output.contains(eth);
}

QString NetWorkMng::getAddress(QString netWorkCard){
    QString networkInfo;
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    arguments << "-c" << QString("ifconfig %1 | grep 'inet ' | awk '{print $2}' | sed 's/addr://'").arg(netWorkCard);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished(100);
    netWorkCardAddr = QString(getAddressProcess->readAll()).trimmed();

    arguments.clear();
    arguments << "-c" << QString("ifconfig %1 | grep 'ether'  | awk '{print $2}'").arg(netWorkCard);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished(100);
    netWorkCardMac = QString(getAddressProcess->readAll()).trimmed();

    arguments.clear();
    arguments << "-c" << QString("ifconfig %1 | grep 'inet ' | awk '{print $4}' | sed 's/Mask://'").arg(netWorkCard);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished(100);
    netWorkCardMask = QString(getAddressProcess->readAll()).trimmed();

    arguments.clear();
    arguments << "-c" << QString("ip -4 route list 0/0  |  awk '/default/ { print $3 }'");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished(100);
    netWorkCardGW = QString(getAddressProcess->readAll()).trimmed();

    arguments.clear();
    arguments << "-c" << QString("cat /etc/resolv.conf  | grep nameserver | awk '{print $2}'");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished(100);
    netWorkCardDNS = QString(getAddressProcess->readAll()).trimmed();
    netWorkCardDNS = netWorkCardDNS.replace("\n",",");

    networkInfo = "IP Address," + netWorkCardAddr +"\tNetmask," +netWorkCardMask + "\tDefault Gateway," +netWorkCardGW +
            "\tDNS Server," +netWorkCardDNS + "\tMAC Address," +netWorkCardMac;

    arguments.clear();
    return networkInfo;
}

void NetWorkMng::resetNetwork()
{
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    qDebug() << "networking restarting";

    arguments << "-c" << QString("ifconfig eth0 down");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();

    arguments.clear();
    arguments << "-c" << QString("ifconfig eth0 up");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();

    arguments << "-c" << QString("ifconfig eth1 down");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();

    arguments.clear();
    arguments << "-c" << QString("ifconfig eth1 up");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();

    arguments.clear();
    arguments << "-c" << QString("systemctl daemon-reload");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();

    arguments.clear();
    arguments << "-c" << QString("/etc/init.d/dhcpcd restart");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();
    arguments.clear();
    for(int j=0; j < 10000; j++)
    {
        for(int i=0; i < 10000000; i++){}
    }
    qDebug() << "networking restarted";
}
void NetWorkMng::resetNetwork2(){
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    qDebug() << "networking restarting";
    arguments << "-c" << QString("ifconfig eth0 down");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();

    arguments.clear();
    arguments << "-c" << QString("ifconfig eth0 up");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();

//    arguments.clear();
//    arguments << "-c" << QString("systemctl daemon-reload");
//    getAddressProcess->start(prog , arguments);
//    getAddressProcess->waitForFinished();

    arguments.clear();
    arguments << "-c" << QString("/etc/init.d/networking restart");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();
    arguments.clear();
    qDebug() << "networking restarted";

    getAddress("eth0");
}
void NetWorkMng::resetNtp(){
    QString prog = "/bin/bash";//shell
    QStringList arguments;
    arguments << "-c" << QString("systemctl daemon-reload");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();

    arguments.clear();
    arguments << "-c" << QString("systemctl restart systemd-timesyncd");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();
    arguments.clear();

    for(int j=0; j < 10000; j++){
        for(int i=0; i < 10000; i++){}
    }

    arguments.clear();
    arguments << "-c" << QString("systemctl restart systemd-timesyncd");
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();
    arguments.clear();
    qDebug() << "systemctl restart systemd-timesyncd";
}
//int NetWorkMng::setNTPServer(QString ntpServer)
//{
//    ifstream filein("/etc/dhcpcd.conf"); //File to read from
//    ofstream fileout("/tmp/dhcpcd.conf"); //Temporary file

//    if(!filein || !fileout)
//    {
//        cout << "Error opening files!" << endl;
//        return 1;
//    }
//    string strTemp;
//       //bool found = false;
//    while ( std::getline(filein, strTemp) )
//    {
//        if(QString::fromStdString(strTemp).contains("option ntp_servers")){
//            strTemp = "option ntp_servers "+ntpServer.toStdString();
//        }
//        fileout << strTemp << '\n';
//    }
//    QString prog = "/bin/bash";//shell
//    QStringList arguments;
//    arguments << "-c" << QString("cp /tmp/dhcpcd.conf /etc/dhcpcd.conf");
//    getAddressProcess->start(prog , arguments);
//    getAddressProcess->waitForFinished();
//    arguments.clear();
//    return 0;
//}

void NetWorkMng::setNTPServer(QString ntpServer){
    QString filename = "/etc/systemd/timesyncd.conf";
    QString data;
    if (ntpServer != "")
        data = QString("[Time]\n"
                   "NTP=%1\n"
                   "FallbackNTP=0.debian.pool.ntp.org 1.debian.pool.ntp.org 2.debian.pool.ntp.org 3.debian.pool.ntp.org\n").arg(ntpServer);
    else
        data = QString("[Time]\n"
                   "#NTP=%1\n"
                   "#FallbackNTP=0.debian.pool.ntp.org 1.debian.pool.ntp.org 2.debian.pool.ntp.org 3.debian.pool.ntp.org\n").arg(ntpServer);

    QByteArray dataAyyay(data.toLocal8Bit());
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << dataAyyay;
    file.close();
    system("timedatectl set-ntp 1");
}
void NetWorkMng::initPhyNetworkConfig(QString ipaddr,uint8_t netmask,QString gateway,QString dns1,QString dns2, QString phyNetworkName)
{
    QString data;
    if (netmask <= 0) netmask = 24;
    if(((ipaddr!="0.0.0.0") && (netmask > 0) && (gateway!="") && (dns1!="") && (dns2!="")) ||
            ((ipaddr!="0.0.0.0") && (netmask > 0) && (gateway!="") && (dns1=="") && (dns2!="")) ||
            ((ipaddr!="0.0.0.0") && (netmask > 0) && (gateway!="") && (dns1!="") && (dns2==""))){
        data = QString("# Static IP configuration:\n"
                           "interface %6\n"
                           "    static ip_address=%1/%2\n"
                           "    static routers=%3\n"
                           "    static domain_name_servers=%4 %5\n").arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2).arg(phyNetworkName);
    }
    else if((ipaddr!="0.0.0.0") && (netmask > 0) && (gateway!="") && (dns1=="") && (dns2=="")){
        data = QString("# Static IP configuration:\n"
                           "interface %6\n"
                           "    static ip_address=%1/%2\n"
                           "    static routers=%3\n"
                           "#   static domain_name_servers=%4 %5\n").arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2).arg(phyNetworkName);
    }
    else if((ipaddr!="0.0.0.0") && (netmask > 0) && (gateway=="")){
        data = QString("# Static IP configuration:\n"
                           "interface %6\n"
                           "    static ip_address=%1/%2\n"
                           "#   static routers=%3\n"
                           "#   static domain_name_servers=%4 %5\n").arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2).arg(phyNetworkName);
    }

    if (phyNetworkName == "eth0")
        eth0Config = data;
    else if (phyNetworkName == "eth1")
        eth1Config = data;
}

void NetWorkMng::setStaticIpAddr(QString ipaddr,uint8_t netmask,QString gateway,QString dns1,QString dns2, QString phyNetworkName)
{
    QString filename = "/etc/dhcpcd.conf";
    QString data;
    QString fileHeader = "hostname\n"
                         "clientid\n"
                         "persistent\n"
                         "option rapid_commit\n"
                         "option domain_name_servers, domain_name, domain_search, host_name\n"
                         "option classless_static_routes\n"
                         "option ntp_servers\n"
                         "option interface_mtu\n"
                         "require dhcp_server_identifier\n"
                         "slaac private\n"
                         "\n";


    if (netmask <= 0) netmask = 24;
    if(((ipaddr!="0.0.0.0") && (netmask > 0) && (gateway!="") && (dns1!="") && (dns2!="")) ||
            ((ipaddr!="0.0.0.0") && (netmask > 0) && (gateway!="") && (dns1=="") && (dns2!="")) ||
            ((ipaddr!="0.0.0.0") && (netmask > 0) && (gateway!="") && (dns1!="") && (dns2==""))){
        data = QString("# Static IP configuration:\n"
                           "interface %6\n"
                           "    static ip_address=%1/%2\n"
                           "    static routers=%3\n"
                           "    static domain_name_servers=%4 %5\n").arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2).arg(phyNetworkName);
    }
    else if((ipaddr!="0.0.0.0") && (netmask > 0) && (gateway!="") && (dns1=="") && (dns2=="")){
        data = QString("# Static IP configuration:\n"
                           "interface %6\n"
                           "    static ip_address=%1/%2\n"
                           "    static routers=%3\n"
                           "#   static domain_name_servers=%4 %5\n").arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2).arg(phyNetworkName);
    }
    else if((ipaddr!="0.0.0.0") && (netmask > 0) && (gateway=="")){
        data = QString("# Static IP configuration:\n"
                           "interface %6\n"
                           "    static ip_address=%1/%2\n"
                           "#   static routers=%3\n"
                           "#   static domain_name_servers=%4 %5\n").arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2).arg(phyNetworkName);
    }

    if (phyNetworkName == "eth0"){
        eth0Config = data;
    }
    else if (phyNetworkName == "eth1"){
        eth1Config = data;
    }

    QString prog = "/bin/bash";//shell
    QStringList arguments;
    arguments << "-c" << QString("ifconfig %1 %2 netmask %3 down").arg(phyNetworkName).arg(ipaddr).arg(netmask);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();

    arguments.clear();
    arguments << "-c" << QString("ifconfig %1 %2 netmask %3 up").arg(phyNetworkName).arg(ipaddr).arg(netmask);
    getAddressProcess->start(prog , arguments);
    getAddressProcess->waitForFinished();

    QString dhcpcdConf = fileHeader + eth0Config + eth1Config;

    QByteArray dataAyyay(dhcpcdConf.toLocal8Bit());
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << dataAyyay;
    file.close();
}
void NetWorkMng::setDHCPIpAddr(QString phyNetworkName){
    QString filename = "/etc/dhcpcd.conf";
    QString data = QString("hostname\n"
                           "clientid\n"
                           "persistent\n"
                           "option rapid_commit\n"
                           "option domain_name_servers, domain_name, domain_search, host_name\n"
                           "option classless_static_routes\n"
                           "option ntp_servers\n"
                           "option interface_mtu\n"
                           "require dhcp_server_identifier\n"
                           "slaac private\n"
                           "\n");

    if (phyNetworkName == "eth0")
        eth0Config = "";
    else if (phyNetworkName == "eth1")
        eth1Config = "";

    QString dhcpcdConf = data + eth0Config + eth1Config;

    QByteArray dataAyyay(dhcpcdConf.toLocal8Bit());
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << dataAyyay;
    file.close();
}
void NetWorkMng::setDHCPIpAddr2(QString phyNetworkName)
{
    if (phyNetworkName != "eth0") return;
    QString filename = "/etc/network/interfaces.d/eth0";
    QString data = QString("auto eth0\n"
                           "iface eth0 inet dhcp");
    QByteArray dataAyyay(data.toLocal8Bit());
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << dataAyyay;
    file.close();
    emit restartNetwork();
}

void NetWorkMng::setStaticIpAddr2(QString ipaddr,QString netmask,QString gateway,QString dns1,QString dns2, QString phyNetworkName){
    if (phyNetworkName != "eth0") return;
    QString filename = "/etc/network/interfaces.d/eth0";
    QString data;

    if (netmask == "0.0.0.0") netmask = "255.255.255.0";
    if(((ipaddr!="0.0.0.0") &&  (gateway!="0.0.0.0") && (dns1!="0.0.0.0") && (dns2!="0.0.0.0")))
    {
        data = QString("auto eth0\n"
                       "iface eth0 inet static\n"
                       "address %1\n"
                       "netmask %2\n"
                       "gateway %3\n"
                       "dns-nameservers %4\n"
                       "dns-nameservers %5\n"
                       ).arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2);
        QString commanifconfig = QString("ifconfig eth0 %1").arg(ipaddr);
        system(commanifconfig.toStdString().c_str());
    }
    else if(((ipaddr!="0.0.0.0") &&  (gateway!="0.0.0.0") && (dns1!="0.0.0.0") && (dns2=="0.0.0.0")))
    {
        data = QString("auto eth0\n"
                       "iface eth0 inet static\n"
                       "address %1\n"
                       "netmask %2\n"
                       "gateway %3\n"
                       "dns-nameservers %4\n"
                       "# dns-nameservers %5\n"
                       ).arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2);
        QString commanifconfig = QString("ifconfig eth0 %1").arg(ipaddr);
        system(commanifconfig.toStdString().c_str());
    }
    else if(((ipaddr!="0.0.0.0") &&  (gateway!="0.0.0.0") && (dns1=="0.0.0.0") && (dns2=="0.0.0.0")))
    {
        data = QString("auto eth0\n"
                       "iface eth0 inet static\n"
                       "address %1\n"
                       "netmask %2\n"
                       "gateway %3\n"
                       "# dns-nameservers %4\n"
                       "# dns-nameservers %5\n"
                       ).arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2);
        QString commanifconfig = QString("ifconfig eth0 %1").arg(ipaddr);
        system(commanifconfig.toStdString().c_str());
    }
    else if(((ipaddr!="0.0.0.0") &&  (gateway!="0.0.0.0") && (dns1=="0.0.0.0") && (dns2!="0.0.0.0")))
    {
        data = QString("auto eth0\n"
                       "iface eth0 inet static\n"
                       "address %1\n"
                       "netmask %2\n"
                       "gateway %3\n"
                       "# dns-nameservers %4\n"
                       "dns-nameservers %5\n"
                       ).arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2);
        QString commanifconfig = QString("ifconfig eth0 %1").arg(ipaddr);
        system(commanifconfig.toStdString().c_str());
    }
    else if((ipaddr!="0.0.0.0") && (gateway!="0.0.0.0") && (dns1=="0.0.0.0") && (dns2=="0.0.0.0")){
        data = QString("auto eth0\n"
                       "iface eth0 inet static\n"
                       "address %1\n"
                       "netmask %2\n"
                       "gateway %3\n"
                       "# dns-nameservers %4\n"
                       "# dns-nameservers %5\n"
                       ).arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2);
        QString commanifconfig = QString("ifconfig eth0 %1").arg(ipaddr);
        system(commanifconfig.toStdString().c_str());
    }
    else if((ipaddr!="0.0.0.0") && (gateway=="0.0.0.0")){
        data = QString("auto eth0\n"
                       "iface eth0 inet static\n"
                       "address %1\n"
                       "netmask %2\n"
                       "# gateway %3\n"
                       "# dns-nameservers %4\n"
                       "# dns-nameservers %5\n"
                       ).arg(ipaddr).arg(netmask).arg(gateway).arg(dns1).arg(dns2);
        QString commanifconfig = QString("ifconfig eth0 %1").arg(ipaddr);
        system(commanifconfig.toStdString().c_str());
    }
    QByteArray dataAyyay(data.toLocal8Bit());
    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << dataAyyay;
    file.close();

    emit restartNetwork();
}
QString NetWorkMng::readLine(QString fileName)
{
    QFile inputFile(fileName);
    inputFile.open(QIODevice::ReadOnly);
    if (!inputFile.isOpen())
        return "";

    QTextStream stream(&inputFile);
    QString line = stream.readLine();
    inputFile.close();
//    qDebug() << line;
    return line.trimmed();
}
QString NetWorkMng::getUPTime()
{
    system("uptime -p > /etc/uptime");
    QString fileName = QString("/etc/uptime");
    return readLine(fileName);
}

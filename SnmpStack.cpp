#include "SnmpStack.h"
#include <QFile>
#include "QProcess"
#include <QDebug>
SnmpStack* SnmpStack::ins = NULL;
long session_timeout = 100000;
static const char* getRadioName      = "1.3.6.1.4.1.2363.6.1.1.2.0";
static const char* getRadioName2     = "1.3.6.1.4.1.2363.1.1.1.2.0";



void int64ToChar(char a[], int64_t n) {
  memcpy(a, &n, 8);
}

int64_t charTo64bitNum(char a[]) {
  int64_t n = 0;
  memcpy(&n, a, 8);
  return n;
}

namespace
{
    static bool initialized = false;
}

SnmpStack::SnmpStack()
{
    pdu = new snmp_pdu;
    response = new snmp_pdu;
    vars = new variable_list;
    sess_handle = new snmp_session;
    session = new snmp_session;
    system("mkdir -p /opt/iGate4x/snmp");
}

SnmpStack::~SnmpStack()
{
    delete pdu;
    delete response;
    delete vars;
    delete sess_handle;
}

//SnmpStack* SnmpStack::getInstance()
//{
//	if (ins == NULL)
//	{
//        ins = new SnmpStack();
//	}
//	return ins;
//}

bool SnmpStack::init()
{
    if (!initialized)
    {
        init_snmp("snmpapp");
        initialized = true;
        return initialized;
    }

    return false;
}

bool SnmpStack::uninit()
{
    if(initialized)
    {
        snmp_shutdown("snmpapp");
        initialized = false;
        return initialized;
    }

    return false;
}

void SnmpStack::setup_snmp_session(char* host)
{

    init_snmp("poller");
    snmp_sess_init( session );
    session->version = SNMP_VERSION_2c;
    session->community = (u_char*)("public");
    session->community_len = strlen((char*)session->community);
    session->peername = host;
    sess_handle = snmp_open(session);
}



bool SnmpStack::setsnmp_int(std::string ip,oid* oidname,const char* value)
{
    setup_snmp_session((char*)ip.c_str());
    sess_handle->timeout = session_timeout;
    snmp_set_int(sess_handle,oidname,value);
    snmp_close(sess_handle);
    return true;
}

bool SnmpStack::setsnmp_string(std::string ip,oid* oidname,const char* value)
{
    setup_snmp_session((char*)ip.c_str());
    sess_handle->timeout = session_timeout;
    snmp_set_string(sess_handle,oidname,value);
    snmp_close(sess_handle);
    return true;
}

unsigned long SnmpStack::getsnmp_int(std::string ip,const char* oidname)
{
    setup_snmp_session((char*)ip.c_str());
    sess_handle->timeout = session_timeout;
    int64_t val64 = (int64_t) snmp_get_int(sess_handle,oidname);
//    printf("getsnmp_int : %d\n",val64);
    snmp_close(sess_handle);
    return val64;
}

//int SnmpStack::getsnmp_int_new(std::string ip,const char* oidname)
//{
//    QString prog = "/bin/bash";//shell
//    QStringList arguments;
//    QString output;
//    QProcess progetsnmp;
//    int val64 = 0;
//    arguments << "-c" << QString("snmpwalk -v 2c -t 0.05 -c public %1 %2").arg(QString::fromStdString(ip)).arg(QString::fromStdString(oidname));
//    progetsnmp.start(prog , arguments);
//    progetsnmp.waitForFinished();
//    output = QString(progetsnmp.readAll()).trimmed();
//    qDebug() << output;
//    arguments.clear();
//    if (!(output.contains("No Such Object"))){
//        QStringList dataList = output.split(": ");
//        QString data;
//        if(dataList.size() >= 2){
//            data = dataList.at(1);
//            val64 = data.toInt();
//        }
//    }
//    return val64;
//}

//QString SnmpStack::getsnmp_string_new(std::string ip,const char* oidname)
//{
//    QString prog = "/bin/bash";//shell
//    QStringList arguments;
//    QString output;
//    QProcess progetsnmp;
//    QString data("No Such Object");
//    arguments << "-c" << QString("snmpwalk -v 2c -t 0.05 -c public %1 %2").arg(QString::fromStdString(ip)).arg(QString::fromStdString(oidname));
//    qDebug() << arguments;
//    progetsnmp.start(prog , arguments);
//    progetsnmp.waitForFinished();
//    output = QString(progetsnmp.readAll()).trimmed();
//    arguments.clear();
//    if (!(output.contains("No Such Object"))){
//        QStringList dataList = output.split(": ");
//        if(dataList.size() >= 2){
//            data = QString(dataList.at(1)).replace("\"","");
//        }
//    }
//    return data;
//}

QString SnmpStack::getsnmp_string_new(std::string ip, const char* oidname, int id)
{
    QString stringout = "";
    QString data = "";
    QString arguments = QString("snmpwalk -v 2c -t 0.1 -L f /tmp/log.text -c public %1 %2 > /opt/iGate4x/snmp/stringout%3_%4.txt").arg(QString::fromStdString(ip)).arg(QString::fromStdString(oidname)).arg(QString::fromStdString(ip)).arg(id);
    system(arguments.toStdString().c_str());
    stringout = readLine(QString("/opt/iGate4x/snmp/stringout%1_%2.txt").arg(QString::fromStdString(ip)).arg(id));
    if (!(stringout.contains("No Such Object"))){
        QStringList dataList = stringout.split(": ");
        if(dataList.size() >= 2){
            data = QString(dataList.at(1)).replace("\"","");
        }
    }
    return data;
}

int SnmpStack::getsnmp_int_new(std::string ip,const char* oidname,int id)
{
    int val64 = 0;
    QString lineOut = "";
    QString arguments = QString("snmpwalk -v 2c -t 0.1 -L f /tmp/log.text -c public %1 %2 > /opt/iGate4x/snmp/intout%3_%4.txt").arg(QString::fromStdString(ip)).arg(QString::fromStdString(oidname)).arg(QString::fromStdString(ip)).arg(id);
//    qDebug() << "getsnmp_int_new: arguments =" << arguments;
    system(arguments.toStdString().c_str());
    lineOut = readLine(QString("/opt/iGate4x/snmp/intout%1_%2.txt").arg(QString::fromStdString(ip)).arg(id));
    if (!(lineOut.contains("No Such Object")))
    {
        QStringList dataList = lineOut.split(": ");
        QString data;
        if(dataList.size() >= 2){
            data = dataList.at(1);
            val64 = data.toInt();
        }
    }
    return val64;
}

QString SnmpStack::readLine(QString fileName)
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
std::string  SnmpStack::getsnmp_string(std::string ip,const char* oidname)
{
    std::string mesg;
    setup_snmp_session((char*)ip.c_str());
    sess_handle->timeout = session_timeout;
    mesg = snmp_get_string(sess_handle,oidname);
//    printf("getsnmp_int : %s\n",mesg.c_str());
    snmp_close(sess_handle);

    return mesg;
}

unsigned long SnmpStack::snmp_get_int(struct snmp_session *sess_handle, const char* oidname)
{
    int status;
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    read_objid(oidname, id_oid, &id_len);
    snmp_add_null_var(pdu, id_oid, id_len);

    status = snmp_synch_response(sess_handle, pdu, &response);
    if (status == STAT_SUCCESS) {
       if(response->variables->val_len == 0)  return 0;
       for(vars = response->variables; vars; vars = vars->next_variable)
       {
           //print_value(vars->name, vars->name_length, vars);
           int64_t val64 = (int64_t) vars->val.counter64->high;
           return val64;
       }
       snmp_free_pdu(response);
    }

    return 0;
}
std::string SnmpStack::snmp_get_string(struct snmp_session *sess_handle, const char* oidname)
{
    int status;
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    read_objid(oidname, id_oid, &id_len);
    snmp_add_null_var(pdu, id_oid, id_len);

    status = snmp_synch_response(sess_handle, pdu, &response);
    if (status == STAT_SUCCESS) {
       if(response->variables->val_len == 0)  return std::string();
       for(vars = response->variables; vars; vars = vars->next_variable)
       {
           int64_t val64 = (int64_t) vars->val.counter64->high;
           //*(int64_t *)mesg = val64;
           memcpy(mesg, &val64, 8);

           sprintf(buff,"%s",mesg);
           return buff;
       }
       snmp_free_pdu(response);
    }

    return std::string();
}

bool SnmpStack::snmp_set_int(struct snmp_session *sess_handle, oid* oidname, const char* value){
    //snmpset -v1 -c public 10.1.10.79 1.3.6.1.4.1.22154.3.1.2.3.4.0 i 119600
    int length=0;
    while (oidname[length] != '\0')
    {
       length++;
    }
    size_t id_len =length+1;

    pdu = snmp_pdu_create(SNMP_MSG_SET);

    snmp_add_var (pdu, oidname, id_len, 'i', value);
    pdu->trap_type = 0;
    pdu->specific_type=0;

    int status = snmp_send(sess_handle, pdu) == 0;
    if (status == STAT_SUCCESS)
        return true;

    return false;
}
bool SnmpStack::snmp_set_string(struct snmp_session *sess_handle, oid* oidname, const char* value){
    int length=0;
    while (oidname[length] != '\0')
    {
       length++;
    }
    size_t id_len =length+1;

    pdu = snmp_pdu_create(SNMP_MSG_SET);

    snmp_add_var (pdu, oidname, id_len, 'x', value);
    pdu->trap_type = 0;
    pdu->specific_type=0;

    int status = snmp_send(sess_handle, pdu) == 0;
    if (status == STAT_SUCCESS)
        return true;

    return false;
}

bool SnmpStack::snmp_checkConnection(std::string ip)
{
    setup_snmp_session((char*)ip.c_str());
    sess_handle->timeout=1000;

        struct snmp_pdu *pdu;
        struct snmp_pdu *response;

        oid id_oid[MAX_OID_LEN];
        size_t id_len = MAX_OID_LEN;
        int status;

        pdu = snmp_pdu_create(SNMP_MSG_GET);
        read_objid(getRadioName, id_oid, &id_len);
        snmp_add_null_var(pdu, id_oid, id_len);

        status = snmp_synch_response(sess_handle, pdu, &response);

        if (status == STAT_SUCCESS) {
           snmp_free_pdu(response);
           snmp_close(sess_handle);
           return true;
        }

    snmp_free_pdu(response);
    snmp_close(sess_handle);
    return false;
}


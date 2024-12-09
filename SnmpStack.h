#include <stdlib.h>
#include <iostream>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <QString>
#include <QStringList>
#include <QProcess>
// sudo apt-get install libsnmp-dev

class SnmpStack
{
public:

    //
    // Retrives singleton instace of SnmpStack class
    //
    // @return      SnmpStack instance
    //
    SnmpStack();
//    static SnmpStack* getInstance();
    ~SnmpStack();

private:
    //
    // Hidden constructor
    //


    //
    // Hidden destructor
    //

    void setup_snmp_session(char* host);
    unsigned long snmp_get_int(struct snmp_session *sess_handle, const char* oidname);
    std::string snmp_get_string(struct snmp_session *sess_handle, const char* oidname);
    bool snmp_set_int(struct snmp_session *sess_handle, oid* oidname, const char* value);
    bool snmp_set_string(struct snmp_session *sess_handle, oid* oidname, const char* value);

    struct snmp_pdu *pdu;
    struct snmp_pdu *response;
    struct variable_list *vars;
    struct snmp_session *session;

    oid id_oid[MAX_OID_LEN];
    size_t id_len = MAX_OID_LEN;
    char buff[MAX_OID_LEN];
    char mesg[MAX_OID_LEN];

    struct snmp_session   *sess_handle;

    QString readLine(QString fileName);
public:
    unsigned long getsnmp_int(std::string ip,const char* oidname);
    int getsnmp_int_new(std::string ip, const char* oidname, int id);
    std::string getsnmp_string(std::string ip,const char* oidname);
    bool setsnmp_int(std::string ip,oid* oidname,const char* value);
    bool setsnmp_string(std::string ip,oid* oidname,const char* value);
    bool snmp_checkConnection(std::string ip);
    QString getsnmp_string_new(std::string ip,const char* oidname,int id);

    bool init();
    bool uninit();

private:


private:
    //
    // static instance of SnmpStack class
    //
    static SnmpStack* ins;
//    QProcess* getSNMPProcess;
};



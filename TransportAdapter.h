
#ifndef __PJMEDIA_TRANSPORT_ADAPTER_H__
#define __PJMEDIA_TRANSPORT_ADAPTER_H__

#include <pjsua.h>
#include <pjmedia/transport.h>
#include <pjmedia/endpoint.h>
#include <pj/assert.h>
#include <pj/pool.h>
#include <pjmedia.h>
#include <pjmedia/stream.h>
#include <arpa/inet.h>
#include "ed137_rtp.h"
#include <QString>
#include <QDateTime>
PJ_BEGIN_DECL

PJ_DEF(pj_status_t) pjmedia_custom_tp_adapter_create( pjmedia_endpt *endpt,
                           const char *name,
                           pjmedia_transport *transport,
                           pj_bool_t del_base,pj_bool_t radiocall,pj_bool_t callIn, const char *calltype, pjsua_call_id callId,
                           pjmedia_transport **p_tp,
                           const char *callIndex,const char *trxmode, int keepAlivePeroid ,pj_bool_t connToRadio, pj_bool_t pttWithPayload);

pj_status_t decodeRtp (void* pkt, custom_rtp_hdr **hdr);
pj_uint32_t get_ed137_value(pjmedia_transport *tp);
pj_status_t setAdapterPtt (pjmedia_transport *tp, bool pttval, int priority, int userRec);
pj_status_t setTxRxSlaveEnable (pjmedia_transport *tp,pj_bool_t rx, pj_bool_t tx);
pj_status_t setAdapterRadioModeAndType (pjmedia_transport *tp,char const* type,char const* txrxmode);
//pj_status_t setAdapterQslOn (pjmedia_transport *tp,bool sqlval,int priority);
pj_status_t setAdapterQslOn (pjmedia_transport *tp,bool sqlval,int priority, pj_uint32_t bssi);
pj_status_t setAdapterPttId (pjmedia_transport *tp,int pttid);
//pj_status_t setTransportSlaveEnable (pjmedia_transport *tp,bool rx, bool tx);
pj_status_t setcallRecorder (pjmedia_transport *tp,bool val);
pj_status_t setCallType (pjmedia_transport *tp,char const* calltype);

long long getR2SStatus(pjmedia_transport *tp);
void sendR2SStatus(pjmedia_transport *tp);

struct tp_adapter
{
    pjmedia_transport  base;
    pj_bool_t		   del_base;
    pj_pool_t		  *pool;
    void		      *stream_user_data;
    void	         (*stream_rtp_cb)(void *user_data, void *pkt, pj_ssize_t);
    void	         (*stream_rtcp_cb)(void *user_data, void *pkt, pj_ssize_t);
    void                *stream_ref;
    pjmedia_transport *slave_tp;
    pj_bool_t          radiostatus;
    qint16             rtpFalse;
    pj_bool_t          pttstatus;
    pj_bool_t          sqlstatus;
    pjsua_call_id      callID;
    pj_uint8_t         pttpriority;
    pj_uint8_t         sqlpriority;
    pj_uint8_t         ed137_bssi;
    pj_uint8_t         rssiIndex;
    pj_uint8_t         pttid;
    pj_uint32_t        ed137_value;
    pj_uint32_t        payloadsize;
    char               calltype[64];

    pj_uint8_t         pkt_buff[256];
    pj_size_t          bufSize;
    pj_uint8_t         payload_buff[256];
    pj_size_t          payload_bufSize;

    pj_uint8_t         send_pkt_buff[256];
    pj_uint8_t         tmp_payload_buf[256];
    pj_size_t          send_bufSize;
    pj_uint8_t         send_payload_buff[256];
    pj_size_t          send_payload_bufSize;

    char               callIndex[64];
    char               trxmode[64];
    long long          r2sPacket = QDateTime::currentMSecsSinceEpoch();
    int                keepAlivePeroid = 200;
    long long          r2sSendtime = QDateTime::currentMSecsSinceEpoch();
    pj_bool_t          callIn;
    pj_bool_t          connToRadio;
    pj_bool_t          pttWithPayload;
    pj_bool_t          firstR2SPacket;
    pj_bool_t          callRecorder = false;
    int                packetCnt;
    pj_bool_t          rxSlaveEnable = false;
    pj_bool_t          txSlaveEnable = false;
    pj_bool_t          rxSlaveEnableChanged = false;
    pj_bool_t          txSlaveEnableChanged = false;
    int                trxSlaveEnableChangedCount = 0;
    pj_bool_t          rtpAudio = false;

};

PJ_END_DECL

#endif

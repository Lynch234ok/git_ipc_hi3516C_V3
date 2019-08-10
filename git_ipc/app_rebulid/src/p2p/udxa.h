#ifndef JAP2P_UDXA_H
#define JAP2P_UDXA_H

#include "udx/udxos.h"
#include "udx/FastUdx.h"



/*esee status define*/
#define ESTATUS_NONE            0
#define ESTATUS_AUTH			1
#define ESTATUS_AUTH_ACK		2
#define ESTATUS_HEARTBEAT		3
#define ESTATUS_HEARTBEAT_ACK   4

/*esee filter result*/
#define EFILTER_RAW 0
#define EFILTER_HOLEREQ 1
#define EFILTER_HOLE_CHANGE 2  // recved a hole package from client,don't need send hole package anymore
#define EFILTER_TURNREQ 3



class IUdxaHooks
{
public:
	virtual void OnUdxaConnect(IUdxTcp *pTcp, int erro){};
	virtual void OnUdxaRecv(IUdxTcp *pTcp, BYTE *pData, int len){};
	virtual void OnUdxaMsgRecv(IUdxTcp * pTcp,BYTE* pData,int len){};
	virtual void OnUdxaDisconnect(IUdxTcp * pTcp){};	
};


class CUdxa: public IUdxUnkownPackSink, public IUdxTcpSink
{
public:
	CUdxa(){};
	virtual ~CUdxa(){};
public:
	//IUdxUnkownPackSink
	virtual void OnUnkownData(SOCKADDR *pAddr, BYTE *pData, long len);
	virtual void OnThreadExit(){};

	//IUdxTcpSink
	virtual void OnStreamConnect(IUdxTcp *pTcp, int erro);
	virtual void OnStreamRead(IUdxTcp *pTcp, BYTE *pData,int len);
	virtual void OnStreamMsgRead(IUdxTcp * pTcp,BYTE* pData,int len);
	virtual void OnStreamBroken(IUdxTcp *pTcp);

	//set hooks
	void SetUdxaHooks(IUdxaHooks *pUdxaHooks){
		m_pHooks = pUdxaHooks;
	};

public:
	IFastUdx *m_pUdx;
	IUdxaHooks *m_pHooks;
	
	LListNode *m_proto_llist;//put the parsed esee_proto in this linkedlist
	UINT32 m_status;//esee status for alivethread's how to work;
	bool m_bDeviceOnline; //specify the status of the device, update in alivethread

	CHAR m_sn[64];//device serial No.
	CHAR m_eseeid[64];//eseeid,svr echoed which based on the sn sent 

	UINT32 m_svrip;//eseesvr ip
	UINT16 m_svrport;//eseesvr port
	UINT32 m_externip;	//extern ip
	UINT16 m_externport;//extern port

	//temp store the holeinfo
	void  *m_pHole;
	UINT32 m_hole_random;
	UINT32 m_hole_clientip;
	UINT16 m_hole_clientport;

	//temp store the turninfo from esee xml
	void *m_pTurn;
	UINT32 m_turn_svrip;
	UINT16 m_turn_svrport;
	UINT32 m_turn_clientip;
	UINT16 m_turn_clientport;
	CHAR m_turn_channelname[128];
};

typedef struct {
	bool m_turn_mark;
}UdxaTurnMark;


//udxa util
SINT32 UdxaFilter(CUdxa *pUdxa);
BOOL UdxaSend(IUdxTcp *pTcp, const void *buf, UINT32 len);
SINT32 UdxaSendto(IFastUdx *pUdx, const void* buf, UINT32 len, UINT32 ip, UINT16 port);
//esee proto pack send
SINT32 UdxaSendAuth(const CHAR *sn, CUdxa *pUdxa);
SINT32 UdxaSendHeartbeat(const CHAR *eseeseed, CUdxa *pUdxa);
SINT32 UdxaSendHolereqConfirm(UINT32 clientip, UINT16 clientport, UINT32 random, CUdxa *pUdxa);
SINT32 UdxaSendHoleto(UINT32 clientip, UINT16 clientport,UINT32 random, UINT32 ttl, CUdxa *pUdxa);
SINT32 UdxSendTurnready(UINT32 svrip, UINT16 svrport, CUdxa *pUdxa);


//typedef void *Udx_Handle;
CUdxa *UdxaInit(const char *sn, IUdxaHooks *pHooks);
void UdxaDestroy(void);



#endif// end of udxa_base.h


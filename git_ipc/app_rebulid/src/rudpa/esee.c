/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:		esee.c
 * Describle:API that communicate to the esee platform 
 * History: 
 * Last modified:	2013-03-19 09:22
 =============================================================*/
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <resolv.h>

#include "rudpa_common.h"
#include "rudp_session.h"

#include "turn.h"
#include "esee.h"
#include "esee_protocol.h"
#include "Aes.h"
#include "base64.h"
#include "ezxml.h"
#include "union_protocol.h"
#include <sys/prctl.h>

extern char  gESEE_MAC[64];
extern char *gESEE_PLAT_IP;
extern uint32_t gESEE_PLAT_PORT;

static unsigned char snKey[16] ={
	0x55, 0x93, 0xe3, 0x7d, 0xa5, 0x64, 0x1a,0x85,
	0xb6, 0xc9, 0xac, 0x24, 0xc3, 0x2c,0xbf, 0x2c
	};

int EseeSendPkt(Esee *thiz,unsigned short nChannel, char* cmd,struct sockaddr_in *_2addr)
{
	char tick_buf[10] = {0};
	sprintf(tick_buf,"%u",(uint32_t)time(NULL));
	char *Pkt = WriteProtocol(cmd,  tick_buf);

	_RUDPA_DEBUG(" %s===%s\n",cmd,Pkt);

	if(NULL == thiz->m_l)
	{
		_RUDPA_ERROR("the listener handle is no ready\t msg send failed\n");
		return 0;
	}
	 
	int ret = direct_send(thiz->m_l,Pkt,strlen(Pkt),_2addr);	 

	if (-1 == ret) {
		perror("Esee send pkt error");
		return 0;
	}
	FREE(Pkt);
	usleep(1000);
	return !0;
}


int EseeSendTo(Esee *thiz,EseeCmd cmd,struct sockaddr_in *_2addr)
{
	int ret;
	switch(cmd)
	{
		case STurnAuth:
			{
				ret = EseeSendPkt(thiz,0,"STurnAuth",_2addr);
			}
			break;
		case SHeartbeat:
			{
				ret = EseeSendPkt(thiz,0,"SHeartbeat",_2addr);
			}
			break;
		case STurnReady:
			{
				ret = EseeSendPkt(thiz,0,"STurnReady",_2addr);
			}
			break;
		case SConfirmTraversal:
			{
				ret = EseeSendPkt(thiz,0,"SConfirmTraversal",_2addr);
			}
			break;
		case SHoleClient:
			{
				ret = EseeSendPkt(thiz,0,"SHoleClient",_2addr);				
			}
			break;
		default:
			break;
	}
	return ret;
}


void *EseeThread(void *pParam)
{

	Esee *thiz = (Esee*)pParam;
	time_t curTime ;
	time_t reloginTime;
	prctl(PR_SET_NAME, "EseeThread");

	for(;;)
	{
		usleep(10 * 1000);
		if(0 == thiz->m_EseePlatInfoValid){			
			continue;
		}		
		
		switch(thiz->m_Status)
		{
			case ES_IDLE:
				{	
					thiz->m_Status = ES_TURN_AUTH;
					thiz->m_TurnAuthReCnt = 0;
				}
				break;
			case ES_TURN_AUTH: // turn_auth
				{
					thiz->b_turnAuth = false;
					EseeSendTo(thiz,STurnAuth,&thiz->esee_server);
					thiz->m_Status = ES_TURN_AUTH_ACK;
					curTime = time(NULL);
				}
				break;					
			case ES_TURN_AUTH_ACK: // wait for turn_auth ack
				{
					if(thiz->b_turnAuth 
						|| (time(NULL) - curTime >= ESEE_TURN_AUTH_TIMEOUT))
					{
						if(thiz->b_turnAuth)
						{	
							reloginTime = time(NULL);
							thiz->m_HeartbeatReCnt = 0;
							thiz->m_Status = ES_HEARTBEAT;							
						}else{
							thiz->m_TurnAuthReCnt++;
							thiz->m_Status = ES_TURN_AUTH;
						}
						
						if(ESEE_TURN_AUTH_RE_CNT == thiz->m_TurnAuthReCnt)
						{
							thiz->m_TurnAuthReCnt = 0;
							thiz->m_Status = ES_IDLE;
							_RUDPA_ERROR("Rudpa login failed\n");
						}
					}
				}
				break;
			case ES_HEARTBEAT://heartbeat
				{	
					thiz->b_heartbeat = false;
					EseeSendTo(thiz,SHeartbeat,&thiz->esee_server);
					thiz->m_Status = ES_HEARTBEAT_ACK;
					curTime = time(NULL);	
				}
				break;		
			case ES_HEARTBEAT_ACK://heart beat ack
				{					
					if(thiz->b_heartbeat 
						|| (time(NULL) - curTime >= ESEE_HEARTBEAT_TIMEOUT))
					{
						if(thiz->b_heartbeat)
						{
							/*get a valid heartbeat ack,sleep a while,resent heartbeat*/			
							if(0 == thiz->m_externIp[0] && 0 == thiz->m_externPort[0])
							{
								sprintf(thiz->m_externIp,"%s",TagTable[ESEE_TAG_EXTERIP].TagTxt);
								sprintf(thiz->m_externPort,"%s",TagTable[ESEE_TAG_EXTERPORT].TagTxt);
							}
							else if(0 != strcmp(thiz->m_externIp,TagTable[ESEE_TAG_EXTERIP].TagTxt)
								|| 0 != strcmp(thiz->m_externPort,TagTable[ESEE_TAG_EXTERPORT].TagTxt))
							{
								sprintf(thiz->m_externIp,"%s",TagTable[ESEE_TAG_EXTERIP].TagTxt);
								sprintf(thiz->m_externPort,"%s",TagTable[ESEE_TAG_EXTERPORT].TagTxt);
								thiz->m_Status = ES_TURN_AUTH;
								_RUDPA_ERROR("Extern_addr changed,relogin ESEE\n");
								break;
							}

							//read the status code  0 offline 1 online
							if(0 == atoi(TagTable[ESEE_TAG_STATUS].TagTxt)){
								_RUDPA_TRACE("offline! time(%d)\n",time(NULL));
								thiz->m_Status = ES_IDLE;
							}
							//even get a valid heartbeat echo, there's a need send a new util heartbeat timout expires
							if(time(NULL) - curTime >= ESEE_HEARTBEAT_TIMEOUT){
								thiz->m_Status = ES_HEARTBEAT;							
							}														
						}else{//must be timeout
							thiz->m_HeartbeatReCnt++;
							thiz->m_Status = ES_HEARTBEAT;
						}					

						if(ESEE_HEARTBEAT_RE_CNT == thiz->m_HeartbeatReCnt)
						{
							thiz->m_HeartbeatReCnt = 0;
							thiz->m_Status = ES_IDLE;
						}
					}	
					
					if(time(NULL) - reloginTime > ESEE_RELOGIN_TIME){
						thiz->m_Status = ES_IDLE;
					}
				}
				break;
			case ES_TURNREADY://ready turn
				{
					thiz->b_turn_ready = false;
					EseeSendTo(thiz,STurnReady,&thiz->esee_server);
					thiz->m_Status = ES_TURNREADY_ACK;
					curTime = time(NULL);	
				}
				break;
			
			case ES_TURNREADY_ACK://get ack of ready turn
				{
					if(thiz->b_turn_ready 
						|| (time(NULL) - curTime >= ESEE_READYTURN_TIMEOUT))
					{
						if(thiz->b_turn_ready)
						{
							_RUDPA_DEBUG("ES_TURNREADY_ACK: turn all is ready!!\n");
							thiz->m_Status = ES_HEARTBEAT;							
						}
						else
						{
							_RUDPA_DEBUG("ES_TURNREADY_ACK:Timeout !! \n");	
							thiz->m_ReadyTurnReCnt++;
							thiz->m_Status = ES_TURNREADY;							
						}

						if(ESEE_READYTURN_TIMEOUT == thiz->m_ReadyTurnReCnt)
						{
							thiz->m_ReadyTurnReCnt = 0;
							thiz->m_Status = ES_HEARTBEAT;
						}
					}
				}
				break;		
			case ES_CONFIRM_TRAVERSAL://confirm the traversal req
				{
					EseeSendTo(thiz,SConfirmTraversal,&thiz->esee_server);
					thiz->m_Status = ES_HEARTBEAT;
				}
				break;		
			default:
				break;
		}
	}
	_RUDPA_ERROR("\n\nESEE thread abort!!\n\n");
	return NULL;
}

int EseeEventProc(Esee *thiz, _2Esee *pData,EseeStatus status)
{
	switch(status)
	{
		case ES_TURNREADY:
			{
				thiz->m_Status = ES_TURNREADY;
			}
			break;		
		default:
			break;
	}
	return 0;
}


int EseeListener(Esee *thiz, SESSIONLISTENER_HANDLE *l)
{
	thiz->m_l = l;
	return 0;
}

int EseeDataProc(void *uProtocol,SESSIONLISTENER_HANDLE *l,SESSEION_EVENT_t e,void *pData,int nDatasize)
{
	UnionProtocol *up = (UnionProtocol *)uProtocol;
	Esee *esee  =up->up_esee;
	RECV_t *pRecv = (RECV_t*)pData;
	struct sockaddr_in *from = (struct sockaddr_in*)pRecv->from;
	//printf("#4\n");
	if(0 != strncmp(pRecv->realbuf,"<esee", 5))
	{
		_RUDPA_DEBUG("NOT esee pkt\n");
		return EE_HEADERROR;
	}
	_RUDPA_DEBUG("Get msg:%s\n",pRecv->realbuf);	

	int cnt = ReadProtocol(pRecv->realbuf );	
	if(cnt <= 0)
	{
		_RUDPA_ERROR("read Esee Pkt error\n");
	}
	int cmd = atoi(TagTable[ESEE_TAG_CMD].TagTxt);	
	
	_RUDPA_DEBUG("Get cmd:%s\n",TagTable[ESEE_TAG_CMD].TagTxt);

	switch(cmd){
		case SResponseIdentify:
			{
				esee->b_turnAuth = true;			 
				_RUDPA_TRACE(" ID \033[31m%s\033[0m Login ESEE!!!\n", 
										TagTable[ESEE_TAG_ID].TagTxt);
			}
			break;
		case SResponseHeart:
			{
				/*
				 *Get a heart beat     
				 */
				esee->b_heartbeat  = true;
			}
			break;
		case SResponseTurn:
			{
				if(NULL == up->up_turn)
				{
					_RUDPA_ERROR("rudpa:the turn handle is not ready now\n");
					break;
				}
				/*jump to the turn eventProc call,start req turn*/
				_RUDPA_TRACE("SResponseTurn:: ID:%s\n",TagTable[ESEE_TAG_ID].TagTxt);
				Turn *turn =	up->up_turn;
				/*define the _2turn ,pass to the turn eventproc()*/
				_2Turn _2turn;
				_2turn.clientIp = strdup(TagTable[ESEE_TAG_CLIENTIP].TagTxt);
				_2turn.clientPort= strdup(TagTable[ESEE_TAG_CLIENTPORT].TagTxt);
				/*turnserver eg:210.1.2.45:4567*/
				_2turn.turnIp= strdup(TagTable[ESEE_TAG_TURNSERVER].TagTxt);
				_2turn.turnPort = strstr(_2turn.turnIp, ":");
				*_2turn.turnPort++ = 0;
				_2turn.sId = strdup(TagTable[ESEE_TAG_ID].TagTxt);

				turn->EventProc(turn,&_2turn,TS_TURNREQ);

			}
			break;
		case STurnAck:
			{
				_RUDPA_TRACE("STurnAck:Get plat ready_ack\n");
				esee->b_turn_ready = true;			
			}
			break;
		case STraversalReq:
			{
				if(NULL == up->up_traversal)
				{
					_RUDPA_ERROR("the thraversal Handle isn't ready now");
					break;
				}
				esee->m_Status = ES_CONFIRM_TRAVERSAL;
				_RUDPA_TRACE("STraversalReq:%s:%s:%s\n",
						TagTable[ESEE_TAG_CLIENTIP].TagTxt,
						TagTable[ESEE_TAG_CLIENTPORT].TagTxt,
						TagTable[ESEE_TAG_RANDOM].TagTxt);

				_2Traversal _2Travsl;
				_2Travsl.clientIp = inet_addr(TagTable[ESEE_TAG_CLIENTIP].TagTxt);
				_2Travsl.clientPort = atoi(TagTable[ESEE_TAG_CLIENTPORT].TagTxt);
				_2Travsl.random = atoi(TagTable[ESEE_TAG_RANDOM].TagTxt);
				_2Travsl.TTL = 30;				
				up->up_traversal->EventProc(up,&_2Travsl,TraS_CONFIRM_HOLE);
			}
			break;
		case CHoleDevice:
			{
				_RUDPA_DEBUG("CHoleDevice:Get a client Hole!!\n");

					_2Traversal _2Travsl =
					{
						.clientIp = from->sin_addr.s_addr,
						.clientPort = ntohs(from->sin_port),
						.random = atoi(TagTable[ESEE_TAG_RANDOM].TagTxt),
						.TTL = 30,													
					};
					//_2Travsl.clientPort += 1;

					SetTagTxt("clientip",inet_ntoa(*(struct in_addr*)&_2Travsl.clientIp));
					char port[12] = {0};
					sprintf(port,"%d",_2Travsl.clientPort);
					SetTagTxt("clientport",port);
					_RUDPA_TRACE("CHoleDevice:%s:%s random:%s\n",
						TagTable[ESEE_TAG_CLIENTIP].TagTxt,
						TagTable[ESEE_TAG_CLIENTPORT].TagTxt,
						TagTable[ESEE_TAG_RANDOM].TagTxt);

					up->up_traversal->EventProc(up,&_2Travsl,Tras_CHANGE_HOLE);
				
			}
			break;
		default:
			break;
	}

	pRecv ->produced = !0;
	return 0;
}

void Encrypt(unsigned char key[], unsigned char input[], unsigned char output[]){
	unsigned char cipher[48] = {0};
	unsigned char First[16] = {0}, Second[16] = {0};
	unsigned char newKey[16] = {0};
	unsigned char randStr[16] = {0};
	char base64[128] = {0};
	RandString(randStr);
	InitAes(16, key);
	Cipher(randStr, newKey);
	InitAes(16, newKey);
	Cipher(input, First);
	Cipher(input + 16, Second);
	memcpy(cipher, First, sizeof(First));
	memcpy(cipher + sizeof(First), Second, sizeof(Second));
	memcpy(cipher + sizeof(First) + sizeof(Second), randStr, sizeof(randStr));
	base64_encode(cipher, base64, 48);
	strcpy((char *)output, base64);
}

static void* GetEseePlatInfo(void *esee)
{	
	Esee *thiz = esee;		
	prctl(PR_SET_NAME, "GetEseePlatInfo");

	for(;;)
	{
		sleep(1);
		#if defined USE_OUTSIDE_ESEEPLAT
		res_init();  //cauz the gethostbyname's cache?
		struct hostent *host =	gethostbyname("www.msndvr.com");		
		if(NULL == host)
		{
			//_RUDPA_ERROR("Get Esee's addr failed\n");			
			continue;
		}
		gESEE_PLAT_IP = inet_ntoa(*(struct in_addr*)host->h_addr);		
		#endif
		
		if(thiz->esee_server.sin_addr.s_addr != inet_addr(gESEE_PLAT_IP))
		{
			_RUDPA_TRACE("new esee server addr (%s)\n", gESEE_PLAT_IP);
			rudpa_log("/tmp/rudpa.esee", "w+", "%s", gESEE_PLAT_IP);
			thiz->m_EseePlatInfoValid = 0;
			thiz->esee_server.sin_family = AF_INET; 
			thiz->esee_server.sin_port = htons(gESEE_PLAT_PORT);
			thiz->esee_server.sin_addr.s_addr = inet_addr(gESEE_PLAT_IP);
			thiz->m_EseePlatInfoValid = 1;
		}			
	}	
	return NULL;
}

Esee *CreateNewEsee(void *uProtocol)
{
	int i;
	Esee *pNew = (Esee*)malloc(sizeof(Esee));
	if(NULL == pNew)
	{
		_RUDPA_ERROR("New Esee:mem malloc failed\n");
		return NULL;
	}

	pNew->m_l = NULL;	
	pNew->m_Status = ES_TURN_AUTH;
	pNew->m_TurnAuthReCnt = 0;
	pNew->m_EseePlatInfoValid = 0;
	pNew->EventProc = EseeEventProc;
	pNew->SetListener = EseeListener;
	pNew->DataProc = EseeDataProc;
	bzero(pNew->m_externIp,24);
	bzero(pNew->m_externPort,8);

	UnionProtocol *up = (UnionProtocol*)uProtocol;
	up->up_esee = pNew;
	pNew->m_uProtocol = up;
	
	/*create a thread get the platinfo,so won't block the main thread*/
	pthread_t plat_threadId;
	pthread_create(&plat_threadId, NULL, GetEseePlatInfo, pNew);
	pthread_detach(plat_threadId);

	/*clear the esee tagtable,just once when create new esee*/	
	for(i=0; i<ESEE_MAX_TAG; i++)
	{
		SetTagTxt(TagTable[i].TagName,"");
	}
	
	unsigned char base64[128] = {0};
	Encrypt(snKey, (unsigned char*)gESEE_MAC , base64);	
	SetTagTxt("sn", base64);	

	/*
	 *bzero(&pNew->client_addr ,sizeof(struct sockaddr_in ));
	 *pNew->client_addr.sin_family = AF_INET;	
	 *pNew->client_addr.sin_port = htons(888);
	 *pNew->client_addr.sin_addr.s_addr = inet_addr("192.168.1.3");
	 *SetTagTxt("random","898989");
	 *SetTagTxt("clientip","192.168.1.3");
	 *SetTagTxt("clientport","2345");
	 *    TRACE("clientip:%s \tclientport:%s \t random:%s\n",TagTable[ESEE_TAG_CLIENTIP].TagTxt,\
	 *            TagTable[ESEE_TAG_CLIENTPORT].TagTxt,TagTable[ESEE_TAG_RANDOM].TagTxt);	
	 *pNew->m_Status = ES_CONFIRM_TRAVERSAL;
	 */
	pNew->m_Status = ES_TURN_AUTH;
	pthread_t threadId;
	pthread_create(&threadId, NULL, EseeThread,pNew);
	pthread_detach(threadId);

	return pNew;
}

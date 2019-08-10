#include "macro_def.h"
#include "bubble_def.h"
#include "socket_tcp.h"
#include "media_buf.h"
#include "sdk/sdk_api.h"
#include "bubble.h"
#include "generic.h"
#include "usrm.h"
#include "spook.h"
#include "global_runtime.h"
#include <linux/tcp.h>

typedef struct BUBBLE_SESSION
{
	uint32_t* trigger;
	bool is_open;
	int is_login;
	int chnl;
	int sock;
	

	lpMEDIABUF_USER user; // the mediabuf user

#define  Bubble_SESSION_RECV_BUF_SZ (2 * 1024)
	uint8_t recv_buf[Bubble_SESSION_RECV_BUF_SZ];
	ssize_t recv_sz;

	int live_ch;
	int live_stream;

	// add by CaoHH
	int live_StreamId;
	bool is_StreamChanged;
	
}BUBBLE_SESSION_t;

static ST_BUBBLE_attr bubble_attr;


#define GET_BUBBLE_FRAME_TYPE(sdk_enc_type, ret_bubble_frame_type)\
	do{\
		switch(sdk_enc_type){\
			default:\
			case 1:\
				ret_bubble_frame_type = MT_IDR;\
				break;\
			case 0:\
				ret_bubble_frame_type = MT_PSLICE;\
				break;\
			case kSDK_ENC_BUF_DATA_PCM:\
			case kSDK_ENC_BUF_DATA_G711A:\
			case kSDK_ENC_BUF_DATA_G711U:\
				ret_bubble_frame_type  = MT_AUDIO;\
				break;\
		}\
	}while(0)


static int bubble_session_init(uint32_t* trigger, int sock, BUBBLE_SESSION_t* p_session, int live_ch, int live_stream)
{
	int ret = 0;
	p_session->trigger = trigger;
	p_session->chnl = 0;
	p_session->is_login = 0;
	p_session->is_open = false;
	p_session->sock = sock;
	
	// create a media buf user
	p_session->user = NULL;

	// live ch/stream, very important to init
	p_session->live_ch = live_ch;
	p_session->live_stream = live_stream;

	// add by CaoHH
	p_session->live_StreamId = -1;
	p_session->is_StreamChanged = false;

	BUBBLE_TRACE("Bubble session init @ (%08x, %08x)!", getpid(), pthread_self());
	return 0;
}

static void bubble_session_destroy(BUBBLE_SESSION_t* p_session)
{
	BUBBLE_TRACE("Bubble session destroy @ (%08x, %08x)!", getpid(), pthread_self());
	
	// reclaim the mediabuf user
	if(p_session->user){
		MEDIABUF_detach(p_session->user);
		p_session->user = NULL;
	}
}

static int bubble_check_msg(BUBBLE_SESSION_t* const session, char *Data, unsigned int size)
{
	MsgPackData *msgpack;
	msgpack = (MsgPackData *)Data;
	struct timespec timetic;
	int send_pack_size, ret;
	char sendData[1024]={0};
	PackHead *packhead = (PackHead *)sendData;
	MsgPackData msg;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp2_r(session->sock, &sockTCP);

	switch(msgpack->cMsgType[0]){
		
		case MSGT_USERVRF:
			{
				//check user
				BUBBLE_TRACE("MSGT_USERVRF ");
				UserVrfB uservrfb;
				UserVrf* uservrf = msgpack->pMsg;
				USRM_I_KNOW_U_t * usrm = NULL;
				BUBBLE_TRACE("%s/%s", uservrf->sUserName, uservrf->sPassWord);
				usrm = USRM_login(uservrf->sUserName, uservrf->sPassWord);
				if(usrm != NULL){
					uservrfb.bVerify = 1;
					session->is_login = 1;
					USRM_logout(usrm);
				}else{
					uservrfb.bVerify = 0;
				    session->is_login = 0;
				}
				//send ack


				packhead->cHeadChar = 0xaa;
				clock_gettime(CLOCK_MONOTONIC, &timetic);
				packhead->uiTicket = htonl(timetic.tv_sec + timetic.tv_nsec/1000);
				packhead->cPackType = PT_MSGPACK;

				msg.cMsgType[0] = MSGT_USERVRF_B;
				msg.uiLength = sizeof(msg.cMsgType) + sizeof(UserVrfB);

				packhead->uiLength = htonl(STRUCT_MEMBER_POS(PackHead,pData)
												- STRUCT_MEMBER_POS(PackHead,cPackType)
												+ STRUCT_MEMBER_POS(MsgPackData,pMsg)
												+ sizeof(UserVrfB));
				memcpy(packhead->pData, &msg, STRUCT_MEMBER_POS(MsgPackData,pMsg));
				memcpy(packhead->pData + STRUCT_MEMBER_POS(MsgPackData,pMsg),
						&uservrfb, sizeof(UserVrfB));

				send_pack_size = STRUCT_MEMBER_POS(PackHead,pData)
												+ STRUCT_MEMBER_POS(MsgPackData,pMsg)
												+ sizeof(UserVrfB);
			}
			break;
		case MSGT_CHLREQ:
			{
				BUBBLE_TRACE("MSGT_CHLREQ ");
				packhead->cHeadChar = 0xaa;
				clock_gettime(CLOCK_MONOTONIC, &timetic);
				packhead->uiTicket = htonl(timetic.tv_sec + timetic.tv_nsec/1000);
				packhead->cPackType = PT_MSGPACK;

				msg.cMsgType[0] = MSGT_CHLREQ_B;
				msg.uiLength = sizeof(msg.cMsgType) + sizeof(ChlReqB);
				ChlReqB	chlreqb;
				chlreqb.nChannelNum = 1;
				packhead->uiLength = htonl(STRUCT_MEMBER_POS(PackHead,pData)
												- STRUCT_MEMBER_POS(PackHead,cPackType)
												+ STRUCT_MEMBER_POS(MsgPackData,pMsg)
												+ sizeof(ChlReqB));
				memcpy(packhead->pData, &msg, STRUCT_MEMBER_POS(MsgPackData,pMsg));
				memcpy(packhead->pData + STRUCT_MEMBER_POS(MsgPackData,pMsg),
						&chlreqb, sizeof(ChlReqB));

				send_pack_size = STRUCT_MEMBER_POS(PackHead,pData)
												+ STRUCT_MEMBER_POS(MsgPackData,pMsg)
												+ sizeof(ChlReqB);
			}
			break;
		default:
			return -1;
		}
	// enable nodelay
	tcp->set_nodelay(tcp, true);
	tcp->set_send_timeout(tcp, 4, 0);
	tcp->set_recv_timeout(tcp, 4, 0);
	ret = tcp->send2(tcp, sendData, send_pack_size, 0);
	if(ret < 0){ 
		// FIXME:
		BUBBLE_TRACE("send check failed!!:%s(%d)", strerror(errno), errno);
	}
	return ret;
}

static int bubble_ack(BUBBLE_SESSION_t* session, void *arg, char type)
{
	int ret = 0;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp2_r(session->sock, &sockTCP);
	switch(type){
		case MSGT_USERVRF_B:
			break;
		case MSGT_CHLREQ_B:
			break;
		case 0x08://max user
			{
				PackHead pack_head;
				struct timespec timetic;
				pack_head.cHeadChar = 0xaa;
				pack_head.uiLength = htonl(sizeof(PackHead)
						- STRUCT_MEMBER_POS(PackHead,cPackType));
				pack_head.cPackType = type;//0x08
				clock_gettime(CLOCK_MONOTONIC, &timetic);
				pack_head.uiTicket = htonl(timetic.tv_sec*1000 + timetic.tv_nsec/1000000);
				pack_head.pData[0] = 0x01;

				int send_size = sizeof(PackHead);

				tcp->set_nodelay(tcp, true);
				tcp->set_send_timeout(tcp, 4, 0);
				tcp->set_recv_timeout(tcp, 4, 0);
				ret = tcp->send2(tcp, &pack_head, send_size, 0);
				if(ret < 0){ 
					// FIXME:
					BUBBLE_TRACE("send ack failed!!:%s(%d)", strerror(errno), errno);
				}
			}
			break;
		default:
			break;
	}
	return ret;
}

static int bubble_SendAudio(BUBBLE_SESSION_t* session, char *buf, int size, uint64_t timestamp, enSDK_ENC_BUF_DATA_TYPE enc_type)
{
#pragma pack(4)
		typedef struct AUDIO_BUF_ATTR
		{
			int entries;
			int packsize; // 320
			uint64_t pts;
			time_t gtime;
			char encode[8];
			int samplerate;
			int samplewidth;
		}AUDIO_BUF_ATTR_t;
#pragma pack()

	char tmpSndBuf[128*1024];
	int pack_index = 0;
	int pack_size = STRUCT_MEMBER_POS(PackHead, pData)
						- STRUCT_MEMBER_POS(PackHead,cPackType)
						+ STRUCT_MEMBER_POS(MediaPackData, pData) + size + sizeof(AUDIO_BUF_ATTR_t);
	//BUBBLE_TRACE("pack_size = %d  - %d", pack_size, size);
	PackHead *pack_head = tmpSndBuf;
	MediaPackData media_pack_head;
	//struct timespec timetic;
	pack_head->cHeadChar = 0xaa;
	pack_head->uiLength = htonl(pack_size);
	pack_head->cPackType = PT_MEDIAPACK;
	pack_head->uiTicket = htonl(timestamp/1000);
	media_pack_head.uiLength = htonl(size + sizeof(AUDIO_BUF_ATTR_t));
	AUDIO_BUF_ATTR_t audio_buf_attr =
		{
			.entries = 1, // should be only 1
			.packsize = size, // set
			.pts = timestamp, // set
			.gtime = timestamp / 1000000, // set
			.encode = "g711a",
			.samplerate = 8000, // set
			.samplewidth = 16, // set
		};


    if(enc_type == kSDK_ENC_BUF_DATA_G711A) {
        snprintf(audio_buf_attr.encode, sizeof(audio_buf_attr.encode), "%s", "g711a");
    }
    else if(enc_type == kSDK_ENC_BUF_DATA_AAC) {
        snprintf(audio_buf_attr.encode, sizeof(audio_buf_attr.encode), "%s", "aac");
    }
    else {
        snprintf(audio_buf_attr.encode, sizeof(audio_buf_attr.encode), "%s", "g711a");
    }

	media_pack_head.cMediaType = MT_AUDIO;
	media_pack_head.cId = session->chnl;
	pack_index = STRUCT_MEMBER_POS(PackHead,pData);
	memcpy(&tmpSndBuf[pack_index], &media_pack_head, STRUCT_MEMBER_POS(MediaPackData,pData));
	pack_index += STRUCT_MEMBER_POS(MediaPackData,pData);

	int send_size = pack_index;

	/*SOCKET_RW_CTX rw_ctx;
	SOCKETRW_rwinit(&rw_ctx, session->sock, (void *)tmpSndBuf, send_size, 5);
	SOCKETRW_writen(&rw_ctx);
//	BUBBLE_TRACE("size %d  result = %d   %c ", size, rw_ctx.result, bIFrame ? 'I':'P');

	if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
		return -1;
	else{
		SOCKETRW_rwinit(&rw_ctx, session->sock, &audio_buf_attr, sizeof(AUDIO_BUF_ATTR_t), 5);
		SOCKETRW_writen(&rw_ctx);
		if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
			return -1;
		else{
			SOCKETRW_rwinit(&rw_ctx, session->sock, (void *)buf, size, 5);
			SOCKETRW_writen(&rw_ctx);
			if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
				return -1;
			else
				return rw_ctx.actual_len;
		}
	}*/

	int ret = 0;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp2_r(session->sock, &sockTCP);
	
	tcp->set_nodelay(tcp, true);
	tcp->set_send_timeout(tcp, 4, 0);
	tcp->set_recv_timeout(tcp, 4, 0);
	ret = tcp->send2(tcp, tmpSndBuf, send_size, 0);
	if(ret < 0){ 
		// FIXME:
		BUBBLE_TRACE("send audio Header failed!!:%s(%d)", strerror(errno), errno);
	}else{
		ret = tcp->send2(tcp, &audio_buf_attr, sizeof(AUDIO_BUF_ATTR_t), 0);
		if(ret < 0){
		}else{
			ret = tcp->send2(tcp, buf, size, 0);
		}
	}
	return ret;
}


static int bubble_SendVideo(BUBBLE_SESSION_t* session, char *buf, int size, int bIFrame, uint64_t timestamp)
{
	char tmpSndBuf[128*1024];
	int pack_index = 0;
	int pack_size = STRUCT_MEMBER_POS(PackHead, pData)
						- STRUCT_MEMBER_POS(PackHead,cPackType)
						+ STRUCT_MEMBER_POS(MediaPackData, pData) + size;
	//BUBBLE_TRACE("pack_size = %d  - %d", pack_size, size);
	PackHead *pack_head = tmpSndBuf;
	MediaPackData media_pack_head;
	//struct timespec timetic;
	pack_head->cHeadChar = 0xaa;
	pack_head->uiLength = htonl(pack_size);
	pack_head->cPackType = PT_MEDIAPACK;
	//clock_gettime(CLOCK_MONOTONIC, &timetic);
	//pack_head->uiTicket = htonl(timetic.tv_sec*1000 + timetic.tv_nsec/1000000);
	pack_head->uiTicket = htonl(timestamp/1000);
	media_pack_head.uiLength = htonl(size);
	//BUBBLE_TRACE("length: %d  size: %d", media_pack_head.uiLength, size);

	//GET_BUBBLE_FRAME_TYPE(iFrameType, media_pack_head.cMediaType);
	//printf("type:%d\r\n", media_pack_head.cMediaType);
	media_pack_head.cMediaType = bIFrame ? MT_IDR : MT_PSLICE;
	media_pack_head.cId = session->chnl;
	pack_index = STRUCT_MEMBER_POS(PackHead,pData);
	memcpy(&tmpSndBuf[pack_index], &media_pack_head, STRUCT_MEMBER_POS(MediaPackData,pData));
	pack_index += STRUCT_MEMBER_POS(MediaPackData,pData);

	int send_size = pack_index;

	/*SOCKET_RW_CTX rw_ctx;
	SOCKETRW_rwinit(&rw_ctx, session->sock, (void *)tmpSndBuf, send_size, 5);
	SOCKETRW_writen(&rw_ctx);
//	BUBBLE_TRACE("size %d  result = %d   %c ", size, rw_ctx.result, bIFrame ? 'I':'P');

	if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
		return -1;
	else{
		SOCKETRW_rwinit(&rw_ctx, session->sock, (void *)buf, size, 5);
		SOCKETRW_writen(&rw_ctx);
		if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
			return -1;
		else
			return rw_ctx.actual_len;
	}*/

	int ret = 0;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp2_r(session->sock, &sockTCP);

	// enable nodelay
	tcp->set_nodelay(tcp, true);
	tcp->set_send_timeout(tcp, 4, 0);
	tcp->set_recv_timeout(tcp, 4, 0);
	ret = tcp->send2(tcp, tmpSndBuf, send_size, 0);
	if(ret < 0){ 
		// FIXME:
		BUBBLE_TRACE("send frameHeader failed!!:%s(%d)", strerror(errno), errno);
	}else{
		ret = tcp->send2(tcp, buf, size, 0);
		if(ret < 0){
		}
	}
	return ret;
}

static int bubble_send_live_stream(BUBBLE_SESSION_t* session)
{
	int send_ret = 0;
	static unsigned char is_first_i_frame = 1, i_frame_cnt = 0;

	// check if the stream was changed
	if (session->is_StreamChanged)
	{
		if(session->user){
			MEDIABUF_detach(session->user);
			session->user = NULL;
		}
		session->is_StreamChanged = false;
	}
	
	// check the mediabuf attached
	if(!session->user){
		int mediabuf_ch;
		switch(session->live_StreamId)
		{
		case 0:		mediabuf_ch = MEDIABUF_lookup_byname("ch0_0.264"); break;
		case 1:		mediabuf_ch = MEDIABUF_lookup_byname("ch0_1.264"); break;
		case 2:		mediabuf_ch = MEDIABUF_lookup_byname("ch0_1.264"); break;
		default:	mediabuf_ch = MEDIABUF_lookup_byname("ch0_0.264"); break;
		}
		if(mediabuf_ch >= 0){
			session->user = MEDIABUF_attach(mediabuf_ch);
			if(NULL == session->user){
				BUBBLE_TRACE("Media pool is full!");
				bubble_ack(session, NULL, 0x08);
				sleep(3);
				return -1;
			}else{
				MEDIABUF_sync(session->user);
				is_first_i_frame = 1;
				i_frame_cnt = 0;
			}
		}
	}

	GLOBAL_enter_live();

	// send live stream until media pool empty or error occurs!
	//MEDIABUF_sync(session->user);
	while(*session->trigger)
	{
		bool out_frame = false;
		
		if(0 == MEDIABUF_out_try_lock(session->user)){
			void* send_ptr = 0;
			ssize_t send_sz = 0;

			// out a frame from media buf
			if(0 == MEDIABUF_out(session->user, &send_ptr, NULL, &send_sz)){
				const lpSDK_ENC_BUF_ATTR const attr = (lpSDK_ENC_BUF_ATTR)send_ptr;
				//printf("MEDIABUF_out:%s-%d\r\n", attr->h264.keyframe?"I":"P", send_sz);
				if(attr->type == kSDK_ENC_BUF_DATA_H264 || attr->type == kSDK_ENC_BUF_DATA_H265){//video
					if(is_first_i_frame && attr->h264.keyframe){
						if(++i_frame_cnt >= 2){
							is_first_i_frame = 0;
						}
					}
					if(i_frame_cnt >= 1){
						send_ret = bubble_SendVideo(session,
									send_ptr + sizeof(stSDK_ENC_BUF_ATTR),
									send_sz - sizeof(stSDK_ENC_BUF_ATTR),
									attr->h264.keyframe, 
									attr->timestamp_us);
					}
				}else if(attr->type == kSDK_ENC_BUF_DATA_G711A){//audio
					//if(session->live_StreamId == 0){//just for main stream
						send_ret = bubble_SendAudio(session,
								send_ptr + sizeof(stSDK_ENC_BUF_ATTR),
								send_sz - sizeof(stSDK_ENC_BUF_ATTR),
								attr->timestamp_us, kSDK_ENC_BUF_DATA_G711A);
					//}
				}			
                else if(attr->type == kSDK_ENC_BUF_DATA_AAC) {
                    send_ret = bubble_SendAudio(session,
                            send_ptr + sizeof(stSDK_ENC_BUF_ATTR),
                            send_sz - sizeof(stSDK_ENC_BUF_ATTR),
                            attr->timestamp_us, kSDK_ENC_BUF_DATA_AAC);
                }
				out_frame = true;
			}

			// out unlock
			MEDIABUF_out_unlock(session->user);
			if(false == out_frame){
				usleep(1000);
			}
			
		}else{
			// FIXME: meida out lock error!
		}

		if(false == out_frame || send_ret < 0){
			break;
		}
	}

	GLOBAL_leave_live();

	if(send_ret < 0){
		return -1;
	}
	return 0;
}

static int process_recv_pack(BUBBLE_SESSION_t* session)
{
	char* Bubble_Buff = session->recv_buf;
	int   Bubble_Size = session->recv_sz;

//	BUBBLE_TRACE("size = %d", session->recv_sz);

	PackHead *packhead;
	uint32_t uiPackLength;
	unsigned char bcheckhead = 0;

    while(Bubble_Size > 5) {
		packhead = (PackHead*)Bubble_Buff;
		uiPackLength = ntohl(packhead->uiLength);
        if(0xaa != *Bubble_Buff) {
			//BUBBLE_TRACE("packhead.cHeadChar = %x", *Bubble_Buff);
			if(bcheckhead){
				Bubble_Size -= 1;
       			Bubble_Buff += 1;
				continue;
			}else{
				return -1;
			}
        }
        if(uiPackLength + STRUCT_MEMBER_POS(PackHead,uiLength) + sizeof(packhead->uiLength)> (unsigned long)Bubble_Size) { //Received Data Package Size Error.
        	BUBBLE_TRACE("packhead.uiLength = %d", uiPackLength);
			if(bcheckhead){
				Bubble_Size -= 1;
       			Bubble_Buff += 1;
				continue;
			}else{
            	return -1;
			}
        }else{
			bcheckhead = 1;
		}

		unsigned char *pDatatmp = (unsigned char*)Bubble_Buff + STRUCT_MEMBER_POS(PackHead,pData);

		switch (packhead->cPackType) {
			case 0x00: {    //PT_MSGPACK
				MsgPackData *msgpack;
				msgpack = (MsgPackData *)pDatatmp;
				bubble_check_msg(session, pDatatmp, msgpack->uiLength);
				BUBBLE_TRACE("recv msg!!");
				Bubble_Buff = Bubble_Buff + STRUCT_MEMBER_POS(PackHead,pData) + sizeof(MsgPackData);
				Bubble_Size = Bubble_Size - STRUCT_MEMBER_POS(PackHead,pData) - sizeof(MsgPackData);
			}
				break;
			case 0x01: {    //PT_MEDIAPACK,
				MediaPackData *mediapackdata;
				mediapackdata = (MediaPackData *)pDatatmp;

			}
				break;
			case 0x02: { //ï¿½ï¿½ï¿½ï¿½ PT_HEARTBEATPACK
				return 0;
				// MTPRINTF("heartbeat\n");
			}
				break;
			case 0x03: { //ï¿½Ö¸ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ù¶Èµï¿½ï¿½ï¿½ï¿½ï¿½
				//pSockInf->bIFrameSend = false;
			}
				break;
			case 0x04: { //OpenId PT_OPENCHL
				BUBBLE_TRACE("recv open channel");
				int nID = *pDatatmp;
				if (1 == (*(pDatatmp + 1)) && session->is_login) {
					session->chnl = nID;
					session->is_open = true;
					if (-1 != session->live_StreamId && 0 != session->live_StreamId)
					{
						session->is_StreamChanged = true;
					}
					session->live_StreamId = 0;
					BUBBLE_TRACE("channel: %d", session->chnl);
				}
				else {
					BUBBLE_TRACE("close channel %d", nID);
					session->is_open = false; // stop to send live media
				}
				Bubble_Buff = Bubble_Buff + 
					STRUCT_MEMBER_POS(PackHead,pData) + 
					STRUCT_MEMBER_POS(OpenChlData,bopened) + sizeof(char);
				Bubble_Size = Bubble_Size - 
					STRUCT_MEMBER_POS(PackHead,pData) - 
					STRUCT_MEMBER_POS(OpenChlData,bopened) - sizeof(char);;
			}
				break;

			case PT_OPENSTREAM:
			{
				BubbleOpenStream * pParam = (BubbleOpenStream *)pDatatmp;
				if (!session->is_login)
				{
					return 0;
				}
				BUBBLE_TRACE("Open stream:channel:%d stream:%d operation:%d",pParam->uiChannel,pParam->uiStreamId,pParam->uiOpened);
				session->chnl = pParam->uiChannel;
				session->is_open = pParam->uiOpened ? true : false;

				if (-1 != session->live_StreamId && pParam->uiStreamId != session->live_StreamId)
				{
					session->is_StreamChanged = true;
				}
				session->live_StreamId = pParam->uiStreamId;

				Bubble_Buff = Bubble_Buff + STRUCT_MEMBER_POS(PackHead,pData) + sizeof(BubbleOpenStream);
				Bubble_Size = Bubble_Size - STRUCT_MEMBER_POS(PackHead,pData) - sizeof(BubbleOpenStream);

				return 0;
				break;
			}
			case 0x05: { // ClientId
				//pSockInf->nClientID = (pData[0] << 24) + (pData[1] << 16) + (pData[2] << 8) + (pData[3]);
			}
				break;
			default:
				return 0;
		}
    }
}


SPOOK_SESSION_PROBE_t BUBBLE_probe(const void *msg, ssize_t msg_sz)
{
    PackHead *pack = (PackHead *)msg;

    if (msg_sz < STRUCT_MEMBER_POS(PackHead,uiLength) + sizeof(pack->uiLength))
    {
        return SPOOK_PROBE_UNDETERMIN;
    }

    if (0xaa != pack->cHeadChar)
    {
        return SPOOK_PROBE_MISMATCH;
    }

    uint32_t uiPackLength = ntohl(pack->uiLength);
    if (msg_sz < uiPackLength + STRUCT_MEMBER_POS(PackHead,uiLength) + sizeof(pack->uiLength))
    {
        return SPOOK_PROBE_UNDETERMIN;
    }

    return SPOOK_PROBE_MATCH;
}

static int bubble_loop(BUBBLE_SESSION_t* bubble_session)
{
	int ret = 0;
	int flags = 1;
	int time_out_cnt = 0;
	ret = setsockopt(bubble_session->sock, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(flags));
	if(ret < 0){
		//failded
	}
	while(*bubble_session->trigger)
    {
		struct timeval select_timeo = { .tv_sec = 0, .tv_usec = 50000, };
		fd_set wfd_set, rfd_set, err_set; // all kinds of fd set

        FD_ZERO(&wfd_set);
        FD_ZERO(&rfd_set);
        FD_ZERO(&err_set);
        FD_SET(bubble_session->sock, &wfd_set);
        FD_SET(bubble_session->sock, &rfd_set);
        FD_SET(bubble_session->sock, &err_set);

		// select it, come on!
		ret = select(bubble_session->sock+1, &rfd_set, &wfd_set, &err_set, &select_timeo);
		if(ret < 0){
			// something error
			BUBBLE_TRACE("Bubble select error \"%s\"", strerror(errno));
			return -1;
		}else if(0 == ret){
			// timeout!
			BUBBLE_TRACE("Bubble timeout %d!", time_out_cnt);
			if(time_out_cnt++ > 100){
				break;
			}
			usleep(50000);
		}else{
			// checking read fd set
			time_out_cnt = 0;
			if(FD_ISSET(bubble_session->sock, &rfd_set)){ // need to read
				bubble_session->recv_sz = recv(bubble_session->sock, bubble_session->recv_buf, ARRAY_ITEM(bubble_session->recv_buf), 0);
				if(bubble_session->recv_sz <= 0){
					BUBBLE_TRACE("Bubble receive error \"%s\"!", strerror(errno));
					break;
				}
				process_recv_pack(bubble_session);
			}
			// checkint write fd set
			if(FD_ISSET(bubble_session->sock, &wfd_set)){ // need to write
				if(bubble_session->is_open){
					if(bubble_send_live_stream(bubble_session) < 0){
						BUBBLE_TRACE("Bubble send error \"%s\"!", strerror(errno));
						break;
					}
				}else{
					usleep(50000);
				}
			}
			// checking error set
			if(FD_ISSET(bubble_session->sock, &err_set)) {
				BUBBLE_TRACE("Hey! Where am i ??");
			}
		}
    }
}

SPOOK_SESSION_LOOP_t BUBBLE_loop(bool *trigger, int sock, time_t *read_pts)
{
	int ret = 0;
	BUBBLE_SESSION_t bubble_session;
	bubble_session_init(trigger, sock, &bubble_session, -1, -1); // live ch/stream = -1, ignore these 2 para
    ret = bubble_loop(&bubble_session);
	bubble_session_destroy(&bubble_session);
	if(ret < 0){
		return SPOOK_LOOP_FAILURE;
	}
    return SPOOK_LOOP_SUCCESS;
}

/*
GET /bubble/live HTTP/1.1

*/
int BUBBLE_over_http_cgi(HTTPD_SESSION_t* http_session)
{
	int i = 0, ret = 0;
	SPOOK_SESSION_LOOP_t session_ret = SPOOK_LOOP_SUCCESS;
	char response_buf[1024] = {""};
	const char* const http_version = AVAL_STRDUPA(http_session->request_line.version);
	HTTP_HEADER_t* http_header = NULL;
	time_t read_pts;
	char* left_buf = NULL;
	ssize_t left_size = 0;
	AVal av_live_ch = AVC("0"), av_live_stream = AVC("0");
	int live_ch = 0, live_stream = 0;
	const char* query_string = AVAL_STRDUPA(http_session->request_line.uri_query_string);
	BUBBLE_SESSION_t bubble_session;

	// read the query string
	http_read_query_string(query_string, "ch", &av_live_ch);
	http_read_query_string(query_string, "stream", &av_live_stream);
	live_ch = atoi(AVAL_STRDUPA(av_live_ch));
	live_stream = atoi(AVAL_STRDUPA(av_live_stream));

	// response an HTTP header
	http_header = http_response_header_new(http_version, 200, NULL);
	http_header->add_tag_text(http_header, "Content-Type", "video/bubble");
	http_header->add_tag_text(http_header, "Connection", "keep-alive");
	http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
	http_response_header_free(http_header);
	http_header = NULL;
	// add some user infomation
	left_buf = response_buf + strlen(response_buf);
	left_size = ARRAY_ITEM(response_buf) - strlen(response_buf);
	ret = snprintf(left_buf, left_size,
		"<bubble version=\"1.0\" vin=\"1\">"
			"<vin0 stream=\"3\">"
				"<stream0 name=\"720p.264\" size=\"1280x720\" x1=\"yes\" x2=\"yes\" x4=\"yes\" />\r\n"
				"<stream1 name=\"360p.264\" size=\"640x360\" x1=\"yes\" x2=\"yes\" x4=\"yes\" />\r\n"
				"<stream2 name=\"qvga.264\" size=\"320x240\" x1=\"yes\" x2=\"yes\" x4=\"yes\" />\r\n"
			"<vin0>\r\n"
		"</bubble>");
	
	// fill the header with ascii code
	left_buf = response_buf + strlen(response_buf);
	left_size = ARRAY_ITEM(response_buf) - strlen(response_buf);
	//for(i = 0; i < left_size; ++i){
	//	left_buf[i] = ' ' + (i % (0x7f - ' ' + 1));
	//}
	memset(left_buf, '#', left_size);

	ret = jsock_send(http_session->sock, response_buf, ARRAY_ITEM(response_buf)); // send all the buffer coz align to data packet 512
	if(ret < 0){
		// FIXME:
	}

	// go on the native bubble
	read_pts = time(NULL);
	bubble_session_init(http_session->trigger, http_session->sock, &bubble_session, live_ch, live_stream);
	ret = bubble_loop(&bubble_session);
	bubble_session_destroy(&bubble_session);

	if(ret < 0){
		// FIXME:
	}
	return 0;
}


/*
GET /bubble/live HTTP/1.1

*/
int BUBBLE_over_http_cgi2(LP_HTTP_CONTEXT context)
{
	int i = 0, ret = 0;
	SPOOK_SESSION_LOOP_t session_ret = SPOOK_LOOP_SUCCESS;
	char response_buf[1024] = {""};
	HTTP_HEADER_t* http_header = NULL;
	time_t read_pts;
	char* left_buf = NULL;
	ssize_t left_size = 0;
	const char *str_live_ch = NULL, *str_live_stream = NULL;
	int live_ch = 0, live_stream = 0, main_width = 0, main_height = 0, sub_width = 0, sub_height = 0;
	BUBBLE_SESSION_t bubble_session;
	LP_HTTP_QUERY_PARA_LIST query_list = HTTP_UTIL_parse_query_as_para(context->request_header->query);

	// read the query string
	str_live_ch = query_list->read(query_list, "ch");
	if(!str_live_ch){
		str_live_ch = "0";
	}
	str_live_stream = query_list->read(query_list, "stream");
	if(!str_live_stream){
		str_live_stream = "0";
	}
	query_list->free(query_list);
	query_list = NULL;

	// response an HTTP header
	http_header = http_response_header_new(context->request_header->version, 200, NULL);
	http_header->add_tag_text(http_header, "Content-Type", "video/bubble");
	http_header->add_tag_text(http_header, "Connection", "keep-alive");
	http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
	http_response_header_free(http_header);
	http_header = NULL;
	// add some user infomation
	left_buf = response_buf + strlen(response_buf);
	left_size = ARRAY_ITEM(response_buf) - strlen(response_buf);

	bubble_attr.GET_VENC_RESOLUTION(1, &main_width, &main_height);
	bubble_attr.GET_VENC_RESOLUTION(2, &sub_width, &sub_height);
	ret = snprintf(left_buf, left_size,
		"<bubble version=\"1.0\" vin=\"1\">"
			"<vin0 stream=\"2\">"
				"<stream0 name=\"720p.%d\" size=\"%dx%d\" x1=\"yes\" x2=\"yes\" x4=\"yes\" />\r\n"
				"<stream1 name=\"360p.%d\" size=\"%dx%d\" x1=\"yes\" x2=\"yes\" x4=\"yes\" />\r\n"
			"<vin0>\r\n"
		"</bubble>", bubble_attr.GET_VENC_TYPE(1),main_width, main_height, 
		 bubble_attr.GET_VENC_TYPE(2), sub_width, sub_height);

	BUBBLE_TRACE("%s", left_buf);
	// fill the header with ascii code
	left_buf = response_buf + strlen(response_buf);
	left_size = ARRAY_ITEM(response_buf) - strlen(response_buf);
	//for(i = 0; i < left_size; ++i){
	//	left_buf[i] = ' ' + (i % (0x7f - ' ' + 1));
	//}
	memset(left_buf, '#', left_size);

	ret = jsock_send(context->sock, response_buf, ARRAY_ITEM(response_buf)); // send all the buffer coz align to data packet 512
	if(ret < 0){
		// FIXME:
	}

	// go on the native bubble
	read_pts = time(NULL);
	bubble_session_init(context->trigger, context->sock, &bubble_session, live_ch, live_stream);
	ret = bubble_loop(&bubble_session);
	bubble_session_destroy(&bubble_session);

	if(ret < 0){
		// FIXME:
	}
	return 0;
}

// ¹Ø±Õbubble 9013ËÑË÷¶Ë¿Ú
//static bool discovery_listener_trigger = false;
//static pthread_t discovery_listener_tid = (pthread_t)NULL;

static void bubble_response_discovery(int sock, struct sockaddr *peer_addr)
{
	int ret = 0;
	char send_buffer[256] = {""};
	char sock_ipaddr[32] = {""};
	struct ifreq ifr;

	// get local ip
	const char *eth = getenv("DEF_ETHER");
	if(NULL == eth){
		eth = "eth0";
	}
	//eth = "wlan0";
	//BUBBLE_TRACE("interface for bubble discovery:%s", eth);
	strcpy(ifr.ifr_name, eth);
	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0){
		perror("ioctl");
	}
	strcpy(sock_ipaddr, inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	
	ret = snprintf(send_buffer, sizeof(send_buffer),
			"JAIP%s&ID%s&PORT%d&HTTP%d&CH%d", sock_ipaddr, g_esee_id,
			SPOOK_listen_port(), SPOOK_listen_port(), 1);

	BUBBLE_TRACE(send_buffer);
	
	ret = sendto(sock, send_buffer, strlen(send_buffer), 0, peer_addr, sizeof(struct sockaddr));

}

// ¹Ø±Õbubble 9013ËÑË÷¶Ë¿Ú
#if 0
static void* bubble_discovery_listener(void* arg)
{
	int ret = 0;
	int discovery_sock = -1;
	int broadcast_flag = 1;
	struct sockaddr_in server_addr;
	in_port_t listener_port = 9013;

	pthread_detach(pthread_self());
	discovery_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(discovery_sock < 0){
		BUBBLE_TRACE("Discovery socket create failed!");
		pthread_exit(NULL);
	}
	
	ret = setsockopt(discovery_sock, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast_flag, sizeof(broadcast_flag));
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(listener_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(discovery_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(ret < 0){
		close(discovery_sock);
		BUBBLE_TRACE("Discovery socket bind failed!");
		pthread_exit(NULL);
	}
	
	while(discovery_listener_trigger)
	{
		fd_set read_set;
		struct timeval select_timeo;
		
		FD_ZERO(&read_set);
		FD_SET(discovery_sock, &read_set);

		select_timeo.tv_sec = 1;
		select_timeo.tv_usec = 0;

		ret = select(discovery_sock + 1, &read_set, NULL,NULL, &select_timeo);
		if(ret < 0){
			BUBBLE_TRACE("Discovery select error!");
			break;
		}else if(0 == ret){
			
		}else if (FD_ISSET(discovery_sock, &read_set)){
			char recv_buffer[256];
			char send_buffer[256];
			struct sockaddr_in peer_addr;
			socklen_t const peer_addrlen = sizeof(peer_addr);
			
			memset(recv_buffer, 0, sizeof(recv_buffer));
			ret = recvfrom(discovery_sock, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&peer_addr, &peer_addrlen);
			
			if(ret < 0){
				// do something
				
			}else{
				//fix me
				if(strcasestr(recv_buffer,"SEARCHJA")){

					char *DevId = strcasestr(recv_buffer,"SEARCHJA") + strlen("SEARCHJA");
					char *DevIdEnd = strstr(DevId,"&");
					unsigned long DevLen = DevIdEnd - DevId;

					if (!strncasecmp(DevId,g_esee_id,DevLen))
					{
						bubble_response_discovery(discovery_sock, &peer_addr);
					}
					//BUBBLE_TRACE("SEARCHJA");
				}
				/*else if(strcasestr(recv_buffer,"SEARCHDEV"))
				{
					bubble_response_discovery(discovery_sock, &peer_addr);
					BUBBLE_TRACE("SEARCHDEV");
				}*/
			}
		}
	}
	pthread_exit(NULL);
}
#endif

int BUBBLE_init(LP_BUBBLE_attr arg)
{
	// ¹Ø±Õbubble 9013ËÑË÷¶Ë¿Ú
	/*if(!discovery_listener_tid){
		discovery_listener_trigger = true;
		pthread_create(&discovery_listener_tid, 0, bubble_discovery_listener, NULL);
		return 0;
	}*/
	bubble_attr = *arg;

	return 0;
}

void BUBBLE_destroy()
{
	// ¹Ø±Õbubble 9013ËÑË÷¶Ë¿Ú
	/*
	if(discovery_listener_tid){
		discovery_listener_trigger = false;
		pthread_join(discovery_listener_tid, NULL);
		discovery_listener_tid = (pthread_t)NULL;
	}
	*/
}



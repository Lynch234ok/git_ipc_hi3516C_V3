#include "socket_rw.h"
#include "sdk/sdk_api.h"
#include "regRW_def.h"
#include "spook/regRW.h"
#include "generic.h"

#include "sensor.h"
#include "securedat.h"

extern uint32_t g_hardware_version;

static RegRWSession_t* regRW_create_session(bool* trigger, int sock)
{
	RegRWSession_t* const session = (RegRWSession_t*)(calloc(sizeof(RegRWSession_t), 1));

	session->trigger = trigger;
	session->is_init = 1;
	session->sock = sock;
	SOCKET_RW_CTX rw_ctx;
	SOCKETRW_rwinit(&rw_ctx, sock, (void *)session->recv_buf, ARRAY_SIZE(session->recv_buf), 1);
	SOCKETRW_read(&rw_ctx);
	session->recv_sz = rw_ctx.actual_len;
	if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS){
		printf("recv failed\r\n");
		free(session);
		return NULL;
	}

	return session;
}

static int regRW_send_read_ret(RegRWSession_t *session, char *sendbuf, int send_size)
{
	SOCKET_RW_CTX rw_ctx;
	char tmp[128];
	int i;
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, sendbuf, send_size);
	for(i = 0; i < send_size; i++){
		printf("%02x-", tmp[i]);
	}
	printf("\r\n");
	SOCKETRW_rwinit(&rw_ctx, session->sock, (void *)sendbuf, send_size, 5);
	SOCKETRW_writen(&rw_ctx);
	if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
	{
		return -1;
	}
	
	return rw_ctx.actual_len;
}

static void regRW_destroy_session(RegRWSession_t* const session)
{
	free(session);
}

static void regRW_parse(RegRWSession_t *session)
{
	char* buff = (char*)session->recv_buf;
	int  size = session->recv_sz;
	char sn_read[32];
	char ret_buff[128];
	PackHead_t ret_pack_head;

	PackHead_t* pack_head = (PackHead_t *)buff;
	if(pack_head->uiLength == (size - 8)){	
		switch(pack_head->iPackType){
			/*case REGRW_TYPE_MT9D131:
				{
					Mt9d131reg_t *reg_info = (Mt9d131reg_t *)(buff + sizeof(PackHead_t));
					if(pack_head->iOperation == 1){//write
						SENSOR_spec_reg_write(reg_info->page, reg_info->addr, reg_info->value);
					}
					else if(pack_head->iOperation == 0){//read
						reg_info->value = SENSOR_spec_reg_read(reg_info->page, reg_info->addr);
						regRW_send_read_ret(session, buff, size);
					}
				}
				break;*/
			case REGRW_TYPE_JUANSN:
				{
					Juansn_t *juansn = (Juansn_t *)(buff + sizeof(PackHead_t));
					if(juansn && strlen(juansn->sn) < 32){
						printf("%s-%d\r\n", juansn->sn, strlen(juansn->sn));
						uint32_t chip_id;
						/*int ret;
						sdk_sys->get_chip_id(&chip_id);
						switch(chip_id){
							case SDK_M388C1G:
							case SDK_HI3518A_V100:
								ret = memcmp(juansn->sn, "C1", 2);
								break;
							case SDK_HI3518C_V100:
								ret = memcmp(juansn->sn, "C2", 2);
								break;
							case SDK_HI3518E_V100:
							case SDK_HI3518E_V200:
								ret = memcmp(juansn->sn, "C4", 2);
								break;
							case SDK_HI3516A_V100:
								ret = memcmp(juansn->sn, "C5", 2);
								break;
							case SDK_HI3516C_V100:
							case SDK_HI3516C_V200:
								ret = memcmp(juansn->sn, "C3", 2);
								break;
						}*/
#if !defined(WIFI)
						if(0 != UC_SNAuthenChk()){
							UC_SNumberSet(juansn->sn, strlen(juansn->sn));
						}
#else//defined(WIFI)
						if(g_hardware_version == 0x00010001){//v1.0.1, old hardware version with encryption chip
							UC_SNumberSet(juansn->sn, strlen(juansn->sn));
						}
#endif
					}
					memset(&ret_pack_head, 0, sizeof(ret_pack_head));
					memset(ret_buff, 0, sizeof(ret_buff));
					ret_pack_head.cHeadChar[0] = 0xbb;
					ret_pack_head.cHeadChar[1] = 0xdd;
					ret_pack_head.cHeadChar[2] = 0xcc;
					ret_pack_head.cHeadChar[3] = 0xaa;
					ret_pack_head.uiLength = 12;
					ret_pack_head.iOperation = 0;
					ret_pack_head.iPackType = REGRW_TYPE_RET_TYPE;
					Ret_type_t ret_type;

					memset(sn_read, 0, sizeof(sn_read));
					if(0 == UC_SNumberGet(sn_read)) {
						if((NULL != juansn) && !strcmp(sn_read, juansn->sn)){
							ret_type.type_value = 100;
						}else{
							ret_type.type_value = 200;
						}
					}else{
						ret_type.type_value = 200;
					}
					memcpy(ret_buff, &ret_pack_head, sizeof(ret_pack_head));
					memcpy(ret_buff+sizeof(ret_pack_head), &ret_type, sizeof(ret_type));
					regRW_send_read_ret(session, ret_buff, sizeof(ret_pack_head)+sizeof(ret_type));
				}
				break;
			default:
				printf("unknow reg type!\r\n");
				break;
		}
	}
}


SPOOK_SESSION_PROBE_t SENSOR_REGRW_probe(const void* msg, ssize_t msg_sz)
{
	if (msg_sz < 4 )
		return SPOOK_PROBE_UNDETERMIN;

	char *_msg = (char *)msg;

	int ret = (_msg[0] == 0xbb) && (_msg[1] == 0xdd) && (_msg[2] == 0xcc) && (_msg[3] == 0xaa);

	if (ret != 1) {
		return SPOOK_PROBE_MISMATCH;
	}
	else if (ret == 1) {
		return SPOOK_PROBE_MATCH;
	}
	return SPOOK_PROBE_MISMATCH;
}

SPOOK_SESSION_LOOP_t SENSOR_REGRW_loop(bool* trigger, int sock, time_t *read_pts)
{
	//	MediaBufUser_t* user = MEDIABUF_user_attach(1);
	
		RegRWSession_t* const session = regRW_create_session(trigger, sock);
	
		struct timeval timeout;
		fd_set read_set;
		int select_ret;

		signal(SIGPIPE, SIG_IGN);
		
		while(*session->trigger)
		{	
			timeout.tv_sec = 0;
			timeout.tv_usec = 10*1000;//read command every 10ms
	
			FD_ZERO(&read_set);
			FD_SET(sock, &read_set);
			select_ret = select(sock + 1, &read_set, NULL, NULL, &timeout);
	
	
			if (select_ret < 0)
			{
				*trigger = 0;
				printf("error stop\n");
				continue;
			}
			else if(select_ret == 0)
			{
	//			printf("running...\n");
			}
			else
			{
				//printf("select read\n");
				//char buf[MAX_BUF_SIZE];
	
				SOCKET_RW_CTX rw_ctx;
				SOCKETRW_rwinit(&rw_ctx, sock, (void *)session->recv_buf, sizeof(session->recv_buf), 1);
				SOCKETRW_read(&rw_ctx);
				session->recv_sz = rw_ctx.actual_len;
				if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
				{
					*trigger = 0;
					printf("read error stop\n");
				}
				else
				{
					//*read_pts= time(NULL);
	//				buf[read_ret] = 0;
					session->recv_buf[session->recv_sz] = 0;
					regRW_parse(session);
				}
			}
		}
	
		// destroy
		regRW_destroy_session(session);
	
		return SPOOK_LOOP_SUCCESS;

}

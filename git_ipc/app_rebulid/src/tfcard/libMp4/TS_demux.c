/*
 * TS.c
 * use to get AAC or H265 H264 FRAME from MP4(TS) file
 *  Created on: 2018-3-21
 *      Author: LiangWJ
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpegts.h"
#include <string.h>
#include "mpeg_ts.h"
#include "TS_demux.h"

typedef struct
{
	void * fd;
	int frame_size;
	char *tmp_video;
	int video_post;
	char *tmp_audio;
	int audio_post;
	FRAME_SLICE_E slicetype;
	int video_pes_mark;
	int audio_pes_mark;
	int pmt_type;
	int pat_type;
	int video_type;
	int audio_type;
	uint64_t v_ts;
	uint64_t a_ts;
}stMP4,*lpMP4;

int read_filedes(FILE *fd,lpFILEDESCIP_S filedes)
{
	char buf_tmp[189];

	int ret=fread(buf_tmp,1,188,fd);
	if(ret<188)
	{
		printf("file too small\n");
		return-1;
	}
	MPEGTS_PACKET* _packet= (MPEGTS_PACKET*)buf_tmp;
	if (!IS_MPEG_HEAD(_packet))
	{
		printf("not a mepgts_head\n");
		return -3;
	}
	int pid=MPEG_PID(_packet);
	if(pid!=TS_PID_DESCIP)
	{
		printf("NO JUAN TS HEAD EXIST\n");
		fseek(fd,0,SEEK_SET);
		return-1;
	}
	int lenght=sizeof(stFILEDESCIP_S);
	printf("-----read des sucess----sturct size:%d\n",lenght);
	memcpy(filedes,buf_tmp+4,lenght);
	return 0;
}
int NK_MP4_getinfo(lpNK_MP4 demuxer,lpFILEDESCIP_S filedes)
{
	lpMP4 mp4demuxer=(lpMP4)demuxer;
	int ret=read_filedes(mp4demuxer->fd,filedes);
	if(ret<0)
	{
		printf("NO DES HEAD FILE!!!!!!QUIT!!!!\n");
		return ret;
	}
	return 0;
}

lpNK_MP4 NK_MP4_DemuxerOpen(char *path)
{
	FILE *fd=fopen(path,"r+");
	if(fd==NULL){
		printf("open file failed\n");
		return NULL;
	}
	lpMP4 hand=malloc(sizeof(stMP4));
	if(hand==NULL)
	{
		printf("malloc hand failed\n");
		fclose(fd);
		return NULL;
	}
	memset(hand,0,sizeof(stMP4));
	hand->fd=fd;
	hand->video_pes_mark=TS_PID_VIDEO;
	hand->audio_pes_mark=TS_PID_AUDIO;
	hand->pmt_type=TS_PID_PMT;
	hand->pat_type=0X00;
	hand->video_type=TS_PMT_STREAMTYPE_H264_VIDEO;
	hand->audio_type=TS_PMT_STREAMTYPE_AAC_AUDIO;
	hand->tmp_video=NULL;
	hand->tmp_audio=NULL;
	return (lpNK_MP4)hand;
}

int  NK_MP4_DemuxerClose(lpNK_MP4 demux){

	lpMP4 mp4demuxer=(lpMP4)demux;
	if(mp4demuxer->tmp_audio)
	{
		free(mp4demuxer->tmp_audio);
		mp4demuxer->tmp_audio=NULL;
	}
	if(mp4demuxer->tmp_video)
	{
		free(mp4demuxer->tmp_video);
		mp4demuxer->tmp_video=NULL;
	}
    if(mp4demuxer->fd)
    {
        fclose(mp4demuxer->fd);
        mp4demuxer->fd = NULL;
    }
	if(demux){
		free(demux);
		demux=NULL;
	}
	return 0;
}




int MPEGTS_PES_framesize(MPEGTS_PES* _pes)
{

	int size=MPEGTS_PES_PES_PACKET_LENGTH(_pes);
	size=size-sizeof(PES_OPTION_S)-sizeof(PES_PTS_S);
	int dts_pts=_pes->PTS_DTS_flag;
	if(dts_pts==3)
		size-=sizeof(PES_PTS_S);
	return size;
}



int  MPEGTS_PMT_ENTRY_get_type(MPEGTS_PMT_ENTRY* _entry,lpMP4 mp4demux)
{

	int type= _entry->stream_type;
	if(type==TS_PMT_STREAMTYPE_H265_VIDEO||type==TS_PMT_STREAMTYPE_H264_VIDEO)
	{
		int video_pes=MPEGTS_PMT_ENTRY_ELEMENTARY_PID(_entry);
			if(video_pes!=mp4demux->video_pes_mark)
			{
				int oldmark=mp4demux->video_pes_mark;
				mp4demux->video_pes_mark=video_pes;
				printf("video_pes_mark pes mark change old 0x%04x-> new 0x%04x\n",oldmark,mp4demux->video_pes_mark);

			}
		mp4demux->video_type=type;
	}
	if(type==TS_PMT_STREAMTYPE_g711_AUDIO||type==TS_PMT_STREAMTYPE_AAC_AUDIO)
	{
		int audio_pes=MPEGTS_PMT_ENTRY_ELEMENTARY_PID(_entry);
			if(audio_pes!=mp4demux->audio_pes_mark)
			{
				int oldmark=mp4demux->audio_pes_mark;
				mp4demux->audio_pes_mark=audio_pes;
				printf("audio_pes_mark pes mark change old 0x%04x-> new 0x%04x\n",oldmark,mp4demux->audio_pes_mark);

			}
			mp4demux->audio_type=type;
	}

	return type;

}

void MPEGTS_PMT_got_type(MPEGTS_PMT* _pmt,lpMP4 mp4demux)
{
	int crc_len = (MPETTS_PMT_SECTION_LENGTH(_pmt) + 3 - 4);//+len之前的3个字节 -crc32_len
	int entry_count = (MPETTS_PMT_SECTION_LENGTH(_pmt) + 3 - 12 - 4) / sizeof(MPEGTS_PMT_ENTRY);//+len之前的3个字节 -PMT包头 -crc32_len
	int i;
	for(i = 0;  i< entry_count; i++)
	{
		MPEGTS_PMT_ENTRY* p_pmt_entry = NULL;
		p_pmt_entry = (MPEGTS_PMT_ENTRY*)(_pmt+1)+i;
		MPEGTS_PMT_ENTRY_get_type(p_pmt_entry,mp4demux);

	}
}

 u_int64_t _MPEGTS_DTS_PTS_VALUE(MPEGTS_DTS_PTS* _ptr)
 {
	 u_int64_t ret = (_ptr->value_32_30 << 30) | (_ptr->value_29_22 << 22) | (_ptr->value_21_15 << 15) | (_ptr->value_14_07 << 7) | (_ptr->value_06_00);
     ret = ret*100/9;
	 return ret;
 }

 int MPEGTS_PES_get_timestamp(MPEGTS_PES* _pes,unsigned long long * ts)
{
	int gotts=0;
	MPEGTS_DTS_PTS* p_pts_or_dts = NULL;
	switch(_pes->PTS_DTS_flag)
	{
	case 0://00 no PTS or DTS data present
		break;
	case 1://01 is forbidden
		break;
	case 2://10 only pts
	case 3://11 pts and dts
		p_pts_or_dts = (MPEGTS_DTS_PTS*)(_pes + 1);
		*ts= MPEGTS_DTS_PTS_VALUE(p_pts_or_dts);
		gotts=1;
		break;
	default:
		break;
	}
	return gotts;
}
int MPEGTS_get_time(MPEGTS_PACKET* _packet,uint64_t *tstime,FRAME_SLICE_E *slicetype,lpMP4 mp4demux)
 {
		if (!IS_MPEG_HEAD(_packet))
		{
			printf("not a mepgts_head\n");//fixme
			return -3;
		}
		int pid=MPEG_PID(_packet);
		//printf("PID						  =0x%04x\n",
	//			pid);
		int siez=0;
		int ts_skip=1;
		int is_key=0;
		if(pid==mp4demux->pat_type)
		{
			MPEGTS_PAT* p_pat = NULL;
			p_pat = (MPEGTS_PAT*)&_packet->data[1];//field_len
			//MPEGTS_PAT_print(p_pat);
		}
		if(pid==mp4demux->pmt_type)
		{
			MPEGTS_PMT* p_pmt = NULL;
			p_pmt = (MPEGTS_PMT*)&_packet->data[1];//field_len
			MPEGTS_PMT_got_type(p_pmt,mp4demux);
		}
		MPEGTS_PES* p_pes = NULL;
		uint64_t p_pes_ts=0;
		MPEGTS_ADAPTATION_FIELD* p_adaptation_field= NULL;
		if(pid==mp4demux->video_pes_mark)
		{
			switch(_packet->adaptation_field_control)
			{
				case 1:
					p_pes = (MPEGTS_PES*)&_packet->data[0];
					if(IS_MPEGTS_PES_HEAD(p_pes))
					{
						unsigned char* p_data = (unsigned char*)&_packet->data[0];
						int pes_size=MPEGTS_PES_PES_PACKET_LENGTH(p_pes);
						int size=MPEGTS_PES_framesize(p_pes);
						int head_size=sizeof(PES_HEAD_S);
						int bewteen=pes_size-size;
						p_data+=bewteen;
						p_data+=head_size;
						if(p_data[3]==0x01&&p_data[4]==0x09&&p_data[5]==0xf0)//only add by lwj //I OR P
						{
							*slicetype=FRAME_SLICE_P;
						}
						if(p_data[3]==0x01&&p_data[4]==0x09&&p_data[5]==0xff)//only add by lwj //I OR P
						{
							*slicetype=FRAME_SLICE_VI;
						}
						MPEGTS_PES_get_timestamp(p_pes,tstime);
						ts_skip=0;
					}
				break;
				case 3:
					p_adaptation_field = (MPEGTS_ADAPTATION_FIELD*)&_packet->data[0];
					p_pes = (MPEGTS_PES*)&_packet->data[p_adaptation_field->adaptation_field_length + 1];
					if(IS_MPEGTS_PES_HEAD(p_pes))
					{
						MPEGTS_PES_get_timestamp(p_pes,tstime);
						ts_skip=0;
						*slicetype=FRAME_SLICE_I;
					}
				break;
			}
		}

		return ts_skip;
}
int NK_MP4_seek(lpNK_MP4 demuxer,FRAME_SLICE_E target_slicetype,uint64_t tstime_us)
{
	if(demuxer==NULL){
		printf("demux is NULL!!!\n");
		return -1;
	}
	lpMP4 mp4demux=(lpMP4)demuxer;
	uint64_t t_now=mp4demux->v_ts;
	char buf_tmp[189];
	long  cur_pos=ftell(mp4demux->fd);
	int result=0;
	uint64_t t_ts=0;
	FRAME_SLICE_E slice_type;

	if(t_now>tstime_us)
	{
		printf("now pos is %llu target :%llu goback\n",t_now,tstime_us);
		fseek(mp4demux->fd,0,SEEK_SET);
	}

		while(1)
		{
			int ret=fread(buf_tmp,1,188,mp4demux->fd);
			if(ret<188){
				printf("[%s:%d]file finished size:%d!!!\n",__func__,__LINE__,ret);
				result=NK_MP4_EOF;
				break;
			}
			int needskip=MPEGTS_get_time((MPEGTS_PACKET*)buf_tmp, &t_ts, &slice_type, mp4demux);
			if(needskip==0)
			{

				if(t_ts>=tstime_us)
				{
					printf("seek pos is %llu target :%llu iskey:%d\n",t_ts,tstime_us,slice_type);
					if(slice_type != target_slicetype)
					{
						continue;
					}
					result=0;
					fseek(mp4demux->fd,-188,SEEK_CUR);
					break;
				}
			}
			if(needskip<0)
			{
				printf("NOT TS HEAD file error!!!!\n");
				result=-1;
				break;
			}
		}
		return result;



}
int MPEGTS_dentifi(MPEGTS_PACKET* _packet,int* frametype,lpMP4 mp4demux)
{
	if (!IS_MPEG_HEAD(_packet))
	{
		printf("not a mepgts_head\n");
		return -3;
	}
	int pid=MPEG_PID(_packet);
	//printf("PID                         =0x%04x\n",
//			pid);
	int siez=0;

	if(pid==mp4demux->pat_type)
	{
		MPEGTS_PAT* p_pat = NULL;
		p_pat = (MPEGTS_PAT*)&_packet->data[1];//field_len
		//MPEGTS_PAT_print(p_pat);
	}
	if(pid==mp4demux->pmt_type)
	{
		MPEGTS_PMT* p_pmt = NULL;
		p_pmt = (MPEGTS_PMT*)&_packet->data[1];//field_len
		MPEGTS_PMT_got_type(p_pmt,mp4demux);
	}
	if(pid==mp4demux->video_pes_mark||pid==mp4demux->audio_pes_mark)
	{

		int isvideo=0;
		if(pid==mp4demux->video_pes_mark)
			isvideo=1;
		MPEGTS_PES* p_pes = NULL;
		MPEGTS_ADAPTATION_FIELD* p_adaptation_field = NULL;
		unsigned char* p_data = NULL;
		int data_len = 0;
		int finnal=1;
		int isVI=0;
		//00空分组没有调整字段,解码器不进行处理
		//01仅含有效负载
		//10仅含调整字段
		switch(_packet->adaptation_field_control)
		{
		case 0:
			break;
		case 1:
			p_data = (unsigned char*)&_packet->data[0];
			data_len = 188 - 4;//head_size
//			MPEGTS_DATA_print(p_data, data_len);
			p_pes = (MPEGTS_PES*)&_packet->data[0];
			if(IS_MPEGTS_PES_HEAD(p_pes))//need fix it
			{
				int pes_size=MPEGTS_PES_PES_PACKET_LENGTH(p_pes);
				int size=MPEGTS_PES_framesize(p_pes);
				int head_size=sizeof(PES_HEAD_S);
				int bewteen=pes_size-size;
				data_len-=head_size;
				data_len -=bewteen;
				p_data+=bewteen;
				p_data+=head_size;
				if(isvideo){
					if(p_data[3]==0x01&&p_data[4]==0x09&&p_data[5]==0xf0)//only add by lwj //I OR P
					{
						mp4demux->slicetype=FRAME_SLICE_P;
						data_len -=6;
						p_data+=6;
					}
					if(p_data[3]==0x01&&p_data[4]==0x09&&p_data[5]==0xff)//only add by lwj //I OR P
					{
						isVI=1;
						mp4demux->slicetype=FRAME_SLICE_VI;
						data_len -=6;
						p_data+=6;
					}

				}
				int have_ts;
				if(isvideo){
				have_ts=MPEGTS_PES_get_timestamp(p_pes,&mp4demux->v_ts);
				}else{
				have_ts=MPEGTS_PES_get_timestamp(p_pes,&mp4demux->a_ts);
				}
				if(have_ts==0)
					printf("------------nodts_nopts\n");
			//	printf("--size:%d bewteen%d\n",size,bewteen);
			}
			if(isvideo){
				memcpy(mp4demux->tmp_video+mp4demux->video_post,p_data,data_len);
				mp4demux->video_post+=data_len;
			}else{
				memcpy(mp4demux->tmp_audio+mp4demux->audio_post,p_data,data_len);
				mp4demux->audio_post+=data_len;
			}
			break;
		case 2:
			printf("pure emtyp ts!!!!!!!!!!!!!!!!!!\n");//need fix it
			p_adaptation_field = (MPEGTS_ADAPTATION_FIELD*)&_packet->data[0];
			p_data = (unsigned char*)&_packet->data[0];
			p_data += p_adaptation_field->adaptation_field_length + 1;
			data_len = 188 - 4 - 1 - p_adaptation_field->adaptation_field_length;//head_size+field_len+field_size
			if(isvideo){
				memcpy(mp4demux->tmp_video+mp4demux->video_post,p_data,data_len);
				mp4demux->video_post+=data_len;
			}else{
				memcpy(mp4demux->tmp_audio+mp4demux->audio_post,p_data,data_len);
				mp4demux->audio_post+=data_len;
			}

		//	MPEGTS_ADAPTATION_FIELD_print(p_adaptation_field);
			break;
		case 3:

			p_adaptation_field = (MPEGTS_ADAPTATION_FIELD*)&_packet->data[0];
			//MPEGTS_ADAPTATION_FIELD_print(p_adaptation_field);
			p_data = (unsigned char*)&_packet->data[0];
			p_data += p_adaptation_field->adaptation_field_length + 1;
			data_len = 188 - 4 - 1 - p_adaptation_field->adaptation_field_length;//head_size+field_len+field_size
			//printf("----data_len%d\n",data_len);
			p_pes = (MPEGTS_PES*)&_packet->data[p_adaptation_field->adaptation_field_length + 1];
			int size=0;
			if(IS_MPEGTS_PES_HEAD(p_pes))
			{

				int pes_size=MPEGTS_PES_PES_PACKET_LENGTH(p_pes);
				size=MPEGTS_PES_framesize(p_pes);
				int bewteen=pes_size+-size;
				int head_size=sizeof(PES_HEAD_S);
				data_len-=head_size;
				data_len -=bewteen;
				p_data+=head_size;
				p_data+=bewteen;
				if(isvideo){
					if(p_data[3]==0x01&&p_data[4]==0x09&&p_data[5]==0xf0)//only add by lwj
					{
						data_len -=6;
						p_data+=6;
					}
				}
				int have_ts;
				if(isvideo){
					have_ts=MPEGTS_PES_get_timestamp(p_pes,&mp4demux->v_ts);
				}else{
					have_ts=MPEGTS_PES_get_timestamp(p_pes,&mp4demux->a_ts);
				}
				if(have_ts==0)
					printf("------------nodts_nopts\n");
				if(isvideo)
				{
					mp4demux->slicetype=FRAME_SLICE_I;
				}
			//	printf("---pes_size:%d  data_len%d----key--size:%d bewteen%d\n",pes_size,data_len,size,bewteen);
				finnal=0;
			}
			if(isvideo){
				memcpy(mp4demux->tmp_video+mp4demux->video_post,p_data,data_len);
				mp4demux->video_post+=data_len;
			}else{
				memcpy(mp4demux->tmp_audio+mp4demux->audio_post,p_data,data_len);
				mp4demux->audio_post+=data_len;
			}
			if(size>0&&size<184){
				finnal=1;
			}
			if(finnal){
				if(isvideo){
				siez=mp4demux->video_post;
				}
				else{
				siez=mp4demux->audio_post;
				}
			}
			break;
		default:
			break;
		}
		if(isvideo){
			*frametype = mp4demux->video_type;
		}
		else{
			*frametype = mp4demux->audio_type;
		}

	}
	return siez;

}
int NK_MP4_Demux_BUF_REPLACE(lpNK_MP4  demuxer,void *tmpvideobuf,void *tmpaudiobuf)
{
	if(demuxer==NULL){
		return -1;
	}
	lpMP4 mp4demuxer=(lpMP4)demuxer;
	if(mp4demuxer->tmp_audio!=NULL)
	{
		printf("audiobuf already exsit !!!\n");
		return -1;
	}
	if(mp4demuxer->tmp_video!=NULL)
	{
		printf("videobuf already exsit!!!\n");
		return -1;
	}
	mp4demuxer->tmp_audio=tmpaudiobuf;
	mp4demuxer->tmp_video=tmpvideobuf;
	return 1;

}
int NK_MP4_BUF_INIT_CHECK(lpMP4 demuxer)
{
	if(demuxer->tmp_audio==NULL)
	{
		printf("malloc default audio buf size:%d\n",DEFAULT_AUDIO_BUF_SIZE);
		demuxer->tmp_audio=malloc(DEFAULT_AUDIO_BUF_SIZE);
		memset(demuxer->tmp_audio,0,DEFAULT_AUDIO_BUF_SIZE);
	}
	if(demuxer->tmp_video==NULL)
	{
		printf("malloc default video buf  size:%d\n",DEFAULT_VIDEO_BUF_SIZE);
		demuxer->tmp_video=malloc(DEFAULT_VIDEO_BUF_SIZE);

		memset(demuxer->tmp_video,0,DEFAULT_VIDEO_BUF_SIZE);
	}
	if(demuxer->tmp_video==NULL && demuxer->tmp_audio==NULL)
	{
		printf("[%s:%d]malloc default video buf  failed\n",__func__,__LINE__);
		return -3;
	}
	return 0;

}

int NK_MP4_DemuxNext(lpNK_MP4 demuxer, void **buf, uint32_t * bufSize, int *frameType, uint64_t *ptsUs,FRAME_SLICE_E * slicetype){
	char buf_tmp[189];
	lpMP4 mp4demuxer=(lpMP4)demuxer;
	int result =0;

	int buf_ret=NK_MP4_BUF_INIT_CHECK(demuxer);
	if(buf_ret)
	{
		printf(" error:malloc audiobuf or videobuf fail!!!!\n");
		return NK_MP4_MALLOC_FAIL;
	}
	while(1)
	{
		int ret=fread(buf_tmp,1,188,mp4demuxer->fd);//need fix it
		if(ret<188)
		{
			printf("[%s:%d|size:%d]file finished !!!\n",__func__,__LINE__,ret);
			result=NK_MP4_EOF;
			break;
		}
		int frame_type=0;
		int size=MPEGTS_dentifi((MPEGTS_PACKET*)buf_tmp,&frame_type,mp4demuxer);//send 188 to demux
		//printf("frame_type=0x%x \n", frame_type);
		if(size>0)
		{
				if(frame_type==TS_PMT_STREAMTYPE_H264_VIDEO||frame_type==TS_PMT_STREAMTYPE_H265_VIDEO)
				{

					*slicetype=mp4demuxer->slicetype;

					*ptsUs=mp4demuxer->v_ts;
				//	mp4demuxer->v_ts=0;
					mp4demuxer->video_post=0;
					*frameType=frame_type;
					*bufSize=size;
					*buf=mp4demuxer->tmp_video;
					}
				else//audio
				{
					*ptsUs=mp4demuxer->a_ts;
				//	mp4demuxer->a_ts=0;
					mp4demuxer->audio_post=0;
					*frameType=frame_type;
					* bufSize=size;
					*buf=mp4demuxer->tmp_audio;
				}
				result=0;
				break;
		}
	}
	return result;
}
/*
1- lpNK_MP4 demuxer：mp4句柄,  void *thumbnailbuf 存储缩略图的buf,外部申请，最大128K
2- 成功返回图片大小 > 0
3- 失败返回 < 0
*/

int NK_MP4_GetJpeg_Thumbnail(lpNK_MP4 demuxer,void *thumbnailbuf)
{
	if(demuxer==NULL)
	{
		printf("NO MP4 demuxer!!!\n");
		return -1;

	}
	if(thumbnailbuf==NULL)
	{
		printf("please malloc a buf to save jpeg\n");
		return -1;
	}
	int ret=-1;
	char * buf=NULL;
	int bufsize=0;
	int frameType=0;
	uint64_t ptsUs=0;
	FRAME_SLICE_E slice_type=0;

	ret=NK_MP4_seek(demuxer,FRAME_SLICE_I,0);
	if(ret<0)
	{
		printf("can't find keyframe for thumbnail\n");
		return -1;
	}
	ret=NK_MP4_DemuxNext(demuxer,(void**)&buf,(uint32_t*)&bufsize,&frameType,&ptsUs,&slice_type);
	if(ret<0)
	{
		printf("can't get the key frame\n");
		return -1;
	}
	int frame_Type=0x00000000;//NK_ENC_BUF_DATA_H264
	if(frameType==TS_PMT_STREAMTYPE_H265_VIDEO)
	{
		frame_Type=0x00000002;//NK_ENC_BUF_DATA_H265
	}
#if 0
	ret=NK_ChangeIFrame2Jpeg(buf,bufsize,thumbnailbuf,128*1024,frame_Type);//128k max jpeg
	if(ret<0)
	{
		printf("can't change i frame to  thumbnail\n");
		return -1;
	}
#endif
	return ret;//jpeg_size
}


#ifdef tsdemux_demo
int main(int argc,char *argv[])
{
	if(argc==1 || argc>2) {
	printf("请输入想要编辑的文件名如:./edit fillen");
	}
	if(argc==2) {
	printf("编辑 %sn",argv[1]);
	}
	char *tmp_v=malloc(1024*1024);
	char *tmp_a=malloc(4096);
	stFILEDESCIP_S filedes;
	memset(&filedes,0,sizeof(filedes));
	lpNK_MP4 MP4HAND=NK_MP4_DemuxerOpen(argv[1]);
	NK_MP4_Demux_BUF_REPLACE(MP4HAND, tmp_v,tmp_a);
	lpNK_MP4 MP4HAND2=fopen("123.h264","w+");
	lpNK_MP4 MP4HAND3=fopen("123.aac","w+");
	if(MP4HAND==NULL){
	printf("---------------HAND NULL\n");
	return 0;
	}
	int ret=NK_MP4_getinfo(MP4HAND,&filedes);
	printf("------------RELOSUON:%dx%d FRAMERATE:%d type:%d\n",filedes.u32Height,filedes.u32Width,filedes.fFrameRate,filedes.enVideoType);
	char * buf=NULL;
	int bufsize=0;
	int frameType=0;
	uint64_t ptsUs=0;
	FRAME_SLICE_E slice_type=0;
	int seektest=0;
	while(1)
	{
		int ret=NK_MP4_DemuxNext(MP4HAND,(void**)&buf,&bufsize,&frameType,&ptsUs,&slice_type);
		if(ret==0)
		{
			printf("------------frametype%d - %llu\n-----slice type:%d\n",frameType,ptsUs,slice_type);
			if(frameType==TS_PMT_STREAMTYPE_H264_VIDEO||frameType==TS_PMT_STREAMTYPE_H265_VIDEO)
			{
				printf("viode bufseiz:%d\n",bufsize);
				fwrite(buf,bufsize,1,MP4HAND2);
			}
			if(frameType==TS_PMT_STREAMTYPE_AAC_AUDIO||frameType==TS_PMT_STREAMTYPE_g711_AUDIO)
			{
				printf("audio bufseiz:%d\n",bufsize);
				fwrite(buf,bufsize,1,MP4HAND3);
			}

		}
		if(seektest==0&&ptsUs>5800000)
		{
			printf("find it\n");
			seektest=1;
			NK_MP4_seek(MP4HAND,FRAME_SLICE_VI,0);
		}
		if(ret==NK_MP4_EOF)
		{
			printf("File end game over\n");
				break;
		}
		if(ret==NK_MP4_MALLOC_FAIL)
		{
			printf("MALLOC FAIL NO SPACE\n");
				break;
		}

	}
	NK_MP4_DemuxerClose(MP4HAND);
	fclose(MP4HAND2);
	fclose(MP4HAND3);
	return 0;
}
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <NkUtils/assert.h>
#include <NkUtils/macro.h>
#include <NkUtils/json.h>
#include <base/ja_process.h>
#include <dirent.h>
#include <pthread.h>
#include "app_debug.h"
#include "generic.h"

#include <mp4demux/hi_demuxer.h>

#include "tfcard_play.h"
#include "../netsdk_def.h"
#include "../netsdk.h"
#include "rl_pack.h"

typedef struct time_slot_t{
	NK_Int index;
	NK_Char type[32];
	NK_Int beginSec;
	NK_Int endSec;
}stTimeSlot,*lpTimeSlot;

typedef struct node_t{
	stTimeSlot timeSlot;
	struct node_t *next;
}stHistoryNode,*lpHistoryNode;

typedef struct play_node_t {
	NK_Int index;
	NK_Int beginSec;
	NK_Int endSec;
	struct play_node_t *next;
}stPlayNode,*lpPlayNode;

typedef struct TFcard_Play_t {
	NK_Char playType[32];
	NK_Char recordPath[64];
	NK_Char datePath[32];
	lpPlayNode head;//播放列表头指针
	lpPlayNode p1;//当前播放位置
	FILE *fid;
}stTFcard_Play,*lpTFcard_Play;

lpTFcard_Play _player = NULL;

static NK_Void utc_to_times(NK_UTC1970 utc, NK_PChar strDate, NK_PChar strTime)
{
	struct tm setTm = {0};

	// use utc time
	gmtime_r((time_t *)(&utc), &setTm);

	sprintf(strDate, "%04d%02d%02d", setTm.tm_year + 1900, setTm.tm_mon + 1, setTm.tm_mday);
	sprintf(strTime, "%02d%02d%02d", setTm.tm_hour, setTm.tm_min, setTm.tm_sec);
}

static NK_Void utc_plus1d_to_begintimes(NK_UTC1970 utc, NK_PChar strDate, NK_PChar strTime)
{
	struct tm setTm = {0};

	// 1 day later
	utc += 24 * 3600;

	// use utc time
	gmtime_r((time_t *)(&utc), &setTm);

	sprintf(strDate, "%04d%02d%02d", setTm.tm_year + 1900, setTm.tm_mon + 1, setTm.tm_mday);
	sprintf(strTime, "%02d%02d%02d", 0, 0, 0);
}

static NK_Void times_to_utc(NK_UTC1970 *utc, NK_PChar strDate, NK_PChar strTime)
{
	struct tm setTm = {0};

	sscanf(strDate, "%04d%02d%02d",&setTm.tm_year,&setTm.tm_mon,&setTm.tm_mday);
	sscanf(strTime, "%02d%02d%02d",&setTm.tm_hour, &setTm.tm_min, &setTm.tm_sec);

	setTm.tm_year -= 1900;
	setTm.tm_mon -= 1;

	// use utc time
	*utc = timegm(&setTm);
}

static NK_Int is_date_exist(NK_PChar strDate,NK_PChar recordPath,NK_PChar rightDatePath)
{
	DIR *dirPtr = NULL;
	struct dirent *entry;
	NK_Int index = 0;//文件夹索引值,用于组建正确文件夹名
	NK_Char tmpDate[12] = {0};

	if(NULL == (dirPtr = opendir(recordPath))){
		APP_TRACE("open dir error : path = %s",recordPath);
		return -1;
	}else{
		while(entry = readdir(dirPtr)){
			if((0 == strcmp(entry->d_name,".")) || (0 == strcmp(entry->d_name,".."))){
				continue;
			}
			else{
				memset(tmpDate, 0, sizeof(tmpDate));
				if(2 == sscanf(entry->d_name,"%d-%s",&index,tmpDate)){
					if(0 == strcmp(tmpDate,strDate)){
						sprintf(rightDatePath, "%s/%04d-%s", recordPath,index,tmpDate);
						closedir(dirPtr);
						return 0;
					}
				}
			}
		}
		closedir(dirPtr);
	}

	return -1;
}

static void printf_all_historyNode(lpHistoryNode head)
{
	lpHistoryNode p1 = head;
	NK_Int i = 0;

  	printf("historyNode printf is :\n");
    while(p1 != NULL)  
    {
        printf("%d : index -> %d, beginSec -> %d, endSec -> %d \n",
			i,p1->timeSlot.index,p1->timeSlot.beginSec,p1->timeSlot.endSec);
        p1 = p1->next;
        i++;
    }
}

static NK_Int free_all_historyNode(lpHistoryNode head)
{
	lpHistoryNode p1;

    while(head != NULL)
    {
        p1 = head->next;
        free(head);
        head = p1;
    }

	return 0;
}

static NK_Boolean is_historyIndex_exist(lpHistoryNode head,NK_Int index,NK_Int beginSec,NK_Int endSec)
{
	lpHistoryNode p1;
	if(head == NULL){
		return NK_False;
	}
	p1 = head;
	while(p1 != NULL){
		if(p1->timeSlot.index == index){
			if(beginSec <= p1->timeSlot.beginSec){
				p1->timeSlot.beginSec = beginSec;
			}
			if(endSec >= p1->timeSlot.endSec){
				p1->timeSlot.endSec = endSec;
			}
			return NK_True;
		}
		p1 = p1->next;
	}

	return NK_False;
}

static lpHistoryNode insert_new_historyNode(lpHistoryNode head,NK_Int index,NK_Int beginSec,NK_Int endSec)
{
	lpHistoryNode p1,p2;
	lpHistoryNode newNode = (lpHistoryNode)malloc(sizeof(stHistoryNode));

	if(NULL == newNode){
		return head;
	}
	newNode->timeSlot.index = index;
	newNode->timeSlot.beginSec = beginSec;
	newNode->timeSlot.endSec = endSec;
	newNode->next = NULL;

	if(head == NULL){
		head = newNode;
		return head;
	}
	if(newNode->timeSlot.index < head->timeSlot.index){
		p1 = head;
		head = newNode;
		newNode->next = p1;
		return head;
	}
	p1 = head;
	p2 = head;

	while(p1 != NULL){
		if(newNode->timeSlot.index < p1->timeSlot.index){
			p2->next = newNode;
			newNode->next = p1;
			return head;
		}
		p2 = p1;
		p1 = p1->next;
	}	
	p2->next = newNode;

	return head;
}

static lpHistoryNode discover_history_file(lpHistoryNode head,NK_Int index,NK_Int beginSec,NK_Int endSec)
{
	if(!is_historyIndex_exist(head,index,beginSec,endSec)){
		head = insert_new_historyNode(head,index,beginSec,endSec);
	}

	return head;
}

static lpHistoryNode get_historyNode_in_date(lpHistoryNode head, NK_PChar datePath)
{
	NK_Int index = 0;
	NK_Int beginSec = 0;
	NK_Int endSec = 0;
	DIR *dirPtr = NULL;
	struct dirent *entry;

	APP_TRACE("get history in datePath : %s",datePath);
	if(NULL == (dirPtr = opendir(datePath))) {
		APP_TRACE("open dir error, path : %s",datePath);
		return NULL;
	}else{
		while(entry = readdir(dirPtr)){
			if((0 == strcmp(entry->d_name,".")) || (0 == strcmp(entry->d_name,".."))){
				continue;
			}
			else{
				if(3 == sscanf(entry->d_name,"%d-%d-%d",&index,&beginSec,&endSec)){
					//printf("found history file : %s\n",entry->d_name);
					head = discover_history_file(head,index,beginSec,endSec);
				}
			}
		}
		closedir(dirPtr);
	}

	return head;
}

static lpHistoryNode tfcard_get_history_node(lpHistoryNode head, NK_PChar strDate, NK_PChar recordPath)
{
	NK_Char rightDatePath[128] = {0};
	lpHistoryNode got_head = NULL;

	if(0 != is_date_exist(strDate, recordPath, rightDatePath)) {
		APP_TRACE("no record on this date, recordPath : %s, date : %s", recordPath, strDate);
		return NULL;
	}

	APP_TRACE("getting historyNode in date begin, rightDatePath : %s", rightDatePath);

	// head got by get_historyNode_in_date must be free!
	got_head = get_historyNode_in_date(head, rightDatePath);
	if(NULL == got_head) {
		APP_TRACE("get historyNode in date is null, rightDatePath : %s",rightDatePath);
		return NULL;
	}

	return got_head;
}

NK_Int TFCARD_get_history(NK_UTC1970 beginUtc, NK_UTC1970 endUtc,
                          NK_PChar type, lpTFCARD_History_List historyList, NK_Int startIndex, NK_Int *historyCnt, NK_PChar mountPath)
{
	NK_Char strDate[12] = {0};
	NK_Char strTime[12] = {0};
	NK_Char recordPath[64] = {0};
	ST_NSDK_SYSTEM_TIME systime = {0};

	lpHistoryNode tmp_head = NULL;
	lpHistoryNode recs_head = NULL;
	NK_UTC1970 btmUtc = 0;
	NK_UTC1970 etmUtc = 0;
	int i = 0;
	int j = 0;

	// 标记两天各自是否存在录像
	bool startday_have_recs = false;
	bool later1day_have_recs = false;

	NK_EXPECT_VERBOSE_RETURN_VAL((historyList != NULL && mountPath != NULL && strlen(mountPath) != 0),-1);
	if(!type){
		type = "default";
	}

	// 录像所在目录
	snprintf(recordPath, sizeof(recordPath), "%s/record", mountPath);

	// 搜索 utc 日期的录像。
	utc_to_times(beginUtc, strDate, strTime);
	APP_TRACE("get history : utc -> %d, Date -> %s, Time -> %s",beginUtc,strDate,strTime);
	tmp_head = tfcard_get_history_node(tmp_head, strDate, recordPath);
	recs_head = tmp_head;

	if (NULL != recs_head) {

		startday_have_recs = true;
		printf_all_historyNode(recs_head);

		btmUtc = 0;
		etmUtc = 0;
		i = 0;
		j = 0;

		//根据查询的链表生成返回上层的历史录像列表
		while (tmp_head != NULL && i < *historyCnt) {

			if (j < startIndex) {
				tmp_head = tmp_head->next;
				j++;
				continue;
			}

			//endUtc
			snprintf(strTime, sizeof(strTime), "%02d%02d%02d",
					 tmp_head->timeSlot.endSec/3600,(tmp_head->timeSlot.endSec%3600)/60,tmp_head->timeSlot.endSec%60);
			times_to_utc(&etmUtc,strDate,strTime);

            if (beginUtc < etmUtc) {
				//beginUtc
				snprintf(strTime, sizeof(strTime), "%02d%02d%02d",
						 tmp_head->timeSlot.beginSec/3600,(tmp_head->timeSlot.beginSec%3600)/60,tmp_head->timeSlot.beginSec%60);
				times_to_utc(&btmUtc,strDate,strTime);

				if (endUtc > btmUtc) {
					sprintf(historyList[i].recordType,"%s", type);
					historyList[i].endTm = etmUtc;
					historyList[i].beginTm = btmUtc;
					i++;
				}
            }

			//APP_TRACE("file is : %04d-%05d-%05d",p1->timeSlot.index,p1->timeSlot.beginSec,p1->timeSlot.endSec);
			tmp_head = tmp_head->next;
			j++;
		}

		// 记得释放链表,否则将造成内存泄露
		free_all_historyNode(recs_head);
	}


	// 如果不是 0 时区，还要搜索后一天
	NETSDK_conf_system_get_time(&systime);
	if (0 != systime.greenwichMeanTime) {

		// 搜索 utc 日期后一天的录像
        utc_plus1d_to_begintimes(beginUtc, strDate, strTime);
		APP_TRACE("get 1 day later history: utc -> %d, Date -> %s, Time -> %s",beginUtc, strDate, strTime);
		tmp_head = NULL;
		tmp_head = tfcard_get_history_node(tmp_head, strDate, recordPath);

		recs_head = tmp_head;
		if (NULL != recs_head) {

			later1day_have_recs = true;
			printf_all_historyNode(recs_head);

			// 变量 i 要保留前面的值
			btmUtc = 0;
			etmUtc = 0;
//			j = 0;

			//根据查询的链表生成返回上层的历史录像列表
			while (tmp_head != NULL && i < *historyCnt) {

				// 第二天的 starIndex 肯定是 0
//				if (j < startIndex) {
//					tmp_head = tmp_head->next;
//					j++;
//					continue;
//				}

				//beginUtc
				memset(strTime, 0, sizeof(strTime));
				snprintf(strTime, sizeof(strTime),"%02d%02d%02d",
						 tmp_head->timeSlot.beginSec/3600,(tmp_head->timeSlot.beginSec%3600)/60,tmp_head->timeSlot.beginSec%60);
				times_to_utc(&btmUtc,strDate,strTime);

                if (endUtc > btmUtc) {
                    //endUtc
                    memset(strTime, 0, sizeof(strTime));
                    snprintf(strTime, sizeof(strTime),"%02d%02d%02d",
                             tmp_head->timeSlot.endSec/3600,(tmp_head->timeSlot.endSec%3600)/60,tmp_head->timeSlot.endSec%60);
                    times_to_utc(&etmUtc,strDate,strTime);

                    if (beginUtc < etmUtc) {
                        sprintf(historyList[i].recordType,"%s", type);
                        historyList[i].beginTm = btmUtc;
                        historyList[i].endTm = etmUtc;
                        i++;
                    }
                }

				//APP_TRACE("file is : %04d-%05d-%05d",p1->timeSlot.index,p1->timeSlot.beginSec,p1->timeSlot.endSec);
				tmp_head = tmp_head->next;
//				j++;
			}

			// 记得释放链表,否则将造成内存泄露
			free_all_historyNode(recs_head);
		}
	}

	if (!startday_have_recs && !later1day_have_recs) {
		APP_TRACE("no record found at device local date, utc: %u", beginUtc);
		return -1;
	}

	//确认录像时段个数
	if(NULL != historyCnt){
		*historyCnt = i;
	}

	// 记得释放链表内存。现在是在前面释放了

	return 0;
}

static void printf_all_playNode(lpPlayNode head)
{
	lpPlayNode p1 = head;
	NK_Int i = 0;

  	printf("playNode printf is :\n");
    while(p1 != NULL) {
        printf("%d : playFileName -> %04d-%05d-%05d\n", i, p1->index,p1->beginSec,p1->endSec);
        p1 = p1->next;
        i++;
    }
}

static NK_Int free_all_playNode(lpPlayNode head)
{
	lpPlayNode p1;

    while(head != NULL)
    {
        p1 = head->next;
        free(head);
        head = p1;
    }

	return 0;
}

static NK_Boolean is_playFile_match(NK_Int beginSec, NK_Int endSec, NK_Int startSec)
{
	return (endSec > startSec ? NK_True : NK_False);
}

static lpPlayNode insert_new_playNode(lpPlayNode head,NK_Int index,NK_Int beginSec,NK_Int endSec)
{
	lpPlayNode p1, p2;
	lpPlayNode newNode = (lpPlayNode)malloc(sizeof(stPlayNode));

	if(NULL == newNode){
		return head;
	}
	newNode->index = index;
	newNode->beginSec = beginSec;
	newNode->endSec = endSec;
	newNode->next = NULL;

	if(head == NULL){
		head = newNode;
		return head;
	}

	if(newNode->endSec < head->endSec){
		p1 = head;
		head = newNode;
		newNode->next = p1;
		return head;
	}

	p1 = head;
	p2 = head;
	while(p1 != NULL){
		if(newNode->endSec < p1->endSec){
			p2->next = newNode;
			newNode->next = p1;
			return head;
		}
		p2 = p1;
		p1 = p1->next;
	}
	p2->next = newNode;

	return head;
}

static lpPlayNode discover_play_file(lpPlayNode head, NK_Int index,NK_Int beginSec,NK_Int endSec,NK_Int startSec)
{
	if(is_playFile_match(beginSec, endSec, startSec)){
		head = insert_new_playNode(head, index, beginSec, endSec);
	}
	
	return head;
}

static lpPlayNode get_playNode_in_date(lpPlayNode head,NK_PChar datePath,NK_Int startSec)
{
	NK_Int index = 0;
	NK_Int beginSec = 0;
	NK_Int endSec = 0;
	DIR *dirPtr = NULL;
	struct dirent *entry;

	APP_TRACE("get play in datePath : %s",datePath);
	if(NULL ==(dirPtr = opendir(datePath))) {
		APP_TRACE("open dir error, path : %s",datePath);
		return NULL;
	}else{
		while(entry = readdir(dirPtr)){
			if((0 == strcmp(entry->d_name,".")) || (0 == strcmp(entry->d_name,".."))){
				continue;
			}
			else{
				if(3 == sscanf(entry->d_name,"%d-%d-%d",&index,&beginSec,&endSec)){
//					printf("found play file : %s\n",entry->d_name);
					head = discover_play_file(head,index,beginSec,endSec,startSec);
				}
			}
		}
		closedir(dirPtr);
	}

	return head;
}

static NK_Int play_file_verify(FILE *fid)
{
	stFile_Tail fileTail = {0};
	NK_Size fileSize = 0;
	NK_Int ret = 0;

	NK_EXPECT_VERBOSE_RETURN_VAL((fid != NULL),-1);

	fseek(fid, 0, SEEK_END);
	fileSize = ftell(fid);
	if(fileSize <= sizeof(stFile_Tail)){
		APP_TRACE("fileSize less than fileTailSize : fid->%d, fileSize = %u",fid,fileSize);
		return -1;
	}
	fseek(fid, -sizeof(stFile_Tail),SEEK_END);
	if(sizeof(stFile_Tail) != (ret = fread((NK_PVoid)&fileTail, 1, sizeof(stFile_Tail), fid))){
		APP_TRACE("fread fileTail error : fid->%d, readLen = %d",fid,ret);
		return -2;
	}
	if(fileSize != (fileTail.filesize + sizeof(stFile_Tail))){
		APP_TRACE("fileSize not equal to fileTail's fileSize : fid->%d, fileSize = %u, fileTail's fileSize = %u, sizeOfTail = %d",
			fid,fileSize,fileTail.filesize,sizeof(stFile_Tail));
		return -3;
	}
	fseek(fid, 0, SEEK_SET);

	return 0;
}

static NK_Size find_first_iFramePos(FILE *fid, NK_Int startSec)
{
	NK_Int ret = 0;
	stFile_Tail fileTail = {0};
	NK_Int i = 0;

	if(0 != play_file_verify(fid)){
		APP_TRACE("play file verify err : fid->%d",fid);
		return -1;
	}
	fseek(fid, -sizeof(stFile_Tail), SEEK_END);
	if(sizeof(stFile_Tail) != (ret = fread((NK_PVoid)&fileTail, 1, sizeof(stFile_Tail), fid))){
		APP_TRACE("2fread fileTail error : fid->%d, readLen = %d",fid, ret);
		return -1;
	}
	for(i = 0; i < fileTail.iFrameCnt; i++){
		if(fileTail.iFrameSec[i] >= startSec){
			fseek(fid, fileTail.iFramePos[i], SEEK_SET);
			return 0;
		}
	}

	return -1;
}

static NK_Int read_a_frame(FILE *fid, lpRecord_Frame_Head frameHead, NK_PByte data, NK_Size dataMaxSize)
{
	NK_Int readLen = 0;

	readLen = fread(frameHead, 1, sizeof(stRecord_Frame_Head),fid);
	if(readLen != sizeof(stRecord_Frame_Head)){
		APP_TRACE("read frame head error : fid -> %d, readLen -> %d",fid, readLen);
		return -1;
	}
	if(frameHead->dataSize > dataMaxSize){
		APP_TRACE("frame dataSize large than buffer's size : dataSize = %u, bufferSize = %u",frameHead->dataSize,dataMaxSize);
		return -1;
	}
	readLen = fread(data, 1, frameHead->dataSize, fid);
	if(readLen != frameHead->dataSize){
		APP_TRACE("read data error : fid -> %d, readLen -> %d",fid, readLen);
		return -1;
	}

	return frameHead->dataSize;
}

static NK_Int next_play_file_fid(FILE *fid)
{
	NK_Char fileName[128] = {0};

	while(_player->p1 != NULL){
		memset(fileName, 0, sizeof(fileName));
		snprintf(fileName, sizeof(fileName), "%s/%04d-%05d-%05d",
			_player->datePath,_player->p1->index,_player->p1->beginSec,_player->p1->endSec);
		if(NULL == (_player->fid = fopen(fileName, "rb"))){
			APP_TRACE("open fid error : fileName->%s",fileName);
			_player->p1 = _player->p1->next;
			continue;
		}
		APP_TRACE("open play file : fileName->%s, fid->%d",fileName,_player->fid);

		if(0 != play_file_verify(_player->fid)){
			APP_TRACE("play file verify err : fid->%d",fid);
			fclose(_player->fid);
			_player->fid = NULL;
			_player->p1 = _player->p1->next;
			continue;
		}else{
			_player->p1 = _player->p1->next;
			APP_TRACE("play file fid is ok : fileName->%s, fid->%d",fileName, _player->fid);
			return 0;
		}
	}

	return -1;
}


static NK_Int play_path_init(NK_UTC1970 beginUtc, NK_PChar mountPath)
{
	NK_Char strDate[12] = {0};
	NK_Char strTime[12] = {0};
	
	utc_to_times(beginUtc, strDate, strTime);
	sprintf(_player->recordPath, "%s/record", mountPath);
	APP_TRACE("play path init : utc -> %d, Date -> %s, Time -> %s",beginUtc,strDate,strTime);

	if(0 != is_date_exist(strDate, _player->recordPath, _player->datePath)){
		APP_TRACE("not date on record,recordPath : %s, date : %s",_player->recordPath,strDate);
		return -1;
	}

	return 0;
}

static NK_Int play_head_init(NK_UTC1970 beginUtc)
{
	struct tm beginTm = {0};
	NK_Int startSec = 0;

	// use utc time
	gmtime_r((time_t *)(&beginUtc),&beginTm);
	startSec = (beginTm.tm_hour *3600 + beginTm.tm_min * 60 + beginTm.tm_sec);
	APP_TRACE("play head init : datePath->%s, startSec -> %d", _player->datePath, startSec);

	if(NULL == (_player->head = get_playNode_in_date(_player->head, _player->datePath, startSec))){
		APP_TRACE("play head init : head is null");
		return -1;
	}
	_player->p1 = _player->head;

//	printf_all_playNode(_player->head);

	return 0;
}

static NK_Int play_fid_init(NK_UTC1970 beginUtc)
{
	struct tm beginTm = {0};
	NK_Int startSec = 0;
	NK_Char fileName[128] = {0};

	// use utc time
	gmtime_r((time_t *)(&beginUtc),&beginTm);
	startSec = (beginTm.tm_hour *3600 + beginTm.tm_min * 60 + beginTm.tm_sec);
	APP_TRACE("play fid init : startSec->%d",startSec);

	while(_player->p1 != NULL){
		memset(fileName, 0, sizeof(fileName));
		snprintf(fileName, sizeof(fileName), "%s/%04d-%05d-%05d",
			_player->datePath,_player->p1->index,_player->p1->beginSec,_player->p1->endSec);
		if(NULL == (_player->fid = fopen(fileName, "rb"))){
			APP_TRACE("open fid error : fileName->%s",fileName);
			_player->p1 = _player->p1->next;
			continue;
		}
		APP_TRACE("open play file : fileName->%s, fid->%d",fileName,_player->fid);

		if(0 != find_first_iFramePos(_player->fid, startSec)){
			APP_TRACE("find first iFramePos err : fileName->%s, fid->%d",fileName,_player->fid);
			fclose(_player->fid);
			_player->fid = NULL;
			_player->p1 = _player->p1->next;
			continue;
		}else{
			_player->p1 = _player->p1->next;
			APP_TRACE("play fid init ok : fileName->%s, startSec->%d, fid->%d, pos->%d",fileName, startSec,_player->fid, (NK_Size)ftell(_player->fid));
			return 0;
		}
	}

	return -1;
}

NK_Int TFcard_Play_read_frame(lpRecord_Frame_Head frameHead, NK_PByte data, NK_Size dataMaxSize)
{
	NK_Boolean hasData = NK_False;
	NK_Int dataSize = -1;

	if(_player){
		do{
			if(_player->fid){
				dataSize = read_a_frame(_player->fid,frameHead,data,dataMaxSize);
				if(dataSize > 0){
					hasData = NK_True;
				}
				else{
					if(_player->fid){
						fclose(_player->fid);
					}
					_player->fid = NULL;
					if(0 != next_play_file_fid(_player->fid)){
						APP_TRACE("get next play file fid error");
						return -1;
					}
				}
			}else{
				if(0 != next_play_file_fid(_player->fid)){
					APP_TRACE("get next play file fid error : when fid is NULL");
					return -1;
				}
			}
		}while(!hasData);
	}

	return dataSize;
}


NK_Int TFCARD_Play_start(NK_UTC1970 beginUtc, NK_PChar playType, NK_PChar mountPath)
{
	NK_EXPECT_VERBOSE_RETURN_VAL((mountPath != NULL && strlen(mountPath) != 0),-1);
	if(!playType){
		playType = "default";
	}

	if(!_player){
		_player = calloc(sizeof(stTFcard_Play), 1);
		
		sprintf(_player->playType, "%s", playType);
		if(0 != play_path_init(beginUtc, mountPath)){
			goto STH_ERR;
		}
		APP_TRACE("play path init ok");
		if(0 != play_head_init(beginUtc)){
			goto STH_ERR;
		}
		APP_TRACE("play head init ok");
		if(0 != play_fid_init(beginUtc)){
			goto STH_ERR;
		}
		APP_TRACE("play fid init ok");
		
		return 0;
	}
/**
 *	fixme : 此处是播放功能已被启用,故返回失败. 暂时不考虑多人同时回放视频的情况.
 */	
	return -1;

/**
 * 以下为初始化错误时,清除资源,故返回失败.
 */
STH_ERR:
	if(_player){
		if(_player->fid){
			fclose(_player->fid);
			_player->fid = NULL;
		}
		free_all_playNode(_player->head);
		free(_player);
		_player = NULL;
	}
	
	return -1;
}

NK_Int TFCARD_Play_stop()
{
	if(_player){
		if(_player->fid){
			fclose(_player->fid);
			_player->fid = NULL;
		}
		if(_player->head){
			free_all_playNode(_player->head);
		}
		free(_player);
		_player = NULL;
	}

	return 0;
}


/*
 * New mp4 record Playback
 */

#undef  HI_DEMUX_HANDLE
typedef HI_U32   HI_DEMUX_HANDLE;

#define MS_2_SECOND 1000

#define kREC_PLAY_DIR_NAME "YYYYMMDD"
#define kREC_PLAY_REC_NAME_PRE "HHmmss-00000-T"
#define kREC_PLAY_MAX_SEARCH_RECS 50000

#define kREC_PLAY_DIR_NAME_LEN 8

// "HHmmss-"
#define kREC_PLAY_REC_NAME_1ST_HYPHEN_POS 6
// "HHmmss-00000-"
#define kREC_PLAY_REC_NAME_2ND_HYPHEN_POS 12

typedef struct {
    FILE *file;
} stREC_PLAY_Ctx, *lpREC_PLAY_Ctx;

typedef struct {
    time_t recBeginTs;
    time_t recEndTs;
    char *path;
} stRecInfo, *lpRecInfo;

typedef struct {
	lpTFCARD_History_List recList;
	size_t recListSize;
	size_t recCnt;
} stHistoryRecList, *lpHistoryRecList;

typedef struct {
	bool isOldTypeRec;
    lpRecInfo recList;
    size_t recListSize;
    size_t recCnt;

    time_t firstRecStartSec;
    time_t lastRecEndSec;

	HI_DEMUX_HANDLE fmtHandle;
	HI_U32 curIndex;
	HI_FORMAT_FILE_INFO_S mp4fileFormat;

    int tzOffset;

	int openType;
	unsigned int frmIntervalUs;
    int64_t sentFilesTimeMs;        // time of sent record files
    int64_t curFileSentMs;          // sent time of the current send file
} stPlay_rec_search_ctx, *lpPlay_rec_search_ctx;

typedef int (*rec_play_search_cb_t)(void * const ctx,
									char *datePath, char *fileName,
                                    time_t recBeginTs, time_t recEndTs,
                                    char recType);


static stHistoryRecList gs_cachedHistoryList = {
        .recList = NULL,
        .recListSize = 0,
        .recCnt = 0
};

static pthread_mutex_t gs_cachedListLock = PTHREAD_MUTEX_INITIALIZER;

extern HI_DEMUX_S  g_stFormat_entry ;

static bool rec_play_is_valid_mp4(char *filename, size_t expected_pre_len)
{
	size_t filename_len = strlen(filename);
	if (filename_len < expected_pre_len) {
		return false;
	} else if (filename_len < 4) {
		return false;
	} else {
		if (0 == strcmp(filename + (filename_len - 4), ".mp4")) {
			return true;
		} else {
			return false;
		}
	}
}

// date_str: YYYYMMDD, time_str: HHmmss-00000-T.
// length of date_str: 8, length of time_str: 14, size of tmp_buf: 8+14+1=23
static int rec_play_time_to_ts(char *date_str, char *time_str,
							   char *tmp_buf,
							   time_t *out_begin_ts, time_t *out_end_ts,
							   char *type)
{
	int duration;

	struct tm beginTm;
	time_t beginTs;

	memcpy(tmp_buf, date_str, 8);
	memcpy(tmp_buf + 8, time_str, 14);
	tmp_buf[22] = '\0';

	if ('-' != tmp_buf[kREC_PLAY_DIR_NAME_LEN + kREC_PLAY_REC_NAME_1ST_HYPHEN_POS]) {
		APP_TRACE("invalid time string: %s", tmp_buf);
		return -1;
	}

	if ('-' != tmp_buf[kREC_PLAY_DIR_NAME_LEN + kREC_PLAY_REC_NAME_2ND_HYPHEN_POS]) {
		APP_TRACE("invalid time string: %s", tmp_buf);
		return -1;
	}

	if (EOF == sscanf(tmp_buf, "%04d%02d%02d%02d%02d%02d-%05d-%c",
					  &beginTm.tm_year, &beginTm.tm_mon, &beginTm.tm_mday,
					  &beginTm.tm_hour, &beginTm.tm_min, &beginTm.tm_sec, &duration, type)) {
		APP_TRACE("invalid time string: %s", tmp_buf);
		return -1;
	}

	beginTm.tm_year -= 1900;
	beginTm.tm_mon -= 1;

	beginTs = timegm(&beginTm);
	*out_begin_ts = beginTs;
	*out_end_ts = beginTs + duration;

	return 0;
}

static int get_history_search_cb(void * const ctx,
								  char *datePath, char *fileName,
								  time_t recBeginTs, time_t recEndTs,
								  char recType)
{

    static const int increment = 200;

	lpHistoryRecList const search_ctx = (lpHistoryRecList)ctx;

	lpTFCARD_History_List recList;
	size_t recListSize = search_ctx->recListSize;
	size_t recCnt = search_ctx->recCnt;

    lpTFCARD_History_List newRecList;

//	APP_TRACE("datePath: %s", datePath);
//	APP_TRACE("fileName: %s", fileName);

	if ((recCnt + 1) > recListSize) {
        newRecList = (lpTFCARD_History_List)realloc(search_ctx->recList,
                                                    (recListSize + increment) * sizeof(stTFCARD_History_List));
        if (NULL == newRecList) {
            APP_TRACE("realloc failed!");
            return -1;
        } else {
            search_ctx->recList = newRecList;
            search_ctx->recListSize += increment;
        }
	}

    recList = search_ctx->recList;
    sprintf(recList[recCnt].recordType,"%c", recType);
    recList[recCnt].beginTm = recBeginTs;
    recList[recCnt].endTm = recEndTs;
    search_ctx->recCnt++;
    return 0;
}

static int play_rec_search_cb(void * const ctx,
                              char *datePath, char *fileName,
                              time_t recBeginTs, time_t recEndTs,
                              char recType)
{
//    APP_TRACE("gotEndRec recBeginTs: %ld", recBeginTs);
//    APP_TRACE("gotEndRec recEndTs: %ld", recEndTs);

    static const int increment  = 500;

    lpPlay_rec_search_ctx const search_ctx = (lpPlay_rec_search_ctx)ctx;

	size_t recListSize = search_ctx->recListSize;
	size_t recCnt = search_ctx->recCnt;

    size_t pathBufLen;
    lpRecInfo newListBuf;
    char *pathBuf;

    // RL_PATH/datePath/fileName'\0'
    pathBufLen = strlen(RL_PATH) + 1 + strlen(datePath) + 1 + strlen(fileName) + 1;

    if ((recCnt + 1) > recListSize) {
        newListBuf = (lpRecInfo)realloc(search_ctx->recList,
                                      (recListSize + increment) * sizeof(stRecInfo));
        if (NULL == newListBuf) {
            APP_TRACE("realloc failed!");
            return -1;
        } else {
            search_ctx->recList = newListBuf;
            search_ctx->recListSize += increment;
        }
    }

    pathBuf = (char *)malloc(pathBufLen);
    if (NULL == pathBuf) {
        APP_TRACE("malloc failed!");
        return -1;
    } else {
        snprintf(pathBuf, pathBufLen, "%s/%s/%s", RL_PATH, datePath, fileName);
//        APP_TRACE("malloced path: %s", pathBuf);
        search_ctx->recList[recCnt].path = pathBuf;
        search_ctx->recList[recCnt].recBeginTs = recBeginTs;
        search_ctx->recList[recCnt].recEndTs = recEndTs;
        search_ctx->recCnt++;
        return 0;
    }
}

static int rec_play_search_recs(void * const ctx,
                                time_t beginTs, time_t endTs,
                                char recType, size_t maxSearchRecs,
								rec_play_search_cb_t cb_collect_result)
{
    int searchedRecs;

    struct tm beginTm;
    struct tm endTm;

    const char * const DirFormat = kREC_PLAY_DIR_NAME;
    const size_t dateNameLen = strlen(DirFormat);

    const char * const RecFormatPre = kREC_PLAY_REC_NAME_PRE;
    const size_t recNamePreLen = strlen(RecFormatPre);

    const char * const RecDir = RL_PATH;
    struct dirent **dirList = NULL;
    int dir_list_size = 0;
    int *used_dates = NULL;
    size_t searched_dates_cnt = 0;

    char searchBeginDate[15];
    char searchEndDate[15];
    bool gotBeginDate;
    bool gotEndDate;
    const size_t beginDateSz = sizeof(searchBeginDate);
    const size_t endDateSz = sizeof(searchEndDate);

    int i;
    int j;
    int ret;

    NK_EXPECT_VERBOSE_RETURN_VAL(NK_TFCARD_is_mounted(), -1);

    NK_EXPECT_VERBOSE_RETURN_VAL(maxSearchRecs > 0, -1);
    NK_EXPECT_VERBOSE_RETURN_VAL(endTs > beginTs, -1);

    NK_EXPECT_VERBOSE_RETURN_VAL(NULL != gmtime_r(&beginTs, &beginTm), -1);
    NK_EXPECT_VERBOSE_RETURN_VAL(NULL != gmtime_r(&endTs, &endTm), -1);

    /*
     * check what date should I search in
     */

    // get ascending order recorded date dirs;
    // compare YYYYMMDD of dirs and beginTm,
    // find first dir which YYYYMMDD bigger than(should smaller than endTime) or queal to beginTm;
    // (notice: ignore incorrect dir names and non-dir names)
    // start from previous result, compare YYYYMMDD of dirs and endTm,
    // find first dir which YYYYMMDD bigger than endTm

    ret = snprintf(searchBeginDate, sizeof(searchBeginDate), "%04d%02d%02d",
                   beginTm.tm_year + 1900, beginTm.tm_mon + 1, beginTm.tm_mday);
    if (ret >= beginDateSz || ret < 0) {
        APP_TRACE("snprintf failed, ret: %d", ret);
        searchedRecs = -1;
        goto FUNC_RETURN;
    }

    ret = snprintf(searchEndDate, sizeof(searchEndDate), "%04d%02d%02d",
                   endTm.tm_year + 1900, endTm.tm_mon + 1, endTm.tm_mday);
    if (ret >= endDateSz || ret < 0) {
        APP_TRACE("snprintf failed, ret: %d", ret);
        searchedRecs = -1;
        goto FUNC_RETURN;
    }

    gotBeginDate = false;
    gotEndDate = false;

    dir_list_size = scandir(RecDir, &dirList, NULL, alphasort);
    if (dir_list_size < 0) {
        APP_TRACE("Failed to scan %s", RecDir);
        dirList = NULL;
        searchedRecs = -1;
        goto FUNC_RETURN;
    } else if (0 == dir_list_size) {
        APP_TRACE("Directory %s is empty", RecDir);
        searchedRecs = 0;
        goto FUNC_RETURN;
    }

    used_dates = malloc(sizeof(int) * dir_list_size);
    if (NULL == used_dates) {
        APP_TRACE("Failed to malloc for %d ints", dir_list_size);
        searchedRecs = -1;
        goto FUNC_RETURN;
    }


    for (i = 0; i < dir_list_size; i++) {

        if (strlen(dirList[i]->d_name) != dateNameLen) {
            ;
        } else if (atoi(dirList[i]->d_name) < 10000101) {
            ;
        } else {
            if (gotBeginDate) {
                if (strcmp(dirList[i]->d_name, searchEndDate) > 0) {
                    gotEndDate = true;
                    // release memory later
                    break;
                } else {
                    used_dates[searched_dates_cnt] = i;
                    searched_dates_cnt++;
                }

            } else {
                ret = strcmp(dirList[i]->d_name, searchBeginDate);
                if (0 == ret) {
                    gotBeginDate = true;
                    used_dates[searched_dates_cnt] = i;
                    searched_dates_cnt++;
                } else if (ret > 0) {
                    if (strcmp(dirList[i]->d_name, searchEndDate) <= 0) {
                        gotBeginDate = true;
                        used_dates[searched_dates_cnt] = i;
                        searched_dates_cnt++;
                    } else {
                        gotBeginDate = true;
                        gotEndDate = true;
                        // release memory later
                        break;
                    }
                } else {
                    ;
                }
            }
        }
    }

    // got date dirs

    // if date dirs is 0, return 0;
    if (0 == searched_dates_cnt) {
        APP_TRACE("No date is in the search range");
        searchedRecs = 0;
        goto FUNC_RETURN;
    }

    // if number of date dirs is 1
    // 1. get ascending order record list
    // 2. search for first HHmmss+daration which bigger than HHmmss of beginTm
    // (HHmmss should smaller than endTm)
    // 3. start from previous result, search for first HHmmss which bigger than HHmmss of endTm
    // 4. got records

    // if number of date dirs is bigger than 1
    // 1. in min date dir, search for first HHmmss+daration which bigger than HHmmss of beginTm,
    //    then got records in this dir
    // 2. in max date dir, search for first HHmmss which bigger than HHmmss of endTm
    //    then got records in this dir
    // 3. all records in date which between min and max date is wanted
    char search_dir[256];
    const size_t search_dir_sz = sizeof(search_dir);
    struct dirent **recFileList = NULL;
    int rec_list_size;

    bool gotBeginRec = false;
    bool gotEndRec = false;
    time_t curRecBeginTime;
    time_t curRecEndTime;
    const time_t beginSearchTime = beginTs;
    const time_t endSearchTime = endTs;
    char localRecType;
    char tmp_buf[23];

    searchedRecs = 0;
    if (1 == searched_dates_cnt) {
        ret = snprintf(search_dir, search_dir_sz, "%s/%s",
                       RecDir, dirList[used_dates[0]]->d_name);
        if (ret >= search_dir_sz || ret < 0) {
            APP_TRACE("snprintf failed, ret: %d", ret);
            searchedRecs = -1;
            goto FUNC_RETURN;
        }

        rec_list_size = scandir(search_dir, &recFileList, NULL, alphasort);
        if (rec_list_size < 0) {
            APP_TRACE("Failed to scan %s", search_dir);
            recFileList = NULL;
            searchedRecs = -1;
            goto FUNC_RETURN;
        } else if (0 == rec_list_size) {
            APP_TRACE("Directory %s is empty", search_dir);
            searchedRecs = 0;
            goto FUNC_RETURN;
        }

//        APP_TRACE(">>>>>>> beginSearchTime: %ld", beginSearchTime);
//        APP_TRACE(">>>>>>> endSearchTime: %ld", endSearchTime);
        for (i = 0; i < rec_list_size; i++) {

            if ((searchedRecs + 1) > maxSearchRecs) {
                ;
            } else if (gotEndRec) {
                ;
            } else if (!rec_play_is_valid_mp4(recFileList[i]->d_name, recNamePreLen)) {
                ;
            } else {
                ret = rec_play_time_to_ts(dirList[used_dates[0]]->d_name,
                                          recFileList[i]->d_name,
                                          tmp_buf, &curRecBeginTime, &curRecEndTime,
                                          &localRecType);
                if (ret != 0) {
                    APP_TRACE("Failed to get time from %s", recFileList[i]->d_name);

                } else if (gotBeginRec) {
                    if (curRecBeginTime > endSearchTime) {
                        gotEndRec = true;
//                        if (debug) {
//                            APP_TRACE("gotEndRec curRecBeginTime: %ld", curRecBeginTime);
//                            APP_TRACE("gotEndRec curRecEndTime: %ld", curRecEndTime);
//                        }
                    } else {
                    	if(((recType & NK_REC_TIMER) && localRecType == 'T')
							|| ((recType & NK_REC_MOTION) && localRecType == 'M')){
							cb_collect_result(ctx,
											  dirList[used_dates[0]]->d_name, recFileList[i]->d_name,
											  curRecBeginTime, curRecEndTime, localRecType);
							searchedRecs++;
                    	}
                    }

                } else {
                    if (curRecEndTime > beginSearchTime) {

                        if (curRecBeginTime <= endSearchTime) {
							if(((recType & NK_REC_TIMER) && localRecType == 'T')
								|| ((recType & NK_REC_MOTION) && localRecType == 'M')){
	                            gotBeginRec = true;
								cb_collect_result(ctx,
												  dirList[used_dates[0]]->d_name, recFileList[i]->d_name,
												  curRecBeginTime, curRecEndTime, localRecType);
								searchedRecs++;
							}
                        } else {
                            gotBeginRec = true;
                            gotEndRec = true;
                        }

                    } else {
                        ;
                    }
                }
            }
            free(recFileList[i]);
        }
        free(recFileList);
        recFileList = NULL;

    } else {

        for (j = 0; j < searched_dates_cnt; j++) {

            if ((searchedRecs + 1) > maxSearchRecs) {
                break;
            }

            ret = snprintf(search_dir, search_dir_sz, "%s/%s",
                           RecDir, dirList[used_dates[j]]->d_name);
            if (ret >= search_dir_sz || ret < 0) {
                APP_TRACE("snprintf failed, ret: %d", ret);
                continue;
            }

            rec_list_size = scandir(search_dir, &recFileList, NULL, alphasort);
            if (rec_list_size < 0) {
                APP_TRACE("Failed to scan %s", search_dir);
                recFileList = NULL;
                continue;
            } else if (0 == rec_list_size) {
                APP_TRACE("Directory %s is empty", search_dir);
                if (NULL != recFileList) {
                    free(recFileList);
                    recFileList = NULL;
                }
                continue;
            }

            if (0 == j) {
                for (i = 0; i < rec_list_size; i++) {
                    if ((searchedRecs + 1) > maxSearchRecs) {
                        ;
                    } else if (!rec_play_is_valid_mp4(recFileList[i]->d_name, recNamePreLen)) {
                        ;
                    } else {
                        ret = rec_play_time_to_ts(dirList[used_dates[j]]->d_name,
                                                  recFileList[i]->d_name,
                                                  tmp_buf, &curRecBeginTime, &curRecEndTime,
                                                  &localRecType);
                        if (ret != 0) {
                            APP_TRACE("Failed to get time from %s", recFileList[i]->d_name);
                        } else if (gotBeginRec) {
							cb_collect_result(ctx,
											  dirList[used_dates[j]]->d_name, recFileList[i]->d_name,
											  curRecBeginTime, curRecEndTime, localRecType);
                            searchedRecs++;
                        } else if (curRecEndTime > beginSearchTime) {
                            gotBeginRec = true;
							cb_collect_result(ctx,
											  dirList[used_dates[j]]->d_name, recFileList[i]->d_name,
											  curRecBeginTime, curRecEndTime, localRecType);
                            searchedRecs++;
                        } else {
                            ;
                        }
                    }
                    free(recFileList[i]);
                }
                free(recFileList);

            } else if ((searched_dates_cnt - 1) == j) {
                for (i = 0; i < rec_list_size; i++) {
                    if ((searchedRecs + 1) > maxSearchRecs) {
                        ;
                    } else if (gotEndRec) {
                        ;
                    } else if (!rec_play_is_valid_mp4(recFileList[i]->d_name, recNamePreLen)) {
                        ;
                    } else {
                        ret = rec_play_time_to_ts(dirList[used_dates[j]]->d_name,
                                                  recFileList[i]->d_name,
                                                  tmp_buf, &curRecBeginTime, &curRecEndTime,
                                                  &localRecType);
                        if (ret != 0) {
                            APP_TRACE("Failed to get time from %s", recFileList[i]->d_name);
                        } else if (curRecBeginTime > endSearchTime) {
                            gotEndRec = true;
                        } else {
							cb_collect_result(ctx,
											  dirList[used_dates[j]]->d_name, recFileList[i]->d_name,
											  curRecBeginTime, curRecEndTime, localRecType);
                            searchedRecs++;
                        }
                    }
                    free(recFileList[i]);
                }
                free(recFileList);
            } else {
                for (i = 0; i < rec_list_size; i++) {
                    if ((searchedRecs + 1) > maxSearchRecs) {
                        ;
                    } else if (!rec_play_is_valid_mp4(recFileList[i]->d_name, recNamePreLen)) {
                        ;
                    } else {
                        ret = rec_play_time_to_ts(dirList[used_dates[j]]->d_name,
                                                  recFileList[i]->d_name,
                                                  tmp_buf, &curRecBeginTime, &curRecEndTime,
                                                  &localRecType);
                        if (ret != 0) {
                            APP_TRACE("Failed to get time from %s", recFileList[i]->d_name);
                        } else {
							cb_collect_result(ctx,
											  dirList[used_dates[j]]->d_name, recFileList[i]->d_name,
											  curRecBeginTime, curRecEndTime, localRecType);
                            searchedRecs++;
                        }
                    }
                    free(recFileList[i]);
                }
                free(recFileList);
            }
            recFileList = NULL;
        }
    }


FUNC_RETURN:

    if (NULL != dirList) {
        for (i = 0; i < dir_list_size; i++) {
            free(dirList[i]);
        }
        free(dirList);
    }

    if (NULL != used_dates) {
        free(used_dates);
    }

    return searchedRecs;
}


static void rec_play_release_demux(lpPlay_rec_search_ctx pSearch_ctx)
{
	HI_S32 HiRet;

    if (pSearch_ctx->fmtHandle > 0) {
		HiRet = g_stFormat_entry.stFmtFun.fmt_close(pSearch_ctx->fmtHandle);
        pSearch_ctx->fmtHandle = 0;
		if (HI_SUCCESS == HiRet) {
			APP_TRACE("fmt_close succeed");
		} else {
			APP_TRACE("fmt_close failed! ret: %d", HiRet);
		}
    } else {
		APP_TRACE("fmt_close already done");
	}
}

static int rec_play_open_demux(lpPlay_rec_search_ctx pSearch_ctx)
{
    char *filepath;
	HI_S32 HiRet;

    filepath = pSearch_ctx->recList[pSearch_ctx->curIndex].path;

	HiRet = g_stFormat_entry.stFmtFun.fmt_open(&pSearch_ctx->fmtHandle, filepath);
    if (HI_SUCCESS != HiRet) {
		APP_TRACE("fmt_open failed, ret: %d", HiRet);
        return -1;
    }

	HiRet = g_stFormat_entry.stFmtFun.fmt_probe(pSearch_ctx->fmtHandle);
    if (HI_SUCCESS != HiRet) {
		APP_TRACE("fmt_probe failed, ret: %d", HiRet);
		rec_play_release_demux(pSearch_ctx);
        return -2;
    }

	HiRet = g_stFormat_entry.stFmtFun.fmt_getinfo(pSearch_ctx->fmtHandle, &(pSearch_ctx->mp4fileFormat));
    if (HI_SUCCESS != HiRet) {
		APP_TRACE("fmt_getinfo failed, ret: %d", HiRet);
		rec_play_release_demux(pSearch_ctx);
        return -3;
    }
	pSearch_ctx->frmIntervalUs = (unsigned int)(1000000 / pSearch_ctx->mp4fileFormat.fFrameRate);


    return 0;
}

// pOutRecListSize[in][out]
int REC_PLAY_get_history(time_t beginTs, time_t endTs,
                         char recType, int startIndex,
                         lpTFCARD_History_List outRecList, size_t *pOutRecListSize)
{
    int ret;
    int funcRet = -1;
    size_t i;
    size_t j;
    size_t outRecListSize;
    size_t recCnt;


    NK_EXPECT_VERBOSE_RETURN_VAL(startIndex >= 0, -1);
    NK_EXPECT_VERBOSE_RETURN_VAL(NULL != outRecList, -1);
    NK_EXPECT_VERBOSE_RETURN_VAL(NULL != pOutRecListSize, -1);
    NK_EXPECT_VERBOSE_RETURN_VAL(endTs > beginTs, -1);

    outRecListSize = *pOutRecListSize;

    NK_EXPECT_VERBOSE_RETURN_VAL(outRecListSize > 0, -1);


    ret = pthread_mutex_lock(&gs_cachedListLock);
    if (0 != ret) {
        APP_TRACE("failed to lock! ret: %d", ret);
        return -1;
    }

    if (!NK_TFCARD_is_mounted()) {
        APP_TRACE("tf card is not mounted!");
        funcRet = -1;
        goto UNLOCK_RETURN;
    }


    if (0 == startIndex || NULL == gs_cachedHistoryList.recList) {
        // free old cache

        if (NULL != gs_cachedHistoryList.recList) {
            free(gs_cachedHistoryList.recList);
            gs_cachedHistoryList.recList = NULL;
        }
        gs_cachedHistoryList.recListSize = 0;
        gs_cachedHistoryList.recCnt = 0;

    } else {
        // use cache

        recCnt = gs_cachedHistoryList.recCnt;

        if (startIndex >= recCnt) {
            APP_TRACE("Invalid start index, startIndex: %d, recCnt: %lu", startIndex, recCnt);
            *pOutRecListSize = 0;
            funcRet = 0;
            goto UNLOCK_RETURN;
        }

        for (i = 0, j = (size_t)startIndex; i < outRecListSize && j < recCnt; i++, j++) {
            outRecList[i].beginTm = gs_cachedHistoryList.recList[j].beginTm;
            outRecList[i].endTm = gs_cachedHistoryList.recList[j].endTm;
            snprintf(outRecList[i].recordType, sizeof(outRecList[i].recordType),
                     "%s",  gs_cachedHistoryList.recList[j].recordType);
        }

        *pOutRecListSize = i;

        funcRet = (int)gs_cachedHistoryList.recCnt;
        goto UNLOCK_RETURN;
    }


	ret = rec_play_search_recs((void *)&gs_cachedHistoryList,
							   beginTs, endTs,
							   recType, kREC_PLAY_MAX_SEARCH_RECS,
							   get_history_search_cb);
    if (ret <= 0) {
        if (NULL != gs_cachedHistoryList.recList) {
            free(gs_cachedHistoryList.recList);
            gs_cachedHistoryList.recList = NULL;
        }
        gs_cachedHistoryList.recCnt = 0;
        gs_cachedHistoryList.recListSize = 0;

        APP_TRACE("failed to search record");
        funcRet = -1;
        goto UNLOCK_RETURN;
    }

    recCnt = gs_cachedHistoryList.recCnt;

    if (0 == recCnt) {
        APP_TRACE("no rec found");
        *pOutRecListSize = 0;
        funcRet = 0;
        goto UNLOCK_RETURN;
    }

    if (startIndex >= recCnt) {
        APP_TRACE("Invalid start index, startIndex: %d, recCnt: %lu", startIndex, recCnt);
        *pOutRecListSize = 0;
        funcRet = -1;
        goto UNLOCK_RETURN;
    }

    for (i = 0, j = (size_t)startIndex; i < outRecListSize && j < recCnt; i++, j++) {
        outRecList[i].beginTm = gs_cachedHistoryList.recList[j].beginTm;
        outRecList[i].endTm = gs_cachedHistoryList.recList[j].endTm;
        snprintf(outRecList[i].recordType, sizeof(outRecList[i].recordType),
                 "%s",  gs_cachedHistoryList.recList[j].recordType);
    }

    *pOutRecListSize = i;

    funcRet = (int)gs_cachedHistoryList.recCnt;

UNLOCK_RETURN:

    ret = pthread_mutex_unlock(&gs_cachedListLock);
    if (0 != ret) {
        APP_TRACE("failed to unlock! ret: %d", ret);
    }

    return funcRet;
}

void * REC_PLAY_start(time_t beginTs, time_t endTs, char recType, int openType)
{
    int i;
    int ret;
    time_t firstStartSec;
    time_t lastEndSec;
    lpPlay_rec_search_ctx pSearch_ctx;
    size_t mem_size;
    ST_NSDK_SYSTEM_TIME systime;
    struct timespec tp;


    NK_EXPECT_VERBOSE_RETURN_VAL(NK_TFCARD_is_mounted(), NULL);

    NK_EXPECT_VERBOSE_RETURN_VAL(endTs > beginTs, NULL);

    if (NULL == NETSDK_conf_system_get_time(&systime)) {
        APP_TRACE("NETSDK_conf_system_get_time failed!");
        return NULL;
    }

    mem_size = sizeof(stPlay_rec_search_ctx);
    pSearch_ctx = (lpPlay_rec_search_ctx)malloc(mem_size);
    if (NULL == pSearch_ctx) {
        APP_TRACE("malloc %lu bytes failed!", mem_size);
        return NULL;
    }

	pSearch_ctx->isOldTypeRec = false;
    pSearch_ctx->recList = NULL;
    pSearch_ctx->recListSize = 0;
    pSearch_ctx->recCnt = 0;
    pSearch_ctx->curIndex = 0;
    pSearch_ctx->fmtHandle = 0;
    pSearch_ctx->tzOffset = (systime.greenwichMeanTime / 100) * 3600
							+ (systime.greenwichMeanTime % 100) * 60;

	pSearch_ctx->openType = openType;
	pSearch_ctx->frmIntervalUs = 1000000 / 15;  // fixed at 15fps for now

	pSearch_ctx->curFileSentMs = 0;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    pSearch_ctx->sentFilesTimeMs = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;

    ret = rec_play_search_recs((void *)pSearch_ctx,
                               beginTs, endTs,
                               recType, kREC_PLAY_MAX_SEARCH_RECS,
                               play_rec_search_cb);
    if (ret <= 0) {
        if (NULL != pSearch_ctx->recList) {
            i = 0;
            for ( ; i < pSearch_ctx->recCnt; i++) {
                free(pSearch_ctx->recList[i].path);
            }
            APP_TRACE("%d rec files path freed. total: %lu", i, pSearch_ctx->recCnt);
            free(pSearch_ctx->recList);
        }
        free(pSearch_ctx);

        APP_TRACE("failed to search record");
        return NULL;

    } else {
        time_t firstRecBeginTs = pSearch_ctx->recList[0].recBeginTs;
        time_t lastRecEndTs = pSearch_ctx->recList[pSearch_ctx->recCnt-1].recEndTs;
        time_t lastRecBeginTs = pSearch_ctx->recList[pSearch_ctx->recCnt-1].recBeginTs;

        firstStartSec = beginTs - firstRecBeginTs;
        if (endTs > lastRecEndTs) {
            lastEndSec = lastRecEndTs - lastRecBeginTs;
        } else {
            lastEndSec = endTs - lastRecBeginTs;
        }
        pSearch_ctx->firstRecStartSec = (firstStartSec < 0)?0:firstStartSec;
        pSearch_ctx->lastRecEndSec = (lastEndSec < 0)?0:lastEndSec;
//        APP_TRACE(">>>>>>> pSearch_ctx->firstRecStartSec: %ld", pSearch_ctx->firstRecStartSec);
//        APP_TRACE(">>>>>>> pSearch_ctx->lastRecEndSec: %ld", pSearch_ctx->lastRecEndSec);
        return (void *)pSearch_ctx;
    }
}

int REC_PLAY_stop(void * const ctx)
{
    int i;
    lpPlay_rec_search_ctx const pSearch_ctx = (lpPlay_rec_search_ctx)ctx;


    NK_EXPECT_VERBOSE_RETURN_VAL(NULL != ctx, -1);

	/* 兼容旧格式录像 */
	if(pSearch_ctx->isOldTypeRec == true) {
		return -1;
	}

    /// fixme   stop read first
	rec_play_release_demux(pSearch_ctx);

    if (NULL != pSearch_ctx->recList) {
        i = 0;
        for ( ; i < pSearch_ctx->recCnt; i++) {
//            APP_TRACE("free path %s", pSearch_ctx->recList[i].path);
            free(pSearch_ctx->recList[i].path);
        }
        APP_TRACE("%d rec files path freed. total: %lu", i, pSearch_ctx->recCnt);
        free(pSearch_ctx->recList);
    }
    free(pSearch_ctx);

    return 0;
}

ssize_t REC_PLAY_read_frame(void * const ctx,
                            lpRecord_Frame_Head frameHead,
                            uint8_t *data, size_t dataMaxSize)
{
	const static int64_t MaxDelayTimeMs = 5000;

	int ret;
    HI_S32 HiRet;
    uint64_t tmp_ts_s;
    uint64_t tmp_ts_ms;
    HI_FORMAT_PACKET_S fmtPacket;
    time_t startAtTs;
    struct timespec tp;
    int64_t curTimeMs;
    int64_t sentTimeMs;
    static uint64_t pre_ts_ms = 0;

    lpPlay_rec_search_ctx const pSearch_ctx = (lpPlay_rec_search_ctx)ctx;

    NK_EXPECT_VERBOSE_RETURN_VAL(NULL != ctx, -1);

	/* 兼容旧格式录像 */
	if(pSearch_ctx->isOldTypeRec == true) {
		return -1;
	}

    // REC_PLAY_start() make sure pSearch_ctx->curIndex is started from 0
    do {
        if (pSearch_ctx->curIndex >= pSearch_ctx->recCnt) {
            APP_TRACE("No more file to open, end play");
            return -1;
        }

        if (pSearch_ctx->fmtHandle <= 0) {
            ret = rec_play_open_demux(pSearch_ctx);
            if (0 == ret) {
                // 第一个文件偏移处理
                if (0 == pSearch_ctx->curIndex) {
                    startAtTs = pSearch_ctx->firstRecStartSec >= 0 ? pSearch_ctx->firstRecStartSec : 0;
                    startAtTs = (startAtTs * MS_2_SECOND);
                    HiRet = g_stFormat_entry.stFmtFun.fmt_seek(pSearch_ctx->fmtHandle, 0, startAtTs);
                    if (HI_SUCCESS != HiRet) {
                        APP_TRACE("mp4 seek %s failed, offset at %lds, filename time length: %ld. HiRet: %ld\n",
                                  pSearch_ctx->recList[pSearch_ctx->curIndex].path, pSearch_ctx->firstRecStartSec,
                                  pSearch_ctx->recList[pSearch_ctx->curIndex].recEndTs - pSearch_ctx->recList[pSearch_ctx->curIndex].recBeginTs,
                                  HiRet);
						rec_play_release_demux(pSearch_ctx);
                        pSearch_ctx->curIndex++;
                    } else {
                        APP_TRACE("mp4 seek %s succeed, offset at %lds\n",
                                  pSearch_ctx->recList[pSearch_ctx->curIndex].path, pSearch_ctx->firstRecStartSec);
                        pSearch_ctx->sentFilesTimeMs -= startAtTs;
                        pSearch_ctx->curFileSentMs = startAtTs;
                    }
                } else {
                    if (pSearch_ctx->fmtHandle <= 0) {
                        APP_TRACE("rec_play_open_demux not open demux properly!\n");
                        return -1;
                    }
                    pSearch_ctx->sentFilesTimeMs += pSearch_ctx->curFileSentMs;
                    pSearch_ctx->curFileSentMs = 0;
                }
            } else if (-1 == ret) {
                pSearch_ctx->fmtHandle = 0;
				pSearch_ctx->curIndex++;
            } else {
                // error occured while open demux
                return -1;
            }
        } else {
            HiRet = g_stFormat_entry.stFmtFun.fmt_read(pSearch_ctx->fmtHandle, &fmtPacket);
            if (HI_SUCCESS == HiRet) {
                break;
            } else {
                APP_TRACE("fmt_read failed! try next file\n");
				rec_play_release_demux(pSearch_ctx);
                pSearch_ctx->curIndex++;
            }
        }
    } while (1);


    // 最后一个文件结束点判断
    if (pSearch_ctx->curIndex == pSearch_ctx->recCnt - 1)
    {
        tmp_ts_s = (uint64_t)pSearch_ctx->lastRecEndSec;
        tmp_ts_ms = tmp_ts_s * 1000;

        if (fmtPacket.s64Pts >= tmp_ts_ms) {
            APP_TRACE("End frame reached, end play");
            goto FREE_FAIL_RETURN;
        }
    }


    // 填写结构体
	if (fmtPacket.u32StreamIndex == pSearch_ctx->mp4fileFormat.s32UsedAudioStreamIndex)	{

        // 音频
		frameHead->sampleRate = pSearch_ctx->mp4fileFormat.u32SampleRate;
		frameHead->codec = NK_TFCARD_ACODEC_AAC;
		frameHead->sampleWidth = 16;

	} else if (fmtPacket.u32StreamIndex == pSearch_ctx->mp4fileFormat.s32UsedVideoStreamIndex) {

        // 视频
        frameHead->fps      = pSearch_ctx->mp4fileFormat.fFrameRate;
		frameHead->width    = pSearch_ctx->mp4fileFormat.stSteamResolution[0].u32Width;
		frameHead->height   = pSearch_ctx->mp4fileFormat.stSteamResolution[0].u32Height;
        frameHead->isKeyFrame = fmtPacket.bKeyFrame;

		if (HI_FORMAT_VIDEO_TYPE_H264 == pSearch_ctx->mp4fileFormat.enVideoType) {
			frameHead->codec = NK_TFCARD_VCODEC_H264;
		} else if (HI_FORMAT_VIDEO_TYPE_H265 == pSearch_ctx->mp4fileFormat.enVideoType) {
			frameHead->codec = NK_TFCARD_VCODEC_H265;
		} else {
            APP_TRACE("unexpected video frame: %d.", pSearch_ctx->mp4fileFormat.enVideoType);
            goto IGNORE_FREME;
		}

        // Should not sent frame too fast when playback
        if (0 == pSearch_ctx->openType) {
			int64_t tmpMs;

            clock_gettime(CLOCK_MONOTONIC, &tp);
            curTimeMs = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
            sentTimeMs = pSearch_ctx->sentFilesTimeMs + pSearch_ctx->curFileSentMs;

			// 1. prevent sent frame too fast
			// 2. if delay bigger than MaxDelayTimeMs, not sent frame without sleep
			tmpMs = curTimeMs - sentTimeMs;
			if (tmpMs < 0) {
//                APP_TRACE("Sleep... curTimeMs: %lld, sentTimeMs: %lld", curTimeMs, sentTimeMs);
                usleep(pSearch_ctx->frmIntervalUs);
            } else if (tmpMs > MaxDelayTimeMs) {
				tmpMs += MaxDelayTimeMs;
				pSearch_ctx->sentFilesTimeMs += tmpMs;
				usleep(pSearch_ctx->frmIntervalUs);
			}

            pSearch_ctx->curFileSentMs = fmtPacket.s64Pts;
        }

    } else if (fmtPacket.u32StreamIndex == pSearch_ctx->mp4fileFormat.s32UsedDataIndex)	{
        // MP4 data
        frameHead->codec = NK_TFCARD_CODEC_DATA;
    } else {

        APP_TRACE("unexpected frame, stream index: %d", pSearch_ctx->mp4fileFormat.s32UsedVideoStreamIndex);
        goto IGNORE_FREME;
    }

	// 填数据
	if (fmtPacket.u32Size > dataMaxSize) {
        APP_TRACE("frame size too large!! size: %u, max: %lu\n", fmtPacket.u32Size, dataMaxSize );
        goto IGNORE_FREME;
	}
	memcpy(data, fmtPacket.pu8Data, fmtPacket.u32Size);
	frameHead->dataSize = fmtPacket.u32Size;

    // can't use pSearch_ctx->recList[pSearch_ctx->curIndex].recBeginTs * 1000. because: 1500451229 * 1000 = 1507642696
    tmp_ts_s = (uint64_t)pSearch_ctx->recList[pSearch_ctx->curIndex].recBeginTs;
    tmp_ts_ms = (tmp_ts_s - pSearch_ctx->tzOffset) * 1000;
    if(frameHead->codec == NK_TFCARD_CODEC_DATA) {
        frameHead->sysTime_ms = pre_ts_ms;
    }
    else {
        frameHead->sysTime_ms = tmp_ts_ms + (uint64_t)fmtPacket.s64Pts;
    }

    if(fmtPacket.u32StreamIndex == pSearch_ctx->mp4fileFormat.s32UsedVideoStreamIndex) {
        pre_ts_ms = frameHead->sysTime_ms;
    }

    HiRet = g_stFormat_entry.stFmtFun.fmt_free(pSearch_ctx->fmtHandle, &fmtPacket);
    if (HI_SUCCESS != HiRet) {
        APP_TRACE("fmt_free failed!, ret: %d", HiRet);
    }

	return frameHead->dataSize;

FREE_FAIL_RETURN:

    HiRet = g_stFormat_entry.stFmtFun.fmt_free(pSearch_ctx->fmtHandle, &fmtPacket);
    if (HI_SUCCESS != HiRet) {
        APP_TRACE("fmt_free failed!, ret: %d", HiRet);
    }

    if (pSearch_ctx->fmtHandle > 0) {
		rec_play_release_demux(pSearch_ctx);
    }
    return -1;

IGNORE_FREME:

    HiRet = g_stFormat_entry.stFmtFun.fmt_free(pSearch_ctx->fmtHandle, &fmtPacket);
    if (HI_SUCCESS != HiRet) {
        APP_TRACE("fmt_free failed!, ret: %d", HiRet);
    }
    return 0;
}

/*
 * get dates which have records
 *
 * @param   p_list[out] pointer to list of strings.
 * @return   number of strings in the list. list and strings must be freed by caller.
 * 			 if return value equal or less than 0, then nothing need to be freed.
 *
 * call example:
 *
 * char **list;
 * int num;
 * int i;
 *
 * num = REC_PLAY_get_date_list(&list);
 *
 * if (num <= 0) {
 *  abort;
 * }
 *
 * for (i = 0; i < num; i++) {
 *     printf("date: %s\n", list[i]);
 *     free(list[i]);
 * }
 * free(list);
 */
int REC_PLAY_get_date_list(char ***p_list)
{
	NK_EXPECT_VERBOSE_RETURN_VAL(NK_TFCARD_is_mounted(), -1);

	const char * const DirFormat = kREC_PLAY_DIR_NAME;
	const size_t dateNameLen = strlen(DirFormat);
	const size_t dateNameBufSz = dateNameLen + 1;

	const char * const RecDir = RL_PATH;
	struct dirent **dirList = NULL;
	int dir_list_size;
	char ** date_list;
	int date_cnt;

	char *tmp_p;
	int i;

	bool errOccur = false;


	*p_list = NULL;

	dir_list_size = scandir(RL_PATH, &dirList, 0, alphasort);
	if (dir_list_size < 0) {
		APP_TRACE("Failed to scan %s", RecDir);
		return -1;
	} else if (0 == dir_list_size) {
		APP_TRACE("Directory %s is empty", RecDir);
		if (NULL != dirList) {
			free(dirList);
		}
		return 0;
	}

	date_list = (char **)malloc(dir_list_size * sizeof(char *));
	if (NULL == date_list) {
		APP_TRACE("Failed to malloc for date list");
		return -1;
	}

	date_cnt = 0;
	for (i = 0; i < dir_list_size; i++) {

		if (errOccur) {
			;
		} else if (strlen(dirList[i]->d_name) != dateNameLen) {
			;
		} else if (atoi(dirList[i]->d_name) < 10000101) {
			;
		} else {
			tmp_p = malloc(dateNameBufSz);
			if (NULL == tmp_p) {
				APP_TRACE("Failed to malloc for date string");
				errOccur = true;
			} else {
				date_list[date_cnt] = tmp_p;
				snprintf(tmp_p, dateNameBufSz, "%s", dirList[i]->d_name);
				date_cnt++;
			}
		}

		free(dirList[i]);
	}
	free(dirList);

	if (errOccur) {
		for (i = 0; i < date_cnt; i++) {
			free(date_list[i]);
		}
		free(date_list);
		return -1;
	}

	if (0 == date_cnt) {
		free(date_list);
		return 0;
	}

	*p_list = date_list;
	return date_cnt;
}

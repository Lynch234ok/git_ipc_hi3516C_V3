
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

#include "tfcard_record.h"
#include "bsp/keytime.h"
#include <sys/prctl.h>

struct path_par_t{
	NK_Int indexMax;
	NK_Char maxDate[20];
	NK_Int indexMin;
	NK_Char minDate[20];
	NK_Char mountPath[32];
	NK_Char recordPath[64];
	NK_Char datePath[32];
};

struct file_par_t{
	NK_Int indexMax;
	NK_Char fileName[32];
	NK_Int frameCnt;
	NK_Int iFrameCnt;
	NK_Size iFramePos[IFRAME_MAX_CNT_SAVE];
	NK_Int iFrameSec[IFRAME_MAX_CNT_SAVE];
	NK_Int beginSec;
	NK_Int endSec;
	NK_Size filesize;
	FILE *fid;
};

struct buffer_par_t{
	NK_PByte buffer;
	NK_Size bufLen;
	pthread_mutex_t bufMutex;
	NK_Int run_flag;
};

struct mem_size_par_t{
	NK_Size bufferMallocSize;
};

typedef struct TFcard_Record_t{
	NK_Char record_type[32];
	struct path_par_t path_par;
	struct file_par_t file_par;
	struct file_par_t tmp_file_par;
	struct buffer_par_t writer_buf;
	struct buffer_par_t tmp_buf;
	struct mem_size_par_t mem_size_par;
	pthread_t writerTid;
	fTFcardOnGetFreeSpace getFreeSpace;
}stTFcard_Record,*lpTFcard_Record;

static stTFcard_Record _recorder = {0};
static lpTFcard_Record recorder = NULL;

static NK_Void get_utc_date(NK_PChar str_date)
{
	NK_UTC1970 utc;
	struct tm setTm;
	
	utc = time(NULL);
	gmtime_r((time_t *)(&utc), &setTm);
	sprintf(str_date, "%04d%02d%02d",setTm.tm_year + 1900, setTm.tm_mon+1, setTm.tm_mday);
	NK_Log()->debug("utc date : %s",str_date);
}

static NK_Void get_local_date(NK_PChar str_date)
{
	NK_UTC1970 utc;
	struct tm setTm;
	
	utc = time(NULL);
	localtime_r((time_t *)(&utc), &setTm);
	sprintf(str_date, "%04d%02d%02d",setTm.tm_year + 1900, setTm.tm_mon+1, setTm.tm_mday);
	NK_Log()->debug("local date : %s",str_date);
}

static NK_Boolean check_date_exist(NK_PChar recordPath,NK_Int *reIndex,NK_PChar strDate)
{
	DIR *dirPtr = NULL;
	struct dirent *entry;
	NK_Int index = 0;//文件夹索引值,用于组建正确文件夹名
	NK_PChar tmpDate[20] = {0};

	if(NULL == (dirPtr = opendir(recordPath))){
		NK_Log()->warn("open dir error : path = %s",recordPath);
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
						//sprintf(rightDatePath, "%s/%04d-%s", recordPath,index,tmpDate);
						*reIndex = index;
						closedir(dirPtr);
						return NK_True;
					}
				}
			}
		}
		closedir(dirPtr);
	}

	return NK_False;
}

static NK_Void get_sec_of_today(NK_Int *sec)
{
	NK_UTC1970 utc;
	struct tm setTm;
	
	utc = time(NULL);

	// use utc time
	gmtime_r((time_t *)(&utc), &setTm);

	*sec = (setTm.tm_hour * 3600 + setTm.tm_min * 60 + setTm.tm_sec);
	//NK_Log()->debug("sec of today : %d",*sec);
}

static NK_Int get_index_num(NK_PChar Path, NK_Int *maxIndex, NK_Int *minIndex,NK_PChar maxDate,NK_PChar minDate)
{
	DIR *dirPtr = NULL;
	struct dirent *entry;
	int max = -10000;//初始化相对值,方便比较
	int min = 10000;
	char rightPath[80]={0};

	snprintf(rightPath,sizeof(rightPath),"%s",Path);
	if(NULL == (dirPtr = opendir(rightPath))){
		NK_Log()->warn("open dir error : path = %s",rightPath);
		return -1;
	}else{
		struct stat buf = {0};
		int tmpNum = -1;
		char path[128] = {0};
		char tmpChar[20]={0};

		while(entry = readdir(dirPtr)){
			if((0 == strcmp(entry->d_name,".")) || (0 == strcmp(entry->d_name,".."))){
				continue;
			}
			else{
				memset(tmpChar, 0, sizeof(tmpChar));
				if(2 == sscanf(entry->d_name,"%d-%s",&tmpNum,tmpChar)){
					NK_Log()->debug("%s init : getIndexNum: %d\n",Path,tmpNum);
					if(tmpNum > max){
						max = tmpNum;
						*maxIndex = max;
						snprintf(maxDate, sizeof(tmpChar), "%s", tmpChar);
					}
					if(tmpNum < min){
						min = tmpNum;
						*minIndex = min;
						snprintf(minDate, sizeof(tmpChar), "%s", tmpChar);
					}
				}
			}
		}
		closedir(dirPtr);
	}
	//空目录时,赋予初始值
	if(max == -10000 && min == 10000){
		NK_Log()->info("Path is empty!");
		NK_Char nowDate[20] = {0};

		get_utc_date(nowDate);
		*maxIndex = 1;
		*minIndex = 1;
		snprintf(maxDate, sizeof(nowDate), "%s", nowDate);
		snprintf(minDate, sizeof(nowDate), "%s", nowDate);
	}
	
	return 0;
}

#define MAX_TIMESTAMP_INTERVAL_OF_FILENAME (1800)
static NK_Int check_file_endTm(NK_Int srcEndTm, NK_Int retEndTm)
{
	if(retEndTm < srcEndTm){
		NK_Log()->info("use src time:%d > %d", srcEndTm, retEndTm);
        if (srcEndTm - retEndTm > 86000) {
            return 86400;
        }
		return srcEndTm;
	}else if(retEndTm > (srcEndTm + MAX_TIMESTAMP_INTERVAL_OF_FILENAME)){
		NK_Log()->info("out of max frame interval time:%d > %d", srcEndTm, retEndTm);
		return srcEndTm;
	}else{
		return retEndTm;
	}

	return retEndTm;
}

static NK_Int copy_from_buffer_to_write()
{
	NK_PByte tmp = NULL;
	NK_Int i = 0;

	if(recorder->writer_buf.run_flag){
		//NK_Log()->debug("switch to write buf:%d", recorder->tmp_buf.bufLen);
		pthread_mutex_lock(&recorder->writer_buf.bufMutex);
		//update buffer
		tmp = recorder->writer_buf.buffer;
		recorder->writer_buf.buffer = recorder->tmp_buf.buffer;
		recorder->tmp_buf.buffer = tmp;
		recorder->writer_buf.bufLen = recorder->tmp_buf.bufLen;
		recorder->tmp_buf.bufLen = 0;
		//update file par
		recorder->file_par.frameCnt += recorder->tmp_file_par.frameCnt;
		recorder->tmp_file_par.frameCnt = 0;
		for(i = 0; ((i < recorder->tmp_file_par.iFrameCnt) && ((i + recorder->file_par.iFrameCnt) < IFRAME_MAX_CNT_SAVE)); i++) {
			recorder->file_par.iFramePos[recorder->file_par.iFrameCnt + i] = (recorder->file_par.filesize + recorder->tmp_file_par.iFramePos[i]);
			recorder->tmp_file_par.iFramePos[i] = 0;
			recorder->file_par.iFrameSec[recorder->file_par.iFrameCnt + i] = recorder->tmp_file_par.iFrameSec[i];
			recorder->tmp_file_par.iFrameSec[i] = 0;
		}
		recorder->file_par.iFrameCnt += recorder->tmp_file_par.iFrameCnt;
		recorder->tmp_file_par.iFrameCnt = 0;
		//recorder->file_par.filesize + recorder->tmp_file_par.filesize;
		recorder->tmp_file_par.filesize = 0;

        // 临时文件结束秒数和录像文件上次的结束秒数相差过大，很可能是第二天第一段录像使用了前一天结束时的临时录像
        if (recorder->tmp_file_par.endSec - recorder->file_par.endSec  > 86000) {
            recorder->file_par.endSec += (recorder->tmp_file_par.endSec - recorder->tmp_file_par.beginSec);
        } else {
            recorder->file_par.endSec = recorder->tmp_file_par.endSec;
        }
        recorder->tmp_file_par.beginSec = -1;
        recorder->tmp_file_par.endSec = -1;

		pthread_mutex_unlock(&recorder->writer_buf.bufMutex);

		return 0;
	}

	return -1;
}

static NK_Int record_write_into_buf(lpRecord_Frame_Head frameHead,NK_PByte data)
{
	NK_Int headLen = 0;
	NK_Int dataLen = 0;
	NK_Int writeLen = 0;
	NK_Int posTmp = 0;
	NK_Int endTm = 0;
	static NK_UInt64 old_ts = 0;

	if(recorder && recorder->tmp_buf.buffer){
		//time to copy from buffer to write
		if((sizeof(stRecord_Frame_Head) + frameHead->dataSize + recorder->tmp_buf.bufLen) > (recorder->mem_size_par.bufferMallocSize)){
			if(0 != copy_from_buffer_to_write()){
				return -1;
			}
		}
		//backup
		posTmp = recorder->tmp_buf.bufLen;
		//mmecpy
		headLen = sizeof(stRecord_Frame_Head);
		memcpy((void *)(recorder->tmp_buf.buffer)+(recorder->tmp_buf.bufLen), frameHead, headLen);
		recorder->tmp_buf.bufLen += headLen;
		dataLen = frameHead->dataSize;
		memcpy((void *)(recorder->tmp_buf.buffer)+(recorder->tmp_buf.bufLen), data, dataLen);
		recorder->tmp_buf.bufLen += dataLen;
		//fflush tmp_file_par
		recorder->tmp_file_par.frameCnt++;
		get_sec_of_today(&endTm);
		if(((frameHead->codec == NK_TFCARD_VCODEC_H264) || (frameHead->codec == NK_TFCARD_VCODEC_H265))){
			NK_Log()->debug("frame %s size:%d  time:%d/%llu", frameHead->isKeyFrame?"I":"P", frameHead->dataSize, endTm, 
				frameHead->coderStamp_ms);
			if(frameHead->coderStamp_ms - old_ts > 120){
				NK_Log()->debug("frame %d time out :%llu/%llu", frameHead->dataSize, old_ts,
				frameHead->coderStamp_ms);
			}
			old_ts = frameHead->coderStamp_ms;
		}
		if((recorder->tmp_file_par.endSec < 0) || (recorder->file_par.filesize == 0)){//fixme:判断文件长度,避免日程跨天结束时间Bug
			recorder->tmp_file_par.endSec = endTm;
		}else{
			endTm = check_file_endTm(recorder->tmp_file_par.endSec,endTm);
		}
		recorder->tmp_file_par.endSec = endTm;
        if (recorder->tmp_file_par.beginSec < 0) {
            recorder->tmp_file_par.beginSec = recorder->tmp_file_par.endSec;
        }
		if(((frameHead->codec == NK_TFCARD_VCODEC_H264) || (frameHead->codec == NK_TFCARD_VCODEC_H265)) && 
			(frameHead->isKeyFrame) && (recorder->tmp_file_par.iFrameCnt < IFRAME_MAX_CNT_SAVE)){
			recorder->tmp_file_par.iFramePos[recorder->tmp_file_par.iFrameCnt] = posTmp;
			recorder->tmp_file_par.iFrameSec[recorder->tmp_file_par.iFrameCnt] = endTm;
			recorder->tmp_file_par.iFrameCnt++;
		}
		recorder->tmp_file_par.filesize += (headLen + dataLen);

		return 0;
	}

	NK_Log()->warn("recorder or tmp buf null");
	return -1;
}

static NK_Int record_write_into_flash(FILE *fid, NK_PByte writeBuf,NK_Int writeLen)
{
	if(!fid || !writeBuf || (writeLen <= 0)){
		NK_Log()->error("fid, writeBuf or writeLen error : writeLen = %d",writeLen);
		return -1;
	}
	if(writeLen != fwrite(writeBuf, 1, writeLen, fid)){
		NK_Log()->error("write into flash error");
		return -1;
	}

	return 0;
}

static NK_Int record_write_into_filetail(FILE *fid, struct file_par_t *file_par)
{
	int i = 0;
	struct file_tail_t fileTail = {0};

	if(!fid || !file_par){
		NK_Log()->error("fid or file_par error");
		return -1;
	}
	
	NK_Log()->info("write filetail, fid : %d",(int)fid);
	if(recorder){
		sprintf(fileTail.record_type, "%s", recorder->record_type);
		fileTail.frameCnt = file_par->frameCnt;
		fileTail.iFrameCnt = file_par->iFrameCnt;
		for(i = 0; i < fileTail.iFrameCnt; i++){
			fileTail.iFramePos[i] = file_par->iFramePos[i];
			NK_Log()->debug("Pos = %u",fileTail.iFramePos[i]);
			fileTail.iFrameSec[i] = file_par->iFrameSec[i];
			NK_Log()->debug("sec = %d",fileTail.iFrameSec[i]);
		}

		fileTail.beginSec = file_par->beginSec;
		fileTail.endSec = file_par->endSec;
		fileTail.filesize = file_par->filesize;

		NK_Log()->info("FileTail: \
type->%s,\
frameCnt->%d,\
iFrameCnt->%d,\
iFramePos->%d,\
iFrameSec->%d,\
beginSec->%d,\
endSec->%d,\
fileSize->%d\n",
fileTail.record_type,
fileTail.frameCnt,
fileTail.iFrameCnt,
fileTail.iFramePos[0],
fileTail.iFrameSec[0],
fileTail.beginSec,
fileTail.endSec,
fileTail.filesize);

		if(sizeof(struct file_tail_t) != fwrite(&fileTail, 1, sizeof(struct file_tail_t), fid)){
			NK_Log()->warn("write into filetail error\n");
			return -1;
		}
		return 0;
	}

	NK_Log()->info("recorder error when write fileTail");
	return -1;
}

static NK_Int create_new_file()
{
	NK_Int sec = 0;
	NK_Char maxDate[12] = {0};
	NK_Char dirPath[80] = {0};
	NK_Char filePath[128] = {0};
	NK_Char cmdLine[128] = {0};
	NK_Int checkIndex = 0;

	//check date dir
	get_utc_date(maxDate);
	if(0 != strcmp(recorder->path_par.maxDate,maxDate))
	{
		if(check_date_exist(recorder->path_par.recordPath,&checkIndex,maxDate)){
			NK_Char filePath[64] = {0};
			NK_Char fileMaxName[32] = {0};
			NK_Char fileMinName[32] = {0};
			NK_Int fileMaxIndex = 0;
			NK_Int fileMinIndex = 0;

			recorder->path_par.indexMax = checkIndex;
			snprintf(recorder->path_par.maxDate,sizeof(recorder->path_par.maxDate), "%s", maxDate);
			snprintf(filePath, sizeof(filePath), "%s/%04d-%s",recorder->path_par.recordPath,recorder->path_par.indexMax,recorder->path_par.maxDate);
			if(0 == get_index_num(filePath, &fileMaxIndex, &fileMinIndex, fileMaxName,fileMinName)){
				recorder->file_par.indexMax = fileMaxIndex + 1;
			}else{
				recorder->file_par.indexMax = 1;
			}
		}else{
			recorder->path_par.indexMax++;
			recorder->file_par.indexMax = 1;
			snprintf(recorder->path_par.maxDate,sizeof(recorder->path_par.maxDate), "%s", maxDate);
		}
	}
	snprintf(dirPath, sizeof(dirPath), "%s/%04d-%s",recorder->path_par.recordPath,recorder->path_par.indexMax,recorder->path_par.maxDate);
	if(!IS_FILE_EXIST(dirPath)){
		NK_Log()->info("create dir frist");
		snprintf(cmdLine, sizeof(cmdLine), "mkdir -p %s", dirPath);
		NK_SYSTEM(cmdLine);
	}
	//check filename
	get_sec_of_today(&sec);
	NK_Log()->info("create_new_file %d",sec);
	snprintf(recorder->file_par.fileName,sizeof(recorder->file_par.fileName), "%05d", sec);
	
	//open file
	snprintf(filePath, sizeof(filePath), "%s/%04d-%s/%s",
		recorder->path_par.recordPath,recorder->path_par.indexMax,recorder->path_par.maxDate,recorder->file_par.fileName);
	recorder->file_par.fid = fopen(filePath, "wb");
	NK_Log()->info("open fid : %d",recorder->file_par.fid);
	if(!recorder->file_par.fid){
		NK_Log()->info("create a new file failed : path = %s",filePath);
		NK_Char cmd[256] = {0};
		snprintf(cmd, sizeof(cmd), "rm -rf %s", filePath);
		NK_SYSTEM(cmd);
		return -1;
	}
	NK_Log()->info("create a new file success : path = %s",filePath);
	//fflush file_par
	recorder->file_par.frameCnt = 0;
	recorder->file_par.iFrameCnt = 0;
	recorder->file_par.beginSec = sec;
	recorder->file_par.endSec = sec;
	recorder->file_par.filesize = 0;

	return 0;
}

static NK_Int get_oldest_dir(NK_PChar dirPath, NK_PChar oldestName, NK_Size oldestNameLen)
{
    struct dirent **dirList;
    NK_Boolean gotDir;
    NK_Int list_num = 0;
    int ret;
    int i;

    list_num = scandir(dirPath, &dirList, NULL, alphasort);
    if (list_num < 0) {
        NK_Log()->error("Failed to scandir %s. Errno: %d", dirPath, errno);
        return -1;
    }

    gotDir = NK_False;
    for (i = 0; i < list_num; i++) {

        if (gotDir) {
            free(dirList[i]);
            continue;
        }

        if ((0 == strcmp(dirList[i]->d_name, ".")) || (0 == strcmp(dirList[i]->d_name, ".."))) {
            free(dirList[i]);
            continue;
        }

        ret = snprintf(oldestName, oldestNameLen, "%s", dirList[i]->d_name);
        if (ret <= 0 || ret >= oldestNameLen) {
            NK_Log()->error("%s:%d snprintf oldestName error, ret: %d, buf len: %u",
                            __FUNCTION__, __LINE__, ret, oldestNameLen);
        } else {
            gotDir = NK_True;
            free(dirList[i]);
            continue;
        }

        free(dirList[i]);
    }
    free(dirList);

    if (!gotDir) {
        return -1;
    }
	
	return 0;
}

static NK_Int get_file_cnt(NK_PChar dirPath)
{
	int fileCnt = 0;
	DIR *dirPtr = NULL;
	struct dirent *entry;

	if(NULL == (dirPtr = opendir(dirPath))){
		NK_Log()->warn("open dir error");
		return 0;
	}else{
		while(entry = readdir(dirPtr)){
			if((0 == strcmp(entry->d_name,".")) || (0 == strcmp(entry->d_name,".."))){
				continue;
			}
			else{
				fileCnt++;
			}
		}
		closedir(dirPtr);
	}

	return fileCnt;
}

static NK_Int recylce_old_file()
{
	NK_Char oldestName[64] = {0};
	NK_Char datePath[80] = {0};
	NK_Char recylce_cmd[128] = {0};
    struct stat path_stat;
    struct dirent **fileList;
    size_t files5DelCnt = 0;
    NK_Boolean dirDeleted;
    NK_Boolean freeSpaceOK;
    NK_Int list_num = 0;
    NK_Int freeSpace;
	int i = 0;

    do {
        files5DelCnt = 0;
        dirDeleted = NK_False;
        freeSpaceOK = NK_False;

        //获取最老文件夹或文件
        NK_Log()->info("Record Path: %s", recorder->path_par.recordPath);
        if (0 != get_oldest_dir(recorder->path_par.recordPath, oldestName, sizeof(oldestName))) {
            NK_Log()->warn("get oldest dir error");
            return -1;
        }
        NK_Log()->info("Oldest Dir Name: %s", oldestName);

        snprintf(datePath, sizeof(datePath), "%s/%s", recorder->path_par.recordPath, oldestName);
        NK_Log()->info("Oldest Dir Path: %s\n", datePath);


        list_num = scandir(datePath, &fileList, NULL, alphasort);

        if (0 == list_num) {
            NK_Log()->warn("scandir return 0, dir: %s", datePath);
            if (0 != rmdir(datePath)) {
                NK_Log()->error("rmdir failed, dir: %s", datePath);
            }
            free(fileList);
            return -1;
        }

        if (list_num < 0) {
            NK_Log()->error("Failed to scandir %s. Errno: %d", datePath, errno);
            if (0 != stat(datePath, &path_stat)) {
                NK_Log()->error("Failed to stat file. Errno: %d", datePath, errno);
            } else {
                if (!S_ISDIR(path_stat.st_mode)) {
                    snprintf(recylce_cmd, sizeof(recylce_cmd), "rm %s", datePath);
                    NK_SYSTEM(recylce_cmd);
                    continue;
                }
            }
            return -1;
        }


        for (i = 0; i < list_num; i++) {

            if (freeSpaceOK || dirDeleted) {
                free(fileList[i]);
                continue;
            }

            if (files5DelCnt >= 5) {

                if (!NK_TFCARD_is_mounted()) {
                    freeSpaceOK = NK_True;
                    free(fileList[i]);
                    continue;
                }

                freeSpace = recorder->getFreeSpace(recorder->path_par.mountPath);
                if (freeSpace >= 256) {
                    freeSpaceOK = NK_True;
                    free(fileList[i]);
                    continue;
                }
                files5DelCnt = 0;
            }

            // list_num include "." and ".."
            if ((list_num - i) < 7) {

                snprintf(recylce_cmd, sizeof(recylce_cmd), "rm -r %s", datePath);
                NK_Log()->info("less than 5 files, recylce dir: %s", datePath);
                NK_SYSTEM(recylce_cmd);
                dirDeleted = NK_True;

                if (!NK_TFCARD_is_mounted()) {
                    freeSpaceOK = NK_True;
                    free(fileList[i]);
                    continue;
                }

                freeSpace = recorder->getFreeSpace(recorder->path_par.mountPath);
                if (freeSpace >= 256) {
                    freeSpaceOK = NK_True;
                }

                free(fileList[i]);
                continue;
            }

            if ((0 == strcmp(fileList[i]->d_name, ".")) || (0 == strcmp(fileList[i]->d_name, ".."))) {
                free(fileList[i]);
                continue;
            }

            snprintf(recylce_cmd, sizeof(recylce_cmd), "rm -r %s/%s", datePath, fileList[i]->d_name);
            NK_SYSTEM(recylce_cmd);

            files5DelCnt++;

            free(fileList[i]);
        }
        free(fileList);

    } while (!freeSpaceOK);

	return 0;
}


static pthread_t recycle_pid = NULL;
static NK_Void *remove_file_proc(void *arg)
{
	prctl(PR_SET_NAME, "remove_file_proc");
	pthread_detach(pthread_self());
	recylce_old_file();
	recycle_pid = NULL;
}

static NK_Int recycle_files(NK_Int freeSpace)
{
	if(!recycle_pid && freeSpace < 256){
		NK_Log()->info("****Undercapacity,recylce old file now  %dMB*****", freeSpace);
		pthread_create(&recycle_pid, NULL, remove_file_proc, NULL);
	}
}

#define FILE_MAX_SIZE	(1024 * 1024 * 8)
static NK_Void TFcard_Record_write()
{
	NK_Int freeSpace = 0;
	
	NK_EXPECT_VERBOSE_RETURN((recorder != NULL));
	prctl(PR_SET_NAME, "TFcard_Record_write");

	pthread_detach(pthread_self());
	while(recorder && recorder->writer_buf.run_flag){
		pthread_mutex_lock(&recorder->writer_buf.bufMutex);
		//创建录像文件
		if(!recorder->file_par.fid){
			NK_Log()->info("fid null");

            //检查 tf 卡是否已挂载
            if (!NK_TFCARD_is_mounted()) {
                NK_Log()->warn("tf card is not mounted!");
                pthread_mutex_unlock(&recorder->writer_buf.bufMutex);
                goto STH_ERR;
            }

			//如果空间不足,回收文件
			freeSpace = recorder->getFreeSpace(recorder->path_par.mountPath);

			recycle_files(freeSpace);
			/*while(freeSpace < 200 && freeSpace != 0){
				NK_Log()->info("****Undercapacity,recylce old file now*****");
				if(0 != recylce_old_file()){
					NK_Log()->warn("recylce old file error!");
					pthread_mutex_unlock(&recorder->writer_buf.bufMutex);
					goto STH_ERR;
				}
				freeSpace = recorder->getFreeSpace(recorder->path_par.mountPath);
			}*/

            // 如果可用空间过小, 则不进行新录像
            if (freeSpace < 10) {
                NK_Log()->warn("free space of tf card is too small (< 10 MB), not create new record!");
                pthread_mutex_unlock(&recorder->writer_buf.bufMutex);
                goto STH_ERR;
            }

			if(0 != create_new_file())
			{
				NK_Log()->warn("create new file error!");
				pthread_mutex_unlock(&recorder->writer_buf.bufMutex);
				goto STH_ERR;
			}
		}
		//缓冲区收到数据
		if(recorder->writer_buf.buffer && (recorder->writer_buf.bufLen > 0)){
			if(0 != record_write_into_flash(recorder->file_par.fid,recorder->writer_buf.buffer,recorder->writer_buf.bufLen)){
				pthread_mutex_unlock(&recorder->writer_buf.bufMutex);
				goto STH_ERR;
			}
			recorder->file_par.filesize += recorder->writer_buf.bufLen;
			recorder->writer_buf.bufLen = 0;
		}
		//达到8M ,打包一个文件
		if(recorder->file_par.filesize >= FILE_MAX_SIZE || 86400 == recorder->file_par.endSec){
			if(0 != record_write_into_filetail(recorder->file_par.fid,&(recorder->file_par))){
				pthread_mutex_unlock(&recorder->writer_buf.bufMutex);
				goto STH_ERR;
			}
			fclose(recorder->file_par.fid);
			recorder->file_par.fid = NULL;

			//文件改名
			NK_Char oldName[128] = {0};
			NK_Char newName[128] = {0};

			snprintf(oldName, sizeof(oldName), "%s/%04d-%s/%05d",
				recorder->path_par.recordPath,recorder->path_par.indexMax,recorder->path_par.maxDate,recorder->file_par.beginSec);
			snprintf(newName, sizeof(newName), "%s/%04d-%s/%04d-%05d-%05d",
				recorder->path_par.recordPath,recorder->path_par.indexMax,recorder->path_par.maxDate,
				recorder->file_par.indexMax,recorder->file_par.beginSec,recorder->file_par.endSec);
			errno = 0;
			if(0 != rename(oldName,newName)){
				NK_Log()->warn("rename failed, oldName : %s \n newName : %s, errno = %d",oldName,newName, errno);
			}
			NK_Log()->info("one file ok! path : %s", newName);
		}
		pthread_mutex_unlock(&recorder->writer_buf.bufMutex);

		//NK_Log()->flush();
		usleep(10000);
	}

	//剩余数据********>>>>>>
	if(recorder->writer_buf.buffer && (recorder->writer_buf.bufLen > 0)){
		NK_Log()->info("write remain data after stop");
		pthread_mutex_lock(&recorder->writer_buf.bufMutex);
		//写入flash
		if(0 != record_write_into_flash(recorder->file_par.fid,recorder->writer_buf.buffer,recorder->writer_buf.bufLen)){
				pthread_mutex_unlock(&recorder->writer_buf.bufMutex);
				goto STH_ERR;
		}
		recorder->file_par.filesize += recorder->writer_buf.bufLen;
		recorder->writer_buf.bufLen = 0;
		pthread_mutex_unlock(&recorder->writer_buf.bufMutex);
	}
	//正常结束写入文件尾部******>>>>>
	if(recorder->file_par.fid){
		NK_Log()->info("write filetail after stop");
		pthread_mutex_lock(&recorder->writer_buf.bufMutex);
		//写入文件尾部
		if(0 != record_write_into_filetail(recorder->file_par.fid,&(recorder->file_par))){
				pthread_mutex_unlock(&recorder->writer_buf.bufMutex);
				goto STH_ERR;
		}
		fclose(recorder->file_par.fid);
		recorder->file_par.fid = NULL;

		//文件改名
		NK_Char oldName[128] = {0};
		NK_Char newName[128] = {0};

		snprintf(oldName, sizeof(oldName), "%s/%04d-%s/%05d",
			recorder->path_par.recordPath,recorder->path_par.indexMax,recorder->path_par.maxDate,recorder->file_par.beginSec);
		snprintf(newName, sizeof(newName), "%s/%04d-%s/%04d-%05d-%05d",
			recorder->path_par.recordPath,recorder->path_par.indexMax,recorder->path_par.maxDate,
			recorder->file_par.indexMax,recorder->file_par.beginSec,recorder->file_par.endSec);
		errno = 0;
		if(0 != rename(oldName,newName)){
			NK_Log()->warn("rename failed, oldName : %s \n newName : %s, errno = %d",oldName,newName, errno);
		}
		NK_Log()->info("one file ok! path : %s",newName);
		pthread_mutex_unlock(&recorder->writer_buf.bufMutex);
	}

	NK_Log()->flush();
	// 释放缓冲区,清除信号量
STH_ERR:
	NK_Log()->info("exit write pthread begin!!!");
	//通知写帧函数停止送帧
	recorder->writer_buf.run_flag =  NK_False;
	recorder->tmp_buf.run_flag = NK_False;

	pthread_mutex_lock(&recorder->writer_buf.bufMutex);
	if(recorder->writer_buf.buffer){
		free(recorder->writer_buf.buffer);
	}
	recorder->writer_buf.buffer = NULL;
	pthread_mutex_unlock(&recorder->writer_buf.bufMutex);

	if(recorder->file_par.fid){
		fclose(recorder->file_par.fid);
	}
	recorder->file_par.fid = NULL;

	recorder->writerTid = THREAD_ZEROID();
	NK_Log()->info("exit write pthread end!!!");
	pthread_exit(NULL);
}

NK_Int TFcard_Record_write_frame(lpRecord_Frame_Head frameHead,NK_PByte data)
{	
	int ret = 0;

	NK_EXPECT_VERBOSE_RETURN_VAL((frameHead != NULL && data != NULL),-1);

	//检查是否启动录像 && 录像写线程是否正常运行
	if(recorder && (recorder->writer_buf.run_flag) &&
		(recorder->tmp_buf.buffer) && (recorder->tmp_buf.run_flag))
	{	
		pthread_mutex_lock(&recorder->tmp_buf.bufMutex);
		ret = record_write_into_buf(frameHead,data);
		if(0 != ret){
			NK_Log()->warn("write frame : write_into_buf failed");
			pthread_mutex_unlock(&recorder->tmp_buf.bufMutex);
			return -1;
		}
		pthread_mutex_unlock(&recorder->tmp_buf.bufMutex);
		return 0;
	}

	return -1;
}

NK_Int TFcard_Record_stop()
{
    int ret;

	if (NULL != recorder) {

		// 必须首先触发录像写线程退出，不然可能引起段错误
		recorder->writer_buf.run_flag = NK_False;

		//等待录像写线程退出
		while (!THREAD_IS_ZEROID(recorder->writerTid)) {
			usleep(10000);
		}

		//清除缓冲区
		if (NULL != recorder->writer_buf.buffer) {
			free(recorder->writer_buf.buffer);
            recorder->writer_buf.buffer = NULL;
		}
		if (NULL != recorder->tmp_buf.buffer) {
			free(recorder->tmp_buf.buffer);
            recorder->tmp_buf.buffer = NULL;
		}
		if (NULL != recorder->file_par.fid) {
			fclose(recorder->file_par.fid);
            recorder->file_par.fid = NULL;
		}

        ret = pthread_mutex_destroy(&recorder->writer_buf.bufMutex);
        if (ret != 0) {
            NK_Log()->error("%s:%d writer buf mutex destroy error, ret: %d",
                            __FUNCTION__, __LINE__, ret);
        }

        ret = pthread_mutex_destroy(&recorder->tmp_buf.bufMutex);
        if (ret != 0) {
            NK_Log()->error("%s:%d tmp buf mutex destroy error, ret: %d",
                            __FUNCTION__, __LINE__, ret);
        }

		if (NULL != recorder) {
			free(recorder);
            recorder = NULL;
		}
#ifdef LED_CTRL
        initLedContrl(LED_REC_ID, true, LED_DARK_MODE);
#endif
	}

	return 0;
}

static NK_Int recorder_init(NK_PChar record_type)
{
	if(!record_type || 0 == strlen(record_type)){
		record_type = "default";
	}
	sprintf(recorder->record_type, "%s", record_type);
}

static NK_Int path_par_init(NK_PChar mountPath)
{
	NK_Int max = 0,min = 0;
	NK_Char maxName[20] = {0};
	NK_Char minName[20] = {0};
	NK_Char mkdirPath[64] = {0};
	NK_Char checkMaxDate[20] = {0};
	NK_Int checkMaxIndex = 0;

	snprintf(recorder->path_par.mountPath, sizeof(recorder->path_par.mountPath), "%s", mountPath);
	snprintf(recorder->path_par.recordPath,sizeof(recorder->path_par.recordPath), "%s/record",mountPath);
	if(!IS_FILE_EXIST(recorder->path_par.recordPath)){
		snprintf(mkdirPath, sizeof(mkdirPath), "mkdir -p %s", recorder->path_par.recordPath);
		NK_SYSTEM(mkdirPath);
	}
	get_utc_date(checkMaxDate);
	if(check_date_exist(recorder->path_par.recordPath,&checkMaxIndex,checkMaxDate)){
		recorder->path_par.indexMax = checkMaxIndex;
		snprintf(recorder->path_par.maxDate, sizeof(recorder->path_par.maxDate), "%s", checkMaxDate);
		return 0;
	}else{
		if(0 == get_index_num(recorder->path_par.recordPath,&max,&min,maxName,minName)){
			recorder->path_par.indexMax = max;
			recorder->path_par.indexMin = min;
			snprintf(recorder->path_par.maxDate, sizeof(recorder->path_par.maxDate), "%s", maxName);
			snprintf(recorder->path_par.minDate, sizeof(recorder->path_par.minDate), "%s", minName);
			return 0;
		}
	}
	return -1;
}

static NK_Int file_par_init()
{
	NK_Int max = 0,min = 0;
	NK_Char maxName[20] = {0};
	NK_Char minName[20] = {0};
	NK_Char fileIndexPath[64] = {0};

	memset(&recorder->file_par, 0, sizeof(struct file_par_t));
	memset(&recorder->tmp_file_par, 0, sizeof(struct file_par_t));
	
	snprintf(fileIndexPath, sizeof(fileIndexPath), "%s/%04d-%s",
		recorder->path_par.recordPath,recorder->path_par.indexMax,recorder->path_par.maxDate);
	//只获取最大索引值,其他值无意义. 不可使用
	if(0 == get_index_num(fileIndexPath, &max, &min, maxName,minName)){
		recorder->file_par.indexMax = max + 1;
	}else{
		recorder->file_par.indexMax = 1;
	}
	recorder->tmp_file_par.beginSec = -1;
	recorder->tmp_file_par.endSec = -1;
	recorder->tmp_file_par.filesize = 0;
	recorder->file_par.beginSec = -1;
	recorder->file_par.endSec = -1;
	recorder->file_par.filesize = 0;
	recorder->file_par.fid = NULL;
}

static NK_Int mem_size_par_init(NK_Size memSize)
{
	if(memSize < 384){
		NK_Log()->info("mem size error,it must large than 384, size = %d",memSize);
		recorder->mem_size_par.bufferMallocSize = (384 * 1024);
		//return -1;
	}else{
		recorder->mem_size_par.bufferMallocSize = (memSize * 1024);
	}

	return 0;
}

static NK_Int buffer_par_init(NK_Size memSize)
{
    int ret;
    bool writerBufMuxCreated = false;
    bool tmpBufMuxCreated = false;

    recorder->writer_buf.buffer = NULL;
    recorder->tmp_buf.buffer = NULL;


	//writer
	recorder->writer_buf.buffer = calloc(1, memSize);
	if (NULL == recorder->writer_buf.buffer) {
		NK_Log()->error("writer buf calloc error");
		goto ERR_RETURN;
	}

    ret = pthread_mutex_init(&recorder->writer_buf.bufMutex, NULL);
    if (0 != ret) {
        NK_Log()->error("writer buf mutex init error, ret: %d", ret);
        goto ERR_RETURN;
    }
    writerBufMuxCreated = true;

    recorder->writer_buf.bufLen = 0;
	recorder->writer_buf.run_flag = NK_True;


	//tmp buffer
	recorder->tmp_buf.buffer = calloc(1, memSize);
	if (NULL == recorder->tmp_buf.buffer) {
		NK_Log()->error("tmp buf calloc error");
		goto ERR_RETURN;
	}

    ret = pthread_mutex_init(&recorder->tmp_buf.bufMutex, NULL);
    if (0 != ret) {
        NK_Log()->error("tmp buf mutex init error, ret: %d", ret);
        goto ERR_RETURN;
    }
    tmpBufMuxCreated = true;

    recorder->tmp_buf.bufLen = 0;
	recorder->tmp_buf.run_flag = NK_True;

	return 0;


ERR_RETURN:

    if (NULL != recorder->writer_buf.buffer) {
        free(recorder->writer_buf.buffer);
        recorder->writer_buf.buffer = NULL;
    }

    if (NULL != recorder->tmp_buf.buffer) {
        free(recorder->tmp_buf.buffer);
        recorder->tmp_buf.buffer = NULL;
    }

    if (writerBufMuxCreated) {
        ret = pthread_mutex_destroy(&recorder->writer_buf.bufMutex);
        if (ret != 0) {
            NK_Log()->error("%s:%d writer buf mutex destroy error, ret: %d",
                            __FUNCTION__, __LINE__, ret);
        }
    }

    if (tmpBufMuxCreated) {
        ret = pthread_mutex_destroy(&recorder->tmp_buf.bufMutex);
        if (ret != 0) {
            NK_Log()->error("%s:%d tmp buf mutex destroy error, ret: %d",
                            __FUNCTION__, __LINE__, ret);
        }
    }

    return -1;
}

NK_Int TFcard_Record_init(NK_PChar record_type,fTFcardOnGetFreeSpace getFreeSpace,NK_PChar mountPath,NK_Size maxBufferSizeKb)
{
	NK_Int ret;
	
	if(!recorder){
		recorder = calloc(1,sizeof(stTFcard_Record));
		if(recorder == NULL){
			NK_Log()->error("recorder handle calloc error!");
			return -1;
		}
		recorder_init(record_type);
		path_par_init(mountPath);
		file_par_init();
		if(0 != mem_size_par_init(maxBufferSizeKb)){
			if(recorder){
				free(recorder);
				recorder = NULL;
			}
			return -1;
		}
		if(0 != buffer_par_init(recorder->mem_size_par.bufferMallocSize)){
			if(recorder){
				free(recorder);
				recorder = NULL;
			}
			return -1;
		}
		//回调-获取TF卡剩余容量
		recorder->getFreeSpace = getFreeSpace;
		pthread_create(&recorder->writerTid, NULL, TFcard_Record_write, NULL);
		NK_Log()->info("TFcard_Record_init ok");
#ifdef LED_CTRL
        initLedContrl(LED_REC_ID, true, LED_MIN_MODE);
#endif
		return 0;
	}

	return -1;
}



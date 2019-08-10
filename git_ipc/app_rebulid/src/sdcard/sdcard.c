
#include "sdcard.h"
#include <assert.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/types.h>
//#include <netlink-kernel.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <fcntl.h>
#include <unistd.h>
#include "sdcard_db.h"
#include "generic.h"
#include "frank_trace.h"
#include <sys/prctl.h>

#define kSD_DETECT_UNKWON_ERR (-1)
#define kSD_DETECT_OK (0)
#define kSD_DETECT_DEV_NOT_EXIST (1000)
#define kSD_DETECT_DEV_NOT_FORMAT (2000)
//#define 

typedef struct SDCARD_MEDIA
{
	int detectCode;

	int stat;
	
	bool formatRequest;

	char mountPoint[128];
	char mountDIR[128];
	char cachePath[128];
	
	bool available;

	// recorder controller
	bool recorderON;
	pthread_t recorderTID;
	fSDCARD_RECORDER_APPLICATION recorderApp;

	// listener controller
	bool listenerON;
	pthread_t listenerTID;
	
}ST_SDCARD_MEDIA, *LP_SDCARD_MEDIA;

static ST_SDCARD_MEDIA _sdCardMedia =
{
	.detectCode = kSD_DETECT_UNKWON_ERR,

	.stat = kSD_STAT_NOT_INIT,
	
	.formatRequest = false,

	.mountPoint = {""},
	.mountDIR = {""},
	.cachePath = {""},
	
	.available = false,

	.recorderON = false,
	.recorderTID = (pthread_t)NULL,
	.recorderApp = NULL,
	
};
static LP_SDCARD_MEDIA _lpSDCardMedia = NULL;

bool SDCARD_available()
{
	if(_lpSDCardMedia){
		return _lpSDCardMedia->available;
	}
	return false;
}

static int sdcard_check_dir_exist()
{
	int fd;
	fd = open("/media/sdcard/media/1.txt", O_RDWR | O_CREAT | O_DIRECT);
	if(fd <= 0){
		return 0;
	}else{
		close(fd);
		system("rm /media/sdcard/media/1.txt");
		fd = open("/media/sdcard/gallery/2.txt", O_RDWR | O_CREAT | O_DIRECT);
		if(fd <= 0){
			return 0;
		}
		close(fd);
		system("rm /media/sdcard/gallery/2.txt");
	} 
	return 1;
}

static int sdcard_try_mount(const char *deviceName, const char *mountDIR)
{
	if(IS_FILE_EXIST(deviceName)){
		
		MAKE_DIRECTORY(mountDIR);
		_ASSERT(CHECK_DIR_EXIST(mountDIR), "Create Directory \"%s\" Error!", mountDIR);
			
		umount2(mountDIR, MNT_FORCE | MNT_DETACH); // like 'umount -l'
		//if(0 == mount(deviceName, mountDIR, "vfat", 0, NULL)){
		if(0 == mount(deviceName, mountDIR, "vfat", MS_NOATIME, NULL)){
			_TRACE("Mount FileSystem Success");
			_lpSDCardMedia->detectCode = kSD_DETECT_OK;

			// success to mount
			// FIXME:
			char cmd[128] = {""};
			snprintf(cmd, sizeof(cmd), "ls -l %s/*.REC", mountDIR);
			_TRACE("List Files \"%s\"", cmd);
			system(cmd);
			
			snprintf(cmd, sizeof(cmd), "rm -f %s/*.REC", mountDIR);
			_TRACE("Remove Files \"%s\"", cmd);
			system(cmd);
			return 0;
		}else{
			//perror("mount");
			_TRACE("Mount FileSystem Error (errno=%d pattern=%s)", errno, strerror(errno));
			_lpSDCardMedia->detectCode = kSD_DETECT_DEV_NOT_FORMAT;
		}
	}else{
		_TRACE("Device Not Existed");
		_lpSDCardMedia->detectCode = kSD_DETECT_DEV_NOT_EXIST;
	}
	return -1;
}

static void sdcard_umount(const char *mountDIR)
{
	if(_lpSDCardMedia){
		_TRACE("File System Umount \"%s\"", mountDIR);
		umount2(mountDIR, MNT_DETACH); // like 'umount -l'
	}
}

static int sdcard_fsck(const char *deviceName)
{
	char fsckCmd[128] = {""};
	snprintf(fsckCmd, sizeof(fsckCmd), "/usr/share/ipcam/sdcard_tools/fsck.fat -v -a %s", deviceName);
	system(fsckCmd);
	return 0;
}

static int sdcard_fdisk()
{
	const char *scriptPath = "/tmp/fdisk.script";
	char sysCommand[64];
	FILE* fID = NULL;
	const char scriptContent[] =
		"d\n"
		"1\n"
		"d\n"
		"2\n"
		"d\n"
		"3\n"
		"d\n"
		"4\n"
		"n\n"
		"p\n" // primary partition
		"1\n" // partition number
		"\n" // first cylinder
		"\n" // last cylinder
		"w\n"; // write

	fID = fopen(scriptPath, "w+b");
	if(NULL != fID){
		fwrite(scriptContent, 1, strlen(scriptContent), fID);
        fclose(fID);
		// run system
		// FIXME:

		// fdisk
		snprintf(sysCommand, sizeof(sysCommand), "fdisk /dev/mmcblk0 < %s", scriptPath);
		_TRACE("FDisk \"%s\"", sysCommand);
		system(sysCommand);
		REMOVE_FILE(scriptPath);

		return 0;
	}
	return -1;
}

static int sdcard_format()
{
	bool formatted = false;
    char cmd[256] = {0};
	if(0 == sdcard_fdisk()){
		// FIXME:
        snprintf(cmd, sizeof(cmd), "%s/sdcard_tools/mkfs.fat -F 32 /dev/mmcblk0p1", IPCAM_ENV_HOME_DIR);
		system(cmd);

		// create base filesystem
		if(0 == sdcard_try_mount(_lpSDCardMedia->mountPoint, _lpSDCardMedia->mountDIR)){
			system("mkdir -p /media/sdcard/media"); // FIXME:
			system("mkdir -p /media/sdcard/gallery"); // FIXME:

			sdcard_umount(_lpSDCardMedia->mountDIR);
			_TRACE("Format Success!!");
			formatted = true;
		}
	}

	return formatted ? 0 : -1;
}

static bool sdcard_recorder_access()
{
	return (_lpSDCardMedia
		&& _lpSDCardMedia->recorderON
		&& !_lpSDCardMedia->formatRequest) ? true : false;
}

#define SDCARD_NOT_RECORDER_NOW	0

#if SDCARD_NOT_RECORDER_NOW
static void sdcard_not_recorder_now(void)
{
	char buf_name[64] = {0};

	if(_lpSDCardMedia && _lpSDCardMedia->recorderON)
	{
		memset(buf_name, 0, sizeof(buf_name));
		snprintf(buf_name, sizeof(buf_name), "%s/media", _lpSDCardMedia->mountDIR);

		while(false == _lpSDCardMedia->formatRequest && 0 == access(_lpSDCardMedia->mountPoint, F_OK) && 0 == access(buf_name, F_OK))
		{
			printf("It's working...........................\n");
			sleep(2);
		}
	}
}
#endif

static void *sdcard_recorder(void *arg)
{
	int sdCardMount = 0;
	int sdCardFSCheck = false;
	
	pthread_detach(pthread_self()); // detach mode
	prctl(PR_SET_NAME, "sdcard_recorder");
	while(_lpSDCardMedia && _lpSDCardMedia->recorderON){
		// prepare the sd-card
		if(_lpSDCardMedia->formatRequest){
			_TRACE("Begin to Format SD Card!");
			_lpSDCardMedia->stat = kSD_STAT_FORMATTING; // formatting
			sdcard_umount(_lpSDCardMedia->mountDIR); // umount firstly
			sdcard_format();
			_TRACE("End to Format SD Card!");
			_lpSDCardMedia->formatRequest = false; // clear flag
			// prepare the 
		}else{		
			sdCardMount = sdcard_try_mount(_lpSDCardMedia->mountPoint, _lpSDCardMedia->mountDIR);
			_TRACE("Try Mount %d", sdCardMount);
			if(0 != sdCardMount){
				switch(_lpSDCardMedia->detectCode){
				case kSD_DETECT_UNKWON_ERR: // seem impossible
				case kSD_DETECT_OK: // impossible too
					break;
				
				case kSD_DETECT_DEV_NOT_EXIST:
					// keep detecting and wait for injecteing
					_lpSDCardMedia->stat = kSD_STAT_EJECTED;
					sleep(1);
					break;
					
				case kSD_DETECT_DEV_NOT_FORMAT:
					_lpSDCardMedia->stat = kSD_STAT_FS_ERROR;
					sleep(1);
					break;
				default:
					break;
				}
			}else{
				if(!sdcard_check_dir_exist()){
					_TRACE("Need to format!");
					_lpSDCardMedia->detectCode = kSD_DETECT_DEV_NOT_FORMAT;
					_lpSDCardMedia->stat = kSD_STAT_FS_ERROR;
					sleep(1);
					continue;
				}
				_lpSDCardMedia->stat = kSD_STAT_ON_WORK;
				if(!sdCardFSCheck){
					sdcard_umount(_lpSDCardMedia->mountDIR); // umount firstly
					sdcard_fsck(_lpSDCardMedia->mountPoint);
					sdCardFSCheck = true;
					continue;
				}else{
					int mediaID = -1;
					int sdCardFreeSize = 0;
					char dbPath[128];

					snprintf(dbPath, sizeof(dbPath), "%s/sdcard.db", _lpSDCardMedia->mountDIR);
					
					if(0 == SDCARD_db_open(dbPath, _lpSDCardMedia->cachePath)){
						// enter recorder writer callback
#if SDCARD_NOT_RECORDER_NOW
						sdcard_not_recorder_now();
#else
						_lpSDCardMedia->recorderApp(sdcard_recorder_access);
#endif
						SDCARD_db_close();
					}else{
						// report database destroyed
						if(0 == SDCARD_db_create(dbPath, _lpSDCardMedia->cachePath)){
							// enter recorder writer callback
#if SDCARD_NOT_RECORDER_NOW
							sdcard_not_recorder_now();
#else
							_lpSDCardMedia->recorderApp(sdcard_recorder_access);
#endif
							SDCARD_db_close();
						}
					}
			
					sdcard_umount(_lpSDCardMedia->mountDIR);
					sdCardFSCheck = false; // next time you need to check again
				}
			}
		}
	}

	// double check the sdcard ejected
	//sdcard_umount();
	_lpSDCardMedia->stat = kSD_STAT_NOT_INIT;

	// cleanup the environment
	_lpSDCardMedia->recorderON = false;
	_lpSDCardMedia->recorderTID = THREAD_ZEROID();
	pthread_exit(NULL);
}

int SDCARD_request_format()
{
	if(_lpSDCardMedia){
		_lpSDCardMedia->formatRequest = true;
		_TRACE("Request SD Card Format!");
		return 0;
	}
	return -1;
}

int SDCARD_status()
{
	if(_lpSDCardMedia){
		return _lpSDCardMedia->stat;
	}
	return kSD_STAT_NOT_INIT;
}

static void *sdcard_listener(void *arg)
{
	int ret = 0;
	int sockHotPlug = -1;
	char recvBuf[2 * 1024];
	int recvBufMax = 4 * 1024;
	struct sockaddr_nl snl;

	memset(&snl, 0, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	//open socket
	sockHotPlug = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	_ASSERT(sockHotPlug > 0, "Socket Hot Plug Init Error!");
	
	setsockopt(sockHotPlug, SOL_SOCKET, SO_RCVBUFFORCE, &recvBufMax, sizeof(recvBufMax)); 
	ret = bind(sockHotPlug, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
	_ASSERT(0 == ret, "Socket Hot Bind Error!");

	prctl(PR_SET_NAME, "sdcard_listener");
	pthread_detach(pthread_self());
	while(_lpSDCardMedia->listenerON){
		int recvN = -1;
		
		recvN = recv(sockHotPlug, &recvBuf, sizeof(recvBuf), MSG_DONTWAIT);
		if(recvN <= 0){
			sleep(1);
			continue;
		}

		recvBuf[recvN] = 0;
		_TRACE("Receive %s", recvBuf);
		// TODO:
	}

	// close netlink socket
	close(sockHotPlug);
	sockHotPlug = -1;
	// clean up
	_lpSDCardMedia->listenerON = false;
	_lpSDCardMedia->listenerTID = THREAD_ZEROID();
	pthread_exit(NULL);
}

static void sdcard_start_listener()
{
	int ret = -1;
	if(THREAD_IS_ZEROID(_lpSDCardMedia->listenerTID)){
		_lpSDCardMedia->listenerON = true;
		ret = pthread_create(&_lpSDCardMedia->listenerTID, NULL, sdcard_listener, NULL);
		_ASSERT(0 == ret, "Create Thread Error!");
		usleep(10000);
		return 0;
	}
	return -1;
}

static void sdcard_stop_listener()
{
	_lpSDCardMedia->listenerON = false;
	while(!THREAD_IS_ZEROID(_lpSDCardMedia->listenerTID)){
		usleep(20000); // wait until the thread exit
	}
}

static void sdcard_start_recorder()
{
	int ret = -1;
	if(THREAD_IS_ZEROID(_lpSDCardMedia->recorderTID)){
		_lpSDCardMedia->recorderON = true;
		ret = pthread_create(&_lpSDCardMedia->recorderTID, NULL, sdcard_recorder, NULL);
		_ASSERT(0 == ret, "Create Thread Error!");
		usleep(10000);
	}
	return -1;
}

static void sdcard_stop_recorder()
{
	_lpSDCardMedia->recorderON = false;
	while(!THREAD_IS_ZEROID(_lpSDCardMedia->recorderTID)){
		usleep(20000); // wait until the thread exit
	}
}

int SDCARD_init(const char *mountPoint, const char *mountDIR, const char *cachePath, fSDCARD_RECORDER_APPLICATION recorderApp)
{
	int ret = -1;
	if(!_lpSDCardMedia){
		
		_lpSDCardMedia = &_sdCardMedia;
		// init elements
		strncpy(_lpSDCardMedia->mountPoint, mountPoint, sizeof(_lpSDCardMedia->mountPoint));
		strncpy(_lpSDCardMedia->mountDIR, mountDIR, sizeof(_lpSDCardMedia->mountDIR));
		strncpy(_lpSDCardMedia->cachePath, cachePath, sizeof(_lpSDCardMedia->cachePath));
		
		_lpSDCardMedia->available = false;
		
		_lpSDCardMedia->recorderON = false;
		_lpSDCardMedia->recorderTID = (pthread_t)NULL;
		_lpSDCardMedia->recorderApp = recorderApp;

		// start background thread
		sdcard_start_recorder();
		sdcard_start_listener();
		return 0;
	}
	return -1;
}

void SDCARD_destroy()
{
	if(_lpSDCardMedia){
		// stop background thread
		sdcard_stop_listener();
		sdcard_stop_recorder();
		
		// release
		_lpSDCardMedia = NULL;
	}
}


/////////////////////////////////////////////////////////////////////////////////////
// sdcard  writer
///////////////


#define kSDCARD_WRITER_CACHE_SIZE (1024 * 1024)

typedef struct SDCARD_WRITER {
	int fID;

	void *cache;
	int cacheMax;
	int cachePtr;
	
}ST_SDCARD_WRITER, *LP_SDCARD_WRITER;

const char *sdcard_media_filepath(int fileID, char *result, int resultMax)
{
	snprintf(result, resultMax, "%s/media/file%016d.dat", _lpSDCardMedia->mountDIR, fileID);
	return result;
}

int SDCARD_media_open_range(int beginFileID, int endFileID, int *fIDs, int nFIDMax)
{
	int i = 0;
	char filePath[128];
	int rangeID = 0;
	
	for(i = beginFileID; i <= endFileID && rangeID < nFIDMax; ++i){
		sdcard_media_filepath(i, filePath, sizeof(filePath));
		fIDs[rangeID] = open(filePath, O_RDONLY);
		if(fIDs[rangeID] > 0){
			_TRACE("Opening File \"%s\"", filePath);
			++rangeID;
		}else{
			_TRACE("Failed to Open File \"%s\"", filePath);
		}
	}
	return rangeID;
}

int SDCARD_media_close_range(int *fIDs, int nFIDMax)
{
	int i = 0;
	for(i = 0; i < nFIDMax; ++i){
		close(fIDs[i]);
		fIDs[i] = -1;
	}
	return 0;
}

void *SDCARD_media_open(int fileID)
{
	char filePath[128];
	int const pageSize = getpagesize();
	LP_SDCARD_WRITER wtr = calloc(sizeof(ST_SDCARD_WRITER) + kSDCARD_WRITER_CACHE_SIZE + pageSize, 1);
	if(NULL != wtr){
		sdcard_media_filepath(fileID, filePath, sizeof(filePath));
		wtr->fID = open(filePath, O_RDWR | O_CREAT | O_DIRECT);
		_ASSERT(wtr->fID > 0, "Create File \"%s\" Failed!", filePath);

		wtr->cache = (void *)(wtr + 1);
		while(0 != (((int)wtr->cache) % pageSize)){
			++wtr->cache;
		}
		wtr->cacheMax = kSDCARD_WRITER_CACHE_SIZE;
		wtr->cachePtr = 0;

		_TRACE("Cache Open File \"%s\" Success!", filePath);
		return (void *)wtr;
	}
	return NULL;
}

void SDCARD_media_close(void *fID)
{
	LP_SDCARD_WRITER wtr = (LP_SDCARD_WRITER)fID;
	if(NULL != wtr){
		close(wtr->fID);
		wtr->fID = -1;
		free(wtr);
	}
}

bool SDCARD_media_exist(int fileID)
{
	char filePath[128];
	snprintf(filePath, sizeof(filePath), "%s/media/file%016d.dat", _lpSDCardMedia->mountDIR, fileID);
	return CHECK_FILE_EXIST(filePath) ? true : false;
}

int SDCARD_media_remove(int fileID)
{
	char filePath[128];
	snprintf(filePath, sizeof(filePath), "%s/media/file%016d.dat", _lpSDCardMedia->mountDIR, fileID);
	REMOVE_FILE(filePath);
	return CHECK_FILE_EXIST(filePath) ? -1 : 0;
}

static int cache_write(LP_SDCARD_WRITER wtr, void *data, int dataLength)
{
	int writeN = -1;
	if(dataLength > 0){
		int const cacheFree = wtr->cacheMax - wtr->cachePtr;
		
		if(dataLength < cacheFree){
			// it can be writen to cache
			memcpy(wtr->cache + wtr->cachePtr, data, dataLength);
			wtr->cachePtr += dataLength; // remember to move the pointer it's very important
			//_TRACE("Cache Pointer=%d Max=%d", wtr->cachePtr, wtr->cacheMax);
			return dataLength;
		}else{
			memcpy(wtr->cache + wtr->cachePtr, data, cacheFree);
			// to the remaining data
			data += cacheFree;
			dataLength -= cacheFree;
			wtr->cachePtr += cacheFree;
			_ASSERT(wtr->cachePtr == wtr->cacheMax, "Something Error!");
			// reset the cache context
			
			wtr->cachePtr = 0; // reset the cache pointer
			// flush to file
			writeN = write(wtr->fID, wtr->cache, wtr->cacheMax);
			if(writeN != wtr->cacheMax){
				_TRACE("Cache Flush Error(%d) \"%s\"", errno, strerror(errno));
				return -1;
			}

			return cacheFree + cache_write(wtr, data, dataLength);
		}
	}
	return -1;
}

static int cache_tell(LP_SDCARD_WRITER wtr)
{
	int pos = lseek(wtr->fID, 0, SEEK_CUR);
	if(pos >= 0){
		pos += wtr->cachePtr;
	}
	return pos;
}

// +----------------------------------
// | magic ("FRANK ") 6bytes
// +----------------------------------
// | xml length 2bytes
// +----------------------------------
// | xml header
// +----------------------------------
// | payload
// +----------------------------------
// | check sum (8 + xml header length + payload length) 4 byte
// +----------------------------------

int SDCARD_media_write(void *fID, const LP_SDCARD_FRAMER framer, void *data, int dataLength)
{
	LP_SDCARD_WRITER const writer = (LP_SDCARD_WRITER)fID;
	int writeN = 0;
	int framerLength = 0;
	char framerBuf[1024];
	int checkSum = 0;

	framerLength = SDCARD_framer_make(framer, framerBuf, sizeof(framerBuf));
	_ASSERT(framerLength > 0, "Framer Make Error!");

	// write header to file
	writeN = cache_write(writer, framerBuf, framerLength);
	if(writeN != framerLength){
		return -1;
	}

	// write payload data to file
	if(NULL != data && dataLength > 0){
		writeN = cache_write(writer, data, dataLength);
		if(writeN != dataLength){
			return -1;
		}
	}else{
		dataLength = 0;
	}

	// calculate the total size
	checkSum = htonl(framerLength + dataLength);

	// write protected size
	writeN = cache_write(writer, &checkSum, sizeof(checkSum));
	if(writeN != sizeof(checkSum)){
		return -1;
	}

	//_TRACE("Writer success to write %d", frameHeadLength + dataLength);
	return framerLength + dataLength + sizeof(checkSum);
}

int SDCARD_media_read(int fID, LP_SDCARD_FRAMER framer, void *data, int dataMax)
{
	int parseLength = 0, readN = 0;

	parseLength = SDCARD_framer_parse(fID, framer, data, dataMax);
	if(parseLength > 0){
		int checkSum = 0;
		readN = read(fID, &checkSum, sizeof(checkSum));
		if(sizeof(checkSum) != readN){
			perror("read");
			return -1;
		}
		// check sum
		if(htonl(parseLength) == checkSum){
			//_TRACE("Success to Read Media Length = %d", framer->dataLength);
			return parseLength + sizeof(checkSum);
		}else{
			_TRACE("Check Sum %d/%d Error", parseLength, ntohl(checkSum));
		}
	}
	//_TRACE("Parse Length = %d", parseLength);
	return -1;
}

int SDCARD_media_tell(void *fID)
{
	LP_SDCARD_WRITER const wtr = (LP_SDCARD_WRITER)fID;
	return cache_tell(wtr);
}


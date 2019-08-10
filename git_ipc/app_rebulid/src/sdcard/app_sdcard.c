
#include "app_sdcard.h"
#include "ticker.h"
#include "generic.h"
#include "frank_trace.h"
#include "media_buf.h"
#include "sdk/sdk_api.h"

static int _channelID = 1;
static int _sessionID = 0;
static int _beginUTC = 0;
static int _beginFileID = 0;
static int _beginFileOffset = 0;
static int _endUTC = 0;
static int _endFileID = 0;
static int _endFileOffset = 0;
static const char *_type = NULL;

static int _baseTimestampMS = 0;
static int _timestampMS = 0;
static int _timeS = 0;

#define kSDCARD_MEDIA_DISK_RESERVED_MB (64 * 2)
//#define kSDCARD_MEDIA_DISK_RESERVED_MB (1024 + 512)
#define kSDCARD_MEDIA_FILE_FULL_BONDARY_BYTE (64 * 1024 * 1024 - 2 * 1024) // byte
//#define kSDCARD_MEDIA_FILE_FULL_BONDARY_BYTE (16 * 1024 * 1024 - 2 * 1024) // byte
#define kSDCARD_MEDIA_FILE_FLUSH_TIME_S (30)

static int app_sdcard_start_session(int channelID, int utc, int fileID, int fileOffset, const char *type)
{
	if(0 == _sessionID){
		time_t const t = time(NULL);
		
		// ignore channelID here
		_beginUTC = _endUTC = utc ? utc : t;
		_beginFileID = _endFileID = fileID;
		_beginFileOffset = _endFileOffset = fileOffset;
		if(NULL != _type){
			free(_type);
			_type = NULL;
		}
		_type = strdup(type);
		_sessionID = SDCARD_db_media_create_session(channelID, _endUTC, _endFileID, _endFileOffset, _type);
		_baseTimestampMS = 0; // clear timestamp
		_ASSERT(_sessionID > 0, "SD Card Database Create Session Failed!");
		SDCARD_db_flush();
		return 0;
	}
	return -1;
}

static int app_sdcard_update_session(int sessionID, int utc, int fileID, int fileOffset)
{
	int ret = -1;
	if(0 != _sessionID){
		time_t const t = time(NULL);
		
		// ignore channelID here
		_endUTC = utc ? utc : t;
		_endFileID = fileID;
		_endFileOffset = fileOffset;
		ret = SDCARD_db_media_update_session(_sessionID, _endUTC, _endFileID, _endFileOffset);
		_ASSERT(0 == ret, "SD Card Database Update Session Failed!");
		return 0;
	}
	return -1;
}

static int app_sdcard_stop_session(int sessionID)
{
	if(0 != _sessionID){
		//SDCARD_db_flush(); // flush database
		// clean up context
		_sessionID = 0;
		_beginUTC = 0;
		_beginFileID = 0;
		_beginFileOffset = 0;
		_endUTC = 0;
		_endFileID = 0;
		_endFileOffset = 0;		
		free(_type);
		_type = NULL;
		_baseTimestampMS = 0; // clear timestamp
		return 0;
	}
	return -1;
}

static int app_sdcard_restart_session(int channelID, int utc, int fileID, int fileOffset, const char *type)
{
	int ret = 0;
	if(_sessionID){
		ret = app_sdcard_update_session(_sessionID, 0, fileID, fileOffset);
		_ASSERT(0 == ret, "SD Card Update Session Failed!");
		// new a session
		ret = app_sdcard_stop_session(_sessionID);
		_ASSERT(0 == ret, "SD Card Commit Session Failed!");
	}
	ret = app_sdcard_start_session(_channelID, 0, fileID, fileOffset, type);
	_ASSERT(0 == ret, "SD Card Start Session Failed!");
	return 0;
}

static int app_sdcard_add_thumbnail(int fID)
{
	// ad a picture as a thumbnail
	int err = 0;
	int writeN = 0;
	char filePath[64];
	FILE *picFID = NULL;
	int picSize = 0;
	
	snprintf(filePath, sizeof(filePath), "/tmp/SDCardThumbNail%08x%08x%08x.jpg", (unsigned int)time(NULL), (unsigned int)getpid(), (unsigned int)rand());
	picFID = fopen(filePath, "w+b"); // write to temp file
	if(NULL != picFID){
		if(0 == sdk_enc->snapshot(_channelID - 1, kSDK_ENC_SNAPSHOT_QUALITY_HIGH, 320, 240, picFID)){
			// success
			GET_FILE_SIZE(filePath, picSize);
			//_TRACE("Snapshort Size %d", picSize);
		}
		fclose(picFID);
		picFID = NULL;
	}

	if(picSize > 0){
		char *picBuf = alloca(picSize);
		picFID = fopen(filePath, "rb"); // read from temp file
		if(NULL != picFID){
			picSize = fread(picBuf, 1, picSize, picFID);
			//_TRACE("Cache Size %d", picSize);
			fclose(picFID);
			picFID = NULL;
		}

		if(picSize > 0){
			ST_SDCARD_FRAMER sdCardFramer;

			sdCardFramer.type = kSDF_TYPE_PIC;
			sdCardFramer.dataLength = picSize;
			sdCardFramer.timestampMS = _timestampMS;
			sdCardFramer.timeS = _timeS;

			if(sdCardFramer.timestampMS > 0){
				LP_SDCARD_FRAMER_PIC framerPicture = &sdCardFramer.picture;
							
				framerPicture->channelID = _channelID; // FIXME:
				framerPicture->codec = kSDF_P_CODEC_JPEG;
				framerPicture->sessionID = _sessionID;
				framerPicture->resolutionWidth = 320;
				framerPicture->resolutionHeight = 240;
					
				writeN = SDCARD_media_write(fID, &sdCardFramer, picBuf, picSize);
				if(writeN < 0){
					err = errno;
				}else{
					_TRACE("Success to Put a Picture");
				}
			}
		}
	}

	// remove file withouth condiction
	REMOVE_FILE(filePath);
	return 0 == err ? writeN : 0;
}


int APP_SDCARD_recorder(fSDCARD_RECORDER_ACCESS access)
{
	int i = 0;
	int ret = -1;
	
	char mediaBufName[32] = {""};
	int mainBufID = -1, subBufID = -1;
	lpMEDIABUF_USER mainBufUser = NULL;
	lpMEDIABUF_USER subBufUser = NULL;
	time_t flushTime = time(NULL);

	int err = 0; // loop condition

	snprintf(mediaBufName, sizeof(mediaBufName), "ch0_0.264"); // FIXME:
	mainBufID = MEDIABUF_lookup_byname(mediaBufName);
	if(mainBufID >= 0){
		mainBufUser = MEDIABUF_attach(mainBufID);
		//mediaBufSpeed = MEDIABUF_in_speed(mainBufID);
		if(NULL != mainBufUser){
			MEDIABUF_sync(mainBufUser);
		}
	}
	snprintf(mediaBufName, sizeof(mediaBufName), "ch0_2.264"); // FIXME:
	mainBufID = MEDIABUF_lookup_byname(mediaBufName);
	if(mainBufID >= 0){
		subBufUser = MEDIABUF_attach(mainBufID);
		if(NULL != subBufUser){
			MEDIABUF_sync(subBufUser);
		}
	}
	
	if(NULL != mainBufUser && NULL != subBufUser){
		int mediaFileID = -1;
		int sdCardFreeMB = 0;
		
		_TRACE("SD writer on work!");

		// get media free file ID
		mediaFileID = SDCARD_db_media_free_file_id();

		// create database session
		ret = app_sdcard_start_session(_channelID, 0, mediaFileID, 0, "time");
		_ASSERT(0 == ret, "SD Card Start Session Failed!");

		while(access() && 0 == err){
			void *fID = NULL;
			int fileWriten = 0;

			SDCARD_db_dump_media();
			
			// check the disk free capacity
			sdCardFreeMB = GET_DISK_FREE_MB(kSD_MOUNT_FOLDER);
			_TRACE("SD Card Free %dMB", sdCardFreeMB);

			// if the sd card is full
			if(sdCardFreeMB < kSDCARD_MEDIA_DISK_RESERVED_MB){
				// disk is full try to remove some file?
				int sessionID = SDCARD_db_media_oldest_session();
				ST_SDCARD_MEDIA_SESSION sessionCtnt;
				bool sessionRemove = true;

				// FIXME: you need to check the cycle recording
				while(access() && sdCardFreeMB < kSDCARD_MEDIA_DISK_RESERVED_MB){
					_TRACE("SD Card Full! %dMB Left", sdCardFreeMB);

					if(sessionID == _sessionID){
						const char *sessionType = strdupa(_type); // continue this session type
						
						ret = app_sdcard_restart_session(_channelID, 0, mediaFileID, fileWriten, sessionType);
						_ASSERT(0 == ret, "SD Card Start Session Failed!");

						_TRACE("New SessionID(%d) This Quater", _sessionID);
						continue;
					}

					if(SDCARD_db_media_get_session(sessionID, &sessionCtnt)){
						int beginFileID = sessionCtnt.beginFileID;
						int endFileID = sessionCtnt.endFileID;

						if(SDCARD_db_media_get_session(sessionID + 1, &sessionCtnt)){
							// check overlap
							if(sessionCtnt.beginFileID == endFileID){
								endFileID -= 1; // remove the next session header
							}
						}
						
						_TRACE("How About Remove Session(%d) File Range(%d, %d)", sessionID, beginFileID, endFileID);
						for(i = beginFileID; i <= endFileID; ++i){
							if(0 != SDCARD_media_remove(i)){
								_TRACE("Remove File(%d) Failed!", i);
								sessionRemove = false;
							}
						}

						if(sessionRemove){
							SDCARD_db_media_remove_session(sessionID);
						}
					}
						
					++sessionID; // check next session
					sdCardFreeMB = GET_DISK_FREE_MB(kSD_MOUNT_FOLDER);
				}
			}
			

			fID = SDCARD_media_open(mediaFileID);
			if(NULL != fID){
				time_t fileBegin = time(NULL);

				fileWriten = 0;
				while(access() && 0 == err){
					int writeN = 0;
					bool mediaBufOut = false;
					
					if(0 == MEDIABUF_out_lock(mainBufUser)){
						lpSDK_ENC_BUF_ATTR const rawAttr;	
						
						// out a frame from main media buf
						if(0 == MEDIABUF_out(mainBufUser, &rawAttr, NULL, NULL)){
							ST_SDCARD_FRAMER sdCardFramer;
							void *raw = (void*)(rawAttr + 1);
							ssize_t rawLength = rawAttr->data_sz;

							if(0 == _baseTimestampMS){
								// get base timestamp ms
								_baseTimestampMS = (rawAttr->timestamp_us + 500) / 1000;
							}

							sdCardFramer.type = kSDF_TYPE_VIDEO;
							sdCardFramer.dataLength = rawAttr->data_sz;
							sdCardFramer.timestampMS = _timestampMS = ((rawAttr->timestamp_us + 500) / 1000) - _baseTimestampMS;
							sdCardFramer.timeS = _timeS = (typeof(sdCardFramer.timeS))(rawAttr->time_us >> 32);

							//_TRACE("sdCardFramer.timeS = %d", sdCardFramer.timeS);
							
							if(kSDK_ENC_BUF_DATA_H264 == rawAttr->type){
								LP_SDCARD_FRAMER_VIDEO framerVideo = &sdCardFramer.video;
								
								//video
								if(rawAttr->h264.keyframe){
									// check whether it need to new a session every key frame every quater
									if((_beginUTC / (3600 / 4)) != (time(NULL) / (3600 / 4))){
										const char *sessionType = strdupa(_type); // continue this session type
						
										ret = app_sdcard_restart_session(_channelID, 0, mediaFileID, fileWriten, sessionType);
										_ASSERT(0 == ret, "SD Card Start Session Failed!");

										_TRACE("New SessionID(%d) This Quater", _sessionID);
									}
								}
								
								framerVideo->channelID = 1; // FIXME:
								framerVideo->channelSubID = 0; // FIXME:
								framerVideo->codec = kSDF_V_CODEC_H264;
								framerVideo->sessionID = _sessionID;
								framerVideo->frameRate = rawAttr->h264.fps;
								framerVideo->keyFrame = rawAttr->h264.keyframe ? true : false;
								framerVideo->profile = kSDF_V_PROFILE_BASELINE;
								framerVideo->resolutionWidth = rawAttr->h264.width;
								framerVideo->resolutionHeight = rawAttr->h264.height;
								
								writeN = SDCARD_media_write(fID, &sdCardFramer, raw, rawLength);
								if(writeN < 0){
									err = errno;
								}
								fileWriten += writeN;
							}else if(kSDK_ENC_BUF_DATA_G711A == rawAttr->type){//audio
								LP_SDCARD_FRAMER_AUDIO framerAudio = &sdCardFramer.audio;

								framerAudio->sessionID = _sessionID;
								framerAudio->channelID = 1; // FIXME:
								framerAudio->codec = kSDF_A_CODEC_G711A;
								framerAudio->sampleRate = rawAttr->g711a.sample_rate;
								framerAudio->sampleBitWidth = rawAttr->g711a.sample_width;

								writeN = SDCARD_media_write(fID, &sdCardFramer, raw, rawLength);
								if(writeN < 0){
									err = errno;
								}
								fileWriten += writeN;
							}
							mediaBufOut = true;
							
						}

						// out unlock
						MEDIABUF_out_unlock(mainBufUser);
					}else{
						// FIXME: meida out lock error!
					}

					// out a frame from sub media buf
					if(_baseTimestampMS > 0){
						if(0 == MEDIABUF_out_lock(subBufUser)){
							lpSDK_ENC_BUF_ATTR const rawAttr;	
							
							// out a frame from main media buf
							if(0 == MEDIABUF_out(subBufUser, &rawAttr, NULL, NULL)){
								ST_SDCARD_FRAMER sdCardFramer;
								void *raw = (void*)(rawAttr + 1);
								ssize_t rawLength = rawAttr->data_sz;

								sdCardFramer.type = kSDF_TYPE_VIDEO;
								sdCardFramer.dataLength = rawAttr->data_sz;
								sdCardFramer.timestampMS = _timestampMS = ((rawAttr->timestamp_us + 500) / 1000) - _baseTimestampMS;
								sdCardFramer.timeS = _timeS = (typeof(sdCardFramer.timeS))(rawAttr->time_us >> 32);

								if(sdCardFramer.timestampMS > 0){
									// only write the later frame differ to main stream
									if(kSDK_ENC_BUF_DATA_H264 == rawAttr->type){
										LP_SDCARD_FRAMER_VIDEO framerVideo = &sdCardFramer.video;
												
										framerVideo->channelID = 1; // FIXME:
										framerVideo->channelSubID = 1; // FIXME:
										framerVideo->codec = kSDF_V_CODEC_H264;
										framerVideo->sessionID = _sessionID;
										framerVideo->frameRate = rawAttr->h264.fps;
										framerVideo->keyFrame = rawAttr->h264.keyframe ? true : false;
										framerVideo->profile = kSDF_V_PROFILE_BASELINE;
										framerVideo->resolutionWidth = rawAttr->h264.width;
										framerVideo->resolutionHeight = rawAttr->h264.height;
										
										writeN = SDCARD_media_write(fID, &sdCardFramer, raw, rawLength);
										if(writeN < 0){
											err = errno;
										}
										fileWriten += writeN;
									}else if(kSDK_ENC_BUF_DATA_G711A == rawAttr->type){//audio
										// ignore the audio frame in sub media buffer
									}
								}
								mediaBufOut = true;

							}
							// out unlock
							MEDIABUF_out_unlock(subBufUser);
						}
					}

					if(0 == err){
						// check end of file
						if(fileWriten >= kSDCARD_MEDIA_FILE_FULL_BONDARY_BYTE){
							ST_SDCARD_FRAMER sdCardFramer;
							sdCardFramer.type = kSDF_TYPE_EOF;
							sdCardFramer.dataLength = 0;
							sdCardFramer.timestampMS = _timestampMS; // current timestamp
							sdCardFramer.timeS = _timeS;

							writeN = SDCARD_media_write(fID, &sdCardFramer, NULL, 0);
							if(writeN < 0){
								err = errno;
							}
							fileWriten += writeN;

							SDCARD_db_flush(); // flush to file
							break;
						}else{
							time_t const curTime = time(NULL);
							if(abs(curTime - flushTime) > kSDCARD_MEDIA_FILE_FLUSH_TIME_S){
								// update session every some seconds
								// note a picture as an shortcut

								int thumbPicSize = 0;
								thumbPicSize = app_sdcard_add_thumbnail(fID);
								if(thumbPicSize > 0){
									fileWriten += thumbPicSize;
								}
								
								ret = app_sdcard_update_session(_sessionID, 0, mediaFileID, fileWriten);					
								_ASSERT(0 == ret, "SD Card Update Session Failed!");
								
								SDCARD_db_flush(); // flush to file
								//SDCARD_db_dump_media();
								flushTime = curTime;
							}
						}	
					}
					
					if(!mediaBufOut){
						usleep(MEDIABUF_in_speed(mainBufID));
					}
				}

				SDCARD_media_close(fID);
				fID = NULL;
				mediaFileID += 1;

				_TRACE("Media File(%d) Finished! Time %ds", mediaFileID - 1, (int)(time(NULL) - fileBegin));
				
			}else{
				sleep(5); // open file error try to umount the filesystem
				err = errno;
			}
		}

		ret = app_sdcard_stop_session(_sessionID);
		_ASSERT(0 == ret, "SD Card Commit Session Failed!");
		
		_TRACE("SD writer off work!");

	
	}else{
		// nothing to do
		// sleep once and exit
		sleep(1);
	}

	// detach all the users
	if(mainBufUser){
		MEDIABUF_detach(mainBufUser);
		mainBufUser = NULL;
	}
	if(subBufUser){
		MEDIABUF_detach(subBufUser);
		subBufUser = NULL;
	}
	return 0;
}


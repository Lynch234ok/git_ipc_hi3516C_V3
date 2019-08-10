
#include "netsdk_private.h"

static ST_NSDK_PRIVATE _netsdk = {
	.audio_conf = NULL,
	.video_conf = NULL,
	.io_conf = NULL,
	// callback function
	.videoInputChannelChanged = NULL,
	.audioInputChannelChanged = NULL,
	.motionDetectionChannelChanged = NULL,
	.motionDetectionChannelStatus = NULL,
	.videoEncodeChannelChanged = NULL,
	.audioEncodeChannelChanged = NULL,
	.videoEncodeRequestKeyFrame = NULL,
	.videoEncodeSnapShot = NULL,
	.alarmInputChannelChanged = NULL,
	.alarmOutputChannelChanged = NULL,
	
};
LP_NSDK_PRIVATE netsdk = NULL;

LP_NSDK_PRIVATE NETSDK_private_get()
{
	netsdk = &_netsdk;
	// init the read/write lock
	pthread_rwlock_init(&netsdk->audio_sync, NULL);
	pthread_rwlock_init(&netsdk->video_sync, NULL);
	pthread_rwlock_init(&netsdk->io_sync, NULL);
	pthread_rwlock_init(&netsdk->ptz_sync, NULL);
	pthread_rwlock_init(&netsdk->network_sync, NULL);
	pthread_rwlock_init(&netsdk->image_sync, NULL);
	pthread_rwlock_init(&netsdk->system_sync, NULL);
    netsdk->lock_sync_enabled = 1;
	// success and return handle
	return netsdk;
}

void NETSDK_private_put()
{
	// destroy all the lock
	pthread_rwlock_destroy(&netsdk->system_sync);
	pthread_rwlock_destroy(&netsdk->image_sync);
	pthread_rwlock_destroy(&netsdk->network_sync);
	pthread_rwlock_destroy(&netsdk->ptz_sync);
	pthread_rwlock_destroy(&netsdk->io_sync);
	pthread_rwlock_destroy(&netsdk->video_sync);
	pthread_rwlock_destroy(&netsdk->audio_sync);
	// release handle
	netsdk = NULL;
}

int NETSDK_private_read_lock(pthread_rwlock_t *sync)
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }

	if(0 == pthread_rwlock_rdlock(sync)){
		return 0;
	}
	return -1;
}

int NETSDK_private_try_read_lock(pthread_rwlock_t *sync)
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }

	if(0 == pthread_rwlock_tryrdlock(sync)){
		return 0;
	}
	return -1;
}

int NETSDK_private_write_lock(pthread_rwlock_t *sync)
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }

	if(0 == pthread_rwlock_wrlock(sync)){
		return 0;
	}
	return -1;
}

int NETSDK_private_try_write_lock(pthread_rwlock_t *sync)
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }

	if(0 == pthread_rwlock_trywrlock(sync)){
		return 0;
	}
	return -1;
}

int NETSDK_private_unlock(pthread_rwlock_t *sync)
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }

	if(0 == pthread_rwlock_unlock(sync)){
		return 0;
	}
	return -1;
}


int NETSDK_map_str2dec(const ST_NSDK_MAP_STR_DEC map[], int map_items, const char *str_key, int def_val)
{
	int i = 0;
	for(i = 0; i < map_items; ++i){
		LP_NSDK_MAP_STR_DEC map_item = map + i;
		if(strlen(map_item->str) == strlen(str_key)
			&& 0 == strcasecmp(map_item->str, str_key)){
			return map_item->dec;
		}
	}
	return def_val;
}

const char *NETSDK_map_dec2str(const ST_NSDK_MAP_STR_DEC map[], int map_items, int dec_key, const char *def_val)
{
	int i = 0;
	for(i = 0; i < map_items; ++i){
		LP_NSDK_MAP_STR_DEC map_item = map + i;
		if(map_item->dec == dec_key){
			return map_item->str;
		}
	}
	return def_val;
}





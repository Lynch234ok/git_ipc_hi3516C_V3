#include <stdio.h>
#include <stdlib.h>
#include <NkUtils/assert.h>
#include <NkUtils/macro.h>
#include <sound.h>
#include <app_debug.h>
#include "netsdk.h"
#include "mutex_f.h"
#include "hi_comm_aio.h"
#include "sound_queue.h"
#include "app_reboot_ontime.h"
#include "generic.h"
#include "wpa_supplicant/include/wpa_status.h"
#include "sound.h"
#include "p2p/p2pdevice.h"

static char *soundFileName[SOUND_FILE_CNT] = 
{
	"Memory_card_status_exception",
	"Reset_success",
	"Start_firmware_update",
	"Connection_success",
	"Connection_please_wait",
	"AP_build_up",
	"AP_building",
	"Upgrade_failed",
	"Upgrade_completed",
	"Network_configure_success",
	"Network_configure_failed",
	"Network_configuring",
	"System_starting_completed",
	"System_booting",
	"Completing_updates",
	"Switch_to_AP_mode",
	"Switch_to_station_mode",
	"Please_configure_network",
	"Start_soundwave_configuring",
	"Please_setup_again",
	"Please_wait",
	"SD_card_detected",
	"SD_card_ejected",
	"SD_card_exception",
	"Switch_smart_configure",
	"WiFi_connect_timeout",
	"Configuration_mode",
	"Firmware_update_failed",
	"Password_error",
	"Restore_factory_settings",
	"Upgrade",
	"WiFi_connecting",
	"WiFi_connection_completed",
	"WiFi_connection_failed",
	"WiFi_setting",
	"Button",
	"OC_Dong",
	"Pairing_Mode",
	"Alarm",
	"Custom_sound",
};

static int playWavFile(unsigned char *file)
{
    FILE* fId = NULL;
    unsigned int dataLen;
    unsigned char pcm16BitBuf[16 * 1024];
    AIO_ATTR_S hiAinAttr;
    unsigned int pcm16BitLength;
#ifdef M388C          
/////FIXME
	pcm16BitLength = 640;
#else
	HI_MPI_AI_GetPubAttr(0, &hiAinAttr);
	pcm16BitLength = hiAinAttr.u32PtNumPerFrm << 1;
#endif

    fId = fopen(file, "rb");
	if(NULL == fId) {
        printf("Err:open %s fail!\n", file);
        return -1;
    }

    if(NULL != SOUND_QueueisInit())
    {
        /* ∂¡»°wavŒƒº˛ */
        SOUND_writeQueue_lock();
        for(;;) {
            memset(pcm16BitBuf, 0, sizeof(pcm16BitBuf));
            if((dataLen = fread(pcm16BitBuf, 1, pcm16BitLength, fId)) > 0) {
                SOUND_writeQueue2(pcm16BitBuf, pcm16BitLength, emSOUND_DATA_TYPE_WAV_FILE, emSOUND_PRIORITY_FIRST);
            }
            else {
                break;
            }

        }
        SOUND_writeQueue_unlock();
    }
    fclose(fId);
    return 0;	

}


NK_Int
SearchFileAndPlay(enSoundFile SoundType, NK_Boolean block)
{
	NK_Char sound_file_path[256];
	ST_NSDK_SYSTEM_SETTING info;

    /* »Áπ˚AUDIO_OUTPUT_ENABLEŒƒº˛¥Ê‘⁄‘Ú≤ª≤•∑≈»Œ∫Œ”Ô“Ù */
    if(WPA_getWifiConnectedFlag() == true) {
        /* ∞¥º¸“Ù∫Õtfø®“Ï≥£≥˝Õ‚ */
        if ((SOUND_SD_card_exception == SoundType) || (SOUND_Upgrade == SoundType)) {
            ;
        } else if ((SOUND_Button == SoundType) || (SOUND_Alarm == SoundType) || (SOUND_Alarm_custom == SoundType)) {
            ;
        } else {
            return 0;
        }
    }
	memset(&info, 0, sizeof(ST_NSDK_SYSTEM_SETTING));
	NETSDK_conf_system_get_setting_info(&info);

	if(!info.promptSound.enabled){
		return 0;
	}

	if (SoundType >= SOUND_FILE_CNT && info.promptSound.enabled == false) {
		printf("No such sound file !!!\n");
		return -1;
	}
    // ÊâãÊú∫ÂÆöÂà∂Êä•Ë≠¶ÊèêÁ§∫Èü≥
    if(SOUND_Alarm_custom == SoundType){
        sprintf(sound_file_path, "%s/%s", DST_FILE_PATH, soundFileName[SoundType]);
    }
    else if(SOUND_Alarm == SoundType) {
        sprintf(sound_file_path, "%s/%s.wav", SOUND_FILE_FOLDER, soundFileName[SoundType]);
    }
    else {
        sprintf(sound_file_path, "%s/sound_%s/%s.wav", SOUND_FILE_FOLDER, info.promptSound.soundTypeStr, soundFileName[SoundType]);
    }
#if 0
	if(info.promptSound.soundType == kNSDK_SYSTEM_PROMPT_SOUND_TYPE_CHINESE){
		sprintf(sound_file_path, "%s/%s.wav", SOUND_FILE_FOLDER_CHINESE, soundFileName[SoundType]);
	}
	else if(info.promptSound.soundType == kNSDK_SYSTEM_PROMPT_SOUND_TYPE_ENGLISH)
	{
		sprintf(sound_file_path, "%s/%s.wav", SOUND_FILE_FOLDER_ENGLISH, soundFileName[SoundType]);
	}
	else
	{	
		sprintf(sound_file_path, "%s/%s.wav", SOUND_FILE_FOLDER_CHINESE, soundFileName[SoundType]);
	}
#endif
	printf("%s\n", sound_file_path);
	
    playWavFile(sound_file_path);


	return 0;
}

NK_Int
Search_TF_FileAndPlay(unsigned char *tf_sound_file_path)
{
	if(NULL == tf_sound_file_path)
	{
		printf("File not found in TF card\n");
		return -1;
	}

	printf("%s\n", tf_sound_file_path);

	playWavFile(tf_sound_file_path);

	return 0;
}

NK_Int
sound_mutexinit()
{
	/*if(!au_mutex){
		au_mutex = MUTEX_create();
	}*/
	return 0;
}



#include <NkUtils/types.h>

#if !defined(NK_SOUND_H_)
#define NK_SOUND_H_

#define SOUND_FILE_FOLDER_CHINESE	"/media/custom/sound_chinese"
#define SOUND_FILE_FOLDER_ENGLISH	"/media/custom/sound_english"
#define SOUND_FILE_FOLDER	"/media/custom"


typedef enum 
{
	SOUND_Memory_card_status_exception = 0,
	SOUND_Reset_success,
	SOUND_Start_firmware_update,
	SOUND_Connection_success,
	SOUND_Connection_please_wait,
	SOUND_AP_build_up,
	SOUND_AP_building,
	SOUND_Upgrade_failed,
	SOUND_Upgrade_completed,
	SOUND_Network_configure_success,
	SOUND_Network_configure_failed,
	SOUND_Network_configuring,
	SOUND_System_starting_completed,
	SOUND_System_booting,
	SOUND_Completing_updates,
	SOUND_Switch_to_AP_mode,
	SOUND_Switch_to_station_mode,
	SOUND_Please_configure_network,
	SOUND_Start_soundwave_configuring,
	SOUND_Please_setup_again,
	SOUND_Please_wait,
	SOUND_SD_card_detected,
	SOUND_SD_card_ejected,
	SOUND_SD_card_exception,
	SOUND_Switch_smart_configure,
	SOUND_WiFi_connect_timeout,
	SOUND_Configuration_mode,
	SOUND_Firmware_update_failed,
	SOUND_Password_error,
	SOUND_Restore_factory_settings,
	SOUND_Upgrade,
	SOUND_WiFi_connecting,
	SOUND_WiFi_connection_completed,
	SOUND_WiFi_connection_failed,
	SOUND_WiFi_setting,
	SOUND_Button,
	SOUND_OC_Dong,
	SOUND_Pairing_Mode,
	SOUND_Alarm,
	SOUND_Alarm_custom,
	SOUND_FILE_CNT,
}enSoundFile;

/**
 * 播放声音。
 *
 * @param[in]		file_path			声音 wav 文件，目前只支持 PCM 8k-16bits。
 * @param[in]		block				阻塞标识。
 *
 * @return		播放成功返回 0，播放失败返回 -1。
 */
extern NK_Int
PlaySound(NK_PChar file_path, NK_Boolean block);

extern NK_Int
SearchFileAndPlay(enSoundFile SoundType, NK_Boolean block);

extern NK_Int
Search_TF_FileAndPlay(unsigned char *tf_sound_file_path);

#endif /* NK_SOUND_H_ */

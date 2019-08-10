#ifndef _PRODUCTION_TEST_H
#define _PRODUCTION_TEST_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct product_test_info{
    int isWifi;
	char staEssid[32];
	char staPassword[32];
	char staStaticIp[32];
	char staMask[8];
	char staNetmask[32];
	char staGateway[32];
	char staDns[32];

    /**
     * 能力集capability
     */
    int isCapability;
    char model[64];
    int audioInput;
    int audioOutput;
    int lightControl;
    int bulbControl;
    int ptz;
    int sdCard;
    int fisheye;

	/**
	 * 4g测试
	 */
	 int  is4gTest;
	 int  enable;
     char pingNetwork[32];
     char startVoiceFile[64];
     char succeedVoiceFile[64];
     char failedVoiceFile[64];

     /**
      * custom定制设备功能
      */
    int isCustom;
    int pir;
    int pirTrigger;

}ST_PRODUCT_TEST_INFO, *LP_PRODUCT_TEST_INFO;

extern LP_PRODUCT_TEST_INFO PRODUCT_TEST_get_info(LP_PRODUCT_TEST_INFO info);
extern int PRODUCT_TEST_init();
extern int PRODUCT_TEST_destroy();

extern int PRODUCT_TEST_getCustom(LP_PRODUCT_TEST_INFO info);

#ifdef __cplusplus
};
#endif
#endif /*_PRODUCTION_TEST_H*/



#ifndef __PTZ_H__
#define __PTZ_H__


typedef enum
{
	PTZ_CMD_NULL = -1,
	PTZ_CMD_UP,
	PTZ_CMD_DOWN,
	PTZ_CMD_LEFT,
	PTZ_CMD_RIGHT,
	PTZ_CMD_LEFT_UP,
	PTZ_CMD_RIGHT_UP,
	PTZ_CMD_LEFT_DOWN,
	PTZ_CMD_RIGHT_DOWN,
	PTZ_CMD_AUTOPAN,
	PTZ_CMD_IRIS_OPEN,
	PTZ_CMD_IRIS_CLOSE,
	PTZ_CMD_ZOOM_IN,
	PTZ_CMD_ZOOM_OUT,
	PTZ_CMD_FOCUS_FAR,
	PTZ_CMD_FOCUS_NEAR,
	PTZ_CMD_STOP,
	PTZ_CMD_WIPPER_ON,
	PTZ_CMD_WIPPER_OFF,
	PTZ_CMD_LIGHT_ON,
	PTZ_CMD_LIGHT_OFF,
	PTZ_CMD_POWER_ON,
	PTZ_CMD_POWER_OFF,
	PTZ_CMD_GOTO_PRESET,
	PTZ_CMD_SET_PRESET,
	PTZ_CMD_CLEAR_PRESET,
	PTZ_CMD_TOUR,
	PTZ_COMMAND_CNT,
}PTZ_COMMAND;

#define MAX_PROTOCOL_NAME_SIZE (32)

typedef struct PTZ
{
	// private

	// public
	int (*Init)(void);
	void (*Destroy)(void);
	int (*Config)(int nChn, const char* szProtocol, int nBaud, unsigned char u8Addr);
	int (*Send)(int nChn, PTZ_COMMAND enCmd, unsigned char u8Param);
	//
}PTZ_t;


#ifdef __cplusplus
extern "C" {
#endif
extern int PTZ_Init(PTZ_t *ptzAttr);
extern void PTZ_Destroy();
extern int PTZ_Send(int nChn, PTZ_COMMAND enCmd, unsigned char u8Param);
extern int PTZ_Cmd_str2int(const char *ptz_cmd);
extern int PTZ_Config(int nChn, const char* szProtocol, int nBaud, unsigned char u8Addr);
extern int PTZ_P2P_Cmd_str2int(const char *ptz_cmd);
#ifdef __cplusplus
}
#endif

#endif //__PTZ_H__


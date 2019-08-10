#ifndef __AUTO_FOCUS_H__
#define __AUTO_FOCUS_H__


int APP_MOTOR_Init(void);
int APP_MOTOR_Exit(void);
int APP_MOTOR_StartAutoFocus(void);
int APP_MOTOR_StopAutoFocus(void);


int APP_MOTOR_StartZoom(int isForward);
int APP_MOTOR_StopZoom();

int APP_MOTOR_SetPreset(int index);
int APP_MOTOR_GotoPreset(int index);
int APP_MOTOR_ClearPreset(int index);

#endif

#ifndef __STEPPER_H__
#define __STEPPER_H__

enum {
	GPIO_FOCUS_A_N = 0, //BLUE
	GPIO_FOCUS_A_P,     //BLACK

	GPIO_ZOOM_A_N,      //GREEN
	GPIO_ZOOM_A_P,      //ORANGE

	GPIO_ZOOM_B_P,      //YELLOW
	GPIO_ZOOM_B_N,      //PURPLE

	GPIO_FOCUS_B_N,     //WHITE
	GPIO_FOCUS_B_P,     //RED
};

int StepperGpio_Init(void);
int StepperGpio_Set(int Gpio, int Value);
int StepperGpio_Exit(void);

enum {
	STEPPER_SPD_300PPS = (1000000/300),
	STEPPER_SPD_400PPS = (1000000/400),
	STEPPER_SPD_500PPS = (1000000/500),
	STEPPER_SPD_600PPS = (1000000/600),
	STEPPER_SPD_700PPS = (1000000/700),
	STEPPER_SPD_800PPS = (1000000/800),
};

enum {
	STEPPER_ZOOM_IN   = 0,
	STEPPER_ZOOM_OUT,
};

enum {
	STEPPER_FOCUS_BACKWARD = 0,
	STEPPER_FOCUS_FORWARD,
};

int Stepper_FocusStop(void);
int Stepper_ZoomStop(void);
int Stepper_FocusStep(int IsForward);
int Stepper_ZoomStep(int IsForward);
int Stepper_FocusGotoPos(int Pos, int* pRunning);
int Stepper_ZoomGotoPos(int Pos, int* pRunning);
int Stepper_FocusGetInfo(int * Cur, int * Max);
int Stepper_ZoomGetInfo(int * Cur, int * Max);
int Stepper_Init(int FMax, int ZMax, int FCur, int ZCur);
int Stepper_Exit(void);

int Stepper_SpeedSet(int Speed);
void Stepper_Delay(void);

#endif //__STEPPER_H__

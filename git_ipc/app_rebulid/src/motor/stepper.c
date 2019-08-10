#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "stepper.h"
#include "hisi_hal.h"
#include "af_info.h"

/*
步进电机 4拍驱动
//PA  PB  PC  PD
  AP+ AP+ AP- AP-
  AN- AN- AN+ AN+
  BP- BP+ BP+ BP-
  BN+ BN- BN- BN+
*/

#define DRV_STEP_NUM (4)
static int DrvStep[DRV_STEP_NUM] = { 0x09, 0x0A, 0x06, 0x05, };

static int Stepper_FocusMax = -1;
static int Stepper_ZoomMax  = -1;

static int Stepper_FocusCur = -1;
static int Stepper_ZoomCur  = -1;

static int Stepper_DrvFocusTick(int step)
{
	int Val = 0;

	if((step < 0) || (step >= DRV_STEP_NUM)) {
		return -1;
	}

	Val = DrvStep[step];

	StepperGpio_Set(GPIO_FOCUS_A_P, Val & 0x08);
	StepperGpio_Set(GPIO_FOCUS_A_N, Val & 0x04);
	StepperGpio_Set(GPIO_FOCUS_B_P, Val & 0x02);
	StepperGpio_Set(GPIO_FOCUS_B_N, Val & 0x01);

	return 0;
}

static int Stepper_DrvZoomTick(int step)
{
	int Val = 0;

	if((step < 0) || (step >= DRV_STEP_NUM)) {
		return -1;
	}

	Val = DrvStep[step];

	StepperGpio_Set(GPIO_ZOOM_A_P, Val & 0x08);
	StepperGpio_Set(GPIO_ZOOM_A_N, Val & 0x04);
	StepperGpio_Set(GPIO_ZOOM_B_P, Val & 0x02);
	StepperGpio_Set(GPIO_ZOOM_B_N, Val & 0x01);

	return 0;
}

int Stepper_FocusStop(void)
{
	int Val = 0;

	StepperGpio_Set(GPIO_FOCUS_A_P, Val & 0x08);
	StepperGpio_Set(GPIO_FOCUS_A_N, Val & 0x04);
	StepperGpio_Set(GPIO_FOCUS_B_P, Val & 0x02);
	StepperGpio_Set(GPIO_FOCUS_B_N, Val & 0x01);
}

int Stepper_ZoomStop(void)
{
	int Val = 0;

	StepperGpio_Set(GPIO_ZOOM_A_P, Val & 0x08);
	StepperGpio_Set(GPIO_ZOOM_A_N, Val & 0x04);
	StepperGpio_Set(GPIO_ZOOM_B_P, Val & 0x02);
	StepperGpio_Set(GPIO_ZOOM_B_N, Val & 0x01);

	return 0;
}

int Stepper_FocusStep(int IsForward)
{
	if(IsForward) {
		if((Stepper_FocusCur+1) < Stepper_FocusMax) {
			Stepper_FocusCur += 1;
			Stepper_DrvFocusTick(Stepper_FocusCur%DRV_STEP_NUM);
			Stepper_Delay();
		}
		else {
			Stepper_FocusStop(); //Auto Stop, When Try Running Out of Range;
		}
	}
	else {
		if(Stepper_FocusCur > 0) {
			Stepper_FocusCur -= 1;
			Stepper_DrvFocusTick(Stepper_FocusCur%DRV_STEP_NUM);
			Stepper_Delay();
		}
		else {
			Stepper_FocusStop(); //Auto Stop, When Try Running Out of Range;
		}
	}

	return 0;
}

int Stepper_ZoomStep(int IsForward)
{
	if(IsForward) {
		if((Stepper_ZoomCur+1) < Stepper_ZoomMax) {
			Stepper_ZoomCur += 1;
			Stepper_DrvZoomTick(Stepper_ZoomCur%DRV_STEP_NUM);
			Stepper_Delay();
		}
		else {
			Stepper_ZoomStop(); //Auto Stop, When Try Running Out of Range;
		}
	}
	else {
		if(Stepper_ZoomCur > 0) {
			Stepper_ZoomCur -= 1;
			Stepper_DrvZoomTick(Stepper_ZoomCur%DRV_STEP_NUM);
			Stepper_Delay();
		}
		else {
			Stepper_ZoomStop(); //Auto Stop, When Try Running Out of Range;
		}
	}

	return 0;
}

int Stepper_FocusGotoPos(int Pos, int* pRunning)
{
	if ((Pos < 0) || (Pos >= Stepper_FocusMax)) {
		return -1;
	}

	if (Pos == Stepper_FocusCur) {
		return 0;
	}

	int tmpDifferent = (Pos > Stepper_FocusCur) ? (Pos - Stepper_FocusCur) : (Stepper_FocusCur - Pos);
	int tmpDirection = (Pos > Stepper_FocusCur) ? (STEPPER_FOCUS_FORWARD)  : (STEPPER_FOCUS_BACKWARD);
	int ii;

	for(ii = 0; ii < tmpDifferent; ii += 1) {
		if(*pRunning == 0)
		{
			return -1;
		}
		Stepper_FocusStep(tmpDirection);
	}

	return 0;
}

int Stepper_ZoomGotoPos(int Pos, int* pRunning)
{
	if ((Pos < 0) || (Pos >= Stepper_ZoomMax)) {
		return -1;
	}

	if (Pos == Stepper_ZoomCur) {
		return 0;
	}

	int tmpDifferent = (Pos > Stepper_ZoomCur) ? (Pos - Stepper_ZoomCur) : (Stepper_ZoomCur - Pos);
	int tmpDirection = (Pos > Stepper_ZoomCur) ? (STEPPER_FOCUS_FORWARD)  : (STEPPER_FOCUS_BACKWARD);
	int ii;

	for(ii = 0; ii < tmpDifferent; ii += 1) {
		if(*pRunning == 0)
		{
			return -1;
		}
		Stepper_ZoomStep(tmpDirection);
	}

	return 0;
}

int Stepper_FocusGetInfo(int * Cur, int * Max)
{
	if (Cur) {
		*Cur = Stepper_FocusCur;
	}

	if (Max) {
		*Max = Stepper_FocusMax;
	}

	return 0;
}

int Stepper_ZoomGetInfo(int * Cur, int * Max)
{
	if (Cur) {
		*Cur = Stepper_ZoomCur;
	}

	if (Max) {
		*Max = Stepper_ZoomMax;
	}

	return 0;
}

#define STEPPER_ASSUME_MAX (2500)
int Stepper_Init(int FMax, int ZMax, int FCur, int ZCur)
{
	Stepper_FocusMax = FMax;
	Stepper_ZoomMax  = ZMax;

	Stepper_FocusCur = FCur;
	Stepper_ZoomCur  = ZCur;

	StepperGpio_Init();

	//Force Calibrating Steppers
	if (Stepper_FocusCur < 0) {
		int ii;

		for (ii = STEPPER_ASSUME_MAX; ii > 0; ii -= 1) {
			Stepper_DrvFocusTick(ii%DRV_STEP_NUM);
			Stepper_Delay();
		}

		Stepper_FocusStop();
		Stepper_FocusCur = 0;
	}
	if (Stepper_ZoomCur < 0) {
		int ii;

		for (ii = STEPPER_ASSUME_MAX; ii > 0; ii -= 1) {
			Stepper_DrvZoomTick(ii%DRV_STEP_NUM);
			Stepper_Delay();
		}

		Stepper_ZoomStop();
		Stepper_ZoomCur = 0;
	}

	return 0;
}

int Stepper_Exit(void)
{
	Stepper_FocusStop();
	Stepper_ZoomStop();

	StepperGpio_Exit();

	return 0;
}

static unsigned long gStepper_usDelay = STEPPER_SPD_300PPS;
int Stepper_SpeedSet(int Speed)
{
	gStepper_usDelay = Speed;
}

void Stepper_Delay(void)
{
	usleep(gStepper_usDelay);
}

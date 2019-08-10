/*
 * Copyright (C) 2010  VATICS Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __AUTOFOCUS_H__
#define __AUTOFOCUS_H__

#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef MODULE
#define MODULE
#endif


/* ============================================================================================== */
#define AUTOFOCUS_VERSION MAKEFOURCC(4, 0, 0, 0)
#define AUTOFOCUS_ID_VERSION "4.0.0.0"
/* ============================================================================================== */

/* Auto-Focus state control options */
typedef enum af_state_control_flags
{
	AF_STATECTRL_IDLE = 0,
	AF_STATECTRL_WAITE = 1,
	AF_STATECTRL_PREAF = 2,
	AF_STATECTRL_AF = 3,
	AF_STATECTRL_MF = 4,
	AF_STATECTRL_ZOOM = 5,
	AF_STATECTRL_ADJUST = 6,
	AF_STATECTRL_CONTINUOUSAF = 7,
} EAFStateControlFlags;

/* AF initial param */
typedef struct auto_focus_initial_param
{
	DWORD	dwFrameIntv;

	DWORD	dwFocalPlaneTotalNumber;
	DWORD	dwZoomPlaneTotalNumber;
	DWORD	dwInitialZoomMotorPosition;
	DWORD	dwInitialFocalPlaneIndx;
	DWORD	dwZoomMaxRatio;
	DWORD	dwZtTbSize;

	DWORD	dwInitialStep;
	DWORD	dwThresNoise; // 0 (smallest) ~ 7
	DWORD	dwFineTuneFinishThresh; // 0 (smallest) ~ 3
	DWORD	dwMaxReturnNum; // 0 (smallest) ~ 3
	DWORD	dwRestartUpperDiff; // 0 (smallest) ~ 3
	DWORD	dwRestartLowerDiff; // 0 (smallest) ~ 3	
} TAutoFocusInitialParam;

/* AF status structure */
typedef struct autofocus_state
{
	DWORD dwAFUserCMD; // (i) User command status, ref. EAFStateControlFlags
	BOOL bAEStable;// (i) Check AE Stable

	DWORD dwZoomMotorPosition; // (i) Current zoom position.
	DWORD dwFocusMotorPosition; // (i) Current focus position.

	DWORD dwFocusHorSubWindowNum; // (i)
	DWORD dwFocusVerSubWindowNum; // (i)
	DWORD dwFocusWinSize; // (i)
	DWORD *pdwFocusValue; // (i)

	TAutoFocusCtrlInfo tAFCtrlInfo; // (o) Control focus&zoom motor info.
	BOOL bAFCtrlValid; // (o)
	ELensControlFlags eAFCtrlCmd;// (o)

	BOOL bFocusFinish; // (o) If yes, AF is finish
	DWORD dwZoomRatio; // (o) Current zoom ratio
} TAutoFocusState;

/* AF call back function */
typedef struct autofocus_module 
{
	int (*open)(DWORD dwVersion, DWORD dwDevNum);
	int (*release)(DWORD dwDevNum);
	int (*set_options)(TVideoSignalOptions *ptArg, DWORD dwDevNum);
	int (*one_frame)(TAutoFocusState *ptState, DWORD dwDevNum);
	int (*initial)(TAutoFocusInitialParam *ptParam, DWORD dwDevNum);
} TAutoFocusModule;
/* ============================================================================================== */
#endif // __AUTOEXPOSURE_H__

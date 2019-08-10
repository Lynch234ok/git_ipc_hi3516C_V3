#ifndef __errdefs_h__
#define __errdefs_h__

#define E_CTP_OK				0	//Operation successfully completed
#define E_TIMEOUT				1	//Timeout
#define E_ERROR_PROTO			2	//bad protocol, should be CTP/1.0
#define E_ERROR_ACK				3	//bad acknowlegement from proxy/device
#define E_CTP_HOST				4	//host can't be resolved
#define E_CTP_CONNECT			5	//connect to host failed
#define E_CTP_AUTHENTICATION	6	//bad user name or password
#define E_CTP_RIGHT				7	//have no respective rights
#define E_CTP_COMMAND			8	//bad command
#define E_CTP_PARAMETER			9	//bad parameters in command body
#define E_CTP_SYSTEM			10	//socket error, call WSAGetLastError for detail
#define E_CTP_OTHER				11	//WSAStartup not called
#define E_CONN					12	//can't connect to device's update-server
#define E_NO_CONN				13	//not connected yet
#define E_BUSY					14	//another command is running
#define E_OTHER					15	// or Call Connect after Resume/AsynResume
#define E_PASSIVEMODE			16  //Operation not allowed for passive device
#define E_PEERCLOSED			17
#define E_NO_RESOURCE			18

//100~599: Status-code from proxy/device

#define E_INVALID_OPERATION		1001	//operate on an unsupported hardware configure. 
#define E_INVALID_PARAM			1002

/* Error from Playback system */
#define E_OK				0
#define E_BADFMT			1101	//bad file format
#define E_BADVER			1102	//bad file version
#define E_TAGNOTEXISTED		1103	//has no such tag
#define E_BUFFERTOOSMALL	1104	//size of buffer is less than size of frame
#define E_EOF				1105	//end of file
#define E_CANNOTOPENFILE	1106	//
#define E_WAITDATA			1107	//RemoteReader is waiting for next frame
#define E_WRONG_TARGETPLATFORM 1108	//CTPUpdate. the .pk2 file is not for the target machine

#endif


#if defined(DIAL)


#ifndef GIT_IPC_DIAL_H
#define GIT_IPC_DIAL_H

#include <stdbool.h>

typedef enum {

    emDIAL_ISP_NONE = -1,
    emDIAL_ISP_MOBILE_GSM = 0,
    emDIAL_ISP_MOBILE_TDS1,
    emDIAL_ISP_MOBILE_TDS2,
    emDIAL_ISP_UNICOM_GSM,
    emDIAL_ISP_UNICOM_WCDMA,
    emDIAL_ISP_TELECOM_CDMA1,
    emDIAL_ISP_TELECOM_CDMA2,
    emDIAL_ISP_TELECOM_FDDLTE

} emDIAL_ISP;


extern int DIAL_deamon_run(void);
extern void DIAL_deamon_stop(void);
extern bool DIAL_is_ok(void);
extern emDIAL_ISP DIAL_get_isp( void );


#endif //GIT_IPC_DIAL_H


#endif //defined(DIAL)

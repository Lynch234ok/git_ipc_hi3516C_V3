#if defined(BLUETOOTH)


#ifndef GIT_IPC_BIND_BLUETOOTH_H
#define GIT_IPC_BIND_BLUETOOTH_H


/**
 * 开启蓝牙，准备绑定
 */
int DEV_BIND_BT_start(DEV_BIND_CB_on_recved_data on_recved_data, DEV_BIND_get_near_ap get_near_ap);

/**
 * 在需要的时候，提前中断蓝牙绑定
 */
int DEV_BIND_BT_interrupt();


#endif //GIT_IPC_BIND_BLUETOOTH_H


#endif //defined(BLUETOOTH)
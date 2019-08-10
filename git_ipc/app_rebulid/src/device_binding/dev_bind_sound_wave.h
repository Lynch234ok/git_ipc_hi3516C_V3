#if defined(SOUND_WAVE)


#ifndef GIT_IPC_BIND_SOUND_WAVE_H
#define GIT_IPC_BIND_SOUND_WAVE_H

int SW_init();
void SW_destroy();


/**
 * 开启声波接收功能，准备绑定
 */
int DEV_BIND_SW_start(DEV_BIND_CB_on_recved_data on_recved_data);

/**
 * 在需要的时候，提前中断声波绑定
 */
int DEV_BIND_SW_interrupt();

#endif //GIT_IPC_BIND_SOUND_WAVE_H


#endif //defined(SOUND_WAVE)

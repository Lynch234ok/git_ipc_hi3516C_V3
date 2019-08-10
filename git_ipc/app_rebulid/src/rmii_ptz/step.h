#ifndef __STEP_H__
#define __STEP_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int step_gpio_init();
extern void stepH(int step);
extern void stepV(int step);


#ifdef __cplusplus
}
#endif

#endif// __STEP_H__


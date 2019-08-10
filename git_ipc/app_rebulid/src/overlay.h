
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "sdk/sdk_api.h"

#ifndef OVERLAY_H_
#define OVERLAY_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef enum OVERLAY_FONT_SIZE {
	kOVERLAY_FONT_SIZE_16 = (16),
	kOVERLAY_FONT_SIZE_20 = (20),
	kOVERLAY_FONT_SIZE_24 = (24),
	kOVERLAY_FONT_SIZE_32 = (32),
}enOVERLAY_FONT_SIZE;

typedef uint32_t OVERLAY_TEXT_STYLE_t;
#define OVERLAY_TEXT_STYLE_FOREGROUND_TRANSPARENT (1<<0)
#define OVERLAY_TEXT_STYLE_FOREGROUND_WHRITE (1<<1)
#define OVERLAY_TEXT_STYLE_FOREGROUND_BLACK (1<<2)
#define OVERLAY_TEXT_STYLE_BACKGROUND_TRANSPARENT (1<<4)
#define OVERLAY_TEXT_STYLE_BACKGROUND_WHRITE (1<<5)
#define OVERLAY_TEXT_STYLE_BACKGROUND_BLACK (1<<6)
#define OVERLAY_TEXT_STYLE_ENCLOSED_TRANSPARENT (1<<8)
#define OVERLAY_TEXT_STYLE_ENCLOSED_WHRITE (1<<9)
#define OVERLAY_TEXT_STYLE_ENCLOSED_BLACK (1<<10)

#if defined(HI3518E_V2) | defined(HI3516C_V2) | defined(HI3516C_V3) | defined(HI3516E_V1)
#define OVERLAY_TEXT_STYLE_DEFAULT \
	(OVERLAY_TEXT_STYLE_FOREGROUND_WHRITE\
	|OVERLAY_TEXT_STYLE_BACKGROUND_TRANSPARENT\
	|OVERLAY_TEXT_STYLE_ENCLOSED_BLACK)
#else
#define OVERLAY_TEXT_STYLE_DEFAULT \
	(OVERLAY_TEXT_STYLE_FOREGROUND_WHRITE\
	|OVERLAY_TEXT_STYLE_BACKGROUND_TRANSPARENT\
	|OVERLAY_TEXT_STYLE_ENCLOSED_TRANSPARENT)
#endif


extern ssize_t OVERLAY_put_text(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas,
	int x, int y, enOVERLAY_FONT_SIZE font_size,
	const char *text, OVERLAY_TEXT_STYLE_t style, int ratio);

extern void OVERLAY_set_alpha(uint8_t alpha);
extern bool OVERLAY_font_available(enOVERLAY_FONT_SIZE font_size);

extern int OVERLAY_load_font(enOVERLAY_FONT_SIZE font_size, const char* asc_font, const char* gb2312_font);

extern int OVERLAY_init();
extern void OVERLAY_destroy();


#ifdef __cplusplus
};
#endif
#endif //OVERLAY_H_


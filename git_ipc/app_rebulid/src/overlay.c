
#include "overlay.h"
#include "sdk/sdk_api.h"
#include "app_debug.h"

static uint8_t _def_alpha = 64;
static OVERLAY_TEXT_STYLE_t _def_sytle = OVERLAY_TEXT_STYLE_DEFAULT; 

static uint8_t *_asc_16 = NULL; // bytes 128 x 16 x 1 size 8x16
static uint8_t *_asc_20 = NULL; // bytes 128 x 20 x 2 size 10x20
static uint8_t *_asc_24 = NULL; // bytes 128 x 24 x 2 size 12x24
static uint8_t *_asc_32 = NULL; // bytes 128 x 32 x 2 size 16x32

static uint8_t* _gb2312_16 = NULL; // bytes 87 x 94 x 16 x 2 size 16x16
static uint8_t* _gb2312_20 = NULL; // bytes 87 x 94 x 20 x 3 size 20x20
static uint8_t* _gb2312_24 = NULL; // bytes 87 x 94 x 24 x 3 size 24x24
static uint8_t* _gb2312_32 = NULL; // bytes 87 x 94 x 32 x 4 size 32x32

ssize_t OVERLAY_put_text(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas,
	int x, int y, enOVERLAY_FONT_SIZE font_size, const char *text, OVERLAY_TEXT_STYLE_t style, int ratio)
{

	int i = 0, ii = 0, iii = 0;
	int const x_base = x;
	int const y_base = y;
	
	char *ch = text; // at the beginning of the text
	size_t asc_count = 0;
	size_t asc_width = 0;
	size_t gb2312_width = 0;
	size_t text_width = 0, text_height = 0;

	stSDK_ENC_VIDEO_OVERLAY_PIXEL fg_color, bg_color, enclosed_color;
	bool is_enclosed = false;

	if(!canvas){
		return -1;
	}
	
	switch(font_size){
		case kOVERLAY_FONT_SIZE_16:
			asc_width = 8;
			gb2312_width = 16;
			if(!_asc_16 || !_gb2312_16){
				return -1;
			}
			break;

		case kOVERLAY_FONT_SIZE_20:
			asc_width = 10;
			gb2312_width = 20;
			if(!_asc_20 || !_gb2312_20){
				return -1;
			}
			break;

		case kOVERLAY_FONT_SIZE_24:
			asc_width = 12;
			gb2312_width = 24;
			if(!_asc_24 || !_gb2312_24){
				return -1;
			}
			break;

		case kOVERLAY_FONT_SIZE_32:
			asc_width = 16;
			gb2312_width = 32;
			if(!_asc_32 || !_gb2312_32){
				return -1;
			}
			break;
			
		default:
			// not support font size
			return -1;
	}

	text_height = font_size < canvas->height ? font_size : canvas->height;

	if(!style){
		style = _def_sytle;
	}

	// check style
	if(style & OVERLAY_TEXT_STYLE_BACKGROUND_TRANSPARENT){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(bg_color, 0, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_BACKGROUND_WHRITE){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(bg_color, 255, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_BACKGROUND_BLACK){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(bg_color, 255, 0, 0, 0);
	}
	
	if(style & OVERLAY_TEXT_STYLE_FOREGROUND_TRANSPARENT){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(fg_color, 0, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_FOREGROUND_WHRITE){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(fg_color, 255, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_FOREGROUND_BLACK){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(fg_color, 255, 0, 0, 0);
	}

	if(style & OVERLAY_TEXT_STYLE_ENCLOSED_TRANSPARENT){
		is_enclosed = false;
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(enclosed_color, 0, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_ENCLOSED_WHRITE){
		is_enclosed = true;
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(enclosed_color, 255, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_ENCLOSED_BLACK){
		is_enclosed = true;
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(enclosed_color, 255, 0, 0, 0);
	}

	// the font item stride bytes number, align 8bits / 1byte
	while('\0' != *ch){
		if(*ch < 0x7f){
			if(x + asc_width * ratio > canvas->width){
				break;
			}
			for(i = 0; font_size * ratio > i; i++)
			{
				uint32_t const font = _asc_16[font_size * (*ch) + (i / ratio)];		// 从字库取出一整行
				for(ii = 0; asc_width * ratio > ii; ii++)
				{
					uint32_t current_px = font & 1 << (asc_width - (ii / ratio) - 1);
					canvas->put_pixel(canvas, x + ii, y + i, current_px ? fg_color : bg_color);
				}
			}
			x += asc_width * ratio;
			ch++;
			asc_count++;				// 对齐计数，当字符个数为单数时将进行对齐操作
		}else if(*ch > 0xa0){
			if(1 == asc_count % 2)			// 计算是否需要进行asc对齐
			{
				x += asc_width * ratio;		// 右移一个asc宽度
			}
			asc_count = 0;				// 复位对齐计数

			if(x + gb2312_width * ratio > canvas->width){
				break;
			}

			int const qu_code = ch[0] - 0xa0 - 1;
			int const wei_code = ch[1] - 0xa0 - 1;
			int const offset = (94 * qu_code + wei_code) * 32;
			for(i = 0; font_size * ratio > i; i++)
			{
				uint16_t const font = (_gb2312_16[offset + (i / ratio * 2)] << 8) | (_gb2312_16[offset + (i / ratio * 2) + 1]);
				for(ii = 0; gb2312_width * ratio > ii; ii++)
				{
					uint16_t current_px = font & 1 << (gb2312_width - (ii / ratio) - 1);
					canvas->put_pixel(canvas, x + ii, y + i, current_px ? fg_color : bg_color);
				}
			}
			x += gb2312_width * ratio;
			ch += 2;
		}
	}

	if(is_enclosed){
		if(canvas->height > 0){
			for(i = 1; canvas->height - 1 > i; ++i){			// x = 1 ~ canvas->height - 2
				if(canvas->width > 0){
					for(ii = 1; canvas->width - 1 > ii; ++ii){		// y = 1 ~ canvas->width - 2
						stSDK_ENC_VIDEO_OVERLAY_PIXEL center_color;
						canvas->get_pixel(canvas, x_base + ii, y_base + i, &center_color);
						if(canvas->match_pixel(canvas, center_color, fg_color))
						{
							stSDK_ENC_VIDEO_OVERLAY_PIXEL top_color, buttom_color, left_color, right_color;
							canvas->get_pixel(canvas, x_base + ii, y_base + i - 1, &top_color);
							canvas->get_pixel(canvas, x_base + ii, y_base + i + 1, &buttom_color);
							canvas->get_pixel(canvas, x_base + ii - 1, y_base + i, &left_color);
							canvas->get_pixel(canvas, x_base + ii + 1, y_base + i, &right_color);
							if((!canvas->match_pixel(canvas, top_color, center_color) && !canvas->match_pixel(canvas, top_color, enclosed_color)))
							{
								canvas->put_pixel(canvas, x_base + ii, y_base + i - 1, enclosed_color);
							}
							if((!canvas->match_pixel(canvas, buttom_color, center_color) && !canvas->match_pixel(canvas, buttom_color, enclosed_color)))
							{
								canvas->put_pixel(canvas, x_base + ii, y_base + i + 1, enclosed_color);
							}
							if((!canvas->match_pixel(canvas, left_color, center_color) && !canvas->match_pixel(canvas, left_color, enclosed_color)))
							{
								canvas->put_pixel(canvas, x_base + ii - 1, y_base + i, enclosed_color);
							}
							if((!canvas->match_pixel(canvas, right_color, center_color) && !canvas->match_pixel(canvas, right_color, enclosed_color)))
							{
								canvas->put_pixel(canvas, x_base + ii + 1, y_base + i, enclosed_color);
							}
						}
					}
				}
			}
		}
	}
	
	return x - x_base; // write put size
}


#define    OVERLAY_FONT_LENGTH   24
 
ssize_t OVERLAY_put_text2(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas,
	int x, int y, enOVERLAY_FONT_SIZE font_size, const char *text, OVERLAY_TEXT_STYLE_t style, int ratio,int lenght)
{
	int FONT_CNT  = 0;
	int O_table_x  = 0;
	int O_table_y  = 0;

	int i = 0, ii = 0, iii = 0;
	int const x_base = x;
	int const y_base = y;

	O_table_x = x;
	O_table_y = y;
	
	char *ch = text; // at the beginning of the text
	size_t asc_count = 0;
	size_t asc_width = 0;
	size_t gb2312_width = 0;
	size_t text_width = 0, text_height = 0;

	stSDK_ENC_VIDEO_OVERLAY_PIXEL fg_color, bg_color, enclosed_color;
	bool is_enclosed = false;

	if(!canvas){
		return -1;
	}
	
	switch(font_size){
		case kOVERLAY_FONT_SIZE_16:
			asc_width = 8;
			gb2312_width = 16;
			if(!_asc_16 || !_gb2312_16){
				return -1;
			}
			break;

		case kOVERLAY_FONT_SIZE_20:
			asc_width = 10;
			gb2312_width = 20;
			if(!_asc_20 || !_gb2312_20){
				return -1;
			}
			break;

		case kOVERLAY_FONT_SIZE_24:
			asc_width = 12;
			gb2312_width = 24;
			if(!_asc_24 || !_gb2312_24){
				return -1;
			}
			break;

		case kOVERLAY_FONT_SIZE_32:
			asc_width = 16;
			gb2312_width = 32;
			if(!_asc_32 || !_gb2312_32){
				return -1;
			}
			break;
			
		default:
			// not support font size
			return -1;
	}

//	text_height = font_size * ratio < canvas->height ? font_size * ratio : canvas->height;

	if(!style){
		style = _def_sytle;
	}

	// check style
	if(style & OVERLAY_TEXT_STYLE_BACKGROUND_TRANSPARENT){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(bg_color, 0, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_BACKGROUND_WHRITE){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(bg_color, 255, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_BACKGROUND_BLACK){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(bg_color, 255, 0, 0, 0);
	}
	
	if(style & OVERLAY_TEXT_STYLE_FOREGROUND_TRANSPARENT){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(fg_color, 0, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_FOREGROUND_WHRITE){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(fg_color, 255, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_FOREGROUND_BLACK){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(fg_color, 255, 0, 0, 0);
	}

	if(style & OVERLAY_TEXT_STYLE_ENCLOSED_TRANSPARENT){
		is_enclosed = false;
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(enclosed_color, 0, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_ENCLOSED_WHRITE){
		is_enclosed = true;
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(enclosed_color, 255, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_ENCLOSED_BLACK){
		is_enclosed = true;
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(enclosed_color, 255, 0, 0, 0);
	}

	// the font item stride bytes number, align 8bits / 1byte
	int  m = lenght;
//	while('\0' != *ch){
	while(0 != m){
		m--;
		if(*ch < 0x7f){
			if(x + asc_width * ratio > canvas->width){
				break;
			}
			asc_count++;	
			FONT_CNT++; 
		
			if(FONT_CNT > OVERLAY_FONT_LENGTH){
				y += font_size * ratio ;//asc_width * ratio;
				x = O_table_x;
				FONT_CNT = 1;
			}
			if('\0' != *ch){
				for(i = 0; font_size * ratio > i; i++)
				{
					uint32_t const font = _asc_16[font_size * (*ch) + (i / ratio)]; 	// 从字库取出一整行
					for(ii = 0; asc_width * ratio > ii; ii++)
					{
						uint32_t current_px = font & 1 << (asc_width - (ii / ratio) - 1);
						canvas->put_pixel(canvas, x + ii, y + i, current_px ? fg_color : bg_color);
					}
				}
			}

			x += asc_width * ratio;	
			ch++;

			// 对齐计数，当字符个数为单数时将进行对齐操作
		}else if(*ch > 0xa0){
			if(1 == asc_count % 2)			// 计算是否需要进行asc对齐
			{
				x += asc_width * ratio;		// 右移一个asc宽度			
				FONT_CNT++; 
			}
			asc_count = 0;				// 复位对齐计数
			
			FONT_CNT += 2;
			if(FONT_CNT > OVERLAY_FONT_LENGTH){
				y += font_size * ratio ;//asc_width * ratio;
				x = O_table_x;
				FONT_CNT = 2;
			}

		
			if(x + gb2312_width * ratio > canvas->width){
				break;
			}

			int const qu_code = ch[0] - 0xa0 - 1;
			int const wei_code = ch[1] - 0xa0 - 1;
			int const offset = (94 * qu_code + wei_code) * 32;
			for(i = 0; font_size * ratio > i; i++)
			{
				uint16_t const font = (_gb2312_16[offset + (i / ratio * 2)] << 8) | (_gb2312_16[offset + (i / ratio * 2) + 1]);
				for(ii = 0; gb2312_width * ratio > ii; ii++)
				{
					uint16_t current_px = font & 1 << (gb2312_width - (ii / ratio) - 1);
					canvas->put_pixel(canvas, x + ii, y + i, current_px ? fg_color : bg_color);
				}
			}
			x += gb2312_width * ratio;
			ch += 2;
		}
	}

	text_height += font_size * ratio;
//	text_height = y - y_base;
	text_width =  canvas->width - x_base;
	
	if(is_enclosed){
		for(i = 1; text_height > i; ++i){			// x = 1 ~ text_height - 2
			for(ii = 1; text_width > ii; ++ii){		// y = 1 ~ text_width - 2
				stSDK_ENC_VIDEO_OVERLAY_PIXEL center_color;
				canvas->get_pixel(canvas, x_base + ii, y_base + i, &center_color);
				if(canvas->match_pixel(canvas, center_color, fg_color))
				{
					stSDK_ENC_VIDEO_OVERLAY_PIXEL top_color, buttom_color, left_color, right_color;
					canvas->get_pixel(canvas, x_base + ii, y_base + i - 1, &top_color);
					canvas->get_pixel(canvas, x_base + ii, y_base + i + 1, &buttom_color);
					canvas->get_pixel(canvas, x_base + ii - 1, y_base + i, &left_color);
					canvas->get_pixel(canvas, x_base + ii + 1, y_base + i, &right_color);
					if((!canvas->match_pixel(canvas, top_color, center_color) && !canvas->match_pixel(canvas, top_color, enclosed_color)))
					{
						canvas->put_pixel(canvas, x_base + ii, y_base + i - 1, enclosed_color);
					}
					if((!canvas->match_pixel(canvas, buttom_color, center_color) && !canvas->match_pixel(canvas, buttom_color, enclosed_color)))
					{
						canvas->put_pixel(canvas, x_base + ii, y_base + i + 1, enclosed_color);
					}
					if((!canvas->match_pixel(canvas, left_color, center_color) && !canvas->match_pixel(canvas, left_color, enclosed_color)))
					{
						canvas->put_pixel(canvas, x_base + ii - 1, y_base + i, enclosed_color);
					}
					if((!canvas->match_pixel(canvas, right_color, center_color) && !canvas->match_pixel(canvas, right_color, enclosed_color)))
					{
						canvas->put_pixel(canvas, x_base + ii + 1, y_base + i, enclosed_color);
					}
				}
			}
		}
	}
	
	return x - x_base; // write put size
}

static int overlay_load_font_mem(const char* file_name, uint8_t **ret_mem)
{
	struct stat file_stat={0};
	FILE* fid = NULL;
	if(0 == stat(file_name, &file_stat)){
		fid = fopen(file_name, "rb");
		if(NULL != fid){
			if(NULL != *ret_mem){
				free(*ret_mem);
			}
			*ret_mem = calloc(file_stat.st_size, 1);
			fread(*ret_mem, 1, file_stat.st_size, fid);
			fclose(fid);
			fid = NULL;
			return 0;
		}
	}
	return -1;
}

static void overlay_free_font_mem(uint8_t **mem)
{
    if(mem && *mem){
        free(*mem);
        *mem = NULL;
    }
}

int OVERLAY_load_font(enOVERLAY_FONT_SIZE font_size, const char* asc_font, const char* gb2312_font)
{
	uint8_t **asc_mem = NULL, **gb2312_mem = NULL;
	
	switch(font_size){
		case kOVERLAY_FONT_SIZE_16:
			asc_mem = &_asc_16;
			gb2312_mem = &_gb2312_16;
			break;

		case kOVERLAY_FONT_SIZE_20:
			asc_mem = &_asc_20;
			gb2312_mem = &_gb2312_20;
			break;

		case kOVERLAY_FONT_SIZE_24:
			asc_mem = &_asc_24;
			gb2312_mem = &_gb2312_24;
			break;

		case kOVERLAY_FONT_SIZE_32:
			asc_mem = &_asc_32;
			gb2312_mem = &_gb2312_32;
			break;

		default:
			return -1;
	}

	if(0 == overlay_load_font_mem(asc_font, asc_mem)
		&& 0 == overlay_load_font_mem(gb2312_font, gb2312_mem)){
		return 0;
	}

	free(*asc_mem);
	free(*gb2312_mem);
	return -1;
	
}

void OVERLAY_set_alpha(uint8_t alpha)
{
	_def_alpha = alpha;
}

bool OVERLAY_font_available(enOVERLAY_FONT_SIZE font_size)
{
	switch(font_size){
		case kOVERLAY_FONT_SIZE_16:
			return (NULL != _asc_16 && (NULL != _gb2312_16));
		case kOVERLAY_FONT_SIZE_20:
			return (NULL != _asc_20 && (NULL != _gb2312_20));
		case kOVERLAY_FONT_SIZE_24:
			return (NULL != _asc_24 && (NULL != _gb2312_24));
		case kOVERLAY_FONT_SIZE_32:
			return (NULL != _asc_32 && (NULL != _gb2312_32));
		default: ;
	}
	return false;
}

int OVERLAY_init()
{
	return 0;
}

void OVERLAY_destroy()
{
	overlay_free_font_mem(&_asc_16);
	overlay_free_font_mem(&_asc_20);
	overlay_free_font_mem(&_asc_24);	
	overlay_free_font_mem(&_asc_32);
	overlay_free_font_mem(&_gb2312_16);
	overlay_free_font_mem(&_gb2312_20);
	overlay_free_font_mem(&_gb2312_24);
	overlay_free_font_mem(&_gb2312_32);
}




#ifndef _RGB_H__
#define _RGB_H__

#include <stdint.h>
#define WS2812_MAX_NUMS 144



typedef struct{

	uint8_t total;
	uint8_t rgb_map[WS2812_MAX_NUMS];
	uint8_t	RGB_SHOW_NOW[WS2812_MAX_NUMS][3];
	
	
}RGB;

extern RGB rgb_t;




#endif
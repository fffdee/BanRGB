#ifndef _RGB_H__
#define _RGB_H__

#include <stdint.h>
#define WS2812_MAX_NUMS 100

typedef enum{

	ONLINE = 0,
	NORMAL,
	PWM,
	TRANSFORM,

}MODE;


#define MODE_COUNT 2
#define COLOR_COUNT 8



typedef struct{

	uint8_t total;
	uint8_t color;
	uint8_t temp_rgb[3];
	MODE Mode;
	uint8_t	RGB_SHOW_NOW[WS2812_MAX_NUMS][3];
	
	
}RGB;

extern RGB rgb_t;




#endif
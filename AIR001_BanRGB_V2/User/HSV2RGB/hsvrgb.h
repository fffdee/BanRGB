/**
 * @brief HSV数据与RGB数据转换的头文件
 * @version v3.0
 * @date   2024/01/28
*/

#ifndef hsvrgb_H
#define hsvrgb_H

#include "ws2812b.h"




void RGB2HSV(unsigned char RGB[64][3], int lampIndex);
void HSV2RGB(unsigned char RGB[64][3], int lampIndex);
void HSV_Update_NOW(void);
void HSV_Update_TEMP(void);




#endif

#ifndef FLASH_H__
#define FLASH_H__

#include "air001xx_hal.h"
#include "air001xx_hal_flash.h"
#include "hsvrgb.h"

// #define FLASH_PAGE_SIZE ((uint8_t)0x80)   //one page is 128 bytes

// THE address to start writing and stop writing    needed to be modified later because I have 5 groups to write and read,  0x0800 0000 --- 0x08007FFFH
#define WRITE_START_ADDR ((uint32_t)0X08007C80)
#define WRITE_END_ADDR ((uint32_t)0X08007FFF) /*һ��page 0x80=128 ��128��bytes ����д��32��uint32_t ,  0x20��32��bytes,0x04��4��bytesΪ1��uint32 */

/*details is needed to be redefined later */
/*ȡ0x0800 7000 - 0x0800 7FFF��һ��������26page-32page ��Ϊ����Ϣ�Ĵ洢���� (0x08007000+(0x80 * 25)) = 0x08007C80*/
/* The pages that you need to use (18*5=90 < 128),���������࣬���ǲ���ֻ��ҳ�������ԣ���Ҫ����5��page�����ݣ�ÿ��pageֻ�ÿ�ͷһ���֣�ÿһ��page���0x80*/
/*дFlashֻ�ܽ���1����Ϊ��0���������ɡ�0����Ϊ��1�������������ǽ���0����Ϊ��1��,���Ա���ÿ��д֮ǰ������һҳ*/
/*ÿ��д���ݣ�ֻ������ǰҳ������NUM_PAGE_TO_ERASE Ϊ 1*/
#define NUM_PAGE_TO_ERASE 1

#define Mode1_Start_Addr (uint32_t)(0X08007C80) // page26
#define Mode2_Start_Addr (uint32_t)(0X08007d00) // page27
#define Mode3_Start_Addr (uint32_t)(0X08007d80) // page28
#define Mode4_Start_Addr (uint32_t)(0X08007e00) // page29  XIUZHENG
#define Mode5_Start_Addr (uint32_t)(0X08007e80) // page30  XIUZHENG
#define Mode6_Start_Addr (uint32_t)(0X08007f00) // page31
#define Mode7_Start_Addr (uint32_t)(0X08007f80) // page32

int Flash_Test(void);

/// ������������
void FLASH_Init(void);
void FLASH_Read(uint32_t Address, uint32_t *Buffer, uint32_t NumToRead);
void FLASH_Write(uint32_t Address, const uint32_t *Buffer, uint32_t NumToWrite);
void FLASH_WritePage_NoCheck(uint32_t Address, const uint32_t *Buffer);
void FLASH_ErasePage(uint32_t Address);

void RGB_Storage(RGBModeEnum Modex, uint8_t Source_Array[WS2812_NUMS][3]);
void RGB24to32(uint8_t Data24[WS2812_NUMS][3], uint32_t *Data32);
void ReadRGBDataFromFlash(RGBModeEnum Modex, uint8_t Destination_Array[WS2812_NUMS][3]);
void InitializeRGBArray(void);
//void InitializeRGBArray2(void);
//void InitializeRGBArray3(void);
void Update_Mode_Flash(void);



#endif // !__FLASH_H

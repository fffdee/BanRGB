#ifndef FLASH_H__
#define FLASH_H__

#include "air001xx_hal.h"
#include "air001xx_hal_flash.h"
#include "hsvrgb.h"

// #define FLASH_PAGE_SIZE ((uint8_t)0x80)   //one page is 128 bytes

// THE address to start writing and stop writing    needed to be modified later because I have 5 groups to write and read,  0x0800 0000 --- 0x08007FFFH
#define WRITE_START_ADDR ((uint32_t)0X08007C80)
#define WRITE_END_ADDR ((uint32_t)0X08007FFF) /*一个page 0x80=128 是128个bytes ，能写入32个uint32_t ,  0x20是32个bytes,0x04是4个bytes为1个uint32 */

/*details is needed to be redefined later */
/*取0x0800 7000 - 0x0800 7FFF这一个扇区的26page-32page 作为灯信息的存储区域 (0x08007000+(0x80 * 25)) = 0x08007C80*/
/* The pages that you need to use (18*5=90 < 128),数据量不多，但是擦除只能页擦，所以，需要至少5个page存数据，每个page只用开头一部分，每一个page间距0x80*/
/*写Flash只能将“1”改为“0”，不能由“0”改为“1”，而擦除就是将“0”改为“1”,所以必须每次写之前都擦除一页*/
/*每次写数据，只擦除当前页，所以NUM_PAGE_TO_ERASE 为 1*/
#define NUM_PAGE_TO_ERASE 1

#define Mode1_Start_Addr (uint32_t)(0X08007C80) // page26
#define Mode2_Start_Addr (uint32_t)(0X08007d00) // page27
#define Mode3_Start_Addr (uint32_t)(0X08007d80) // page28
#define Mode4_Start_Addr (uint32_t)(0X08007e00) // page29  XIUZHENG
#define Mode5_Start_Addr (uint32_t)(0X08007e80) // page30  XIUZHENG
#define Mode6_Start_Addr (uint32_t)(0X08007f00) // page31
#define Mode7_Start_Addr (uint32_t)(0X08007f80) // page32

int Flash_Test(void);

/// 导出函数声明
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

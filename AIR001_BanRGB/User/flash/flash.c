
/**
 * @file  flash.c
 *
 * @brief Create by Link-Who on 2023/10/7
 *
 * @note 需要初始化一下几个Mode存储的初始值
 *
 */

#include "flash.h"
#include <string.h>

/*写和擦时间控制器配置；Flash 的写和擦的时间需要进行严谨的控制，否则会造成操作失败。如果需要对 Flash 进行写和擦的操作，
需要根据 HSI 输出频率，参考 FLASH_TS0, FLASH_TS1, FLASH_TS2P, FLASH_TPS3, FLASH_TS3,
FLASH_PERTPE, FLASH_SMERTPE, FLASH_PRGTPE, FLASH_PRETPE 的描述对 Flash 写和擦时间控制
寄存器进行正确的配置。
*/

static uint32_t FlashBuffer[FLASH_PAGE_SIZE >> 2];

/*下面这个函数结合RGB_Storage()完成芯片初次上电时的数据保存*/
void InitializeRGBArray(void)
{
    int led;
    int i = 0;
    int j = 0;
    int rainbow_colors[18][3] = {
        {25, 0, 0},   // 红色
        {25, 4, 0},  // 红橙渐变1
        {25, 8, 0},  // 红橙渐变2
        {25, 16, 0}, // 橙色
        {25, 18, 0}, // 橙黄渐变1
        {25, 20, 0}, // 橙黄渐变2
        {25, 25, 0}, // 黄色
        {12, 25, 0}, // 黄绿渐变1
        {6, 25, 0},  // 黄绿渐变2
        {0, 12, 0},   // 绿色
        {0, 10, 6},  // 绿青渐变1
        {0, 8, 12},  // 绿青渐变2
        {0, 25, 25}, // 青色
        {0, 15, 25}, // 青蓝渐变1
        {0, 10, 25}, // 青蓝渐变2
        {0, 0, 25},   // 蓝色
        {6, 0, 25},  // 蓝紫渐变1
        {12, 0, 12}  // 紫色
    };

	// Mode1
		 RGB_NOW_TEMP_ALLCLEAR();
                HSV_H = 110;
                HSV_S = 1;
                HSV_V = 0.02;
	HSV_Update_NOW();
	RGB_Storage(RGB_Mode1, RGB_SHOW_NOW);   //存储到flash

    // Mode2
    for (i = 0; i < WS2812_NUMS; i++)
    {
        if(i<18)
        {
            for (j = 0; j < 3; j++)
            {
                RGB_SHOW_NOW[i][j] = rainbow_colors[i][j];
            }
        }
        else 
        {
            for (j = 0; j < 3; j++)
            {
                RGB_SHOW_NOW[i][j] = rainbow_colors[17][j];
            }
        }
        
    }
    RGB_Storage(RGB_Mode2, RGB_SHOW_NOW);
	
	// Mode4流星尾

	for (led = 0; led < WS2812_NUMS; led++)
	{
		RGB_SHOW_NOW[led][0] = 255; // red通道设置为最大值
		RGB_SHOW_NOW[led][1] = 0;	// green通道设置为0
		RGB_SHOW_NOW[led][2] = 255; // blue通道设置为0
	}
	RGB_Storage(RGB_Mode4, RGB_SHOW_NOW);


	// Mode 5
	for (led = (WS2812_NUMS-1); led >= 0; led--)
	{
		RGB_SHOW_NOW[led][0] = 0; // red通道设置为最大值
		RGB_SHOW_NOW[led][1] = 255; // green通道设置为0
		RGB_SHOW_NOW[led][2] = 0; // blue通道设置为0
	}
	RGB_Storage(RGB_Mode5, RGB_SHOW_NOW);

	



 



}



void FLASH_Init(void)
{
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS); /* Clear SR register */
    HAL_FLASH_Lock();
}

void FLASH_Read(uint32_t Address, uint32_t *Buffer, uint32_t NumToRead)
{
    while (NumToRead--)
    {
        *Buffer++ = (*((volatile unsigned int *)Address));
        Address += 4; // 4 bytess once write or read
    }
}

void FLASH_WritePage_NoCheck(uint32_t Address, const uint32_t *Buffer)
{
    HAL_FLASH_Unlock(); // 解锁
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_PAGE, Address, (uint32_t *)Buffer);
    HAL_FLASH_Lock(); // 上锁
}

void FLASH_ErasePage(uint32_t Address)
{
    uint32_t PageError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct = {0};
    HAL_FLASH_Unlock();
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGEERASE;
    EraseInitStruct.PageAddress = Address;
    EraseInitStruct.NbPages = NUM_PAGE_TO_ERASE; /* erase nums pages. */
    HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    HAL_FLASH_Lock();
}

void FLASH_Write(uint32_t Address, const uint32_t *Buffer, uint32_t NumToWrite)
{
    uint32_t secpos;    // 页地址
    uint32_t secoff;    // 页内偏移地址(16位字计算)
    uint32_t secremain; // 页内剩余地址(16位字计算)
    uint32_t i;
    uint32_t offaddr;                            // 去掉0X08000000后的地址
    HAL_FLASH_Unlock();                          // 解锁
    offaddr = Address - FLASH_BASE;              // 实际偏移地址.
    secpos = offaddr / FLASH_PAGE_SIZE;          // 页地址
    secoff = (offaddr % FLASH_PAGE_SIZE) >> 2;   // 在页内的偏移(2个字节为基本单位.)
    secremain = (FLASH_PAGE_SIZE >> 2) - secoff; // 页剩余空间大小
    if (NumToWrite <= secremain)
        secremain = NumToWrite; // 不大于该页范围
    while (1)
    {
        FLASH_Read(secpos * FLASH_PAGE_SIZE + FLASH_BASE, FlashBuffer, FLASH_PAGE_SIZE >> 2); // 读出整个页的内容
        for (i = 0; i < (FLASH_PAGE_SIZE >> 2); i++)                                          // 校验数据
        {
            if (FlashBuffer[i] != 0XFFFFFFFF)
            {
                uint32_t PageError = 0;
                FLASH_EraseInitTypeDef EraseInitStruct = {0};
                EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGEERASE;
                EraseInitStruct.PageAddress = Address;
                EraseInitStruct.NbPages = 1; /* erase nums pages. */
                HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
                break;
            }
        }
        for (i = 0; i < secremain; i++) // 复制
        {
            FlashBuffer[i + secoff] = Buffer[i];
        }
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_PAGE, secpos * FLASH_PAGE_SIZE + FLASH_BASE, (uint32_t *)FlashBuffer); // 写入整个页
        if (NumToWrite == secremain)
            break; // 写入结束了
        else       // 写入未结束
        {
            secpos++;                // 页地址增1
            secoff = 0;              // 偏移位置为0
            Buffer += secremain;     // 指针偏移
            Address += secremain;    // 写地址偏移
            NumToWrite -= secremain; // 字节(16位)数递减
            if (NumToWrite > (FLASH_PAGE_SIZE >> 2))
                secremain = (FLASH_PAGE_SIZE >> 2); // 下一个页还是写不完
            else
                secremain = NumToWrite; // 下一个页可以写完了
        }
    }
    HAL_FLASH_Lock(); // 解锁
}

void RGB_Storage(RGBModeEnum Modex, uint8_t Source_Array[WS2812_NUMS][3])
{

    uint32_t Address = 0x00; // 记录写入的地址

    switch (Modex)
    {
    case RGB_Mode1:
        Address = Mode1_Start_Addr;
        break;
    case RGB_Mode2:
        Address = Mode2_Start_Addr;
        break;
    case RGB_Mode3:
        Address = Mode3_Start_Addr;
        break;
    case RGB_Mode4:
        Address = Mode4_Start_Addr;
        break;
    case RGB_Mode5:
        Address = Mode5_Start_Addr;
        break;
    case RGB_Mode6:
        Address = Mode6_Start_Addr;
        break;
    default:
        break; // I want to increase a new printf here;
    }

    RGB24to32(Source_Array, RGB24BUT32); // 将24位RGB数据转换为32位

    FLASH_Init();

    FLASH_ErasePage(Address);

    // for test    FLASH_WritePage_NoCheck(0x08007000, &data);
    FLASH_WritePage_NoCheck(Address, RGB24BUT32);
}

void RGB24to32(uint8_t Data24[WS2812_NUMS][3], uint32_t *Data32)
{
    int i = 0;
    for (i = 0; i < WS2812_NUMS; i++)
    {
        Data32[i] |= ((uint32_t)Data24[i][0] << 16); // 红色通道   Data24[]与RGN_SHOW_NOW[][]的内存分布都是连续的，需要check一下；
        Data32[i] |= ((uint32_t)Data24[i][1] << 8);  // 绿色通道
        Data32[i] |= (uint32_t)Data24[i][2];         // 蓝色通道
    }
}

/*保存在RGB_SHOW_TEMP中*/
void ReadRGBDataFromFlash(RGBModeEnum Modex, uint8_t Destination_Array[WS2812_NUMS][3])
{
    uint32_t Address = 0; // 记录写入的地址
    uint32_t readData = 0;
    int i = 0;
    switch (Modex)
    {
    case RGB_Mode1:
        Address = Mode1_Start_Addr;
        break;
    case RGB_Mode2:
        Address = Mode2_Start_Addr;
        break;
    case RGB_Mode3:
        Address = Mode3_Start_Addr;
        break;
    case RGB_Mode4:
        Address = Mode4_Start_Addr;
        break;
    case RGB_Mode5:
        Address = Mode5_Start_Addr;
        break;
    case RGB_Mode6:
        Address = Mode6_Start_Addr;
        break;
    default:
        break; // I want to increase a new printf here;
    }
    readData = *(__IO uint32_t *)Address;

    // 解析32位数据为RGB并装入RGB数组
    for (i = 0; (i < WS2812_NUMS); i++)
    {
        Destination_Array[i][0] = (char)((readData >> 16) & 0xFF); // 红色通道
        Destination_Array[i][1] = (char)((readData >> 8) & 0xFF);  // 绿色通道
        Destination_Array[i][2] = (char)(readData & 0xFF);         // 蓝色通道

        // 更新Flash地址以读取下一个32位数据
        Address += 4;
        readData = *(__IO uint32_t *)Address;
    }
}

/*读取当前模式的RGB到RGB_SHOW_TEMP，下面update到showNOW,mode1不用转，*/
void Update_Mode_Flash(void)
{
    if (ModeChange_sign == 1)
    {

        // return; // 中断响应后才改，标志为1

        switch (MODE_NOW)
        {
        case RGB_Mode1:
            /*存储最后一个mode的RGB RGB_Storage(RGB_Mode_END)*/
            ReadRGBDataFromFlash(RGB_Mode1, RGB_SHOW_NOW); //
            ModeChange_sign = 0;
            RGB2HSV(RGB_SHOW_NOW, 0); // 更新当前的HSV信息
            //	MODE1_FULL_DATA_PREPARE();
            break;
        case RGB_Mode2:
            ReadRGBDataFromFlash(RGB_Mode2, RGB_SHOW_TEMP);
            ModeChange_sign = 0;

            break;
        case RGB_Mode3:
            ReadRGBDataFromFlash(RGB_Mode3, RGB_SHOW_TEMP);
            ModeChange_sign = 0;
            break;
        case RGB_Mode4:
            ReadRGBDataFromFlash(RGB_Mode4, RGB_SHOW_TEMP);
            ModeChange_sign = 0;
            break;
        case RGB_Mode5:
            ReadRGBDataFromFlash(RGB_Mode5, RGB_SHOW_TEMP);
            ModeChange_sign = 0;
            break;
        case RGB_Mode6:
            ReadRGBDataFromFlash(RGB_Mode6, RGB_SHOW_TEMP);
            ModeChange_sign = 0;
            break;
        default:
            break;
        }
    }
}

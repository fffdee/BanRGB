
/**
 * @file  flash.c
 *
 * @brief Create by Link-Who on 2023/10/7
 *
 * @note ��Ҫ��ʼ��һ�¼���Mode�洢�ĳ�ʼֵ
 *
 */

#include "flash.h"
#include <string.h>

/*д�Ͳ�ʱ����������ã�Flash ��д�Ͳ���ʱ����Ҫ�����Ͻ��Ŀ��ƣ��������ɲ���ʧ�ܡ������Ҫ�� Flash ����д�Ͳ��Ĳ�����
��Ҫ���� HSI ���Ƶ�ʣ��ο� FLASH_TS0, FLASH_TS1, FLASH_TS2P, FLASH_TPS3, FLASH_TS3,
FLASH_PERTPE, FLASH_SMERTPE, FLASH_PRGTPE, FLASH_PRETPE �������� Flash д�Ͳ�ʱ�����
�Ĵ���������ȷ�����á�
*/

static uint32_t FlashBuffer[FLASH_PAGE_SIZE >> 2];

/*��������������RGB_Storage()���оƬ�����ϵ�ʱ�����ݱ���*/
void InitializeRGBArray(void)
{
    int led;
    int i = 0;
    int j = 0;
    int rainbow_colors[18][3] = {
        {25, 0, 0},   // ��ɫ
        {25, 4, 0},  // ��Ƚ���1
        {25, 8, 0},  // ��Ƚ���2
        {25, 16, 0}, // ��ɫ
        {25, 18, 0}, // �Ȼƽ���1
        {25, 20, 0}, // �Ȼƽ���2
        {25, 25, 0}, // ��ɫ
        {12, 25, 0}, // ���̽���1
        {6, 25, 0},  // ���̽���2
        {0, 12, 0},   // ��ɫ
        {0, 10, 6},  // ���ཥ��1
        {0, 8, 12},  // ���ཥ��2
        {0, 25, 25}, // ��ɫ
        {0, 15, 25}, // ��������1
        {0, 10, 25}, // ��������2
        {0, 0, 25},   // ��ɫ
        {6, 0, 25},  // ���Ͻ���1
        {12, 0, 12}  // ��ɫ
    };

	// Mode1
		 RGB_NOW_TEMP_ALLCLEAR();
                HSV_H = 110;
                HSV_S = 1;
                HSV_V = 0.02;
	HSV_Update_NOW();
	RGB_Storage(RGB_Mode1, RGB_SHOW_NOW);   //�洢��flash

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
	
	// Mode4����β

	for (led = 0; led < WS2812_NUMS; led++)
	{
		RGB_SHOW_NOW[led][0] = 255; // redͨ������Ϊ���ֵ
		RGB_SHOW_NOW[led][1] = 0;	// greenͨ������Ϊ0
		RGB_SHOW_NOW[led][2] = 255; // blueͨ������Ϊ0
	}
	RGB_Storage(RGB_Mode4, RGB_SHOW_NOW);


	// Mode 5
	for (led = (WS2812_NUMS-1); led >= 0; led--)
	{
		RGB_SHOW_NOW[led][0] = 0; // redͨ������Ϊ���ֵ
		RGB_SHOW_NOW[led][1] = 255; // greenͨ������Ϊ0
		RGB_SHOW_NOW[led][2] = 0; // blueͨ������Ϊ0
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
    HAL_FLASH_Unlock(); // ����
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_PAGE, Address, (uint32_t *)Buffer);
    HAL_FLASH_Lock(); // ����
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
    uint32_t secpos;    // ҳ��ַ
    uint32_t secoff;    // ҳ��ƫ�Ƶ�ַ(16λ�ּ���)
    uint32_t secremain; // ҳ��ʣ���ַ(16λ�ּ���)
    uint32_t i;
    uint32_t offaddr;                            // ȥ��0X08000000��ĵ�ַ
    HAL_FLASH_Unlock();                          // ����
    offaddr = Address - FLASH_BASE;              // ʵ��ƫ�Ƶ�ַ.
    secpos = offaddr / FLASH_PAGE_SIZE;          // ҳ��ַ
    secoff = (offaddr % FLASH_PAGE_SIZE) >> 2;   // ��ҳ�ڵ�ƫ��(2���ֽ�Ϊ������λ.)
    secremain = (FLASH_PAGE_SIZE >> 2) - secoff; // ҳʣ��ռ��С
    if (NumToWrite <= secremain)
        secremain = NumToWrite; // �����ڸ�ҳ��Χ
    while (1)
    {
        FLASH_Read(secpos * FLASH_PAGE_SIZE + FLASH_BASE, FlashBuffer, FLASH_PAGE_SIZE >> 2); // ��������ҳ������
        for (i = 0; i < (FLASH_PAGE_SIZE >> 2); i++)                                          // У������
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
        for (i = 0; i < secremain; i++) // ����
        {
            FlashBuffer[i + secoff] = Buffer[i];
        }
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_PAGE, secpos * FLASH_PAGE_SIZE + FLASH_BASE, (uint32_t *)FlashBuffer); // д������ҳ
        if (NumToWrite == secremain)
            break; // д�������
        else       // д��δ����
        {
            secpos++;                // ҳ��ַ��1
            secoff = 0;              // ƫ��λ��Ϊ0
            Buffer += secremain;     // ָ��ƫ��
            Address += secremain;    // д��ַƫ��
            NumToWrite -= secremain; // �ֽ�(16λ)���ݼ�
            if (NumToWrite > (FLASH_PAGE_SIZE >> 2))
                secremain = (FLASH_PAGE_SIZE >> 2); // ��һ��ҳ����д����
            else
                secremain = NumToWrite; // ��һ��ҳ����д����
        }
    }
    HAL_FLASH_Lock(); // ����
}

void RGB_Storage(RGBModeEnum Modex, uint8_t Source_Array[WS2812_NUMS][3])
{

    uint32_t Address = 0x00; // ��¼д��ĵ�ַ

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

    RGB24to32(Source_Array, RGB24BUT32); // ��24λRGB����ת��Ϊ32λ

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
        Data32[i] |= ((uint32_t)Data24[i][0] << 16); // ��ɫͨ��   Data24[]��RGN_SHOW_NOW[][]���ڴ�ֲ����������ģ���Ҫcheckһ�£�
        Data32[i] |= ((uint32_t)Data24[i][1] << 8);  // ��ɫͨ��
        Data32[i] |= (uint32_t)Data24[i][2];         // ��ɫͨ��
    }
}

/*������RGB_SHOW_TEMP��*/
void ReadRGBDataFromFlash(RGBModeEnum Modex, uint8_t Destination_Array[WS2812_NUMS][3])
{
    uint32_t Address = 0; // ��¼д��ĵ�ַ
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

    // ����32λ����ΪRGB��װ��RGB����
    for (i = 0; (i < WS2812_NUMS); i++)
    {
        Destination_Array[i][0] = (char)((readData >> 16) & 0xFF); // ��ɫͨ��
        Destination_Array[i][1] = (char)((readData >> 8) & 0xFF);  // ��ɫͨ��
        Destination_Array[i][2] = (char)(readData & 0xFF);         // ��ɫͨ��

        // ����Flash��ַ�Զ�ȡ��һ��32λ����
        Address += 4;
        readData = *(__IO uint32_t *)Address;
    }
}

/*��ȡ��ǰģʽ��RGB��RGB_SHOW_TEMP������update��showNOW,mode1����ת��*/
void Update_Mode_Flash(void)
{
    if (ModeChange_sign == 1)
    {

        // return; // �ж���Ӧ��Ÿģ���־Ϊ1

        switch (MODE_NOW)
        {
        case RGB_Mode1:
            /*�洢���һ��mode��RGB RGB_Storage(RGB_Mode_END)*/
            ReadRGBDataFromFlash(RGB_Mode1, RGB_SHOW_NOW); //
            ModeChange_sign = 0;
            RGB2HSV(RGB_SHOW_NOW, 0); // ���µ�ǰ��HSV��Ϣ
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

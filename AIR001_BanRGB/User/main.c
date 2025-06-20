#include "air001xx_hal.h"
#include "air001xx.h"
#include "uart.h"
#include "string.h"
#include "rgb.h"
#include "stdint.h"
#include "stdlib.h"
#include <stdio.h>
#include <math.h>

#define TRAN_LIMIT 200
#define TRAN_COUNT 15
#define RESET_COUNT 40
#define HRESET_COUNT 100

#define SHORT_PRESS_MIN 20
#define LONG_PRESS_MIN 1000
#define LONG_PRESS_MAX 3000


uint8_t rx_buffer[WS2812_MAX_NUMS*5] = {0}; 

void Error_Handler(void);
static void APP_SystemClockConfig(void); 
uint16_t i;

uint16_t rgb_range[3];
uint16_t rgb_range_old[3];
uint8_t tran_count[3];

uint8_t first_press = 0;
uint8_t key_value = 0;
uint16_t tick_time = 0; 
uint8_t key_type = 0;
uint16_t key_time =0;
uint8_t long_press_flag=0;
uint8_t pwm_flag = 0;
extern volatile uint8_t ch;

IWDG_HandleTypeDef   IwdgHandle;

RGB rgb_t = {
		.total = 64,
		.Mode = ONLINE,
		
		.RGB_SHOW_NOW = {0},
};

void user_key_handle(RGB *p_rgb);
void screen_mode_loop(RGB *p_rgb);
void normal_mode(RGB *p_rgb);
void pwm_mode(RGB *p_rgb);
void transform_mode(RGB *p_rgb);
void clear(RGB *p_rgb);
typedef struct{
	
	MODE ID;
	void (*handle)(RGB *p_rgb);
	
}ModeSelector;


ModeSelector mode_select_t[MODE_COUNT] = {

	{ONLINE,screen_mode_loop},
	{NORMAL,normal_mode},
//	{PWM,pwm_mode},
//	{TRANSFORM,transform_mode},
	
};
// 配置按键
void KEY_CONFIG(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;       
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;         
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; 
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    __HAL_RCC_GPIOB_CLK_ENABLE(); // 使能GPIOB时钟
}



void short_press(RGB *p_rgb)
{
		key_type=1;
		
	
		
	
		if(p_rgb->Mode!=ONLINE){
			for(i=0;i<3;i++){
				p_rgb->temp_rgb[i] = rand()%255;
			}
		}
	
	
}


void long_press(RGB *p_rgb)
{
		key_type=2;
		p_rgb->Mode++;
		if(p_rgb->Mode > MODE_COUNT){
				p_rgb->Mode = ONLINE;
			clear(p_rgb);
		}
}


void longlong_press(RGB *p_rgb)
{
	
}


int main(void)
{
    HAL_Init();
		HAL_InitTick(3);
    APP_SystemClockConfig(); // 系统时钟配置
    Uart_Init(); // 串口初始化
    SPI_CONFIG(); // SPI配置
    KEY_CONFIG(); // 按键配置
    printf("E");
		__HAL_RCC_LSI_ENABLE();
    IwdgHandle.Instance = IWDG;                     
    IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;  
    IwdgHandle.Init.Reload = (1000);               
    HAL_IWDG_Init(&IwdgHandle);

    while (1)
    {	
    
        user_key_handle(&rgb_t);
        if (HAL_IWDG_Refresh(&IwdgHandle) != HAL_OK) // 刷新看门狗
        {
            NVIC_SystemReset(); 
            Error_Handler();
        }
    }
}

// 系统时钟配置
static void APP_SystemClockConfig(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI ;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;						  
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;						  
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz; 
    RCC_OscInitStruct.HSEState = RCC_HSE_OFF;						  
    RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;					  
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;						
    RCC_OscInitStruct.LSEState = RCC_LSE_OFF;							
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;					
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;			  
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI; 
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;	   
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;	   
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  
    __HAL_RCC_FLASH_CLK_ENABLE(); 
    __HAL_RCC_GPIOB_CLK_ENABLE(); 
    __HAL_RCC_GPIOA_CLK_ENABLE(); 
    __HAL_RCC_SPI1_CLK_ENABLE();  
    __HAL_RCC_PWR_CLK_ENABLE();
}

// 错误处理函数
void Error_Handler(void)
{
    while (1)
    {
    }
}

void clear_flag()
{
		key_time = 0;
		tick_time=0;
		
		key_value=0;
}
// 按键检测
void user_key_handle(RGB *p_rgb)
{
    key_value = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6);
	
		if(long_press_flag){
			
			if(HAL_GetTick()%50)
				longlong_press(p_rgb);
		}
		
		 if(key_value==1){
	
			if(tick_time==0){
        tick_time = HAL_GetTick();
			}
			
		
    }
		 
		if(key_value==0){
			
			if(tick_time!=0){
				key_time = HAL_GetTick()-tick_time;
			
				if((key_time>SHORT_PRESS_MIN)&&(key_time<LONG_PRESS_MIN))
				{
					short_press(p_rgb);
					clear_flag();
				}
				if((key_time>LONG_PRESS_MIN)&&(key_time<LONG_PRESS_MAX))
				{
					
					long_press(p_rgb);
					clear_flag();
					
				}
				
				if(key_time>LONG_PRESS_MAX)
				{
					
					long_press_flag=1;
					clear_flag();
					
				}
			}
		
		}
	
	
		if(tick_time==0)
		mode_select_t[p_rgb->Mode].handle(p_rgb);
		
		
		
}

// 彩灯循环显示
void Color_Browse(void)
{			
    RGB *p_rgb = &rgb_t;
    for(i=0; i<240; i++){
        memset(rgb_t.RGB_SHOW_NOW, 0x00, WS2812_MAX_NUMS*3);
        rgb_t.RGB_SHOW_NOW[i%WS2812_MAX_NUMS][1] = i;
        WS2812B_SendRGB((unsigned char(*)[3])p_rgb->RGB_SHOW_NOW);
        HAL_Delay(1);

        memset(rgb_t.RGB_SHOW_NOW, 0x10, WS2812_MAX_NUMS*3);
        WS2812B_SendRGB((unsigned char(*)[3])p_rgb->RGB_SHOW_NOW);
        HAL_Delay(1);
    }
}

// 屏幕模式循环
void screen_mode_loop(RGB *p_rgb)
{
    for(i=0; i<rgb_t.total*5; i++){
        // 处理接收到的数据0xEA
        if(rx_buffer[i] == 0xEA){
						
						
            uint8_t index = rx_buffer[i+1];
            uint8_t count = 0;
            for(; count<3; count++){
                rgb_t.RGB_SHOW_NOW[index][count] = rx_buffer[i+2+count];
                if(index == rgb_t.total-2)
                    p_rgb->RGB_SHOW_NOW[rgb_t.total-1][count] = p_rgb->RGB_SHOW_NOW[0][count];
            }
            HAL_UART_IRQHandler(&UartHandle);
        }
			if(rx_buffer[i] == 0xEC){
					if(rx_buffer[i+2]==(rx_buffer[i+1]+127)&&rx_buffer[i+3]==0xCE){
					
							p_rgb->total = rx_buffer[i+1];
					}
    }
		 
		  
					
		}
    WS2812B_SendRGB((unsigned char(*)[3])p_rgb->RGB_SHOW_NOW);	      
//    for(i=0; i < p_rgb->total; i++){
//        rgb_range[0] += p_rgb->RGB_SHOW_NOW[i][0];
//        rgb_range[1] += p_rgb->RGB_SHOW_NOW[i][1];
//        rgb_range[2] += p_rgb->RGB_SHOW_NOW[i][2];
//    }
//            
//    for(i=0; i<3; i++){
//        rgb_range[i] = rgb_range[i]/p_rgb->total;
//        tran_count[i] = abs(rgb_range[i] - rgb_range_old[i]); 
//    }

//    uint8_t total_rgb = (rgb_range[0] + rgb_range[1] + rgb_range[2])/3;
//    if(total_rgb < TRAN_LIMIT){
//        WS2812B_SendRGB((unsigned char(*)[3])p_rgb->RGB_SHOW_NOW);
//    }else{
//        if(tran_count[0] > TRAN_COUNT || tran_count[1] > TRAN_COUNT || tran_count[2] > TRAN_COUNT ){
//            WS2812B_SendRGB((unsigned char(*)[3])p_rgb->RGB_SHOW_NOW);
//            for(i=0; i<3; i++)
//                rgb_range_old[i] = rgb_range[i];
//        }
//    }
}



void clear(RGB *p_rgb)
{
	for(i=0; i<WS2812_MAX_NUMS; i++)
		{
				p_rgb->RGB_SHOW_NOW[i][0]=0;
				p_rgb->RGB_SHOW_NOW[i][1]=0;
				p_rgb->RGB_SHOW_NOW[i][2]=0;
		}
		WS2812B_SendRGB((unsigned char(*)[3])p_rgb->RGB_SHOW_NOW);
}

void normal_mode(RGB *p_rgb)
{
	
		for(i=0; i<WS2812_MAX_NUMS; i++)
		{
				p_rgb->RGB_SHOW_NOW[i][0]=p_rgb->temp_rgb[0];
				p_rgb->RGB_SHOW_NOW[i][1]=p_rgb->temp_rgb[1];
				p_rgb->RGB_SHOW_NOW[i][2]=p_rgb->temp_rgb[2];
		}
		WS2812B_SendRGB((unsigned char(*)[3])p_rgb->RGB_SHOW_NOW);
		
}


// RGB转HSV函数
void RGB_to_HSV(uint8_t R, uint8_t G, uint8_t B, uint8_t* H, uint8_t* S, uint8_t* V) {
    // 将RGB值转换为0-1之间的浮点数
    float r = R / 255.0;
    float g = G / 255.0;
    float b = B / 255.0;

    // 计算最大值、最小值和差值
    float max_val = fmaxf(fmaxf(r, g), b);
    float min_val = fminf(fminf(r, g), b);
    float delta = max_val - min_val;

    // 计算色相H
    if (delta == 0) {
        *H = 0;
    } else if (max_val == r) {
        *H = (uint8_t)(60 * ((g - b) / delta));
    } else if (max_val == g) {
        *H = (uint8_t)(60 * (2 + (b - r) / delta));
    } else {
        *H = (uint8_t)(60 * (4 + (r - g) / delta));
    }

    // 计算饱和度S
    if (max_val == 0) {
        *S = 0;
    } else {
        *S = (uint8_t)((delta / max_val) * 255);
    }

    // 计算明度V
    *V = (uint8_t)(max_val * 255);
}

// HSV转RGB函数
void HSV_to_RGB(uint8_t H, uint8_t S, uint8_t V, uint8_t* R, uint8_t* G, uint8_t* B) {
    // 将H、S、V转换为浮点数
    float h = H;
    float s = S / 255.0;
    float v = V / 255.0;

    // 计算色相所在的区间
    int hi = (int)(h / 60) % 6;
    float f = (h / 60) - hi;
    float p = v * (1 - s);
    float q = v * (1 - s * f);
    float t = v * (1 - s * (1 - f));

    // 根据不同区间计算RGB值
    float r = 0, g = 0, b = 0;
    switch (hi) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        case 5:
            r = v;
            g = p;
            b = q;
            break;
    }

    // 将RGB值转换为0-255的整数
    *R = (uint8_t)(r * 255);
    *G = (uint8_t)(g * 255);
    *B = (uint8_t)(b * 255);
}

// 呼吸灯效果函数
void breathing_light(uint8_t* R, uint8_t* G, uint8_t* B, uint8_t rate) {
    static uint8_t target_R = 0, target_G = 0, target_B = 0;
    static uint8_t brightness_dir = 0; // 0: 变亮，1: 变暗
    static uint8_t current_V = 0;
    uint8_t H, S, V;

    // 将初始RGB颜色转换为HSV
    RGB_to_HSV(*R, *G, *B, &H, &S, &V);

    // 调整亮度
    if (brightness_dir == 0) {
        current_V += rate;
        if (current_V >= V) {
            current_V = V;
            brightness_dir = 1;
        }
    } else {
        current_V -= rate;
        if (current_V <= 5) {
            current_V = 0;
            brightness_dir = 0;
        }
    }

    // 将调整后的HSV转换回RGB
    HSV_to_RGB(H, S, current_V, R, G, B);
}

// 在PWM模式下使用呼吸灯效果
void pwm_mode(RGB *p_rgb) {
    static uint8_t initial_R = 255, initial_G = 0, initial_B = 0; // 初始颜色为红色
    static uint8_t rate = 5; // 呼吸速度

    // 应用呼吸灯效果
    breathing_light(&initial_R, &initial_G, &initial_B, rate);

    // 将结果应用到所有灯
    for (i = 0; i < p_rgb->total; i++) {
        p_rgb->RGB_SHOW_NOW[i][0] = initial_R;
        p_rgb->RGB_SHOW_NOW[i][1] = initial_G;
        p_rgb->RGB_SHOW_NOW[i][2] = initial_B;
    }

    // 发送数据到LED灯带
    WS2812B_SendRGB((unsigned char(*)[3])p_rgb->RGB_SHOW_NOW);

    // 延时控制呼吸速度
    HAL_Delay(50);
}
		
		


void transform_mode(RGB *p_rgb)
{
		
}



#include "main.h"
#include "sensor.h"
#include "stm32f1xx_hal.h"    // HAL 核心库
#include "tim.h"               // 你在 CubeMX 里生成的定时器句柄头文件
#include "gpio.h"              // GPIO 引脚宏定义
#include <stdint.h>


uint32_t ic_val1 = 0;
uint32_t ic_val2 = 0;
uint8_t is_first_captured = 0;
uint32_t difference = 0;
float distance = 0;

extern TIM_HandleTypeDef htim2;

/**
 * @brief  微秒级延时函数
 * @param  us: 延时时间，单位为微秒
 * @retval 无
 * 
 * @note   该函数使用TIM2定时器实现精确的微秒级延时
 */
void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);  // ?????
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}



void HCSR04_Trigger(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);  // TRIG ???
    delay_us(10);  // ?? 10us
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); // ??
}

/**
 * @brief  TIM输入捕获回调函数，用于处理超声波测距的回波信号
 * @param  htim: TIM句柄指针，指向触发回调的定时器实例
 * @retval None
 * 
 * 该函数通过输入捕获方式测量超声波传感器的回波时间，从而计算出距离。
 * 使用TIM2_CH1通道，通过检测回波信号的上升沿和下降沿来测量高电平持续时间。
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)  // 判断是否为TIM2_CH1通道触发的中断
    {
        if (is_first_captured == 0)   // 第一次捕获，记录上升沿时刻
        {
            ic_val1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);  // 读取第一次捕获值（上升沿时刻）
            is_first_captured = 1;

            // 设置下次捕获为下降沿触发
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
        }
        else if (is_first_captured == 1)  // 第二次捕获，记录下降沿时刻并计算距离
        {
            ic_val2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            if (ic_val2 > ic_val1)
                difference = ic_val2 - ic_val1;
            else
                difference = (0xFFFF - ic_val1) + ic_val2;

            // 距离计算:声速340m/s = 0.034 cm/us，除以2是因为超声波往返时间
            distance = (difference * 0.034) / 2;  // cm

            is_first_captured = 0;

            // 重新设置为上升沿触发，准备下一次测量
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
            __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);
        }
    }
}




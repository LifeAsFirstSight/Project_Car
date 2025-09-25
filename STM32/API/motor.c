#include "motor.h"
#include <stdint.h>
#include "tim.h"
#include "gpio.h"
#include "stm32f1xx_hal.h"

// 使用 TIM4_CH1(PB6) 控制左电机速度
// 使用 TIM4_CH2(PB7) 控制右电机速度
extern TIM_HandleTypeDef htim4;

void Motor_Init(void)
{
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);  // PB6 -> 左电机
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);  // PB7 -> 右电机
}

// ========== 左电机 ==========
void Motor_Left_Forward(uint16_t speed)
{
    HAL_GPIO_WritePin(LEFT_IN1_GPIO_Port, LEFT_IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LEFT_IN2_GPIO_Port, LEFT_IN2_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, speed);
}

void Motor_Left_Backward(uint16_t speed)
{
    HAL_GPIO_WritePin(LEFT_IN1_GPIO_Port, LEFT_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LEFT_IN2_GPIO_Port, LEFT_IN2_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, speed);
}

void Motor_Left_Stop(void)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
    HAL_GPIO_WritePin(LEFT_IN1_GPIO_Port, LEFT_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LEFT_IN2_GPIO_Port, LEFT_IN2_Pin, GPIO_PIN_RESET);
}

// ========== 右电机 ==========
void Motor_Right_Forward(uint16_t speed)
{
    HAL_GPIO_WritePin(RIGHT_IN1_GPIO_Port, RIGHT_IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RIGHT_IN2_GPIO_Port, RIGHT_IN2_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, speed);
}

void Motor_Right_Backward(uint16_t speed)
{
    HAL_GPIO_WritePin(RIGHT_IN1_GPIO_Port, RIGHT_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RIGHT_IN2_GPIO_Port, RIGHT_IN2_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, speed);
}

void Motor_Right_Stop(void)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
    HAL_GPIO_WritePin(RIGHT_IN1_GPIO_Port, RIGHT_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RIGHT_IN2_GPIO_Port, RIGHT_IN2_Pin, GPIO_PIN_RESET);
}

// ========== 小车整体动作 ==========
void Car_Forward(uint16_t speed)
{
    Motor_Left_Forward(speed);
    Motor_Right_Forward(speed);
}

void Car_Backward(uint16_t speed)
{
    Motor_Left_Backward(speed);
    Motor_Right_Backward(speed);
}

void Car_Stop(void)
{
    Motor_Left_Stop();
    Motor_Right_Stop();
}

void Car_Turn_Left(uint16_t speed)
{
    Motor_Left_Forward(speed / 2);   // 内侧慢一点
    Motor_Right_Forward(speed);
}

void Car_Turn_Right(uint16_t speed)
{
    Motor_Left_Forward(speed);
    Motor_Right_Forward(speed / 2);
}

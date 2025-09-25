#ifndef __MOTOR_H  
#define __MOTOR_H  

#include "stm32f1xx_hal.h"  

#define LEFT_IN1_GPIO_Port   GPIOB
#define LEFT_IN1_Pin         GPIO_PIN_3
#define LEFT_IN2_GPIO_Port   GPIOB
#define LEFT_IN2_Pin         GPIO_PIN_4

#define RIGHT_IN1_GPIO_Port  GPIOB
#define RIGHT_IN1_Pin        GPIO_PIN_5
#define RIGHT_IN2_GPIO_Port  GPIOA
#define RIGHT_IN2_Pin        GPIO_PIN_15


void Motor_Init(void);
// 左电机控制
void Motor_Left_Forward(uint16_t speed);
void Motor_Left_Backward(uint16_t speed);
void Motor_Left_Stop(void);
// 右电机控制
void Motor_Right_Forward(uint16_t speed);
void Motor_Right_Backward(uint16_t speed);
void Motor_Right_Stop(void);

// 小车整体动作
void Car_Forward(uint16_t speed);
void Car_Backward(uint16_t speed);
void Car_TurnLeft(uint16_t speed);
void Car_TurnRight(uint16_t speed);
void Motor_Stop(void);



#endif  

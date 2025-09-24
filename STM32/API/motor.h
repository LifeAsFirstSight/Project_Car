#define MOTOR_H
#ifndef MOTOR_H
#include "main.h"

#define LEFT_IN1_GPIO_Port   GPIOB
#define LEFT_IN1_Pin         GPIO_PIN_3
#define LEFT_IN2_GPIO_Port   GPIOB
#define LEFT_IN2_Pin         GPIO_PIN_4

#define RIGHT_IN1_GPIO_Port  GPIOB
#define RIGHT_IN1_Pin        GPIO_PIN_5
#define RIGHT_IN2_GPIO_Port  GPIOA
#define RIGHT_IN2_Pin        GPIO_PIN_15

void Motor_Left_Forward(void);
void Motor_Left_Backward(void);
void Motor_Left_Stop(void);
void Motor_Right_Forward(void);
void Motor_Right_Backward(void);
void Motor_Right_Stop(void);

#endif

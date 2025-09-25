#ifndef __SERVO_H
#define __SERVO_H

#include "tim.h"
#include "stdint.h"

#define SERVO1_Pin_PORT   GPIOA
#define SERVO1_Pin        GPIO_PIN_6
#define SERVO2_Pin_Port   GPIOA
#define SERVO2_Pin        GPIO_PIN_7


// 初始化舵机
void Servo_Init(void);

// 设置舵机角度 (0°~180°)
void Servo_SetAngle(uint8_t channel, uint8_t angle);

#endif




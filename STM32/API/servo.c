#include "servo.h"



// 舵机角度转 PWM 脉宽（0°~180°）
static uint16_t Servo_AngleToPulse(uint8_t angle) {
    if (angle > 180) angle = 180;
    // 0°=500us, 180°=2500us
    return 500 + ((uint32_t)angle * 2000) / 180;
}

void Servo_Init(void) {
    // 启动 PWM 输出 (定时器和通道根据 CubeMX 配置调整)
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); // PA6
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); // PA7
}

void Servo_SetAngle(uint8_t channel, uint8_t angle) {
    uint16_t pulse = Servo_AngleToPulse(angle);

    switch(channel) {
        case 1: // 舵机1 -> PA6 (TIM3_CH1)
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
            break;

        case 2: // 舵机2 -> PA7 (TIM3_CH2)
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
            break;

        default:
            // 不支持的通道
            break;
    }
}

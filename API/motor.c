#include "main.h"
#include "motor.h"

//Setting Motor Status
void Motor_Left_Forward(void) {
    HAL_GPIO_WritePin(LEFT_IN1_GPIO_Port, LEFT_IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LEFT_IN2_GPIO_Port, LEFT_IN2_Pin, GPIO_PIN_RESET);
}

void Motor_Left_Backward(void) {
    HAL_GPIO_WritePin(LEFT_IN1_GPIO_Port, LEFT_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LEFT_IN2_GPIO_Port, LEFT_IN2_Pin, GPIO_PIN_SET);
}

void Motor_Left_Stop(void) {
    HAL_GPIO_WritePin(LEFT_IN1_GPIO_Port, LEFT_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LEFT_IN2_GPIO_Port, LEFT_IN2_Pin, GPIO_PIN_RESET);
}

void Motor_Right_Forward(void) {
    HAL_GPIO_WritePin(RIGHT_IN1_GPIO_Port, RIGHT_IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RIGHT_IN2_GPIO_Port, RIGHT_IN2_Pin, GPIO_PIN_RESET);
}

void Motor_Right_Backward(void) {
    HAL_GPIO_WritePin(RIGHT_IN1_GPIO_Port, RIGHT_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RIGHT_IN2_GPIO_Port, RIGHT_IN2_Pin, GPIO_PIN_SET);
}

void Motor_Right_Stop(void) {
    HAL_GPIO_WritePin(RIGHT_IN1_GPIO_Port, RIGHT_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RIGHT_IN2_GPIO_Port, RIGHT_IN2_Pin, GPIO_PIN_RESET);
}

// =================== ?????? ===================
void Car_Forward(void) {
    Motor_Left_Forward();
    Motor_Right_Forward();
}

void Car_Backward(void) {
    Motor_Left_Backward();
    Motor_Right_Backward();
}

void Car_Stop(void) {
    Motor_Left_Stop();
    Motor_Right_Stop();
}

void Car_Turn_Left(void) {
    Motor_Left_Stop();
    Motor_Right_Forward();
}

void Car_Turn_Right(void) {
    Motor_Left_Forward();
    Motor_Right_Stop();
}

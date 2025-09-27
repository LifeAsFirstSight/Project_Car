#include "main.h"
#include "espcam.h"
#include "motor.h"
#include "sensor.h"
#include "servo.h"
#include "stdio.h"
#include <string.h>
#include <stdlib.h>

// ------------------------- 类型定义 -------------------------

/**
 * @brief 命令类型枚举定义
 * 
 * 定义了系统支持的所有命令类型，包括移动控制和舵机控制命令
 */
typedef enum {
    CMD_STOP = 0,          // 停止命令
    CMD_MOVE_FORWARD,      // 向前移动命令
    CMD_MOVE_BACKWARD,     // 向后移动命令
    CMD_TURN_LEFT,         // 向左转向命令
    CMD_TURN_RIGHT,        // 向右转向命令
    CMD_SERVO1_ANGLE,      // 舵机1角度控制命令
} CommandType;

/**
 * @brief 命令结构体定义
 * 
 * 用于封装单个命令的所有信息，包括命令类型、参数和有效性标识
 */
typedef struct {
    CommandType cmd;       // 命令类型
    uint16_t param;        // 参数（速度或角度）
    uint8_t is_valid;      // 命令是否有效
} Command;

/**
 * @brief 系统状态枚举定义
 * 
 * 定义了系统可能处于的所有运行状态
 */
typedef enum {
    STATE_IDLE,            // 空闲状态
    STATE_MOVING_FORWARD,  // 向前移动状态
    STATE_MOVING_BACKWARD, // 向后移动状态
    STATE_TURNING_LEFT,    // 向左转向状态
    STATE_TURNING_RIGHT,   // 向右转向状态
    STATE_STOPPED,         // 停止状态
} SystemState;

// ------------------------- 全局变量 -------------------------

/**
 * @brief 系统当前运行状态
 * 
 * 记录系统当前所处的运行状态，初始状态为停止状态
 */
static SystemState current_state = STATE_STOPPED;

/**
 * @brief 当前命令缓冲区
 * 
 * 存储当前正在处理的命令，初始为无效的停止命令
 */
static Command current_cmd = {CMD_STOP, 0, 0};

// 默认速度
#define DEFAULT_SPEED 999

// ------------------------- 状态函数 -------------------------

/**
 * @brief 空闲状态处理函数
 * 
 * 处理小车空闲状态下的行为
 */
static void idle_state(void) {
     Car_Stop();
     printf("status free\r\n");
}

/**
 * @brief 前进状态处理函数
 * 
 * 控制小车以100%速度前进，并输出状态信息
 */
static void forward_state(void) {
    Car_Forward(DEFAULT_SPEED);
    printf("status go\r\n");
}

/**
 * @brief 后退状态处理函数
 * 
 * 控制小车以100%速度后退，并输出状态信息
 */
static void backward_state(void) {
    Car_Backward(DEFAULT_SPEED);
    printf("status back\r\n");
}

/**
 * @brief 左转状态处理函数
 * 
 * 控制小车以100%速度左转，并输出状态信息
 */
static void left_state(void) {
    Car_Turn_Left(DEFAULT_SPEED);
    printf("status turnleft\r\n");
}

/**
 * @brief 右转状态处理函数
 * 
 * 控制小车以100%速度右转，并输出状态信息
 */
static void right_state(void) {
    Car_Turn_Right(DEFAULT_SPEED);
    printf("status turnright\r\n");
}

/**
 * @brief 停止状态处理函数
 * 
 * 控制小车停止运动，并输出状态信息
 */
static void stop_state(void) {
    Car_Stop();
    printf("status stop\r\n");
}

// ------------------------- 状态转换 -------------------------

static void transition_state(CommandType cmd) {
    switch(current_state) {
        case STATE_IDLE:
        case STATE_STOPPED:
            switch(cmd) {
                case CMD_MOVE_FORWARD: current_state = STATE_MOVING_FORWARD; break;
                case CMD_MOVE_BACKWARD: current_state = STATE_MOVING_BACKWARD; break;
                case CMD_TURN_LEFT: current_state = STATE_TURNING_LEFT; break;
                case CMD_TURN_RIGHT: current_state = STATE_TURNING_RIGHT; break;
                case CMD_STOP: current_state = STATE_STOPPED; break;
                default: break;
            }
            break;

        case STATE_MOVING_FORWARD:
        case STATE_MOVING_BACKWARD:
        case STATE_TURNING_LEFT:
        case STATE_TURNING_RIGHT:
            if(cmd == CMD_STOP) {
                current_state = STATE_STOPPED;
            } else if(cmd == CMD_MOVE_FORWARD) {
                current_state = STATE_MOVING_FORWARD;
            } else if(cmd == CMD_MOVE_BACKWARD) {
                current_state = STATE_MOVING_BACKWARD;
            } else if(cmd == CMD_TURN_LEFT) {
                current_state = STATE_TURNING_LEFT;
            } else if(cmd == CMD_TURN_RIGHT) {
                current_state = STATE_TURNING_RIGHT;
            }
            break;

        default: break;
    }
}

// ------------------------- 串口命令解析 -------------------------

void parse_uart_command(uint8_t *data, uint16_t len) {
    current_cmd.is_valid = 0;
    if(len < 4) return;  // 至少 "cmdX" 四个字节

    if(memcmp(data, "cmdF", 4) == 0) {
        current_cmd.cmd = CMD_MOVE_FORWARD;
        current_cmd.is_valid = 1;
    } else if(memcmp(data, "cmdB", 4) == 0) {
        current_cmd.cmd = CMD_MOVE_BACKWARD;
        current_cmd.is_valid = 1;
    } else if(memcmp(data, "cmdL", 4) == 0) {
        current_cmd.cmd = CMD_TURN_LEFT;
        current_cmd.is_valid = 1;
    } else if(memcmp(data, "cmdR", 4) == 0) {
        current_cmd.cmd = CMD_TURN_RIGHT;
        current_cmd.is_valid = 1;
    } else if(memcmp(data, "cmdS", 4) == 0) {
        current_cmd.cmd = CMD_STOP;
        current_cmd.is_valid = 1;
    } else if(data[0] == 'V') {
        // 舵机角度: 格式 V120
        char tmp[8] = {0};
        if(len - 1 < sizeof(tmp)-1) {
            memcpy(tmp, &data[1], len-1);
            current_cmd.cmd = CMD_SERVO1_ANGLE;
            current_cmd.param = atoi(tmp);
            if(current_cmd.param <= 180) current_cmd.is_valid = 1;
        }
    } else {
        printf("unknown\r\n");
    }
}

// ------------------------- 状态机主循环 -------------------------

void command_state_machine_run(void) {
    if(current_cmd.is_valid) {
        transition_state(current_cmd.cmd);
        current_cmd.is_valid = 0;
    }

    switch(current_state) {
        case STATE_IDLE: idle_state(); break;
        case STATE_MOVING_FORWARD: forward_state(); break;
        case STATE_MOVING_BACKWARD: backward_state(); break;
        case STATE_TURNING_LEFT: left_state(); break;
        case STATE_TURNING_RIGHT: right_state(); break;
        case STATE_STOPPED: stop_state(); break;
        default: break;
    }
}

// ------------------------- 串口接收处理函数 -------------------------

void uart_dma_process(uint8_t *rx_buffer, uint16_t received_length) {
    if(received_length == 0) return;

    // 解析命令
    parse_uart_command(rx_buffer, received_length);

    // 清空长度，准备下一次接收
    received_length = 0;
}


#include "main.h"
#include "espcam.h"
#include "motor.h"
#include "sensor.h"
#include "servo.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

// ------------------------- 类型定义 -------------------------

typedef enum {
    CMD_STOP = 0,
    CMD_MOVE_FORWARD,
    CMD_MOVE_BACKWARD,
    CMD_TURN_LEFT,
    CMD_TURN_RIGHT,
    CMD_SERVO1_ANGLE,
} CommandType;

typedef struct {
    CommandType cmd;       // 命令类型
    uint16_t param;        // 参数（速度或角度）
    uint8_t is_valid;      // 命令是否有效
} Command;

typedef enum {
    STATE_IDLE,
    STATE_MOVING_FORWARD,
    STATE_MOVING_BACKWARD,
    STATE_TURNING_LEFT,
    STATE_TURNING_RIGHT,
    STATE_STOPPED,
} SystemState;

// ------------------------- 全局变量 -------------------------

static SystemState current_state = STATE_STOPPED;
static Command current_cmd = {CMD_STOP, 0, 0};

// 默认速度
#define DEFAULT_SPEED 100

// ------------------------- 状态函数 -------------------------

static void idle_state(void) {

}

static void forward_state(void) {
    Car_Forward(current_cmd.param ? current_cmd.param : DEFAULT_SPEED);
    printf("状态：前进中\n");
}

static void backward_state(void) {
    Car_Backward(current_cmd.param ? current_cmd.param : DEFAULT_SPEED);
    printf("状态：后退中\n");
}

static void left_state(void) {
    Car_TurnLeft(current_cmd.param ? current_cmd.param : DEFAULT_SPEED);
    printf("状态：左转中\n");
}

static void right_state(void) {
    Car_TurnRight(current_cmd.param ? current_cmd.param : DEFAULT_SPEED);
    printf("状态：右转中\n");
}

static void stop_state(void) {
    Motor_Stop();
    printf("状态：已停止\n");
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
        printf("未知命令\n");
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


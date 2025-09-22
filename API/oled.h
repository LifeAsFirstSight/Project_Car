#ifndef __OLED_H
#define __OLED_H 

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>
#include <stdlib.h>

// 引脚宏定义（根据需求配置）
#define OLED_SCK_PIN    GPIO_PIN_13
#define OLED_SCK_PORT   GPIOB
#define OLED_MOSI_PIN   GPIO_PIN_15
#define OLED_MOSI_PORT  GPIOB

// 引脚宏定义（需与CubeMX中GPIO配置一致，匹配用户指定的引脚映射）
#define OLED_DC_PIN    GPIO_PIN_8
#define OLED_DC_PORT   GPIOA
#define OLED_CS_PIN    GPIO_PIN_12
#define OLED_CS_PORT   GPIOB

// 命令/数据定义
#define OLED_CMD  0
#define OLED_DATA 1

// 函数声明
void OLED_GPIO_Init(void);                  // GPIO初始化
void OLED_WR_Byte(uint8_t dat, uint8_t mode); // 写命令/数据
void OLED_Display_On(void);                 // 开启显示
void OLED_Display_Off(void);                // 关闭显示
void OLED_Refresh(void);                    // 刷新显存到屏幕
void OLED_Clear(void);                      // 清屏
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t); // 画点
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode); // 画线
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r); // 画圆
void OLED_DrawChar(uint8_t x, uint8_t y, uint8_t index);// 显示单个字符
//void OLED_ShowChasr(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1, uint8_t mode); // 显示单个字符
//void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1, uint8_t mode); // 显示字符串
//void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1, uint8_t mode); // 显示数字
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1, uint8_t mode); // 显示汉字
void OLED_ScrollDisplay(uint8_t num, uint8_t space, uint8_t mode); // 滚动显示
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t BMP[], uint8_t mode); // 显示图片
void OLED_Init(void);                       // OLED初始化
void OLED_ColorTurn(uint8_t i);             // 反显控制
void OLED_DisplayTurn(uint8_t i);           // 屏幕旋转控制

#endif
// OLED驱动代码（基于SSD1315控制器，分辨率128x64，使用软件SPI模拟）
// 适用于STM32F1系列，使用HAL库
// 引脚连接：SCK=PB13, MOSI=PB14, DC=PA8, CS=PB12